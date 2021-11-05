/*
Many thanks to nikxha from the ESP8266 forum
*/

#include <ESP8266WiFi.h>
#include <SPI.h>
#include <RFID.h>

#include <ESP8266HTTPClient.h>

// Setup variables:
    int serNum0;
    int serNum1;
    int serNum2;
    int serNum3;
    int serNum4;

/* wiring the MFRC522 to ESP8266 (ESP-12)
RST     = GPIO4 /D2
SDA(SS) = GPIO2 /D4
MOSI    = GPIO13 /D7
MISO    = GPIO12 /D6
SCK     = GPIO14 /D5
GND     = GND
3.3V    = 3.3V
*/

#define RST_PIN  D3 // RST-PIN für RC522 - RFID - SPI - Modul GPIO4
#define SS_PIN  D4  // SDA-PIN für RC522 - RFID - SPI - Modul GPIO2 
#define SERIAL_NUMBER 12345 //to be managed for new devices to never

const char *ssid =  "DAMOUR";     // change according to your Network - cannot be longer than 32 characters!
const char *pass =  "1234567890"; // change according to your Network
unsigned long previousMillis = 0;
unsigned long interval = 2000;

RFID rfid(SS_PIN, RST_PIN); 


void setup() {
  //initiate serial communication at 9600 bps
  Serial.begin(9600);    // Initialize serial communications
  Serial.println(F("Booting...."));
 SPI.begin(); 
 rfid.init();
  
  WiFi.begin(ssid, pass);
  
  int retries = 0;
  while ((WiFi.status() != WL_CONNECTED) && (retries < 10)) {
    retries++;
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(F("WiFi connected"));
  }
  
  Serial.println(F("Ready!"));
  Serial.println(F("======================================================")); 
  Serial.println(F("Scan for Card and print UID:"));
}

void loop() {
  //digitalWrite(led1,HIGH);
   if (rfid.isCard()) {
        if (rfid.readCardSerial()) {
                Serial.println(" ");
                Serial.println("Card found");
                serNum0 = rfid.serNum[0];
                serNum1 = rfid.serNum[1];
                serNum2 = rfid.serNum[2];
                serNum3 = rfid.serNum[3];
                serNum4 = rfid.serNum[4];
               String CARD_N0 = "";
                Serial.println(" ");
                Serial.println("Cardnumber:");
                for(int kk=0; kk<=4; kk++)
                  CARD_N0 += rfid.serNum[kk];
                  Serial.println(CARD_N0);
                 
                  delay(1000);
              //check if the wifi is connected now
              if(WiFi.status() == WL_CONNECTED){
               //send the web request now and check if the result is processed
                HTTPClient http;
                WiFiClient wifiClient;
                // configure traged server and url
                //http.begin("https://192.168.1.12/test.html", "7a 9c f4 db 40 d3 62 5a 6e 21 bc 5c cc 66 c8 3e a1 45 59 38"); //HTTPS
                http.begin(wifiClient,"http://192.168.43.191:4005/card?rfid=" + CARD_N0 + "&sn=" + SERIAL_NUMBER); //HTTP
                Serial.print("[HTTP] GET...\n");
                // start connection and send HTTP header:
                int httpCode = http.GET();
                // httpCode will be negative on error
                if(httpCode > 0) {
                    // HTTP header has been send and Server response header has been handled
                    //Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        
                    // file found at server
                    if(httpCode == HTTP_CODE_OK) {
                      String payload = http.getString();
                      Serial.println(payload);
                      
                      }  
            } else {
                    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
                    
                }
              } else{
                  Serial.println("No WiFi Avaialable Now!!!!!");
                  
                  
                }
              
             }/* else {
                If we have the same ID, just write a dot. 
               Serial.print(".");
             }*/
          }
    
    rfid.halt();
}

