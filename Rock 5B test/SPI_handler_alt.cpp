#include <iostream>
#include <chrono>
#include <cstring>
#include <fstream>
#include "hdf5.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

#define PERIPHERAL_BASE 0x20000000
#define GPIO_BASE (PERIPHERAL_BASE + 0x200000)
#define SPI_BASE (PERIPHERAL_BASE + 0x204000)

#define CLK_SPD 3900000 // 3.9MHz
#define NUM_CHANNELS 16
#define SAMPLES_PER_CHANNEL 500
#define TOTAL_SAMPLES (NUM_CHANNELS * SAMPLES_PER_CHANNEL)
#define SAMPLING_INTERVAL_MS 4

using namespace std;

// Function to read from MCP3008 ADC
int read_mcp3008(int channel) {
    // Initialize SPI
    unsigned char tx[] = {0x01, 0x80 | (channel << 4), 0x00};
    unsigned char rx[3];
    int fd = open("/dev/spidev0.0", O_RDWR);
    ioctl(fd, SPI_IOC_WR_MODE, SPI_MODE_0);
    ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, 8);
    ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, CLK_SPD);

    // Read from ADC
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = 3,
        .delay_usecs = 0,
        .speed_hz = CLK_SPD,
        .bits_per_word = 8,
    };
    ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

    // Close SPI
    close(fd);

    // Extract value from ADC response
    int value = ((rx[1] & 0x03) << 8) | rx[2];
    return value;
}

// Function to write data to HDF5 file
void write_hdf5(int* data) {
    hid_t file_id, dataset_id, dataspace_id, dcpl_id;
    hsize_t dims[2];
    herr_t status;

    // Create a new HDF5 file
    file_id = H5Fcreate("data.h5", H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

    // Define the size of the data to be written
    dims[0] = TOTAL_SAMPLES;
    dims[1] = 1;

    // Create a new dataspace
    dataspace_id = H5Screate_simple(2, dims, NULL);

    // Create a new dataset in the file
    dataset_id = H5Dcreate2(file_id, "/data", H5T_NATIVE_INT, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    // Set up the compression filter
    dcpl_id = H5Pcreate(H5P_DATASET_CREATE);
    H5Pset_deflate(dcpl_id, 6);

    // Write the data to the dataset
    status = H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);

    // Close the dataset, dataspace, and file
    H5Pclose(dcpl_id);
    H5Sclose(dataspace_id);
    H5Fclose(file_id);
}
   
int main() {
    // Map GPIO memory
    int mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    void* gpio_map = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, GPIO_BASE & ~MAP_MASK);

    // Enable DMA controller
    *(volatile unsigned int*)((char*)gpio_map + 0x30) |= (1 << 28);

    // Allocate DMA memory
    int* dma_mem = new int[TOTAL_SAMPLES];
    memset(dma_mem, 0, TOTAL_SAMPLES * sizeof(int));

    // Configure DMA controller
    volatile unsigned int* dma_reg = (volatile unsigned int*)((char*)gpio_map + 0x7000);
    dma_reg[0] = 0x00000001;
    dma_reg[1] = 0x00000006;
    dma_reg[2] = 0x00000000;
    dma_reg[3] = 0x00000000;
    dma_reg[4] = 0x00000000;
    dma_reg[5] = 0x00000000;
    dma_reg[6] = 0x00000000;
    dma_reg[7] = 0x00000000;
    dma_reg[8] = 0x00000000;
    dma_reg[9] = 0x00000000;
    dma_reg[10] = 0x00000000;
    dma_reg[11] = 0x00000000;
    dma_reg[15] = 0x00000000;

    // Configure DMA channel 0
    dma_reg[18] = 0x00001000;
    dma_reg[19] = (unsigned int)(dma_mem);
    dma_reg[20] = 0x00000000;
    dma_reg[21] = 0x00000000;
    dma_reg[22] = 0x00000000;
    dma_reg[23] = 0x00000000;
    dma_reg[24] = 0x00000000;
    dma_reg[25] = 0x00000000;
    dma_reg[26] = 0x00000000;
    dma_reg[27] = 0x00000000;

    // Configure DMA channel 1
    dma_reg[30] = 0x00001000;
    dma_reg[31] = (unsigned int)(dma_mem + SAMPLES_PER_CHANNEL * NUM_CHANNELS);
    dma_reg[32] = 0x00000000;
    dma_reg[33] = 0x00000000;
    dma_reg[34] = 0x00000000;
    dma_reg[35] = 0x00000000;
    dma_reg[36] = 0x00000000;
    dma_reg[37] = 0x00000000;
    dma_reg[38] = 0x00000000;
    dma_reg[39] = 0x00000000;

    // Enable DMA channel 0 and 1
    dma_reg[16] |= 0x00000003;

// Start sampling
auto start_time = chrono::high_resolution_clock::now();

for (int i = 0; i < SAMPLES_PER_CHANNEL; i++) {
    for (int channel = 0; channel < NUM_CHANNELS; channel++) {
        // Sample each channel on SPI 0
        int value = read_mcp3008(channel);
        dma_mem[i * NUM_CHANNELS + channel] = value;

        // Sample each channel on SPI 1
        value = read_mcp3008(channel + NUM_CHANNELS);
        dma_mem[(i + SAMPLES_PER_CHANNEL) * NUM_CHANNELS + channel] = value;
    }

    // Wait for sampling interval
    this_thread::sleep_for(chrono::milliseconds(SAMPLING_INTERVAL_MS));
}

auto end_time = chrono::high_resolution_clock::now();

// Stop DMA channel 0 and 1
dma_reg[16] &= ~0x00000003;

// Unmap GPIO memory
munmap(gpio_map, MAP_SIZE);
close(mem_fd);

// Calculate elapsed time
chrono::duration<double, milli> elapsed_time = end_time - start_time;
cout << "Elapsed time: " << elapsed_time.count() << "ms" << endl;

// Write data to HDF5 file
hid_t file_id, dataset_id, dataspace_id;
hsize_t dims[2];
herr_t status;

// Create a new HDF5 file
file_id = H5Fcreate("data.h5", H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

// Define the size of the data to be written
dims[0] = SAMPLES_PER_CHANNEL;
dims[1] = NUM_CHANNELS * 2;

// Create a new dataspace
dataspace_id = H5Screate_simple(2, dims, NULL);

// Create a new dataset in the file
dataset_id = H5Dcreate2(file_id, "/data", H5T_NATIVE_INT, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

// Write the data to the dataset
status = H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, dma_mem);

// Close the dataset, dataspace, and file
H5Sclose(dataspace_id);
H5Dclose(dataset_id);
H5Fclose(file_id);

// Free DMA memory
delete[] dma_mem;

return 0;
}
