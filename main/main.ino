#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHTesp.h>
#include <String.h>
#include <WiFi.h>
#include "time.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)

#define UP 34
#define DOWN 35
#define OK 32
#define CANCEL 33

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



unsigned long days = 0;
unsigned long hours = 0;
unsigned long minutes = 0;
unsigned long seconds = 0; 

const char* ssid     = "Dialog 4G";
const char* password = "4G4T5ELG98H";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 19800;
const int   daylightOffset_sec = 0;

bool alarm_enabled = false;
int n_alarms = 2;
int alarm_hours[] = {0,0};
int alarm_minutes[] = {1,10};
bool alarm_triggered[] = {false,false};

int current_mode = 0;
int max_modes = 4;
String options[] = {"1 - Set Time","2 - Set Alarm", "3 - Set Alarm 2" , "4 - Remove Alarm"};

void setup() {
  Serial.begin(115200);

  pinMode(BUZZER, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(UP, INPUT);
  pinMode(DOWN, INPUT);
  pinMode(OK, INPUT);
  pinMode(CANCEL, INPUT);

  dhtSensor.setup(DHT, DHTesp::DHT22);

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
  update_time_with_check_alarm();

  if(digitalRead(CANCEL) == LOW){
    Serial.print("Menu");
    delay(1000);
    go_to_menu();
  }
  check_temp();
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

 int wait_for_button_press(){
   while(true){
   if(digitalRead(UP) == LOW){
     delay(200);
     return UP;
   }
   else if(digitalRead(DOWN) == LOW){
     delay(200);
     return DOWN;
   }
   else if(digitalRead(CANCEL) == LOW){
     delay(200);
     return CANCEL;
   }
   else if(digitalRead(OK) == LOW){
     delay(200);
     return OK;
   }
   }
  update_time();   
   //print time();
 }

void go_to_menu(){
  while(digitalRead(CANCEL) == HIGH){
    display.clearDisplay();
    print_line(options[current_mode],2,0,0);

    int pressed = wait_for_button_press();

    if(pressed == UP){
      current_mode += 1;
      current_mode %= max_modes;
      delay(200);
    }

    else if(pressed == DOWN){
      current_mode -= 1;
      if(current_mode < 0){
        current_mode = max_modes -1;
      }
      delay(200);
    }
    else if(pressed == OK){
      Serial.println(current_mode);
      delay(200);
      run_mode(current_mode);
    }
  }
}

void run_mode(int mode){
  /*if(mode == 0){
    set_time();
  }*/
if(mode ==1 || mode == 2) set_alarm(mode-1);
else if(mode ==3) alarm_enabled = false;
}

void set_alarm(int alarm){

  int temp_hour = alarm_hours[alarm];
  while(true){
    display.clearDisplay();
    print_line("Enter hour: " +String(temp_hour), 0,0,2);

    int pressed = wait_for_button_press();
    if(pressed == UP){
      delay(200);
      temp_hour += 1;
      temp_hour %= 24;
    }
    else if (pressed = DOWN){
      delay(200);
      temp_hour -= 1;
      if(temp_hour < 0) temp_hour = 23;   
    }
    else if (pressed = OK){
      delay(200);
      alarm_hours[alarm] = temp_hour;
      break;   
    }
    else if(pressed == CANCEL){
      delay(200);
      break;
    }
  }
  
  int temp_minute = alarm_minutes[alarm];
  while(true){
    display.clearDisplay();
    print_line("Enter minute: " +String(temp_minute), 0,0,2);

    int pressed = wait_for_button_press();
    if(pressed == UP){
      delay(200);
      temp_minute += 1;
      temp_minute %= 60;
    }
    else if (pressed = DOWN){
      delay(200);
      temp_minute -= 1;
      if(temp_minute < 0) temp_hour = 59;   
    }
    else if (pressed = OK){
      delay(200);
      alarm_minutes[alarm] = temp_minute;
      break;   
    }
    else if(pressed == CANCEL){
      delay(200);
      break;
    }
  }
  display.clearDisplay();
  print_line("Alarm is set", 0,0,2);
  delay(1000);
}

void update_time_with_check_alarm(){
  //update time
  display.clearDisplay();
  update_time();
  print_time_now();

  //check for alarms
  if(alarm_enabled){
    //iterqting through all alarms
    for(int i=0; i< n_alarms; i++){
      if(alarm_triggered[i] == false && hours == alarm_hours[i] && minutes == alarm_minutes[i]){
        ring_alarm();//call the ringing function
        alarm_triggered[i] = true;

      }
    }
  }

}

void ring_alarm(){
//show msg on dispaly
display.clearDisplay();
print_line("medice Time", 2, 0, 0);

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

void check_temp(){
  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  bool all_good = true;
  if(data.temperature > 35){
    all_good = false;
    digitalWrite(LED_2,HIGH);
    print_line("Temp High", 1, 40,0);
  }
  else if(data.temperature < 25){
        all_good = false;
    digitalWrite(LED_2,HIGH);
    print_line("Temp Low", 1, 40,0);
  }
  if(data.humidity >85){
        all_good = false;
    digitalWrite(LED_2,HIGH);
    print_line("HUMD High", 1, 50,0);
  }
  else if (data.humidity < 35){
        all_good = false;
    digitalWrite(LED_2,HIGH);
    print_line("humd Low", 1, 50,0);
  }
  if(all_good){
digitalWrite(LED_2, LOW);    
  }
}