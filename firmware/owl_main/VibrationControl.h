#ifndef VIBRATION_CONTROL_H
#define VIBRATION_CONTROL_H

class VibrationControl {
private:
    const uint8_t motorPin;
    const uint8_t pwmChannel;
    const uint32_t pwmFreq;
    const uint8_t pwmResolution;
    bool isActive;
    unsigned long startTime;
    unsigned long duration;
    uint8_t currentDuty;
    uint8_t targetDuty;
    
    static const uint8_t RAMP_STEP = 25;        // PWM steps for soft start
    static const uint16_t RAMP_INTERVAL = 5;    // ms between duty changes
    unsigned long lastRampUpdate;               // time of last duty update
    
public:
    // Updated default frequency to 200 Hz and resolution to 8 bits
    VibrationControl(uint8_t pin, uint8_t channel = 0, uint32_t freq = 200, uint8_t res = 8) 
        : motorPin(pin), pwmChannel(channel), pwmFreq(freq), pwmResolution(res), 
          isActive(false), startTime(0), duration(0), currentDuty(0) {
        ledcSetup(pwmChannel, pwmFreq, pwmResolution);
        ledcAttachPin(motorPin, pwmChannel);
        // Set off state with no inversion
        ledcWrite(pwmChannel, 0);  
        Serial.println("PWM setup complete on motor pin");
    }
    
    void start(unsigned long durationMs, uint8_t maxDuty = 255) {
        isActive = true;
        startTime = millis();
        duration = durationMs;
        currentDuty = 0;
        targetDuty = maxDuty;
        lastRampUpdate = startTime;    // Initialize ramp timer
        // Debug: Log motor start event
        Serial.print("Vibration motor start: duration=");
        Serial.print(durationMs);
        Serial.print("ms, targetDuty=");
        Serial.println(maxDuty);
    }
    
    void stop() {
        isActive = false;
        currentDuty = 0;
        // Set PWM off without inversion
        ledcWrite(pwmChannel, 0);
        // Debug: Log motor stop event
        Serial.println("Vibration motor stop");
    }
    
    void update() {
        if (!isActive) return;
        
        unsigned long currentTime = millis();
        unsigned long elapsed = currentTime - startTime;
        
        // Use a robust timing check instead of modulo condition
        if (currentDuty < targetDuty && (currentTime - lastRampUpdate) >= RAMP_INTERVAL) {
            currentDuty = min(static_cast<uint8_t>(currentDuty + RAMP_STEP), targetDuty);
            // Write PWM duty directly without inversion.
            ledcWrite(pwmChannel, currentDuty);
            lastRampUpdate = currentTime; // update ramp timer
            // Debug: Log ramp update
            Serial.print("Vibration motor ramp update: currentDuty=");
            Serial.println(currentDuty);
        }
        
        // Check if duration expired
        if (elapsed >= duration) {
            stop();
        }
    }
    
    bool isRunning() const { return isActive; }
};

#endif
