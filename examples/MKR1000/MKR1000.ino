/*
  This a simple example of the aREST Library for the Arduino/Genuino MKR1000 board.
  See the README file for more details.

  Written in 2016 by Marco Schwartz under a GPL license.
*/

// Import required libraries
#include <SPI.h>
#include <WiFi101.h>
#include <aREST.h>

// Status
int status = WL_IDLE_STATUS;

// Create aREST instance
aREST rest = aREST();

// WiFi parameters
char ssid[] = "your_wifi_network_name";
char password[] = "your_wifi_network_password";

// The port to listen for incoming TCP connections
#define LISTEN_PORT           80

// Create an instance of the server
WiFiServer server(LISTEN_PORT);

// Variables to be exposed to the API
int temperature;
int humidity;

// Declare functions to be exposed to the API
int ledControl(String command);
void aquariumController(aREST *arest, const String& name, const String& request_url);

void setup(void)
{
  // Start Serial
  Serial.begin(115200);

  // Init variables and expose them to REST API
  temperature = 24;
  humidity = 40;
  rest.variable("temperature",&temperature);
  rest.variable("humidity",&humidity);

  // Function to be exposed
  rest.function("led",ledControl);

  // API-Extension to be exposed
  rest.api_extension("aquarium", aquariumController);

  // Give name and ID to device (ID should be 6 characters long)
  rest.set_id("1");
  rest.set_name("mkr1000");

  // Connect to WiFi
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, password);

    // Wait 10 seconds for connection:
    delay(10000);
  }
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}

void loop() {

  // Handle REST calls
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  while(!client.available()){
    delay(1);
  }
  rest.handle(client);

}

// Custom function accessible by the API
int ledControl(String command) {

  // Get state from command
  int state = command.toInt();

  digitalWrite(6,state);
  return 1;

}

void aquariumController(aREST *arest, const String& name, const String& request_url) {
  // check format of request_url
  if (request_url == F("/aquarium")
      || request_url == F("/aquarium/")) {
    // Send feedback to client
    if (LIGHTWEIGHT) {
      bool isFirstSensor = true;
      auto count = 5;
      for (uint32_t i = 0; i < count; ++i) {
        if (isFirstSensor) {
          isFirstSensor = false;
        } else {
          arest->addToBufferF(F(","));
        }
        auto id = i + 100;
        arest->addToBuffer(id);
      }
    } else {
      arest->addToBufferF(F("\"sensor-ids\": ["));
      bool isFirstSensor = true;
      auto count = 5;
      for (uint32_t i = 0; i < count; ++i) {
        if (isFirstSensor) {
          isFirstSensor = false;
        } else {
          arest->addToBufferF(F(", "));
        }
        arest->addToBufferF(F("\""));
        auto id = i + 100;
        arest->addToBuffer(id);
        arest->addToBufferF(F("\""));
      }
      arest->addToBufferF(F("]"));
    }
  } else if (request_url.startsWith(F("/aquarium/water_limit/lower/set/"))) {
    String args = request_url.substring(32); // 32 = length of "/aquarium/water_limit/lower/set/"

    // Send feedback to client
    if (!LIGHTWEIGHT) {
      arest->addToBufferF(F("\"message\": \"lower water limit set to "));
      arest->addToBuffer(args);
      arest->addToBufferF(F("cm\""));
    }
  } else {
    arest->addToBufferF(F("\"message\": \"Unknown request_url '"));
    arest->addToBuffer(request_url);
    arest->addToBufferF(F("'.\""));
  }
}
