#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <wiringPi.h>
#include <wiringPiSPI.h>

using namespace std;

// Define the SPI channel and MCP3008's channel
const int channel = 0;
const int mcp3008_channel = 0;

int read_mcp3008()
{
    unsigned char data[3];
    data[0] = 1; // Start bit
    data[1] = 0b10000000 | (mcp3008_channel << 4); // Configuration bits: single-ended, channel
    data[2] = 0; // Dummy bit

    wiringPiSPIDataRW(channel, data, 3);

    int result = ((data[1] & 3) << 8) + data[2];
    return result;
}

int main()
{
    // Initialize WiringPi library and SPI channel
    wiringPiSetup();
    wiringPiSPISetup(channel, 1000000);

    // Open the output file
    ofstream output_file("samples.xml");

    // Write the XML header
    output_file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
    output_file << "<samples>" << endl;

    // Set up the data buffer
    vector<int> data(100);

    // Continuously record data
    while (true) {
        // Read the MCP3008 and store the result in the data buffer
        int value = read_mcp3008();
        data.push_back(value);
        data.erase(data.begin());

        // Write the latest data to the output file
        output_file << "<sample>";
        for (int i = 0; i < data.size(); i++) {
            output_file << "<value>" << data[i] << "</value>";
        }
        output_file << "</sample>" << endl;

        // Wait for 100ms before taking the next sample
        delay(100);
    }

    // Write the XML footer
    output_file << "</samples>" << endl;

    // Close the output file
    output_file.close();

    return 0;
}
