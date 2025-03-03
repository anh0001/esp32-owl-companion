/*
  *****************************************************************************************
  * HTTP Control Mechanism for Owl Motor
  *
  * This code integrates HTTP-based remote configuration to control the owl motor's action
  * mode. It works as follows:
  *
  * 1. GET Request – Retrieve Flag Status:
  *    - URL: http://54.250.108.126/getConfig.php?configKey=owl_motor
  *    - The device periodically (every 5 seconds, as defined by HTTP_CHECK_INTERVAL)
  *      sends an HTTP GET request to the above URL to check the current flag status.
  *    - The server responds with a JSON payload containing a "value" field.
  *      If the "value" equals "true", the device interprets it as a trigger to activate
  *      the owl motor action mode.
  *
  * 2. Action Mode Activation:
  *    - Upon receiving a "true" flag, the device sets an internal variable (performAction)
  *      to true and begins the corresponding movement and behavior sequences (nodding, LED updates,
  *      sound playback, etc.) for a predetermined duration.
  *
  * 3. Resetting the Flag:
  *    - Immediately after detecting the flag set to "true", the device issues another HTTP GET
  *      request to reset the flag:
  *      URL: http://54.250.108.126/setConfig.php?configKey=owl_motor&configValue=false
  *    - This ensures that the action mode is triggered only once per external activation.
  *
  * 4. External Activation:
  *    - To manually trigger the action mode from an external system, the flag can be set to true by
  *      sending an HTTP GET request to:
  *      URL: http://54.250.108.126/setConfig.php?configKey=owl_motor&configValue=true
  *
  * Overall, this HTTP-based configuration allows remote and dynamic control over the device’s behavior,
  * enabling a flexible response to external events. The use of JSON parsing (via ArduinoJson) ensures that
  * the configuration payload is correctly interpreted, and the immediate flag reset prevents repetitive triggers.
  *
  *****************************************************************************************
*/

#include <Adafruit_NeoPixel.h>
#include <ESP32Servo.h>
#include "driver/i2s.h"

// Added for embedded WAV playback
#include <pgmspace.h>
#include "owl_hoot.h"   // GENERATED using: xxd -i owl_hooting.wav > owl_hoot.h

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Define pins
#define LED_PIN 2
#define SERVO_PIN 10
#define MOTOR_PIN 4      // GPIO4 controls vibration motor
#define BATT_PIN 0      // ADC pin for battery monitoring
#define NUM_LEDS 1

// I2S Pins
#define I2S_BCLK 7      // Bit clock
#define I2S_LRCLK 8     // Left/right clock
#define I2S_DOUT 6      // Data out

// Battery thresholds (after voltage divider, so actual voltage is 2x these values)
#define BATT_FULL 2.0   // 4.0V
#define BATT_MED 1.85   // 3.7V
#define BATT_LOW 1.7    // 3.4V

// Servo positions and timing
#define SERVO_UP 80     // Upper position (head up/released)
#define SERVO_DOWN 60   // Lower position (head nodding down)
#define NOD_SPEED 30    // Time in milliseconds between each degree of movement

// Vibration patterns
#define VIBRATE_SHORT 100   // Increased from 200ms to 300ms to allow ramp-up
#define VIBRATE_MEDIUM 500   // Medium vibration duration (ms)
#define VIBRATE_LONG 1000    // Long vibration duration (ms)

// Audio configurations
#define SAMPLE_RATE 44100
#define DMA_BUF_COUNT 8
#define DMA_BUF_LEN 1024
#define VOLUME 0.7  // 70% volume

// New definitions for WAV playback

#define WAV_HEADER_SIZE 44
#define I2S_WAV_BUFFER_SIZE 1024

// Updated definitions
#define VOLTAGE_THRESHOLD 3.0  // Minimum voltage to allow vibration
#define VOLTAGE_CHECK_INTERVAL 100  // Check voltage every 100ms

// Owl movement patterns (in milliseconds)
#define MOVEMENT_INTERVAL 5000   // Time between movement sequences
#define RUFFLE_DURATION 200      // Duration of each ruffle vibration
#define RUFFLE_PAUSE 300        // Pause between ruffles

// New WiFi credentials & HTTP configuration
const char* ssid = "TLLMS";
const char* password = "mobimobi";
#define CONFIG_URL "http://54.250.108.126/getConfig.php?configKey=owl_motor"

// Global variables for HTTP polling and action mode
bool performAction = false;
unsigned long actionStartTime = 0;
unsigned long lastHttpCheck = 0;
#define HTTP_CHECK_INTERVAL 5000  // 5 seconds interval
#define ACTION_DURATION 20000     // 60 seconds of action mode

// Create LED and Servo objects
Adafruit_NeoPixel led = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRBW + NEO_KHZ400);
Servo nodServo;

// Variables to track servo movement
int currentPos = SERVO_UP;  // Start with head up
bool nodding = false;
unsigned long lastMoveTime = 0;

// State tracking
bool wasLowBattery = false;
unsigned long lastInteractionTime = 0;
#define IDLE_TIMEOUT 300000  // 5 minutes in milliseconds

// Add voltage checking variable
unsigned long lastVoltageCheck = 0;

// Function to initialize I2S
void initI2S() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = DMA_BUF_COUNT,
        .dma_buf_len = DMA_BUF_LEN,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };
    
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCLK,
        .ws_io_num = I2S_LRCLK,
        .data_out_num = I2S_DOUT,
        .data_in_num = I2S_PIN_NO_CHANGE
    };
    
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
}

// New function to play embedded owl sound WAV
void playOwlSound() {
  size_t offset = WAV_HEADER_SIZE;
  size_t bytesRemaining = owl_sound_size - WAV_HEADER_SIZE;
  uint8_t buffer[I2S_WAV_BUFFER_SIZE];

  while (bytesRemaining > 0) {
    size_t chunkSize = (bytesRemaining < I2S_WAV_BUFFER_SIZE) ? bytesRemaining : I2S_WAV_BUFFER_SIZE;
    memcpy_P(buffer, owl_sound_data + offset, chunkSize);
    size_t bytesWritten;
    i2s_write(I2S_NUM_0, buffer, chunkSize, &bytesWritten, portMAX_DELAY);
    offset += chunkSize;
    bytesRemaining -= chunkSize;
  }
}

// Updated startVibration function: using digitalWrite instead of VibrationControl.
void startVibration(unsigned long duration) {
    float voltage = getBatteryVoltage();
    if (voltage >= VOLTAGE_THRESHOLD) {
        digitalWrite(MOTOR_PIN, HIGH);
        delay(duration);
        digitalWrite(MOTOR_PIN, LOW);
    }
}

// Function to read battery voltage with debug log
float getBatteryVoltage() {
  uint32_t raw = analogRead(BATT_PIN);
  float voltage = (raw * 3.3) / 4095.0;
  float measuredVoltage = voltage * 2.0;
  return measuredVoltage;
}

// Updated checkEvents function without sound calls except owl sound.
void checkEvents() {
    float batteryVoltage = getBatteryVoltage();
    unsigned long currentTime = millis();
    
    // Event 1: Low battery alert
    bool isLowBattery = (batteryVoltage <= BATT_LOW * 2);
    if (isLowBattery && !wasLowBattery) {
        // startVibration(VIBRATE_LONG, MEDIUM_DUTY);
        wasLowBattery = true;
    } else if (!isLowBattery && wasLowBattery) {
        wasLowBattery = false;
    }
    
    // Event 2: Nod completion - retain owl sound.
    if (nodding && currentPos >= (SERVO_UP-1)) {
        startVibration(VIBRATE_SHORT);
        playOwlSound();
        lastInteractionTime = currentTime;
    }
    
    // Event 3: Wake from long idle
    if (currentTime - lastInteractionTime >= IDLE_TIMEOUT) {
        // startVibration(VIBRATE_MEDIUM, MEDIUM_DUTY);
        lastInteractionTime = currentTime;
    }
}

// Function to update LED based on battery level
void updateBatteryLED() {
  float voltage = getBatteryVoltage();
  
  if (voltage >= BATT_FULL * 2) {
    led.setPixelColor(0, led.Color(0, 255, 0, 0));  // Green
  } else if (voltage >= BATT_MED * 2) {
    led.setPixelColor(0, led.Color(0, 0, 255, 0));  // Blue
  } else if (voltage >= BATT_LOW * 2) {
    led.setPixelColor(0, led.Color(255, 255, 0, 0));  // Yellow
  } else {
    static bool blinkState = false;
    blinkState = !blinkState;
    if (blinkState) {
      led.setPixelColor(0, led.Color(255, 0, 0, 0));  // Red
    } else {
      led.clear();
    }
  }
  led.show();
}

// New function to check HTTP configuration
bool checkOwlMotorConfig() {
  HTTPClient http;
  http.begin(CONFIG_URL);
  int httpCode = http.GET();

  if (httpCode > 0) {  // Successful request
    String payload = http.getString();
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (!error) {
      const char* value = doc["value"];
      if (strcmp(value, "true") == 0) {
        // End the current HTTP connection
        http.end();
        
        // Immediately set the config value to false
        HTTPClient httpSet;
        httpSet.begin("http://54.250.108.126/setConfig.php?configKey=owl_motor&configValue=false");
        int httpCodeSet = httpSet.GET();
        if (httpCodeSet > 0) {
          Serial.println("Config reset to false.");
        } else {
          Serial.print("Failed to reset config, error: ");
          Serial.println(httpSet.errorToString(httpCodeSet));
        }
        httpSet.end();
        
        return true;
      }
    } else {
      Serial.print("JSON Parse Error: ");
      Serial.println(error.c_str());
    }
  } else {
    Serial.print("HTTP GET failed, error: ");
    Serial.println(http.errorToString(httpCode));
  }
  http.end();
  return false;
}

void setup() {
  Serial.begin(115200);
  
  // Initialize WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  
  // Set up MOTOR_PIN as an output for digital vibration control.
  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, LOW);
  
  // Initialize LED
  led.begin();
  led.clear();
  led.show();
  
  // Initialize ADC
  analogReadResolution(12);
  
  // Initialize Servo
  ESP32PWM::allocateTimer(0);
  nodServo.setPeriodHertz(50);
  nodServo.attach(SERVO_PIN, 500, 2400);

  // Initialize I2S
  initI2S();
    
  // Initial position
  nodServo.write(SERVO_UP);
  delay(1000);

  // Uncomment to test playing the embedded sound:
  playOwlSound();
  
  // Debug: Log system ready message
  Serial.println("System setup complete.");
  
  // Trigger welcome vibration (blocking for duration)
  startVibration(VIBRATE_SHORT);
}

void loop() {
  static unsigned long lastNodStart = 0;
  static unsigned long lastBatteryCheck = 0;
  static unsigned long lastMovementTime = 0;
  unsigned long currentTime = millis();
  
  // Periodic HTTP configuration check
  if (currentTime - lastHttpCheck >= HTTP_CHECK_INTERVAL) {
    lastHttpCheck = currentTime;
    if (checkOwlMotorConfig()) {
      performAction = true;
      actionStartTime = currentTime;
      Serial.println("Action mode activated for 60 seconds.");
    }
  }
  
  // Execute owl actions only during active action period
  if (performAction) {
    if (currentTime - actionStartTime < ACTION_DURATION) {
      // --- Existing owl behavior code ---
      // Regular battery check
      if (currentTime - lastBatteryCheck >= 1000) {
        updateBatteryLED();
        lastBatteryCheck = currentTime;
      }
      
      // Nodding control
      if (currentTime - lastNodStart >= 3000) {
        nodding = true;
        lastNodStart = currentTime;
        currentPos = SERVO_UP;
      }
      
      if (nodding && (currentTime - lastMoveTime >= NOD_SPEED)) {
        lastMoveTime = currentTime;
        
        if (currentPos > SERVO_DOWN) {
          currentPos--;
          if (currentPos <= SERVO_DOWN) {
            delay(500);
          }
        } else {
          currentPos++;
          if (currentPos >= SERVO_UP) {
            nodding = false;
          }
        }
        
        nodServo.write(currentPos);
      }
      
      // Check for events and update motor
      checkEvents();
      
    } else {
      performAction = false;
      Serial.println("Action mode deactivated.");
    }
  }
  
  // Optionally, add idle behavior if not in action mode
  // ...existing idle or minimal background tasks...
}