#include <FS.h>          // this needs to be first, or it all crashes and burns...
#include <WiFiManager.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <Adafruit_GFX.h>
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <RFID.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_MLX90614 mlx = Adafruit_MLX90614();
/******************************MLX INIT ***************************/
WiFiClient client;
const int buzzer = D0;
double temp_amb;
double temp_obj;
double calibration = 2.36;
double total;
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

const char *ssid =  "claude";     // change according to your Network - cannot be longer than 32 characters!
const char *pass =  "1234512345"; // change according to your Network
unsigned long previousMillis = 0;
unsigned long interval = 2000;
StaticJsonDocument<192> doc;
RFID rfid(SS_PIN, RST_PIN); 


void setup() {
  Serial.begin(115200);    // Initialize serial communications
  Serial.println(F("Booting...."));
  SPI.begin(); 
  rfid.init();
  mlx.begin();         //Initialize MLX90614
  WiFiManager wifiManager;
  delay(1000);
  pinMode(buzzer,OUTPUT);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //initialize with the I2C addr 0x3C (128x64)
  int retries = 0;

 //exit after config instead of connecting
  wifiManager.setBreakAfterConfig(true);

  //reset settings - for testing
  //wifiManager.resetSettings();
  display.clearDisplay();
  display.setCursor(10,40);  
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println("BOOTING...");
  display.display();
  delay(2500);
  //tries to connect to last known settings
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP" with password "password"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("AutoConnectAP", "password")) {
    Serial.println("failed to connect, we should reset as see if it connects");
  display.clearDisplay();
  display.setCursor(10,0);  
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println("SET WIFI FIRST");
  display.display();
    delay(3000);
    ESP.restart();
    delay(5000);
  }
  Serial.println(F("WiFi connected"));
   display.clearDisplay();
  display.setCursor(0,0);  
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println("WIFI");
  display.setCursor(0,30);  
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println("CONNECTED");
  display.display();
  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  delay(2500);
  Serial.println(F("Ready!"));
  Serial.println(F("======================================================")); 
  Serial.println(F("Scan for Card and print UID:"));
  
  display.clearDisplay();
  display.setCursor(0,0);  
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println("SCAN YOUR CARD HERE");
  display.display();
  delay(2500);
}

void loop() {

  temp_amb = mlx.readAmbientTempC();
  temp_obj = mlx.readObjectTempC();
  total= temp_obj + calibration;
  Serial.print("Object temp = ");
  Serial.println(temp_obj + calibration);
  delay(1000);
  //digitalWrite(led1,HIGH);
   if (rfid.isCard()) {
        sendToserver();
     
    rfid.halt();
}
mlx1();
}
void sendToserver(){
  if (rfid.readCardSerial()) {
                Serial.println(" ");
                Serial.println("Card found");
                serNum0 = rfid.serNum[0];
                serNum1 = rfid.serNum[1];
                serNum2 = rfid.serNum[2];
                serNum3 = rfid.serNum[3];
                serNum4 = rfid.serNum[4];
               String Card = "";
                Serial.println(" ");
                Serial.println("Cardnumber:");
                for(int kk=0; kk<=4; kk++)
                  Card += rfid.serNum[kk];
                  Serial.println(Card);
                  //Serial.print("temp: ");
                  //Serial.println(temp);
                   
                    digitalWrite(buzzer,HIGH);
                    delay(200);
                    digitalWrite(buzzer,LOW);
                    delay(200);
                    
              //check if the wifi is connected now
              if(WiFi.status() == WL_CONNECTED){
               //send the web request now and check if the result is processed
                HTTPClient http;
                total= temp_obj + calibration;
                String temp=String(total);
                // configure traged server and url
                //http.begin("https://192.168.1.12/test.html", "7a 9c f4 db 40 d3 62 5a 6e 21 bc 5c cc 66 c8 3e a1 45 59 38"); //HTTPS
                http.begin("http://192.168.8.100:4005/card?rfid="+Card+"&temp="+temp); //HTTP
                Serial.print("[HTTP] GET...\n");
                // start connection and send HTTP head
                int httpCode = http.GET();
                // httpCode will be negative on error
                if(httpCode > 0) {
                    // HTTP header has been send and Server response header has been handled
                    //Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        
                    // file found at server
                    if(httpCode == HTTP_CODE_OK) {
                      String payload = http.getString();
                      Serial.println(payload);
 DeserializationError error = deserializeJson(doc, payload);

if (error) {
  Serial.print(F("deserializeJson() failed: "));
  Serial.println(error.f_str());
  return;
}

const char* firstname = doc["firstname"]; // "RUKUNDO"
const char* lastname = doc["lastname"]; // "Jean claude "
String B_status = doc["status"]; // "Normal"
float temp = doc["temp"]; // 26.95
Serial.println(firstname);
Serial.println(lastname);
Serial.println(B_status);
Serial.println(temp);

if(B_status == "Normal"){             
  display.clearDisplay();
  display.setCursor(10,0);
  display.setTextSize(1);
  display.print("FIRSTNAME:");
  display.print(firstname);
  //display.print((char)247);
  display.setCursor(10,15);
  display.setTextSize(1);
  display.print("LASTNAME:");
  display.print(lastname);
  display.setCursor(10,30);
  display.setTextSize(1);
  display.print("STATUS:");
  display.print(B_status);
  //display.print((char)247);
  display.setCursor(10,45);
  display.setTextSize(1);
  display.print("Body Temp: ");
  display.print(temp_obj + calibration);
  display.print((char)247);
  display.print("C");
  display.setCursor(10,55);
  display.setTextSize(1);
  display.print("PASS: ");
  display.print("ALLOWED");
  //display.print((char)247);
  display.display();
  delay(6000);
  display.clearDisplay();
  display.setCursor(10,15);  
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println("SCAN YOUR CARD");
  display.display();
  
              // open gate afterwards 
       
              
                        
                        }
                        else if(B_status == "Abnormal"){
                          
  display.clearDisplay();
  display.setCursor(10,0);
  display.setTextSize(1);
  display.print("FIRSTNAME:");
  display.print(firstname);
  //display.print((char)247);
  display.setCursor(10,15);
  display.setTextSize(1);
  display.print("LASTNAME:");
  display.print(lastname);
  display.setCursor(10,30);
  display.setTextSize(1);
  display.print("STATUS:");
  display.print(B_status);
  //display.print((char)247);
  display.setCursor(10,45);
  display.setTextSize(1);
  display.print("Body Temp: ");
  display.print(temp_obj + calibration);
  display.print((char)247);
  display.print("C");
  display.setCursor(10,55);
  display.setTextSize(1);
  display.print("PASS: ");
  display.print("DENIED");
  //display.print((char)247);
  display.display();
  delay(6000);
  display.clearDisplay();
  display.setCursor(10,15);  
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println("SCAN YOUR CARD");
  display.display();
  
    
                          }
                          else if (B_status == "Unregistered"){
                            
  display.clearDisplay();
  display.setCursor(10,0);
  display.setTextSize(1);
  display.print("STATUS: ");
  display.print(B_status);
   display.setCursor(10,20);
  display.setTextSize(1);
  display.print("Body Temp: ");
  display.print(temp);
  //display.print((char)247);
  display.setCursor(10,40);
  display.setTextSize(1);
  display.print("PASS: ");
  display.print("DENIED");
  //display.print((char)247);
  display.display();
  delay(6000);
  display.clearDisplay();
  display.setCursor(10,15);  
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println("SCAN YOUR CARD");
  display.display();
                  }                         

                             else if (B_status == "nobody"){
                            
   display.clearDisplay();
  display.setCursor(10,15);  
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println("NO BODY");
  display.setCursor(10,40);  
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println("DETECTED");
  display.display();
  delay(6000);
  display.clearDisplay();
  display.setCursor(10,15);  
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println("SCAN YOUR CARD");
  display.display();
                  }
                  else if (B_status == "nocard"){
                            
   display.clearDisplay();
  display.setCursor(10,15);  
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println("NO BODY");
  display.setCursor(10,40);  
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println("DETECTED");
  display.display();
  delay(6000);
  display.clearDisplay();
  display.setCursor(10,15);  
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println("SCAN YOUR CARD");
  display.display();
                  }
                     
                      }  
            } else {
                    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  display.clearDisplay();
  display.setCursor(25,15);  
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println("Error:500");
  display.setCursor(25,35);
  display.setTextSize(1);
  display.print("Server error");
  display.display();
    for (int i =0; i<3; i++){
                    digitalWrite(buzzer,HIGH);
                    delay(100);
                    digitalWrite(buzzer,LOW);
                    delay(100);
                    }                 
                }
              } else{
                  Serial.println("No WiFi Avaialable Now!!!!!");
  display.clearDisplay();
  display.setCursor(25,15);  
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println("NO WIFI ");
  display.setCursor(25,35);
  display.setTextSize(1);
  display.print("Available");
  display.display();
    for (int i =0; i<6; i++){
                    digitalWrite(buzzer,HIGH);
                    delay(100);
                    digitalWrite(buzzer,LOW);
                    delay(100);
                    }               
                }
              
             }/* else {
                If we have the same ID, just write a dot. 
               Serial.print(".");
             }*/
          }

  void mlx1(){
   if(total > 30 ){
  display.clearDisplay();
  display.setCursor(0,20);
  display.setTextSize(2);
  display.print("T:");
  display.print(temp_obj + calibration);
  display.print((char)247);
  display.print("C");
  display.display();
 
   delay(2000);
    }
    else if(total<30){
      //lcd stuff 
  display.clearDisplay();
  display.setCursor(10,0);
  display.setTextSize(1);
  display.print("SWAP A CARD");
  display.setCursor(20,30);
  display.setTextSize(2);
  display.print("T:");
  display.print(temp_obj + calibration);
  display.print((char)247);
  display.print("C");
  display.display();
      }
    
    }





