#include <WiFi.h>
#include <WebServer.h>
#include <HardwareSerial.h>
#include <math.h>
#include "WaterMonitor.h"   // Include the HTML file

// Network Configuration
// Uncomment the line below to use your home WiFi for easier debugging
#define USE_INTRANET

// Replace with your home WiFi credentials
#define LOCAL_SSID "simpa_coko0"
#define LOCAL_PASS "147258369ssS0"

// Access Point credentials (used when not connected to home WiFi)
#define AP_SSID "WaterMonitor"
#define AP_PASS "123456789"

// Define LED pins for water level indication
#define DANGER_LED 26  // LED for HIGH water level alert
#define NORMAL_LED 27  // LED for NORMAL water level

// Define pins for the ultrasonic sensor
#define TRIG 5  // Trigger pin
#define ECHO 18 // Echo pin
#define NUM_SAMPLES 5 // Number of samples to average

#define CONTAINER_DEPTH 30 // Depth of container in cm

// SIM800L GSM module setup
HardwareSerial SIM800L(1); // Use UART1 (TX=GPIO17, RX=GPIO16)

// Timing variables
unsigned long previousMillis = 0;
const unsigned long interval = 20000; // Send SMS every 30 seconds
unsigned long lastReceivedTime = 0;

// Water level variables
float receivedWaterLevel = 0; // Water level received via WiFi
float measuredWaterLevel = 0; // Water level measured using ultrasonic sensor
bool dangerLedState = false;
bool normalLedState = false;

// Web server variables
IPAddress Actual_IP;
IPAddress PageIP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
WebServer server(80);

// XML buffer for web page updates
char XML[2048];
char buf[64];

// Function to measure water level using the ultrasonic sensor
float getMeasuredWaterLevel() {
    float total = 0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        digitalWrite(TRIG, LOW);
        delayMicroseconds(2);
        digitalWrite(TRIG, HIGH);
        delayMicroseconds(10);
        digitalWrite(TRIG, LOW);
        
        long duration = pulseIn(ECHO, HIGH, 30000); // 30ms timeout
        if (duration == 0) return 400; // If no signal detected, return large value
        
        float distance = duration * 0.034 / 2; // Calculate distance in cm
        total += distance;
        delay(50);
    }
    return total / NUM_SAMPLES;
}

// Function to send SMS via GSM module
void sendSMS(String message) {
    Serial.println("Sending SMS...");
    SIM800L.print("AT+CMGF=1\r");  
    delay(1000);
    SIM800L.println("AT+CMGS=\"+201064614155\"");  // Replace with your phone number
    delay(1000);
    SIM800L.println(message);
    delay(1000);
    SIM800L.println((char)26); // End SMS
    delay(1000);
    Serial.println("SMS Sent!");
}

// Function to update LED status based on water levels
void updateLEDs() {
    // Check both measured and received levels for safety
    bool dangerCondition = (measuredWaterLevel > 15) || (receivedWaterLevel > 15);
    
    if (dangerCondition) {
        digitalWrite(DANGER_LED, HIGH);
        digitalWrite(NORMAL_LED, LOW);
        dangerLedState = true;
        normalLedState = false;
        Serial.println("WARNING: Water level too high! DANGER LED ON.");
    } else {
        digitalWrite(DANGER_LED, LOW);
        digitalWrite(NORMAL_LED, HIGH);
        dangerLedState = false;
        normalLedState = true;
        Serial.println("Water level is normal. NORMAL LED ON.");
    }
}

// Web server function to send the main page
void SendWebsite() {
    Serial.println("Sending web page");
    server.send(200, "text/html", PAGE_MAIN);
}

// Web server function to send XML data for page updates
void SendXML() {
    Serial.println("Sending XML data");
    
    strcpy(XML, "<?xml version = '1.0'?>\n<Data>\n");

    // Send measured water level
    sprintf(buf, "<MEASURED_LEVEL>%.2f</MEASURED_LEVEL>\n", measuredWaterLevel);
    strcat(XML, buf);
    
    // Send received water level
    sprintf(buf, "<RECEIVED_LEVEL>%.2f</RECEIVED_LEVEL>\n", receivedWaterLevel);
    strcat(XML, buf);
    
    // Send received time (time since last WiFi data)
    unsigned long timeSinceReceived = (millis() - lastReceivedTime) / 1000;
    if (lastReceivedTime == 0) {
        strcat(XML, "<RECEIVED_TIME>No data</RECEIVED_TIME>\n");
    } else {
        sprintf(buf, "<RECEIVED_TIME>%lu sec ago</RECEIVED_TIME>\n", timeSinceReceived);
        strcat(XML, buf);
    }
    
    // Send LED states
    sprintf(buf, "<DANGER_LED>%d</DANGER_LED>\n", dangerLedState ? 1 : 0);
    strcat(XML, buf);
    
    sprintf(buf, "<NORMAL_LED>%d</NORMAL_LED>\n", normalLedState ? 1 : 0);
    strcat(XML, buf);
    
    // Send system status
    if ((measuredWaterLevel > 15) || (receivedWaterLevel > 15)) {
        strcat(XML, "<SYSTEM_STATUS>DANGER</SYSTEM_STATUS>\n");
    } else if ((measuredWaterLevel > 10) || (receivedWaterLevel > 10)) {
        strcat(XML, "<SYSTEM_STATUS>WARNING</SYSTEM_STATUS>\n");
    } else {
        strcat(XML, "<SYSTEM_STATUS>NORMAL</SYSTEM_STATUS>\n");
    }
    
    strcat(XML, "</Data>\n");
    
    // Send XML response
    server.send(200, "text/xml", XML);
}

// Function to handle water level data from sender ESP32
void ReceiveWaterLevel() {
    if (server.hasArg("level")) {
        receivedWaterLevel = server.arg("level").toFloat();
        lastReceivedTime = millis(); // Update last received time
        
        Serial.println("\n<<<<< Received Data via WiFi:");
        Serial.print("Received Water Level: ");
        Serial.print(receivedWaterLevel);
        Serial.println(" cm");
        Serial.println("<<<<<");
        
        server.send(200, "text/plain", "Data received successfully");
    } else {
        server.send(400, "text/plain", "Missing water level data");
    }
}

// Function to handle test SMS request from web page
void SendTestSMS() {
    String testMessage = "Test SMS from Water Monitor System\n";
    testMessage += "Measured: " + String(measuredWaterLevel) + " cm\n";
    testMessage += "Received: " + String(receivedWaterLevel) + " cm\n";
    testMessage += "System Status: OK";
    
    sendSMS(testMessage);
    server.send(200, "text/plain", "Test SMS sent successfully");
}

// Function to handle system reset request from web page
void ResetSystem() {
    Serial.println("System reset requested from web interface");
    // Reset variables
    receivedWaterLevel = 0;
    lastReceivedTime = 0;
    // Turn off LEDs
    digitalWrite(DANGER_LED, LOW);
    digitalWrite(NORMAL_LED, LOW);
    dangerLedState = false;
    normalLedState = false;
    
    server.send(200, "text/plain", "System reset completed");
}

// Function to print WiFi status
void printWifiStatus() {
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    long rssi = WiFi.RSSI();
    Serial.print("Signal strength (RSSI): ");
    Serial.print(rssi);
    Serial.println(" dBm");
    Serial.print("Open http://");
    Serial.println(WiFi.localIP());
}

void setup() {
    Serial.begin(115200);
    SIM800L.begin(9600, SERIAL_8N1, 16, 17); // TX=16, RX=17

    // Configure LED pins
    pinMode(DANGER_LED, OUTPUT);
    pinMode(NORMAL_LED, OUTPUT);
    digitalWrite(DANGER_LED, LOW);
    digitalWrite(NORMAL_LED, LOW);

    // Configure ultrasonic sensor pins
    pinMode(TRIG, OUTPUT);
    pinMode(ECHO, INPUT);

    Serial.println("Starting Water Monitor System...");

    // Set ESP32 to Wi-Fi station mode
    WiFi.mode(WIFI_STA);

    // Setup WiFi connection
#ifdef USE_INTRANET
    WiFi.begin(LOCAL_SSID, LOCAL_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.print("Connected to WiFi. IP address: ");
    Serial.println(WiFi.localIP());
    Actual_IP = WiFi.localIP();
#else
    WiFi.softAP(AP_SSID, AP_PASS);
    delay(100);
    WiFi.softAPConfig(PageIP, gateway, subnet);
    delay(100);
    Actual_IP = WiFi.softAPIP();
    Serial.print("Access Point created. IP address: ");
    Serial.println(Actual_IP);
#endif

    printWifiStatus();

    // Setup web server routes
    server.on("/", SendWebsite);                    // Main page
    server.on("/xml", SendXML);                     // XML data for updates
    server.on("/water_level", ReceiveWaterLevel);   // Receive water level from sender
    server.on("/SEND_SMS", SendTestSMS);            // Test SMS function
    server.on("/RESET_SYSTEM", ResetSystem);        // System reset function

    // Start web server
    server.begin();
    Serial.println("Web server started");
    Serial.println("Waiting for water level data from sender...");

    // Initialize SIM800L
    delay(1000);
    SIM800L.print("AT+CMGF=1\r");  // Set SMS mode to text
    delay(1000);
    SIM800L.print("AT+CNMI=2,2,0,0,0\r"); 
    delay(1000);
    SIM800L.println("AT+CMGD=1,4"); // Delete all SMS
    delay(5000);
    
    Serial.println("System Ready!");
}

void loop() {
    unsigned long currentMillis = millis();

    // Measure water level using ultrasonic sensor
    measuredWaterLevel = 21.65 - getMeasuredWaterLevel();

    Serial.print("Measured Water Level: ");
    Serial.print(measuredWaterLevel);
    Serial.println(" cm");

    // Update LED status based on water levels
    updateLEDs();

    // Handle web server clients
    server.handleClient();

    // Check if sender data is stale (no data for more than 1 minute)
    if (lastReceivedTime != 0 && (currentMillis - lastReceivedTime) > 60000) {
        Serial.println("WARNING: No data from sender for over 1 minute!");
        // You could set receivedWaterLevel to 0 or handle this as needed
    }

    // Send SMS periodically
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;

        String smsMessage = "Water Level Report:\n";
        smsMessage += "Measured: " + String(measuredWaterLevel) + " cm\n";
        smsMessage += "Received: " + String(receivedWaterLevel) + " cm\n";
        
        // Add status information
        if ((measuredWaterLevel > 15) || (receivedWaterLevel > 15)) {
            smsMessage += "STATUS: DANGER - High water level!";
        } else if ((measuredWaterLevel > 10) || (receivedWaterLevel > 10)) {
            smsMessage += "STATUS: WARNING - Water level elevated";
        } else {
            smsMessage += "STATUS: NORMAL";
        }

        sendSMS(smsMessage);
    }

    delay(2000); // Wait 2 seconds before next measurement
}