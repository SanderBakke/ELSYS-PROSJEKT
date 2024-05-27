#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

using namespace std;

// Define the MCP3008's channels
const int MCP3008_CHANNEL_COUNT = 8;

// Define the sample rate (in Hz)
const int SAMPLE_RATE = 1000; // 1 kHz

// Define the sample count (for a total of 100ms of data)
const int SAMPLE_COUNT = SAMPLE_RATE / 10;

// Define the XML file path
const string XML_FILE_PATH = "samples.xml";

// Set up the MCP3008 configuration byte for a given channel
void setup_mcp3008_config(unsigned char* config, int channel) {
    config[0] = 0x01; // Start bit
    config[1] = 0x80 | (channel << 4); // Single-ended mode, channel
    config[2] = 0x00; // Null bit
}

// Read a single value from the MCP3008 for a given channel
int read_mcp3008_value(int fd, int channel) {
    unsigned char tx_buffer[3], rx_buffer[3];
    setup_mcp3008_config(tx_buffer, channel);
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx_buffer,
        .rx_buf = (unsigned long)rx_buffer,
        .len = 3,
        .speed_hz = 1000000, // SPI clock speed
        .delay_usecs = 0,
        .bits_per_word = 8,
    };
    ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    return ((rx_buffer[1] & 3) << 8) + rx_buffer[2];
}

int main() {
    // Open the SPI device file
    int fd = open("/dev/spidev0.0", O_RDWR);
    if (fd < 0) {
        cerr << "Failed to open SPI device: " << strerror(errno) << endl;
        return 1;
    }

    // Set up the SPI mode and bits per word
    unsigned char mode = SPI_MODE_0;
    if (ioctl(fd, SPI_IOC_WR_MODE, &mode) < 0) {
        cerr << "Failed to set SPI mode: " << strerror(errno) << endl;
        return 1;
    }
    unsigned char bits = 8;
    if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
        cerr << "Failed to set SPI bits per word: " << strerror(errno) << endl;
        return 1;
    }

    // Set up DMA for faster transfers
    int dma_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (dma_fd < 0) {
        cerr << "Failed to open /dev/mem: " << strerror(errno) << endl;
        return 1;
    }
    void *dma_map = mmap(NULL, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED, dma_fd, 0x7e007000);
    if (dma_map == MAP_FAILED) {
        cerr << "Failed to mmap DMA: " << strerror(errno) << endl;
        return 1;
    }
    volatile uint32_t *dma_reg = (volatile uint32_t *)dma_map;
    dma_reg[DMA_CS] = DMA_RESET;
        dma_reg[DMA_TI] = DMA_SRC_INC | DMA_DEST_INC | DMA_SRC_WIDTH_8 | DMA_DEST_WIDTH_8;
    dma_reg[DMA_TXFR_LEN] = MCP3008_CHANNEL_COUNT * SAMPLE_COUNT * sizeof(unsigned short);
    dma_reg[DMA_DEBUG] = 7;

    // Set up the SPI transfer buffer
    vector<unsigned char> tx_buf(MCP3008_CHANNEL_COUNT * SAMPLE_COUNT * 3);
    for (int i = 0; i < MCP3008_CHANNEL_COUNT; i++) {
        for (int j = 0; j < SAMPLE_COUNT; j++) {
            int index = (i * SAMPLE_COUNT + j) * 3;
            setup_mcp3008_config(&tx_buf[index], i);
        }
    }

    // Perform the SPI transfer using DMA
    unsigned short rx_buf[MCP3008_CHANNEL_COUNT * SAMPLE_COUNT] = {0};
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx_buf.data(),
        .rx_buf = (unsigned long)rx_buf,
        .len = MCP3008_CHANNEL_COUNT * SAMPLE_COUNT * sizeof(unsigned short),
        .delay_usecs = 0,
        .speed_hz = 1000000,
        .bits_per_word = 8,
        .cs_change = 0,
    };
    dma_reg[DMA_CONBLK_AD] = (unsigned long)(&tr);
    dma_reg[DMA_DEBUG] = 7;
    dma_reg[DMA_CS] = DMA_INT_EN | DMA_END | DMA_ACTIVE;

    // Wait for the transfer to complete
    auto start_time = chrono::steady_clock::now();
    while (dma_reg[DMA_CS] & DMA_ACTIVE) {
        if (chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - start_time).count() >= 100) {
            cerr << "DMA transfer timed out" << endl;
            return 1;
        }
    }

    // Close the DMA and SPI device files
    close(dma_fd);
    close(fd);

    // Write the samples to an XML file
    ofstream xml_file(XML_FILE_PATH);
    xml_file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
    xml_file << "<samples>" << endl;
    for (int i = 0; i < MCP3008_CHANNEL_COUNT; i++) {
        xml_file << "\t<channel id=\"" << i << "\">" << endl;
        for (int j = 0; j < SAMPLE_COUNT; j++) {
            xml_file << "\t\t<sample>" << rx_buf[i * SAMPLE_COUNT + j] << "</sample>" << endl;
        }
        xml_file << "\t</channel>" << endl;
    }
    xml_file << "</samples>" << endl;
    xml_file.close();

    return 0;
}
