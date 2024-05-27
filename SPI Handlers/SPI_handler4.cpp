#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <wiringPi.h>
#include <wiringPiSPI.h>

using namespace std;

// Define the SPI channel and MCP3008's channels
const int SPI_CHANNEL = 0;
const int MCP3008_CHANNEL_COUNT = 8;

// Define the sample rate (in Hz)
const int SAMPLE_RATE = 1000; // 1 kHz

// Define the sample count (for a total of 100ms of data)
const int SAMPLE_COUNT = SAMPLE_RATE / 10;

// Define the XML file path
const string XML_FILE_PATH = "samples.xml";

// DMA page size (must be a multiple of 4096)
const int DMA_PAGE_SIZE = 4096;

// DMA buffer size (must be a multiple of DMA_PAGE_SIZE)
const int DMA_BUFFER_SIZE = 65536; // 16 pages

// Set up the MCP3008 configuration byte for a given channel
void setup_mcp3008_config(unsigned char* config, int channel) {
    config[0] = 0x01; // Start bit
    config[1] = 0x80 | (channel << 4); // Single-ended mode, channel
    config[2] = 0x00; // Null bit
}

// Read a single value from the MCP3008 for a given channel
int read_mcp3008_value(int channel) {
    unsigned char config[3];
    setup_mcp3008_config(config, channel);

    unsigned char buffer[3];
    wiringPiSPIDataRW(SPI_CHANNEL, config, 3);
    return ((buffer[1] & 3) << 8) + buffer[2];
}

int main() {
    // Initialize WiringPi library and SPI interface
    wiringPiSetup();
    wiringPiSPISetup(SPI_CHANNEL, 1000000);

    // Set up the DMA buffer
    unsigned char* dma_buffer = new unsigned char[DMA_BUFFER_SIZE];
    int dma_fd = wiringPiSPISetupDMA(SPI_CHANNEL, dma_buffer, DMA_BUFFER_SIZE);
    if (dma_fd < 0) {
        cerr << "Failed to initialize DMA: " << strerror(errno) << endl;
        return 1;
    }

    // Open the output file
    ofstream output_file(XML_FILE_PATH);

    // Write the XML header
    output_file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
    output_file << "<samples>" << endl;

    // Collect data for each channel
    vector<vector<int>> data(MCP3008_CHANNEL_COUNT);
    auto start_time = chrono::high_resolution_clock::now();
    while (chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - start_time).count() < 100) {
        for (int channel = 0; channel < MCP3008_CHANNEL_COUNT; channel++) {
            // Read the MCP3008 value using DMA
            int value = 0;
            unsigned char* buffer = dma_buffer + (channel * 3);
            setup_mcp3008_config(buffer, channel);
            if (wiringPiSPIDataRW(SPI_CHANNEL, buffer, 3) == -1) {
                cerr << "Failed to read MCP3008 value: " << strerror(errno) << endl;
                return 1;
