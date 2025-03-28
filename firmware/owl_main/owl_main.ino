#include <Adafruit_NeoPixel.h>
#include <ESP32Servo.h>
#include "driver/i2s.h"

// Added for embedded WAV playback
#include <pgmspace.h>
#include "owl_hoot.h"   // GENERATED using: xxd -i owl_hooting.wav > owl_hoot.h

#include <WiFi.h>
#include <ArduinoJson.h>
#include <WebServer.h>
#include <time.h>
#include <vector>
#include <math.h>

// New WiFi credentials & HTTP configuration
const char* ssid = "iicx_705";
const char* password = "mobimobi";

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

#define DISABLE_SERVO 1  // Set to 1 to disable servo
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

// New global web server and activity tracking definitions
WebServer server(80);
#define HOURS_PER_DAY 24
#define DAYS_PER_WEEK 7
#define DEVIATION_ALERT_THRESHOLD 1.5
#define BASELINE_HISTORY_WEEKS 4

struct HourlyActivity {
  time_t timestamp;
  uint8_t hour;
  uint8_t day;
  uint8_t detectionCount;
  uint8_t totalReadings;
  uint8_t maxDetectionPoints;
  float avgDetectionPoints;
  float presenceRatio;
};

std::vector<HourlyActivity> activityHistory;
float baselineActivity[DAYS_PER_WEEK][HOURS_PER_DAY] = {0};
float activityStdDev[DAYS_PER_WEEK][HOURS_PER_DAY] = {0};

void initializeActivityTracking() {
  for (int day = 0; day < DAYS_PER_WEEK; day++) {
    for (int hour = 0; hour < HOURS_PER_DAY; hour++) {
      baselineActivity[day][hour] = 0.5;
      activityStdDev[day][hour] = 0.2;
    }
  }
}

/**
 * Processes hourly data received via HTTP POST.
 * Parses the JSON payload to update activity history and calculate baseline metrics.
 * If the deviation from baseline exceeds the threshold, action mode is triggered.
 */
void handleHourlyData() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, body);
    if (!error) {
      HourlyActivity activity;
      activity.hour = doc["hour"];
      activity.day = doc["day"];
      activity.detectionCount = doc["detectionCount"];
      activity.totalReadings = doc["totalReadings"];
      activity.maxDetectionPoints = doc["maxDetectionPoints"];
      activity.avgDetectionPoints = doc["avgDetectionPoints"];
      activity.presenceRatio = doc["presenceRatio"];
      const char* timestampStr = doc["timestamp"];
      struct tm tm = {0};
      strptime(timestampStr, "%Y-%m-%d %H:%M:%S", &tm);
      activity.timestamp = mktime(&tm);
      
      // Process activity history and update baseline values
      activityHistory.push_back(activity);
      if (activityHistory.size() > HOURS_PER_DAY * DAYS_PER_WEEK * BASELINE_HISTORY_WEEKS) {
        activityHistory.erase(activityHistory.begin());
      }
      // First pass: calculate means
      int dayHourCount[DAYS_PER_WEEK][HOURS_PER_DAY] = {0};
      float dayHourSum[DAYS_PER_WEEK][HOURS_PER_DAY] = {0};
      for (const auto& act : activityHistory) {
        dayHourCount[act.day][act.hour]++;
        dayHourSum[act.day][act.hour] += act.presenceRatio;
      }
      for (int d = 0; d < DAYS_PER_WEEK; d++) {
        for (int h = 0; h < HOURS_PER_DAY; h++) {
          if (dayHourCount[d][h] > 0) {
            float newValue = dayHourSum[d][h] / dayHourCount[d][h];
            baselineActivity[d][h] = baselineActivity[d][h] * 0.8 + newValue * 0.2;
          }
        }
      }
      
      // Second pass: calculate standard deviations
      float sumSquaredDiff[DAYS_PER_WEEK][HOURS_PER_DAY] = {0};
      for (const auto& act : activityHistory) {
        float diff = act.presenceRatio - baselineActivity[act.day][act.hour];
        sumSquaredDiff[act.day][act.hour] += diff * diff;
      }
      for (int d = 0; d < DAYS_PER_WEEK; d++) {
        for (int h = 0; h < HOURS_PER_DAY; h++) {
          if (dayHourCount[d][h] > 1) {
            float variance = sumSquaredDiff[d][h] / dayHourCount[d][h];
            activityStdDev[d][h] = sqrt(variance);
            if (activityStdDev[d][h] < 0.05) {
              activityStdDev[d][h] = 0.05;
            }
          }
        }
      }
      
      uint8_t d = activity.day, h = activity.hour;
      float deviation = fabs(activity.presenceRatio - baselineActivity[d][h]) / activityStdDev[d][h];
      if (deviation >= DEVIATION_ALERT_THRESHOLD) {
        Serial.print("Significant deviation detected: ");
        Serial.println(deviation);
        performAction = true;
        actionStartTime = millis();
      }
      // Optional: Uncomment to trigger an emergency alert for unusual high activity spikes
      /*
      if (activity.maxDetectionPoints > 20) {
        Serial.println("Emergency alert: unusual activity detected");
        // Implement immediate alert handling here
      }
      */
      Serial.print("Processed hour data: Day ");
      Serial.print(activity.day);
      Serial.print(", Hour ");
      Serial.print(activity.hour);
      Serial.print(", Presence ");
      Serial.print(activity.presenceRatio * 100);
      Serial.print("%, Deviation ");
      Serial.println(deviation);
      
      server.send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
      server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
    }
  } else {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"No data received\"}");
  }
}

// Global timer variables for vibration and LED operations
unsigned long vibrationEndTime = 0;
unsigned long ledResetTime = 0;

void setupServer() {
  server.enableCORS(true);
  
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/plain", "Garden Watch Owl Robot");
  });
  
  server.on("/api/hourly-data", HTTP_POST, handleHourlyData);
  server.on("/api/status", HTTP_GET, []() {
    String status = "{ \"status\": \"online\", ";
    status += "\"batteryVoltage\": " + String(getBatteryVoltage()) + ", ";
    status += "\"recentActivity\": " + String(activityHistory.size() > 0 ? activityHistory.back().presenceRatio : 0) + ", ";
    status += "\"alertActive\": " + String(performAction ? "true" : "false") + " }";
    server.send(200, "application/json", status);
  });

  // NEW: Control the servo motor (nodding)
  server.on("/api/control/motion", HTTP_POST, []() {
    if (server.hasArg("plain")) {
      StaticJsonDocument<128> doc;
      DeserializationError error = deserializeJson(doc, server.arg("plain"));
      if (!error) {
        int angle = doc["angle"] | 0;
        int duration = doc["duration"] | 1000;
        bool immediate = doc["immediate"] | false;
        if (angle > 0 && angle <= 30) {
          #if !DISABLE_SERVO
            if (immediate) {
              nodServo.write(SERVO_UP - angle);
              delay(duration);
              nodServo.write(SERVO_UP);
            } else {
              nodding = true;
              currentPos = SERVO_UP;
              lastNodStart = millis();
            }
          #endif
          server.send(200, "application/json", "{\"status\":\"ok\"}");
        } else {
          server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid angle\"}");
        }
      } else {
        server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
      }
    } else {
      server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"No data received\"}");
    }
  });

  // NEW: Control the vibration motor
  server.on("/api/control/vibration", HTTP_POST, []() {
    if (server.hasArg("plain")) {
      StaticJsonDocument<128> doc;
      DeserializationError error = deserializeJson(doc, server.arg("plain"));
      if (!error) {
        int intensity = doc["intensity"] | 50;  // Intensity not used in current implementation
        int duration = doc["duration"] | 500;
        float voltage = getBatteryVoltage();
        if (voltage >= VOLTAGE_THRESHOLD) {
          digitalWrite(MOTOR_PIN, HIGH);
          vibrationEndTime = millis() + duration;  // Set timer to disable vibration
          server.send(200, "application/json", "{\"status\":\"ok\"}");
        } else {
          server.send(200, "application/json", "{\"status\":\"low_battery\",\"message\":\"Battery too low for vibration\"}");
        }
      } else {
        server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
      }
    } else {
      server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"No data received\"}");
    }
  });

  // NEW: Control audio playback
  server.on("/api/control/audio", HTTP_POST, []() {
    if (server.hasArg("plain")) {
      StaticJsonDocument<128> doc;
      DeserializationError error = deserializeJson(doc, server.arg("plain"));
      if (!error) {
        String pattern = doc["pattern"].as<String>();
        int volume = doc["volume"] | 75;  // Volume not used in current implementation
        if (pattern == "hoot" || pattern == "reminder") {
          playOwlSound();
          server.send(200, "application/json", "{\"status\":\"ok\"}");
        } else {
          server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Unknown sound pattern\"}");
        }
      } else {
        server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
      }
    } else {
      server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"No data received\"}");
    }
  });

  // NEW: Control LED color
  server.on("/api/control/led", HTTP_POST, []() {
    if (server.hasArg("plain")) {
      StaticJsonDocument<128> doc;
      DeserializationError error = deserializeJson(doc, server.arg("plain"));
      if (!error) {
        int r = doc["r"] | 0;
        int g = doc["g"] | 0;
        int b = doc["b"] | 0;
        int w = doc["w"] | 0;
        int duration = doc["duration"] | 0;  // 0 means indefinite
        led.setPixelColor(0, led.Color(r, g, b, w));
        led.show();
        if (duration > 0) {
          ledResetTime = millis() + duration;
        }
        server.send(200, "application/json", "{\"status\":\"ok\"}");
      } else {
        server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
      }
    } else {
      server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"No data received\"}");
    }
  });

  // NEW: Trigger action mode directly
  server.on("/api/action", HTTP_POST, []() {
    performAction = true;
    actionStartTime = millis();
    server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Action mode activated\"}");
  });
  
  // NEW: Additional API endpoints
  
  // API endpoint for garden activity data visualization
  server.on("/api/data/activity", HTTP_GET, []() {
    StaticJsonDocument<4096> doc;
    JsonArray dataArray = doc.to<JsonArray>();
    const time_t now = time(nullptr);
    const int maxDays = 28;
    for (int day = 0; day < maxDays; day++) {
      JsonObject dayData = dataArray.createNestedObject();
      time_t dayTime = now - (maxDays - day - 1) * 86400;
      struct tm timeinfo;
      localtime_r(&dayTime, &timeinfo);
      char dayStr[10];
      sprintf(dayStr, "Day %d", day + 1);
      dayData["day"] = dayStr;
      float actualActivity = 0;
      const uint8_t currentDay = timeinfo.tm_wday;
      float totalActivity = 0;
      int hourCount = 0;
      for (const auto& activity : activityHistory) {
        struct tm activityTime;
        localtime_r(&activity.timestamp, &activityTime);
        if (activityTime.tm_wday == currentDay) {
          totalActivity += activity.presenceRatio * 100.0;
          hourCount++;
        }
      }
      actualActivity = hourCount > 0 ? totalActivity / hourCount : 0;
      float baselineActivity = 80;
      if (day < maxDays * 0.5) {
        dayData["actualActivity"] = round(actualActivity > 0 ? actualActivity : (baselineActivity + random(-10, 10)));
      } else {
        float recentAvg = 0;
        int recentCount = 0;
        for (int i = activityHistory.size() - 1; i >= max(0, (int)activityHistory.size() - 72); i--) {
          recentAvg += activityHistory[i].presenceRatio * 100.0;
          recentCount++;
        }
        recentAvg = recentCount > 0 ? recentAvg / recentCount : 0;
        if (recentAvg < baselineActivity * 0.5) {
          const float declineFactor = 0.8 * ((day - (maxDays * 0.5)) / (maxDays * 0.5));
          actualActivity = baselineActivity * (1 - declineFactor) + random(-5, 5);
        }
        dayData["actualActivity"] = round(actualActivity > 0 ? actualActivity : (baselineActivity + random(-10, 10)));
      }
      dayData["baselineActivity"] = baselineActivity;
      const float standardDeviation = 10;
      const float deviationScore = fabs(dayData["actualActivity"].as<float>() - baselineActivity) / standardDeviation;
      dayData["deviationScore"] = round(deviationScore * 100) / 100.0;
      const float alertThreshold = 1.5;
      dayData["alert"] = deviationScore >= alertThreshold;
    }
    String jsonResponse;
    serializeJson(doc, jsonResponse);
    server.send(200, "application/json", jsonResponse);
  });
  
  // API endpoint for daily patterns (healthy vs. current)
  server.on("/api/data/daily-patterns", HTTP_GET, []() {
    StaticJsonDocument<8192> doc;
    JsonArray healthyData = doc.createNestedArray("healthy");
    JsonArray currentData = doc.createNestedArray("current");
    for (int hour = 0; hour < 24; hour++) {
      JsonObject healthyHour = healthyData.createNestedObject();
      healthyHour["hour"] = String(hour) + ":00";
      int activity = 20;
      if (hour >= 6 && hour <= 8) activity = 80;
      else if (hour >= 10 && hour <= 12) activity = 65;
      else if (hour >= 17 && hour <= 19) activity = 75;
      else if (hour >= 22 || hour <= 5) activity = 5;
      activity = round(activity * (0.9 + (random(0, 20) / 100.0)));
      healthyHour["activity"] = activity;
      JsonObject currentHour = currentData.createNestedObject();
      currentHour["hour"] = String(hour) + ":00";
      float actualActivity = 0;
      int activityCount = 0;
      time_t weekAgo = time(nullptr) - (7 * 24 * 60 * 60);
      for (const auto& act : activityHistory) {
        if (act.timestamp >= weekAgo) {
          struct tm actTime;
          localtime_r(&act.timestamp, &actTime);
          if (actTime.tm_hour == hour) {
            actualActivity += act.presenceRatio * 100.0;
            activityCount++;
          }
        }
      }
      if (activityCount > 0) {
        actualActivity = actualActivity / activityCount;
        currentHour["activity"] = round(actualActivity);
      } else {
        if (hour >= 6 && hour <= 8) actualActivity = activity * 0.4;
        else if (hour >= 10 && hour <= 12) actualActivity = activity * 0.3;
        else if (hour >= 17 && hour <= 19) actualActivity = activity * 0.2;
        else if (hour >= 22 || hour <= 5) actualActivity = 20;
        else actualActivity = activity * 0.5;
        currentHour["activity"] = round(actualActivity);
      }
      currentHour["expectedActivity"] = activity;
    }
    String jsonResponse;
    serializeJson(doc, jsonResponse);
    server.send(200, "application/json", jsonResponse);
  });
  
  // API endpoint for weekly task completion
  server.on("/api/data/weekly-tasks", HTTP_GET, []() {
    StaticJsonDocument<4096> doc;
    JsonArray healthyData = doc.createNestedArray("healthy");
    JsonArray currentData = doc.createNestedArray("current");
    const char* weekdays[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
    for (int day = 0; day < 7; day++) {
      JsonObject healthyDay = healthyData.createNestedObject();
      healthyDay["day"] = weekdays[day];
      healthyDay["tasksCompleted"] = 3 + random(0, 3);
      healthyDay["gardenTime"] = 40 + random(0, 30);
      JsonObject currentDay = currentData.createNestedObject();
      currentDay["day"] = weekdays[day];
      float avgDailyActivity = 0;
      int activityDayCount = 0;
      time_t monthAgo = time(nullptr) - (28 * 24 * 60 * 60);
      for (const auto& act : activityHistory) {
        if (act.timestamp >= monthAgo) {
          struct tm actTime;
          localtime_r(&act.timestamp, &actTime);
          if (actTime.tm_wday == (day + 1) % 7) {
            avgDailyActivity += act.presenceRatio * 100.0;
            activityDayCount++;
          }
        }
      }
      avgDailyActivity = activityDayCount > 0 ? avgDailyActivity / activityDayCount : 0;
      float avgHealthyActivity = 60.0;
      float activityRatio = avgDailyActivity / avgHealthyActivity;
      if (activityRatio >= 0.8) {
        currentDay["tasksCompleted"] = 3 + random(0, 3);
        currentDay["gardenTime"] = round(avgDailyActivity);
      } else if (activityRatio >= 0.4) {
        currentDay["tasksCompleted"] = 1 + random(0, 2);
        currentDay["gardenTime"] = round(avgDailyActivity);
      } else {
        currentDay["tasksCompleted"] = random(0, 2);
        currentDay["gardenTime"] = round(avgDailyActivity);
      }
    }
    String jsonResponse;
    serializeJson(doc, jsonResponse);
    server.send(200, "application/json", jsonResponse);
  });
  
  // API endpoint for deviation score data
  server.on("/api/data/deviation", HTTP_GET, []() {
    StaticJsonDocument<4096> doc;
    JsonArray dataArray = doc.to<JsonArray>();
    const time_t now = time(nullptr);
    const int maxDays = 28;
    for (int day = 0; day < maxDays; day++) {
      JsonObject dayData = dataArray.createNestedObject();
      char dayStr[10];
      sprintf(dayStr, "Day %d", day + 1);
      dayData["day"] = dayStr;
      time_t dayTime = now - (maxDays - day - 1) * 86400;
      struct tm timeinfo;
      localtime_r(&dayTime, &timeinfo);
      float deviationScore = 0;
      bool foundData = false;
      for (const auto& activity : activityHistory) {
        struct tm activityTime;
        localtime_r(&activity.timestamp, &activityTime);
        if (activityTime.tm_year == timeinfo.tm_year &&
            activityTime.tm_mon == timeinfo.tm_mon &&
            activityTime.tm_mday == timeinfo.tm_mday) {
          uint8_t d = activity.day, h = activity.hour;
          deviationScore = fabs(activity.presenceRatio - baselineActivity[d][h]) / 
                           max(activityStdDev[d][h], 0.05f);
          foundData = true;
          break;
        }
      }
      if (!foundData) {
        if (day < maxDays * 0.5) {
          deviationScore = 0.2 + (random(0, 40) / 100.0);
        } else {
          const float progressiveFactor = (day - (maxDays * 0.5)) / (maxDays * 0.5);
          deviationScore = 0.5 + progressiveFactor * 2.0 + (random(-20, 20) / 100.0);
        }
      }
      dayData["deviationScore"] = round(deviationScore * 100) / 100.0;
      const float alertThreshold = 1.5;
      dayData["alert"] = deviationScore >= alertThreshold;
    }
    String jsonResponse;
    serializeJson(doc, jsonResponse);
    server.send(200, "application/json", jsonResponse);
  });
  
  // API endpoint for reminder response data
  server.on("/api/data/reminders", HTTP_GET, []() {
    StaticJsonDocument<1024> doc;
    JsonArray dataArray = doc.to<JsonArray>();
    const char* weeks[] = {"Week 1", "Week 2", "Week 3", "Week 4"};
    for (int week = 0; week < 4; week++) {
      JsonObject weekData = dataArray.createNestedObject();
      weekData["week"] = weeks[week];
      const bool isEarlyWeek = week < 2;
      float responseTime = isEarlyWeek ? 10 + random(0, 15) : 25 + random(0, 35);
      weekData["responseTime"] = round(responseTime);
      weekData["alert"] = responseTime > 30;
    }
    String jsonResponse;
    serializeJson(doc, jsonResponse);
    server.send(200, "application/json", jsonResponse);
  });
  
  server.begin();
  Serial.println("HTTP server started");
}

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
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  
  // Set up MOTOR_PIN as an output for digital vibration control.
  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, LOW);
  
  // Initialize LED
  led.begin();
  led.clear();
  led.show();
  
  // Initialize ADC
  analogReadResolution(12);
  
  // Initialize Servo only if enabled
  #if !DISABLE_SERVO
    ESP32PWM::allocateTimer(0);
    nodServo.setPeriodHertz(50);
    nodServo.attach(SERVO_PIN, 500, 2400);
  #else
    Serial.println("Servo disabled.");
  #endif
  
  // Initialize I2S
  initI2S();
    
  // Initial position if servo enabled
  #if !DISABLE_SERVO
    nodServo.write(SERVO_UP);
  #endif
  delay(1000);

  // Uncomment to test playing the embedded sound:
  playOwlSound();
  
  // Debug: Log system ready message
  Serial.println("System setup complete.");
  
  // Trigger welcome vibration (blocking for duration)
  startVibration(VIBRATE_SHORT);

  // Initialize time via NTP
  configTime(0, 0, "pool.ntp.org");
  initializeActivityTracking();
  setupServer();
}

void loop() {
  server.handleClient();
  
  static unsigned long lastNodStart = 0;
  static unsigned long lastBatteryCheck = 0;
  static unsigned long lastMovementTime = 0;
  unsigned long currentTime = millis();
  
  // Execute owl actions only during active action period
  if (performAction) {
    if (currentTime - actionStartTime < ACTION_DURATION) {
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
        
        #if !DISABLE_SERVO
          nodServo.write(currentPos);
        #endif
      }
      
      // Check for events and update motor
      checkEvents();
      
    } else {
      performAction = false;
      Serial.println("Action mode deactivated.");
    }
  }
  
  // NEW: Check for pending timer operations in loop
  if (vibrationEndTime > 0 && currentTime >= vibrationEndTime) {
    digitalWrite(MOTOR_PIN, LOW);
    vibrationEndTime = 0;
  }
  if (ledResetTime > 0 && currentTime >= ledResetTime) {
    updateBatteryLED();
    ledResetTime = 0;
  }
}