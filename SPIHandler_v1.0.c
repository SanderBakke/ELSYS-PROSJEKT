/*
adc_sampler.c
Public Domain
January 2018, Kristoffer KjÃ¦rnes & Asgeir BjÃ¸rgan
Based on example code from the pigpio library by Joan @ raspi forum and github
https://github.com/joan2937 | http://abyz.me.uk/rpi/pigpio/

Compile with:
gcc -Wall -lpthread -o adc_sampler adc_sampler.c -lpigpio -lm

Run with:
sudo ./adc_sampler

This code bit bangs SPI on several devices using DMA.

Using DMA to bit bang allows for two advantages
1) the time of the SPI transaction can be guaranteed to within a
   microsecond or so.

2) multiple devices of the same type can be read or written
   simultaneously.

This code reads several MCP3201 ADCs in parallel, and writes the data to a binary file.
Each MCP3201 shares the SPI clock and slave select lines but has
a unique MISO line. The MOSI line is not in use, since the MCP3201 is single
channel ADC without need for any input to initiate sampling.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <plot.h>
#include <pigpio.h>
#include <math.h>
#include <string.h>

/////// USER SHOULD MAKE SURE THESE DEFINES CORRESPOND TO THEIR SETUP ///////
#define ADCS 4      // Number of connected MCP3201.

#define OUTPUT_DATA argv[2] // path and filename to dump buffered ADC data

/* RPi PIN ASSIGNMENTS */
#define MISO1 12  // ADC 1 MISO (BCM 4 aka GPIO 4).
#define MISO2 13   //     2
#define MISO3 16 //     3
#define MISO4 17

#define MOSI 10     // GPIO for SPI MOSI (BCM 10 aka GPIO 10 aka SPI_MOSI). MOSI not in use here due to single ch. ADCs, but must be defined anyway.
#define SPI_SS 8  // GPIO for slave select (BCM 8 aka GPIO 8 aka SPI_CE0).
#define CLK 11   // GPIO for SPI clock (BCM 11 aka GPIO 11 aka SPI_CLK).
/* END RPi PIN ASSIGNMENTS */

#define BITS 12            // Bits per sample.
#define BX 4               // Bit position of data bit B11. (3 first are t_sample + null bit)
#define B0 (BX + BITS - 1) // Bit position of data bit B0.

#define NUM_SAMPLES_IN_BUFFER 300 // Generally make this buffer as large as possible in order to cope with reschedule.

#define REPEAT_MICROS 32 // Reading every x microseconds. Must be no less than 2xB0 defined above

#define FS 31250
#define DEFAULT_NUM_SAMPLES 3000 // Default number of samples for printing in the example. Should give 1sec of data at Tp=32us.
#define SPEED_OF_SOUND 343.2 // meters per second
#define MIC_DISTANCE 0.203 // distance between microphones in meters
#define PI 3.14159265358979323846
#define NUM_IN_CORR_1 48
#define MAX_LAG 24


int MISO[ADCS]={MISO1, MISO2, MISO3, MISO4}; // Must be updated if you change number of ADCs/MISOs above
uint currentSampleIndex = 0;
uint16_t sample_matrix[DEFAULT_NUM_SAMPLES][ADCS];
uint16_t sample_row[ADCS];
double angleX;

//test array
//double r0_array[NUM_IN_CORR_0];

//corralation arrays
double r1_matrixX[ADCS-1][NUM_IN_CORR_1]; //ende lengde til 2 når ferdig
double r1_productX[NUM_IN_CORR_1]; //holder produktet av alle disse


uint16_t* getSamples(const uint16_t src [DEFAULT_NUM_SAMPLES][ADCS],int column);
void getReading(int adcs, int *MISO, int OOL, int bytes, int bits, char *buf);
void add_samples(uint16_t *new_samples);
void cross_correlation(uint16_t* array1, uint16_t* array2, double* result, int maxLag);
void removeDC(uint16_t* src, int len);
void mul_Correlation(int lenCorr, double (*src)[lenCorr], double* dest, int numMul);
double calculate_angle(double* src, int lenCorr, double micDistance );
void write_csv_2D(const uint16_t src [DEFAULT_NUM_SAMPLES][ADCS], int num_values);
void write_csv_1D(const double src[], int num_values);
//void cross_correlation(uint16_t** samples, int num_mics, int num_samples, double* output);


/**
 * This function extracts the MISO bits for each ADC and
 * collates them into a reading per ADC.
 *
 * \param adcs Number of attached ADCs
 * \param MISO The GPIO connected to the ADCs data out
 * \param bytes Bytes between readings
 * \param bits Bits per reading
 * \param buf Output buffer
*/
void getReading(int adcs, int *MISO, int OOL, int bytes, int bits, char *buf)
{
   int p = OOL;
   int i, a;

   for (i=0; i < bits; i++) {
      uint32_t level = rawWaveGetOut(p);
      for (a=0; a < adcs; a++) {
         putBitInBytes(i, buf+(bytes*a), level & (1<<MISO[a]));
      }
      p--;
   }
}

void add_samples(uint16_t *new_samples) {
    memcpy(sample_matrix[currentSampleIndex], new_samples, ADCS * sizeof(uint16_t));
    currentSampleIndex = (currentSampleIndex + 1) % DEFAULT_NUM_SAMPLES; // Increment index, wrap around if necessary
}

//ting å tenke på ved korrelasjon:
//overlapp mellom korrelasjonene innad i 1s
//kjøre fler korrelasjoner på flere seksjoner innad i 1s, to mikrofoner er maks x samples unna hverandre
//bestemme forskyvning utfra korrelasjonstoppene, sjekk om det går an å gange sammen r_12 * r_23 * r_34 for å få en enda mer definert gain! og evt gjøre det samme med r_13 og r_24 osv
//Max lag mellom 2 lengde er gitt av ((numDist * MIC_DISTANCE)/SPEED_OF_SOUND)*FS = 9 med l = 0.01
void cross_correlation(uint16_t* array1, uint16_t* array2, double* result, int maxLag) {
    int n = DEFAULT_NUM_SAMPLES;
    
    for (int lag = -maxLag; lag <= maxLag; lag++) {
        double sum = 0;
        double denumerator = 0;
        for (int i = 0; i < n; i++) {
            int j = i + lag;
            if (j >= 0 && j <= n) {
                sum += (double)array1[i] * (double)array2[j];
                denumerator++;
            }
        }
        result[lag + maxLag] = sum / denumerator;
    }
}
void mul_Correlation(int lenCorr, double (*src)[lenCorr], double* dest, int numMul){
    double maxVal = 0;
    for(int i = 0; i<numMul; i++){ //3 when 4 mics implemented
        for(int j = 0; j<lenCorr;j++){
            if(src[i][j] > maxVal){
                maxVal = src[i][j];
            }
        }
    }
    for(int i = 0; i<lenCorr;i++){
        dest[i] = 1;
        for(int j = 0; j<numMul;j++){ //3 when 4 mics implemented
            dest[i] *= ((src[j][i]/maxVal)*10);
        }
    }
}

void removeDC(uint16_t* src, int len){
    for(int i = 0; i<len;i++){
        src[i] -= 1450; //about observed dc component
    }
}
double calculate_angle(double* src, int lenCorr, double micDistance ){
    double angle = 0;
    double maxCorr = 0;
    int maxIndex = 0;
    for(int i = 0; i<lenCorr; i++){
        if(src[i] > maxCorr){
            maxCorr = src[i];
            maxIndex = i;
        }
    }
        
    double S = maxIndex - ((double)lenCorr/2);
    double b = (S/FS)*SPEED_OF_SOUND;
    if(b/micDistance < 1 && b/micDistance>-1){
        angle = acos(b/micDistance)*(180/PI); //i grader
    }
    return angle;
}

uint16_t* getSamples(const uint16_t src [DEFAULT_NUM_SAMPLES][ADCS],int column) {
    uint16_t* output = malloc(DEFAULT_NUM_SAMPLES * sizeof(uint16_t));
    for(int i = 0; i<DEFAULT_NUM_SAMPLES;i++){
        output[i] = src[i][column];
    }
    return output; //retrurner peker til første element i et array som har kopiert kolonne column
}

void write_csv_2D(const uint16_t src [DEFAULT_NUM_SAMPLES][ADCS], int num_values) {
    // Open the file for writing
    FILE* fp = fopen("inndata.csv", "w");
    if (fp == NULL) {
        printf("Error: Could not open file for writing.\n");
        return;
    }
    
    // Write the values to the file in CSV format
    for (int i = 0; i < num_values; i++) {
        fprintf(fp, "%f,%f\n", (float)src[i][0],(float)src[i][1]);
        //fprintf(fp, "%d\n",src[i][1]);
    }
    
    // Close the file
    fclose(fp);
}

void write_csv_1D(const double src[], int num_values) {
    // Open the file for writing
    FILE* fp = fopen("xcorrupdated.csv", "w");
    if (fp == NULL) {
        printf("Error: Could not open file for writing.\n");
        return;
    }
    
    // Write the values to the file in CSV format
    fprintf(fp,"yval\n");
    for (int i = 0; i < num_values; i++) {
        fprintf(fp, "%f\n",src[i]);
        //fprintf(fp, "%d\n",src[i][1]);
    }
    
    // Close the file
    fclose(fp);
}




int main()
{   
    // SPI transfer settings, time resolution 1us (1MHz system clock is used)
    rawSPI_t rawSPI =
    {
    .clk     =  CLK,  // Defined before
    .mosi    =  MOSI, // Defined before
    .ss_pol  =  1,   // Slave select resting level.
    .ss_us   =  1,   // Wait 1 micro after asserting slave select.
    .clk_pol =  0,   // Clock resting level.
    .clk_pha =  0,   // 0 sample on first edge, 1 sample on second edge.
    .clk_us  =  1,   // 2 clocks needed per bit so 500 kbps.
    };

    // Change timer to use PWM clock instead of PCM clock. Default is PCM
    // clock, but playing sound on the system (e.g. espeak at boot) will start
    // sound systems that will take over the PCM timer and make adc_sampler.c
    // sample at far lower samplerates than what we desire.
    // Changing to PWM should fix this problem.
    gpioCfgClock(5, 0, 0);

    // Initialize the pigpio library
    if (gpioInitialise() < 0) {
    return 1;
    }

    // Set the selected CLK, MOSI and SPI_SS pins as output pins
    gpioSetMode(rawSPI.clk,  PI_OUTPUT);
    gpioSetMode(rawSPI.mosi, PI_OUTPUT);
    gpioSetMode(SPI_SS,      PI_OUTPUT);

    // Flush any old unused wave data.
    gpioWaveClear();
    gpioWaveAddNew();

    // Construct bit-banged SPI reads. Each ADC reading is stored separatedly
    // along a buffer of DMA commands (control blocks). When the DMA engine
    // reaches the end of the buffer, it restarts on the start of the buffer
    int offset = 0;
    int i;
    char buf[2];
    for (i=0; i < NUM_SAMPLES_IN_BUFFER; i++) {
        buf[0] = 0xC0; // Start bit, single ended, channel 0.

        rawWaveAddSPI(&rawSPI, offset, SPI_SS, buf, 2, BX, B0, B0);
        offset += REPEAT_MICROS;
    }

    // Force the same delay after the last command in the buffer
    gpioPulse_t final[2];
    final[0].gpioOn = 0;
    final[0].gpioOff = 0;
    final[0].usDelay = offset;

    final[1].gpioOn = 0; // Need a dummy to force the final delay.
    final[1].gpioOff = 0;
    final[1].usDelay = 0;

    gpioWaveAddGeneric(2, final);

    // Construct the wave from added data.
    int wid = gpioWaveCreate();
    if (wid < 0) {
        fprintf(stderr, "Can't create wave, buffer size %d too large?\n", NUM_SAMPLES_IN_BUFFER);
        printf("%d",wid);
        return 1;
    }

    // Obtain addresses for the top and bottom control blocks (CB) in the DMA
    // output buffer. As the wave is being transmitted, the current CB will be
    // between botCB and topCB inclusive.
    rawWaveInfo_t rwi = rawWaveInfo(wid);
    int botCB = rwi.botCB;
    int topOOL = rwi.topOOL;
    float cbs_per_reading = (float)rwi.numCB / (float)NUM_SAMPLES_IN_BUFFER;

    float expected_sample_freq_khz = 1000.0/(1.0*REPEAT_MICROS);

    //printf("# Starting sampling: %ld samples (expected Tp = %d us, expected Fs = %.3f kHz).\n",
    //DEFAULT_NUM_SAMPLES,REPEAT_MICROS,expected_sample_freq_khz);

    // Start DMA engine and start sending ADC reading commands
    gpioWaveTxSend(wid, PI_WAVE_MODE_REPEAT);

    // Read back the samples
    double start_time = time_time();
    int reading = 0;
    int sample = 0;

    while (sample<DEFAULT_NUM_SAMPLES) { //update width new controls
        // Get position along DMA control block buffer corresponding to the current output command.
        int cb = rawWaveCB() - botCB;
        int now_reading = (float) cb / cbs_per_reading;
        while ((now_reading != reading) && (sample < DEFAULT_NUM_SAMPLES)) {
            // Read samples from DMA input buffer up until the current output command

            // OOL are allocated from the top down. There are BITS bits for each ADC
            // reading and NUM_SAMPLES_IN_BUFFER ADC readings. The readings will be
            // stored in topOOL - 1 to topOOL - (BITS * NUM_SAMPLES_IN_BUFFER).
            // Position of each reading's OOL are calculated relative to the wave's top
            // OOL.
            int reading_address = topOOL - ((reading % NUM_SAMPLES_IN_BUFFER)*BITS) - 1;

            char rx[8];
            getReading(ADCS, MISO, reading_address, 2, BITS, rx);

            for (i=0; i < ADCS; i++) {
                sample_row[i] = ((rx[i*2]<<4) + (rx[(i*2)+1]>>4)); //inserts new values in first row and removes most of DC component (1450)
            }

            //removeDC(sample_row,ADCS); no benefit of this spotted
            add_samples(sample_row); //adds a new sample at current index and increments index, wraps around if maxed

            ++sample;
            if (++reading >= NUM_SAMPLES_IN_BUFFER) {
                reading = 0;
            }
        }
        usleep(1000);
    }
    double end_time = time_time();
    double nominal_period_us = 1.0*(end_time-start_time)/(1.0*DEFAULT_NUM_SAMPLES)*1.0e06;
    double nominal_sample_freq_khz = 1000.0/nominal_period_us;
    //printf("# %ld samples in %.6f seconds (actual T_p = %f us, nominal Fs = %.2f kHz).\n",DEFAULT_NUM_SAMPLES, end_time-start_time, nominal_period_us, nominal_sample_freq_khz);

    //printf("%d %d", sample_matrix, currentSampleIndex);
    for(int i = 25;i>0;i--){
        //printf("| %.2f | %.2f | %.2f | \n",(float)sample_matrix[DEFAULT_NUM_SAMPLES-i][0],(float)sample_matrix[DEFAULT_NUM_SAMPLES-i][1],(float)sample_matrix[DEFAULT_NUM_SAMPLES-i][2]);
    }
    //printf("Sample and save time: %.2f \n", (end_time-start_time));
    
    //x component value
    cross_correlation(getSamples(sample_matrix,0),getSamples(sample_matrix,1),r1_matrixX[0],MAX_LAG);
    cross_correlation(getSamples(sample_matrix,1),getSamples(sample_matrix,2),r1_matrixX[1],MAX_LAG);
    cross_correlation(getSamples(sample_matrix,1),getSamples(sample_matrix,2),r1_matrixX[2],MAX_LAG);
    
    mul_Correlation(NUM_IN_CORR_1,r1_matrixX,r1_productX,2);
    angleX = calculate_angle(r1_productX,NUM_IN_CORR_1,0.05);
    printf("%f\n",angleX);

    gpioTerminate();
    /*
    free(sample_matrix);
    free(sample_row);
    free(r1_matrixX);
    free(r1_productX);
    free(r2_productX);
    */
    return 0;
}