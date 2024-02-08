#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <String.h>
#include <WiFi.h>
#include "time.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


//global variables
unsigned long days = 0;
unsigned long hours = 0;
unsigned long minutes = 0;
unsigned long seconds = 0; 

const char* ssid     = "Dialog 4G";
const char* password = "4G4T5ELG98H";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 19800;
const int   daylightOffset_sec = 0;

void setup() {
  Serial.begin(115200);


  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(3000);
  display.clearDisplay();
  print_line("Welcome to MediBox",2,5,10);
  delay(3000);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
  display.clearDisplay();
  print_line("Connecting to WiFi",1,5,10);
  }
  display.clearDisplay();
  print_line("WiFi connected.", 1,5,10);

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
 
}

void loop() {
  display.clearDisplay();
  print_time_now();
delay(1000);  
}

 void print_line(String text, int text_size, int column, int row){
   
  display.setTextSize(text_size);
  display.setTextColor(WHITE);
  display.setCursor(column, row);
  display.println(text);
  display.display();

 }

 void print_time_now(){
   update_time();
   //print_line(String(days), 2, 0, 0);
   //print_line(":", 2,20,0);
   print_line(String(hours), 2, 30, 0);
   print_line(":", 2,50,0);
   print_line(String(minutes), 2, 60, 0);
   print_line(":", 2,80,0);
   print_line(String(seconds), 2, 90, 0);
 }

 void update_time(void){
   struct tm timeinfo;
   getLocalTime(&timeinfo);

   char day_str[8];
   char hour_str[8];
   char min_str[8];
   char sec_str[8];

   strftime(day_str,8, "%d", &timeinfo);
   strftime(hour_str,8, "%H", &timeinfo);
   strftime(min_str,8, "%M", &timeinfo);
   strftime(sec_str,8, "%S", &timeinfo);

   days = atoi(day_str);
   hours = atoi(hour_str);
   minutes = atoi(min_str);
   seconds = atoi(sec_str);

 }