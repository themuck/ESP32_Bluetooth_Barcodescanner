/* in PlatformIO.ini add the line  

monitor_speed = 115200
board_build.partitions = huge_app.csv

based on https://github.com/espressif/arduino-esp32/blob/master/libraries/BluetoothSerial/examples/SerialToSerialBTM/SerialToSerialBTM.ino

Be sure taht your Barcodescanner can handle Bluetooth with seriel port profile (not HID mode)

i use NETUM NT-1228BC scanner
*/


#include "BluetoothSerial.h"
#include <WiFi.h>
#include <WiFiClient.h>

// Define the pin for the LED
#define LED 2

// Bluetooth setup
BluetoothSerial SerialBT;
String MACadd = "DC:0D:30:6E:F0:5D"; // MAC address for printing
uint8_t address[6]  = {0xDC, 0x0D, 0x30, 0x6E, 0xF0, 0x5D}; // MAC address of the slave BT device

String myName = "ESP32-BT-Master";

// WiFi setup
const char* ssid = "your WIFI";
const char* password = "your PW";

const char* host = "your TCP Server IP"; // IP address of your TCP Server
const int port = 12345; // Port used on your TCP Server

unsigned int counter = 0;
int incomingByte = 0;


// FIFO Buffer setup
const int bufferSize = 50;  // The size of the FIFO buffer
String fifoBuffer[bufferSize];  // The FIFO array
int head = 0;  // Pointer to the first element in the FIFO
int tail = 0;  // Pointer to the next free element in the FIFO
int count = 0;  // The number of elements in the FIFO

String receivedMessage = "";
String storedMessage = "";

WiFiClient client;

// Function to enqueue data into the FIFO buffer
void enqueue(String data) {
  if (count < bufferSize) {
    fifoBuffer[tail] = data;
    tail = (tail + 1) % bufferSize;
    count++;
  } else {
    // The FIFO buffer is full
    Serial.println("FIFO is full.");
  }
}

// Function to dequeue data from the FIFO buffer
String dequeue() {
  if (count > 0) {
    String data = fifoBuffer[head];
    head = (head + 1) % bufferSize;
    count--;
    return data;
  } else {
    // The FIFO buffer is empty
    Serial.println("FIFO is empty.");
    return "$FIFO#EMPTY";  // Error value to indicate that the FIFO buffer is empty
  }
}

// Bluetooth setup function
void BT_Setup(){
  SerialBT.begin(myName, true);
  Serial.print("Connecting to slave BT device with MAC "); Serial.println(MACadd);
  if(SerialBT.connect(address)) {
    Serial.println("Connected Successfully!");
  } else {
    Serial.println("Failed to connect. Make sure the remote device is available and in range.");
  }
}

// WiFi setup function
void WIFI_Setup(){
  // Connect to the WiFi
  WiFi.begin(ssid, password);
  WiFi.setAutoReconnect(true);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  Serial.println("TCP Server started");

}

void setup() {
  // Setup LED pin as an output
  pinMode(LED, OUTPUT);

  // Start serial communication
  Serial.begin(115200);

  // Setup Bluetooth and WiFi
  BT_Setup();
  WIFI_Setup();
}

void loop() {

  // Reconnect Bluetooth if disconnected
  if(!SerialBT.hasClient()){
    SerialBT.end();
    Serial.println("Bluetooth client lost, try to reconnect!");
    BT_Setup();
  }

  // Read incoming Bluetooth data
  while (SerialBT.available() > 0) {
    char receivedChar = SerialBT.read();

    if (receivedChar == '\r') {
      // End of input, process the message
      enqueue(receivedMessage);
      Serial.print("I received: ");
      Serial.println(receivedMessage);

      receivedMessage = ""; // Clear the message
    } else {
      // Add the character to the received message
      receivedMessage += receivedChar;
    }
  }

  // Send data from the FIFO buffer to the PC
  if (count > 0) {
    digitalWrite(LED, HIGH);

    if (client.connect(host, port)) {
      client.print(dequeue());
      Serial.print("Send data from slot: ");
      Serial.println(count);
      client.stop();
    }
  } else {
    digitalWrite(LED, LOW);
  }
}
