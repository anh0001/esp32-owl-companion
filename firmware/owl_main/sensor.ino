#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_AMG88xx.h>
#include <VL53L5x.h>
#include <time.h>

// WiFi credentials
const char* ssid = "TLLMS";
const char* password = "mobimobi";

// Owl robot API endpoint
const char* owlApiEndpoint = "http://[OWL_IP_ADDRESS]/api/hourly-data";

// Sensors
Adafruit_AMG88xx amg;
VL53L5x depthSensor;

// Thresholds
#define THERMAL_MIN_TEMP 22.0
#define THERMAL_MAX_TEMP 31.0
#define DEPTH_MIN 0.5
#define DEPTH_MAX 3.0
#define PRESENCE_THRESHOLD 3

// Timing constants
#define MONITOR_INTERVAL 300000   // Monitor every 5 minutes (300000ms)
#define HOUR_MILLIS 3600000       // Hour in milliseconds

// Activity accumulation variables
struct HourlyStats {
  uint8_t detectionCount;         // How many readings detected presence
  uint8_t totalReadings;          // Total readings taken
  uint8_t maxDetectionPoints;     // Maximum number of detection points in hour
  float avgDetectionPoints;       // Average detection points
  time_t timestamp;               // Starting timestamp for this hour
};

HourlyStats currentHourStats = {0, 0, 0, 0.0, 0};
unsigned long hourStartTime = 0;
unsigned long lastMonitorTime = 0;

void setup() {
  Serial.begin(115200);
  
  // Initialize WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  
  // Initialize sensors
  if (!amg.begin()) {
    Serial.println("AMG8833 thermal sensor not found");
    while (1);
  }
  
  if (!depthSensor.begin()) {
    Serial.println("VL53L5X depth sensor not found");
    while (1);
  }
  
  // Initialize time
  configTime(0, 0, "pool.ntp.org");
  time_t now;
  while (time(&now) < 1000000000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nTime synchronized");
  
  // Initialize hourly tracking
  resetHourlyStats();
  hourStartTime = millis();
  
  Serial.println("Sensor system initialized");
}

// Reset the hourly statistics
void resetHourlyStats() {
  time_t now;
  time(&now);
  
  currentHourStats.detectionCount = 0;
  currentHourStats.totalReadings = 0;
  currentHourStats.maxDetectionPoints = 0;
  currentHourStats.avgDetectionPoints = 0.0;
  currentHourStats.timestamp = now;
}

// Detect human presence and return detection points
uint8_t monitorActivity() {
  float thermalPixels[AMG88xx_PIXEL_ARRAY_SIZE];
  amg.readPixels(thermalPixels);
  
  // Get depth data
  uint8_t depthData[64]; // 8x8 grid
  depthSensor.readMultiRange(depthData);
  
  // Process each pixel in the 8x8 grid
  uint8_t detectionPoints = 0;
  for (int i = 0; i < 64; i++) {
    // Check temperature and depth conditions
    bool validTemp = (thermalPixels[i] >= THERMAL_MIN_TEMP && 
                     thermalPixels[i] <= THERMAL_MAX_TEMP);
    bool validDepth = (depthData[i] >= DEPTH_MIN && 
                      depthData[i] <= DEPTH_MAX);
    
    // Count valid detection points
    if (validTemp && validDepth) {
      detectionPoints++;
    }
  }
  
  return detectionPoints;
}

// Update hourly statistics with new reading
void updateHourlyStats(uint8_t detectionPoints) {
  // Increment total readings
  currentHourStats.totalReadings++;
  
  // Update detection count if presence detected
  if (detectionPoints >= PRESENCE_THRESHOLD) {
    currentHourStats.detectionCount++;
  }
  
  // Update maximum detection points
  if (detectionPoints > currentHourStats.maxDetectionPoints) {
    currentHourStats.maxDetectionPoints = detectionPoints;
  }
  
  // Update running average
  float totalPoints = currentHourStats.avgDetectionPoints * 
                     (currentHourStats.totalReadings - 1) + detectionPoints;
  currentHourStats.avgDetectionPoints = totalPoints / currentHourStats.totalReadings;
}

// Send accumulated hourly data to owl robot
bool sendHourlyData() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected, attempting reconnection...");
    WiFi.begin(ssid, password);
    delay(5000);
    if (WiFi.status() != WL_CONNECTED) {
      return false;
    }
  }
  
  HTTPClient http;
  http.begin(owlApiEndpoint);
  http.addHeader("Content-Type", "application/json");
  
  // Create JSON payload
  StaticJsonDocument<512> doc;
  
  // Hour data
  struct tm timeinfo;
  localtime_r(&currentHourStats.timestamp, &timeinfo);
  
  char timestamp[25];
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);
  
  doc["timestamp"] = timestamp;
  doc["hour"] = timeinfo.tm_hour;
  doc["day"] = timeinfo.tm_wday;
  doc["detectionCount"] = currentHourStats.detectionCount;
  doc["totalReadings"] = currentHourStats.totalReadings;
  doc["maxDetectionPoints"] = currentHourStats.maxDetectionPoints;
  doc["avgDetectionPoints"] = currentHourStats.avgDetectionPoints;
  doc["presenceRatio"] = (float)currentHourStats.detectionCount / 
                        (float)currentHourStats.totalReadings;
  
  String requestBody;
  serializeJson(doc, requestBody);
  
  // Send POST request
  int httpResponseCode = http.POST(requestBody);
  
  if (httpResponseCode > 0) {
    Serial.println("Hourly data sent successfully");
    String response = http.getString();
    Serial.println(response);
    http.end();
    return true;
  } else {
    Serial.print("Error sending data: ");
    Serial.println(httpResponseCode);
    http.end();
    return false;
  }
}

void loop() {
  unsigned long currentTime = millis();
  
  // Check for activity at regular intervals
  if (currentTime - lastMonitorTime >= MONITOR_INTERVAL) {
    lastMonitorTime = currentTime;
    
    // Monitor and update statistics
    uint8_t detectionPoints = monitorActivity();
    updateHourlyStats(detectionPoints);
    
    Serial.print("Monitoring: ");
    Serial.print(detectionPoints);
    Serial.print(" detection points. Hour stats: ");
    Serial.print(currentHourStats.detectionCount);
    Serial.print("/");
    Serial.print(currentHourStats.totalReadings);
    Serial.println(" detections");
  }
  
  // Check if an hour has passed
  if (currentTime - hourStartTime >= HOUR_MILLIS) {
    Serial.println("Hour complete, sending data...");
    
    // Send data to owl robot
    if (sendHourlyData()) {
      // Reset for next hour
      resetHourlyStats();
      hourStartTime = currentTime;
    } else {
      // If sending fails, retry in 5 minutes
      // but keep accumulating data
      Serial.println("Send failed, will retry in 5 minutes");
      delay(10);  // Small delay to avoid blocking
    }
  }
}