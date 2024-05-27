#include <wiringPi.h>
#include <mcp3004.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>

const int SPI_CHANNEL = 0; // SPI channel
const int SPI_SPEED = 1000000; // SPI speed
const int ADC_PIN = 0; // MCP3008 channel to read
const int SAMPLE_RATE_MS = 10; // Sample rate in milliseconds
const int SAMPLE_COUNT = 100; // Number of samples to store
const std::string XML_FILE_NAME = "samples.xml"; // Output XML file name

int main() {
    // Initialize wiringPi library and MCP3008
    wiringPiSetup();
    mcp3004Setup(ADC_PIN, SPI_CHANNEL);

    // Initialize sample buffer
    std::vector<int> sampleBuffer;
    sampleBuffer.reserve(SAMPLE_COUNT);

    // Open XML file and write root element
    std::ofstream xmlFile(XML_FILE_NAME);
    xmlFile << "<samples>" << std::endl;

    // Initialize timer
    auto startTime = std::chrono::steady_clock::now();

    // Continuously collect and update samples
    while (true) {
        // Collect sample
        int sample = analogRead(ADC_PIN);
        sampleBuffer.push_back(sample);

        // Check if sample buffer is full
        if (sampleBuffer.size() > SAMPLE_COUNT) {
            sampleBuffer.erase(sampleBuffer.begin());
        }

        // Check if sample rate interval has passed
        auto endTime = std::chrono::steady_clock::now();
        auto elapsedTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        if (elapsedTimeMs >= SAMPLE_RATE_MS) {
            // Write last SAMPLE_COUNT samples to XML file
            xmlFile << "<sample_data>" << std::endl;
            for (int i = 0; i < sampleBuffer.size(); i++) {
                xmlFile << "<sample>" << sampleBuffer[i] << "</sample>" << std::endl;
            }
            xmlFile << "</sample_data>" << std::endl;
            xmlFile.flush(); // Flush output to file

            // Reset timer
            startTime = endTime;
        }
    }

    // Close XML file
    xmlFile << "</samples>" << std::endl;
    xmlFile.close();

    return 0;
}
