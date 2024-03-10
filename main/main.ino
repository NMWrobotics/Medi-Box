#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHTesp.h>
#include <String.h>
#include <WiFi.h>
#include "time.h"
#include "bitmap.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)

//button pins
#define UP 34 
#define DOWN 35
#define OK 32
#define CANCEL 33

//buzzer LEDs and DHT pins
#define BUZZER 18
#define LED_1 15
#define LED_2 2
#define DHT 12

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//declaration of DHT11
DHTesp dhtSensor;

//global variables
int n_notes = 8;
int C = 262;
int D = 294;
int E = 330;
int F = 349;
int G = 392;
int A = 440;
int B = 494;
int C_H = 523;
int notes[] = {C,D,E,F,G,A,B,C_H};


String weekDay;
unsigned long days = 0;
unsigned long hours = 0;
unsigned long minutes = 0;
unsigned long seconds = 0;
float GMT_offset = 5.5;

const char* ssid     = "Dialog 4G";
const char* password = "4G4T5ELG98H";

const char* ntpServer = "pool.ntp.org";
long  gmtOffset_sec = GMT_offset * 3600;
const int   daylightOffset_sec = 0;

bool alarm_enabled = false;
int n_alarms = 3;
int alarm_hours[] = {0,0,0};
int alarm_minutes[] = {1,10,0};
bool alarm_triggered[] = {false,false,false};

int DHT_temp;
int DHT_humidity;

int previous_mode = 4;
int current_mode = 0;
int next_mode = 1;
int max_modes = 5;
String options[] = {"1 - Set Time Zone","2 - Set Alarm", "3 - Set Alarm 2" ,"4 - Set Alarm 3", "5 - Remove Alarm"};


void setup() {
  //Serial.begin(115200);

  pinMode(BUZZER, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(UP, INPUT);
  pinMode(DOWN, INPUT);
  pinMode(OK, INPUT);
  pinMode(CANCEL, INPUT);

  dhtSensor.setup(DHT, DHTesp::DHT11);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  delay(2000);

  display.clearDisplay();
  display.drawBitmap(0, 0,  epd_bitmap_Intro_medi_box, 128, 64, 1);
  display.display();
  delay(3000);
  display.clearDisplay();

  WiFi.begin(ssid, password);
  
  display.drawBitmap(0, 0,  epd_bitmap_connecting_wifi, 128, 64, 1);
  display.display();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(2, 45);
  //print_line("Connecting",2,3,22);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    display.print(".");
    display.display();
  }
  display.clearDisplay();
  display.drawBitmap(0, 0,  epd_bitmap_wifi_connected, 128, 64, 1);
  display.display();

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
 
}

void loop() {
  update_time_with_check_alarm();

  if(digitalRead(CANCEL) == LOW){
    //Serial.print("Menu");
    delay(1000);
    go_to_menu();
  }
  //Serial.println(GMT_offset);
}

/*********************************functions***************************************/

//function to print a String in OLED
void print_line(String text, int text_size, int column, int row){
   
  display.setTextSize(text_size);
  display.setTextColor(WHITE);
  display.setCursor(column, row);
  display.println(text);
  display.display();

 }

//function to add String to OLED buffer
void print_line_without_display(String text, int text_size, int column, int row){

  display.setTextSize(text_size);
  display.setTextColor(WHITE);
  display.setCursor(column, row);
  display.println(text);
  //display.display();

 }

//function to print Home page and time
void print_time_now(){

  update_time();
  //print_line(String(days), 2, 0, 0);
  //print_line(":", 2,20,0);
  //  print_line(String(hours), 2, 30, 0);
  //  print_line(":", 2,50,0);
  //  print_line(String(minutes), 2, 60, 0);
  //  print_line(":", 2,80,0);
  //  print_line(String(seconds), 2, 90, 0);
  display.drawBitmap(0, 0,  epd_bitmap_main_page, 128, 64, 1);
  display.setCursor(5,26);
  display.setTextSize(3);

  if(hours < 10) display.print(0);
  
  display.print(hours);
  display.print(":");
  
  if(minutes < 10) display.print(0);
  
  display.print(minutes);
  display.setTextSize(2);
  display.setCursor(100,23);
  
  if(seconds < 10) display.print(0);
  
  display.print(seconds);
  print_line_without_display(weekDay, 1, 100, 42);

  print_line_without_display(String(DHT_temp), 1, 80,5);
  print_line_without_display(String(DHT_humidity), 1, 110,5);

  if(alarm_enabled){
    print_line_without_display("Alarms ON", 1,0,49);    
  }
  else{
    print_line_without_display("Alarms OFF", 1, 0,49);
  }

  check_temp();
  display.display();

}

//function to update time from NTP server
void update_time(void){

   struct tm timeinfo;
   getLocalTime(&timeinfo);

   char day_week_str[8];
   char day_str[8];
   char hour_str[8];
   char min_str[8];
   char sec_str[8];

    
   strftime(day_week_str,8, "%a", &timeinfo);
   strftime(day_str,8, "%d", &timeinfo);
   strftime(hour_str,8, "%H", &timeinfo);
   strftime(min_str,8, "%M", &timeinfo);
   strftime(sec_str,8, "%S", &timeinfo);

   weekDay = day_week_str;
   days = atoi(day_str);
   hours = atoi(hour_str);
   minutes = atoi(min_str);
   seconds = atoi(sec_str);

}

//function to take button inputs
int wait_for_button_press(){
  
  while(true){
    
    if(digitalRead(UP) == LOW){
      delay(200);
      //Serial.println("UP");
      return UP;
    }
    else if(digitalRead(DOWN) == LOW){
     delay(200);
     //Serial.println("DOWN");
     return DOWN;
    }
    else if(digitalRead(CANCEL) == LOW){
      delay(200);
      //Serial.println("CANCEL");
      return CANCEL;
    }
    else if(digitalRead(OK) == LOW){
      delay(200);
      //Serial.println("OK");
      return OK;
    }
    update_time();   
   }
   //print time();
 }

//function to print select menu items
void go_to_menu(){
  
  while(digitalRead(CANCEL) == HIGH){
    display.clearDisplay();
    //print_line(options[current_mode],2,0,0);
    print_menu(previous_mode,current_mode,next_mode);
    
    int pressed = wait_for_button_press();

    if(pressed == UP){
      previous_mode = current_mode;
      current_mode += 1;
      current_mode %= max_modes;
      next_mode = current_mode + 1;
      if(next_mode == 5) next_mode = 0;
      delay(200);
    }
    else if(pressed == DOWN){
      next_mode = current_mode;
      current_mode -= 1;
      
      if(current_mode < 0){
        current_mode = max_modes -1;
      }
      
      previous_mode = current_mode -1;
      if(previous_mode < 0) previous_mode = 4;
      delay(200);
    }
    else if(pressed == OK){
      //Serial.println(current_mode);
      delay(200);
      run_mode(current_mode);
    }
  }
}

//function to scroll menu items
void print_menu(int previous_mode, int current_mode, int next_mode){
  display.drawBitmap(0, 22,  epd_bitmap_select_box, 128, 20, 1);

  display.drawBitmap(0, 0,  menu_items[previous_mode], 128, 20, 1);
  display.drawBitmap(0, 22,    menu_items[current_mode], 128, 20, 1);
  display.drawBitmap(0, 44,  menu_items[next_mode], 128, 20, 1);

  display.display();
}

//fucntion to run Modes
void run_mode(int mode){
  if(mode == 0) set_time_zone();
  else if(mode == 1 || mode == 2 || mode == 3) set_alarm(mode-1);
  else if(mode == 4){ //disable alarms
    alarm_enabled = false;
    display.clearDisplay();
    display.drawBitmap(0, 0,  epd_bitmap_alarms_off, 128, 64, 1);
    display.display();
    delay(3000);
  }
}

//fucntion to set time zone
void set_time_zone(){
  
  float temp_GMT_offset = GMT_offset;
  
  while(true){
    display.clearDisplay();
    print_line_without_display("GMT: ",2,0,20);
      
    if(temp_GMT_offset >= 0){
      print_line("+",2,55,20);
      print_line(String(temp_GMT_offset),2,70,20);
    }
    else{
      print_line(String(temp_GMT_offset),2,55,20);
    }
      
    int pressed = wait_for_button_press();

    if(pressed == UP){
      delay(200);
      temp_GMT_offset += 0.5;
    }

    else if(pressed == DOWN){
      delay(200);
      temp_GMT_offset -= 0.5;
    }
    else if(pressed == OK){
      delay(200);
      GMT_offset = temp_GMT_offset;
      gmtOffset_sec = GMT_offset * 3600;
        
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      break;
    }
    else if (pressed == CANCEL){
      delay(200);
      break;
    }
  }
}

//function to set alarms
void set_alarm(int alarm){

  int temp_hour = alarm_hours[alarm];
  while(true){
    display.clearDisplay();
    print_line_without_display("Set hours", 2,10,15);
    print_line(String(temp_hour),2,60,40);

    int pressed = wait_for_button_press();
    
    if(pressed == UP){
      delay(200);
      temp_hour += 1;
      temp_hour %= 24;
    }
    else if (pressed == DOWN){
      delay(200);
      temp_hour -= 1;
      if(temp_hour < 0){
        temp_hour = 23;   
      }
    }
    else if (pressed == OK){
      delay(200);
      alarm_hours[alarm] = temp_hour;
      break;   
    }
    else if (pressed == CANCEL){
      delay(200);
      break;
    }
  }
  
  int temp_minute = alarm_minutes[alarm];
  while(true){
    display.clearDisplay();
    print_line_without_display("Set Minute", 2,5,15);
    print_line(String(temp_minute),2,60,40);

    int pressed = wait_for_button_press();
    if(pressed == UP){
      delay(200);
      temp_minute += 1;
      temp_minute %= 60;
    }
    else if (pressed == DOWN){
      delay(200);
      temp_minute -= 1;
      if(temp_minute < 0) temp_minute = 59;   
    }
    else if (pressed == OK){
      delay(200);
      alarm_minutes[alarm] = temp_minute;
      alarm_enabled = true;
      
      display.clearDisplay();
      display.drawBitmap(0, 0,  alarm_icons[alarm], 128, 64, 1);
      display.display();
      delay(3000);
      
      break;   
    }
    else if(pressed == CANCEL){
      delay(200);
      break;
    }
  }
}

//function to check alarms with time
void update_time_with_check_alarm(){
  //update time
  display.clearDisplay();
  update_time();
  print_time_now();

  //check for alarms
  if(alarm_enabled){
    //iterating through all alarms
    for(int i=0; i< n_alarms; i++){
      if(alarm_triggered[i] == false && hours == alarm_hours[i] && minutes == alarm_minutes[i]){
        ring_alarm();//call the ringing function
        alarm_triggered[i] = true;

      }
    }
  }
}

//function to play buzzer and turn led on
void ring_alarm(){
  //show msg on dispaly
  display.clearDisplay();
  display.drawBitmap(0, 0,  epd_bitmap_medicine_time, 128, 64, 1);
  print_line("Press Cancel to STOP",1,5,55);

  //light up LED1
  digitalWrite(LED_1,HIGH);

  //buzzer ring
  while(digitalRead(CANCEL) == HIGH){
    for(int i = 0; i < n_notes; i++){
      if(digitalRead(CANCEL) == LOW){
        delay(200);
        break;
      }
      tone(BUZZER, notes[i]);
      delay(500);
      noTone(BUZZER);
      delay(2);
    }
  }
  delay(200);
  digitalWrite(LED_1,LOW);
}

//function to check temp and humidity
void check_temp(){
  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  bool all_good = true;

  DHT_temp =  (int)data.temperature;
  DHT_humidity = (int)data.humidity;
  
  char buffer_temp[8];

  //Serial.println(data.temperature);
  //Serial.println(data.humidity);
  
  if(all_good){
    print_line_without_display("All Good", 1, 0,57);
  }
  if(data.temperature >32){
    all_good = false;
    digitalWrite(LED_2,HIGH);
    print_line_without_display("Temp High", 1, 0,57);
  }
  else if(data.temperature < 26){
    all_good = false;
    digitalWrite(LED_2,HIGH);
    print_line_without_display("Temp Low", 1, 0,57);
  }
  if(data.humidity >80){
    all_good = false;
    digitalWrite(LED_2,HIGH);
    print_line_without_display("HUMD High", 1, 0,57);
  }
  else if (data.humidity < 60){
    all_good = false;
    digitalWrite(LED_2,HIGH);
    print_line_without_display("Humd Low", 1, 0,57);
  }
  if(all_good){
    digitalWrite(LED_2, LOW);    
  }
}