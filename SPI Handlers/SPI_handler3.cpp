#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

#define CHANNEL_COUNT 8

int main() {
  // Initialize WiringPi library and SPI interface
  wiringPiSetup();
  int spi_fd = wiringPiSPISetup(0, 1000000); // SPI channel 0, clock speed 1MHz

  // Set up MCP3008 configuration byte
  unsigned char mcp3008_config[3];
  mcp3008_config[0] = 0x01; // Start bit
  mcp3008_config[1] = 0x80; // Single-ended mode, channel 0
  mcp3008_config[2] = 0x00; // Null bit

  // Read all channels for 100ms
  int sample_count = 0;
  int samples[CHANNEL_COUNT] = {0};
  unsigned long start_time = millis();
  while (millis() - start_time < 100) {
    for (int channel = 0; channel < CHANNEL_COUNT; channel++) {
      // Configure MCP3008 for current channel
      mcp3008_config[1] = 0x80 | (channel << 4);

      // Send MCP3008 configuration bytes and read result
      unsigned char rx_buffer[3];
      wiringPiSPIDataRW(0, mcp3008_config, 3);
      samples[channel] += ((rx_buffer[1] & 0x03) << 8) + rx_buffer[2];

      // Increment sample count
      sample_count++;
    }
  }

  // Output results to console
  printf("Sampled %d values:\n", sample_count);
  for (int channel = 0; channel < CHANNEL_COUNT; channel++) {
    printf("Channel %d: %d\n", channel, samples[channel]);
  }

  return 0;
}
