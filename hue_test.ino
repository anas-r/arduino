#include <string.h>

#include <SPI.h>
#include <WiFiNINA.h>
#include <MQTT.h>
#include <ArduinoJson.h>

/* We need objects to handle WiFi connectivity
*/
WiFiClient wifi_client;
MQTTClient mqtt_client;

unsigned long lastMillis = 0;


/* In which pins is my buttons plugged? */
#define POTENTIOMETER_PIN A4 // TODO: uncomment this line if the potentiometer is connected to analogic pin 4, or adapt...
#define BUTTON_PIN 2

/* And associated variables to tell:
    1. which WiFi network to connect to
    2. what are the MQTT broket IP address and TCP port

*/

bool mqttFlag = true;
const char* wifi_ssid     = "lab-iot-1"; // TODO: change this
const char* wifi_password = "lab-iot-1"; // TODO: change this

const size_t capacity = 1000;

/* Some variables to handle measurements. */
int potentiometerValue;
int buttonState = 0;       // variable for reading the pushbutton status
int buttonStateOld = 0;

uint32_t t0, t;

/* Time between displays. */
#define DELTA_T 1000

String data_on = "{\"on\":true}";
String data_off = "{\"on\":false}";


void setup() {
  // monitoring via Serial is always nice when possible
  Serial.begin(9600) ;
  delay(100) ;
  Serial.println("Initializing...\n") ;
  /*reconnect();*/

  // initialize the Potentiometer and button pin as inputs:
  pinMode(POTENTIOMETER_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT);

  mqtt_client.begin("max.isasecret.com", 1723, wifi_client);
  mqtt_client.onMessage(messageReceived);

  // Time begins now!
  t0 = t = millis() ;

  connect();
}

void loop() {

  t = millis() ;
  if ( (t - t0) >= DELTA_T ) {
    t0 = t ;
    // read the states:
    buttonState = digitalRead(BUTTON_PIN);
    potentiometerValue = analogRead(POTENTIOMETER_PIN);
    /*
        Serial.print("potentiometer value: ");
        Serial.print(potentiometerValue);

        Serial.print(" ; button value: ");
        Serial.println(buttonState);
    */

  }

  float potValueF = potentiometerValue / 1020.0 * 255 ;
  int potValue = potValueF;


  if (WiFi.status() != WL_CONNECTED) {
    connect();
  }

  // Receiving an MQTT Message and sending a PUT request at the same doesn't work ...
  // State change
  /*if (buttonState == 1) {
    buttonStateOld = 0;
    }

    if (buttonStateOld != 1){
      buttonStateOld = 1;
      post(data_on);
    } else {
      post(data_off);
    }*/
  /*
     mqtt_client.loop();
     if (!mqtt_client.connected()) {
      connect();
     }*/
  // publish a message roughly every second
  /*if (millis() - lastMillis > 1000) {
    lastMillis = millis();
    mqtt_client.publish("/hello", "stuff");
    }*/

  //Sending the brightness parameter to the Hue
  post(brightness(potValue));
  delay(500);
}

void connect() {
  WiFi.begin(wifi_ssid, wifi_password) ;
  Serial.print("Connecting to ");
  Serial.print(wifi_ssid);
  Serial.print("\n") ;

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    delay(500);
  }

  Serial.print("\n");
  Serial.print("WiFi connected\n");
  Serial.print("IP address: \n");
  Serial.print(WiFi.localIP());
  Serial.print("\n") ;

  Serial.print("\nconnecting...");
  while (!mqtt_client.connect("", "majinfo2019", "Y@_oK2")) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nconnected to MQTT!");
  //for (int i=-5;i<12;i++){
  mqtt_client.subscribe("LightAHC\\-1\\status");
  //}

  // client.unsubscribe("/hello");
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
}

void post(String dataToSend) {
  //Serial.println("PUT connecting...");
  //Serial.println(dataToSend);
  if (wifi_client.connect("192.168.1.131", 80)) {
    Serial.println("PUT connected");
    wifi_client.println("PUT /api/SGHPyrkxUsiXS0zx8Xy9xsLipShqXjeqRZ2j22eh/lights/11/state HTTP/1.1");
    wifi_client.println("Host: 192.168.1.131");
    wifi_client.println("User-Agent: Arduino/1.0");
    wifi_client.println("Connection: close");
    wifi_client.println("Content-Type: application/json;");
    wifi_client.print("Content-Length: ");
    wifi_client.println(dataToSend.length());
    wifi_client.println();
    wifi_client.println(dataToSend);
  } else {
    Serial.println("connection failed");
  }
}


String collect() { //GET Requests function
  if (wifi_client.connect("192.168.1.131", 80)) {
    //Serial.println("COLLECT connected");
    wifi_client.println("GET /api/SGHPyrkxUsiXS0zx8Xy9xsLipShqXjeqRZ2j22eh/lights/11/ HTTP/1.1");
    wifi_client.println("Host: 192.168.1.131");
    wifi_client.println("Connection: close");
    wifi_client.println();
  }
  else {
    Serial.println("connection failed");
  }
  //Serial.println("[Response:]");
  while (wifi_client.connected() || wifi_client.available())
  {
    if (wifi_client.available())
    {
      String line = wifi_client.readStringUntil('\n');
      if (line.startsWith("{")) {
        return line;
      }
    }
  }
}

// A Formatting function
String brightness(int bri) {
  return "{\"bri\":" + String(bri) + "}";
}
