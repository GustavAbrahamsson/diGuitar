#include <Arduino.h>
#include <driver/i2s.h>

const i2s_port_t I2S_PORT = I2S_NUM_0;
#define ARRAY_SIZE 16
#define I2S_COMM_FORMAT_STAND_I2S 0x01
//#define I2S_CHANNEL_FMT_ONLY_LEFT 1

void setup()
{
  esp_err_t err;
// The I2S config as per the example
  const i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX), // Receive, not transfer
    .sample_rate = 5000,                         // 5KHz
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // 16bit samples
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // use left channel
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,     // Interrupt level 1
    .dma_buf_count = 3,                           // number of buffers
    .dma_buf_len = ARRAY_SIZE                     // samples per buffer
  };

  // The pin config as per the setup
  const i2s_pin_config_t pin_config = {
    .bck_io_num = 8,   // Serial Clock (SCK)
    .ws_io_num = 9,    // Word Select (WS)
    .data_out_num = I2S_PIN_NO_CHANGE, // not used (only for speakers)
    .data_in_num = 10   // Serial Data (SD)
  };

  // Configuring the I2S driver and pins.
  // This function must be called before any I2S driver read/write operations.
  err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  if (err != ESP_OK) {
    Serial.printf("Failed installing driver: %d\n", err);
    while (true);
  }
  err = i2s_set_pin(I2S_PORT, &pin_config);
  if (err != ESP_OK) {
    Serial.printf("Failed setting pin: %d\n", err);
    while (true);
  }
  Serial.println("I2S driver installed.");
  }
  
int16_t sBuffer[ARRAY_SIZE];
void loop()
{
    size_t bytesIn = 0;
    esp_err_t result = i2s_read(I2S_PORT, sBuffer, sizeof(sBuffer), &bytesIn, /*portMAX_DELAY*/ 10); // no timeout
    if (result == ESP_OK && bytesIn > 0)
    {
    	// do your processing
   }
}
 