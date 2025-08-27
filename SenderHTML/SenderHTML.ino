#include <WiFi.h>
#include <HTTPClient.h>
#include <math.h>

// Replace with your home WiFi credentials
#define WIFI_SSID "simpa_coko0"
#define WIFI_PASS "147258369ssS0"

// Replace with the IP address of your receiver ESP32
// You can find this IP in the receiver's serial monitor
#define RECEIVER_IP "192.168.1.106"  // Change this to your receiver's IP
#define RECEIVER_PORT 80

// Define pins for the ultrasonic sensor connected to the ESP32
#define TRIG 5  // Trigger pin for the ultrasonic sensor
#define ECHO 18 // Echo pin for the ultrasonic sensor
#define NUM_SAMPLES 5 // Number of samples to average for more stable readings

// Define LED pins for water level indication
#define DANGER_LED 2  // LED for HIGH water level alert
#define NORMAL_LED 4  // LED for NORMAL water level

#define CONTAINER_DEPTH 30 // Depth of container in cm

// Timing variables
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 5000; // Send data every 5 seconds

// WiFi connection variables
bool wifiConnected = false;
unsigned long lastWiFiCheck = 0;
const unsigned long wifiCheckInterval = 30000; // Check WiFi every 30 seconds

// HTTP client
HTTPClient http;

// Function to connect to WiFi
void connectToWiFi() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Connecting to WiFi...");
        WiFi.begin(WIFI_SSID, WIFI_PASS);
        
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            Serial.print(".");
            attempts++;
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            wifiConnected = true;
            Serial.println();
            Serial.print("Connected to WiFi! IP address: ");
            Serial.println(WiFi.localIP());
            Serial.print("Signal strength (RSSI): ");
            Serial.print(WiFi.RSSI());
            Serial.println(" dBm");
        } else {
            wifiConnected = false;
            Serial.println();
            Serial.println("Failed to connect to WiFi");
        }
    } else {
        wifiConnected = true;
    }
}

// Function to measure distance using the ultrasonic sensor
float getWaterLevel() {
    float total = 0;
    int validReadings = 0;
    
    for (int i = 0; i < NUM_SAMPLES; i++) {
        digitalWrite(TRIG, LOW);
        delayMicroseconds(2);
        digitalWrite(TRIG, HIGH);
        delayMicroseconds(10);
        digitalWrite(TRIG, LOW);
        
        long duration = pulseIn(ECHO, HIGH, 30000); // 30ms timeout
        if (duration > 0) { // Valid reading
            float distance = duration * 0.034 / 2; // Calculate distance in cm
            total += distance;
            validReadings++;
        }
        delay(50);
    }
    
    if (validReadings > 0) {
        return total / validReadings; // Average of valid readings
    } else {
        return 400; // Return large value if no valid readings
    }
}

// Function to send water level data to receiver
bool sendWaterLevelData(float waterLevel) {
    if (!wifiConnected) {
        Serial.println("WiFi not connected, cannot send data");
        return false;
    }
    
    // Calculate actual water level (distance from container bottom)
    float actualWaterLevel = 21.49 - waterLevel; // Same calculation as in receiver
    
    // Prepare HTTP POST request
    String serverURL = "http://" + String(RECEIVER_IP) + ":" + String(RECEIVER_PORT) + "/water_level";
    
    http.begin(serverURL);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    
    // Prepare POST data
    String postData = "level=" + String(actualWaterLevel, 2);
    
    Serial.print("Sending data to: ");
    Serial.println(serverURL);
    Serial.print("POST data: ");
    Serial.println(postData);
    
    // Send POST request
    int httpResponseCode = http.POST(postData);
    
    bool success = false;
    if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        Serial.print("Response: ");
        Serial.println(response);
        
        if (httpResponseCode == 200) {
            success = true;
            Serial.println("✓ Data sent successfully!");
        }
    } else {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
        Serial.println("Check if receiver IP address is correct");
    }
    
    http.end();
    return success;
}

void setup() {
    // Set the ultrasonic sensor pins as output (TRIG) and input (ECHO)
    pinMode(TRIG, OUTPUT);
    pinMode(ECHO, INPUT);
    
    // Initialize LED pins
    pinMode(DANGER_LED, OUTPUT);
    pinMode(NORMAL_LED, OUTPUT);
    
    // Initially turn both LEDs OFF
    digitalWrite(DANGER_LED, LOW);
    digitalWrite(NORMAL_LED, LOW);
    
    // Initialize Serial Monitor for debugging
    Serial.begin(115200);
    Serial.println();
    Serial.println("Water Level Sender - WiFi Version");
    Serial.println("=================================");
    
    // Set ESP32 in Wi-Fi Station mode
    WiFi.mode(WIFI_STA);
    
    // Connect to WiFi
    connectToWiFi();
    
    Serial.println("Setup completed!");
    Serial.print("Receiver IP: ");
    Serial.println(RECEIVER_IP);
    Serial.print("Send interval: ");
    Serial.print(sendInterval / 1000);
    Serial.println(" seconds");
}

void loop() {
    unsigned long currentTime = millis();
    
    // Check WiFi connection periodically
    if (currentTime - lastWiFiCheck >= wifiCheckInterval) {
        lastWiFiCheck = currentTime;
        
        if (WiFi.status() != WL_CONNECTED) {
            wifiConnected = false;
            Serial.println("WiFi connection lost, reconnecting...");
            connectToWiFi();
        } else {
            wifiConnected = true;
        }
    }
    
    // Measure water level distance
    float waterLevel = getWaterLevel();
    
    Serial.print("Water Level Distance: ");
    Serial.print(waterLevel);
    Serial.println(" cm");

    // LED control based on water level threshold
    if (waterLevel > 15) { // Adjusted threshold to 15 cm instead of 25
        digitalWrite(DANGER_LED, HIGH);
        digitalWrite(NORMAL_LED, LOW);
        Serial.println("WARNING: Water level too high! DANGER LED ON.");
    } else {
        digitalWrite(DANGER_LED, LOW);
        digitalWrite(NORMAL_LED, HIGH);
        Serial.println("Water level is normal. NORMAL LED ON.");
    }

    // Send data to receiver at specified intervals
    if (currentTime - lastSendTime >= sendInterval) {
        lastSendTime = currentTime;
        
        if (wifiConnected) {
            Serial.println("\n--- Sending Data ---");
            bool success = sendWaterLevelData(waterLevel);
            
            if (!success) {
                Serial.println("Failed to send data, will retry next cycle");
            }
            Serial.println("--- End Send ---\n");
        } else {
            Serial.println("WiFi not connected, skipping data send");
        }
    }
    
    delay(1000); // Wait 1 second before next measurement
}