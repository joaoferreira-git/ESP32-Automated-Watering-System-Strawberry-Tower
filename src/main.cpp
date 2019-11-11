#include <Arduino.h>

#include "SSD1306Wire.h"
#include "ThingSpeak.h"
#include "WiFi.h"
#include "secrets.h"  //lib of all the stored passwords
#include "DHT.h"
#include "Adafruit_Sensor.h"

const int relay = 16;  //Pin 16 of the ESP32 to control the relay
const int sensor = 25;  //Pin 25 of the ESP32 to power the moisture sensor
const int moisture = 36;  //Pin 36 of the ESP32 to receive the analog signal from the moisture sensor

int value; //siganl from the moisture sensor
int pump; //variable being used to send ON/OFF info to thingspeak

SSD1306Wire  display(0x3c, 5, 4); //OLED display on pin 5 and 4

WiFiClient  client;

#define DHTPIN 14 //here we use Pin 14 of ESP32 to read data
#define DHTTYPE DHT11 //our sensor is DHT11 type
DHT dht(DHTPIN, DHTTYPE);//create an instance of DHT sensor

void setup() {
    Serial.begin(115200); //starting serial for debuging
    delay(1000);

    Serial.println("Initializing OLED Display");
    display.init(); //Initializing OLED Display
    display.flipScreenVertically();
    display.setContrast(255);
    display.setFont(ArialMT_Plain_10);

    WiFi.mode(WIFI_STA);   
    ThingSpeak.begin(client);  // Initialize ThingSpeak

    dht.begin(); //call begin to start sensor

    pinMode(relay, OUTPUT);
    pinMode(moisture, INPUT);
    digitalWrite(relay, HIGH);
}

void loop() {

    if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(SECRET_SSID, SECRET_PASS);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected.");
    display.drawString(0, 0,"Connected to Wifi"); //Show connected to Wifi on the OLED
    display.display();
    delay(3000);
  }

    digitalWrite(sensor, HIGH); //Activate power to the moisture probe for limited time to avoid corrusion
    delay(500);

    int pump = 0;

    value = analogRead(moisture); //Read analog value
    Serial.print("analog: ");
    Serial.println(value);

    String analog = String(value);

    display.setColor(BLACK);  //Being used to refresh only part of the screen 
    display.fillRect(0, 40, 128, 10); 
    display.setColor(WHITE);

    display.drawString(0, 40, "Analog: ");
    display.drawString(40, 40, analog);
    display.display();

    value = constrain(value, 0, 2500);   //Keep the ranges!
    value = map(value, 0, 2500, 0, 100); //Map value : 900 will be 100% and 0 will be 0%
    delay(500);

    Serial.print("moisture level: ");
    Serial.println(value);

    String myString = String(value);

    display.setColor(BLACK);
    display.fillRect(0, 0, 120, 10);
    display.setColor(WHITE);

    display.drawString(0, 0, "Moisture Level: ");
    display.drawString(73, 0, myString);
    display.drawString(91, 0, "%");
    display.display();

    float h = dht.readHumidity();     //Read the humidity 

        String myhumidity = String(h);

        display.setColor(BLACK);
        display.fillRect(0, 12, 128, 10);
        display.setColor(WHITE);

        display.drawString(0, 9, "Humidity: ");
        display.drawString(45, 9, myhumidity);
        display.drawString(73, 9, "%");
        display.display();
       
    float t = dht.readTemperature();    // Read temperature as Celsius (the default)

        String mytemperature = String(t);

        display.setColor(BLACK);
        display.fillRect(0, 21, 128, 9);
        display.setColor(WHITE);

        display.drawString(0, 18, "Temperature: ");
        display.drawString(65, 18, mytemperature);
        display.drawString(93, 18, "ºC");
        display.display();

    delay(2000);

        if (value < 20)
        {
            pump = 1;

            digitalWrite(relay, LOW); //relay on to start the pump for 10 sec
            Serial.println("relay pin low");

            display.setColor(BLACK);
            display.fillRect(0, 29, 128, 9);
            display.setColor(WHITE);

            display.drawString(0, 27, "        Water Pump ON");
            display.display();
            delay(10000); //Change the delay based on much time you want the pump to be running (10 sec default)

            digitalWrite(relay, HIGH); //relay off to stop the pump
            Serial.println("relay pin high");

            display.setColor(BLACK);
            display.fillRect(0, 29, 128, 9);
            display.setColor(WHITE);

            display.drawString(0, 27, "        Water Pump OFF");
            display.display();
            delay(3000);
        }

    digitalWrite(sensor, LOW); //power to sensor off

    ThingSpeak.setField(1, value);
    ThingSpeak.setField(2, mytemperature);
    ThingSpeak.setField(3, myhumidity);
    ThingSpeak.setField(4, pump);
   Serial.println(pump);
   int x = ThingSpeak.writeFields(SECRET_CH_ID, SECRET_WRITE_APIKEY);  //Updating thingspeak channel
  if(x == 200){
    Serial.println("Channel update successful.");
    display.drawString(0, 50, "      Update sucessfull");
    display.display();
    delay(3000);

        display.setColor(BLACK);
        display.fillRect(0, 53, 128, 9);
        display.setColor(WHITE);
  }
  else{
    Serial.println("Problem updating channel." + String(x));
    display.drawString(0, 50, "      Error updating");
    display.display();
    delay(3000);

        display.setColor(BLACK);
        display.fillRect(0, 53, 128, 9);
        display.setColor(WHITE);
  }

    delay(20000);  //20 sec delay, fastest update rate for thingspeak is 15 sec
}