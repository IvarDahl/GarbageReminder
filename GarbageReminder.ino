/*
 * GarbageReminder v1.0
 * Written by Ivar Dahl (ivar.dahl@gmail.com)
 * 
 * Feel free to copy, tweak, share, sell or whatever you want to do. This is just a hobby, and if someone else likes it, that's great :)
 * This code is somewhat spaghetti. In the next version I will add a SD card reader/writer, to store the calendar (or maybe the garbage company has an API?). 
 * That makes it easier to maintain the calendar.
 * 
 * In this version: 
 * Garbage dates in this area are on a two week schedule. Every other garbage day, they pick up unsorted garbage. 
 * The other garabage date, they pick up unsorted AND sorted (plastic and paper).
 * 
 * The code calculates the next garbarge date (14 days ahead), when the user presses the button on the current garbage date. 
 * As garbage day in this area is Monday, the system first alerts on the preceding Thursday, just in case we plan to leave for the weekend.
 * When the user acknowledge the alert, the system waits until Sunday evening, and alerts again. When acknowledged on Sunday, the final alert activates
 * on Monday morning. When aknowledged on Monday, the system updates and displays the next garbage date. This date is stored in EEPROM.
 * The LED flashes once (repetetive) if it is time for unsorted only, and flashes three times (repetetive) if it's time for unsorted AND sorted garbage.
 * 
 * As some garbage dates don't follow the standard 14 day schedule (due to holidays etc), there are som hard coded exceptions in the code.
 * 
 * To set date and time from serial monitor command line:
 * ddmmyyyyhhmmsst [ENTER] (day month year hour minute seconds) t = instruct prog to set time
 * Eks: 01052019120000t = 01.05.19 12:00:00 (t = Set Time)
 * 
 * To set next alert date from serial monitor command line:
 * ddmmyyyyhhmm[1|0]n [ENTER](day month year hour minute [restavfall, mat|alle spann] n = instruct prog to set and save next alert time
 * Eks: 2005201918001 = Set next alert to 20.05.19 18:00 (Last digit 1 = Rastavfall + mat - If set to 0 -> Rastavfall, mat, papp, plast)
 * 
 */
#include <Wire.h>
#include "RTClib.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include <EEPROM.h>



  struct NextAlert {
  int Day;
  int Month;
  int Year;
  int Hour;
  int Minute;
  int Partial;
};
NextAlert nextAlertData;

#define LEDPIN 10
#define ALERT_ACK_PIN 7
#define OLED_RESET 4
#define DSTPIN 9
Adafruit_SH1106 display(OLED_RESET);


RTC_DS3231 rtc;
DateTime future;

//char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
//*************************************************************************
//  getNextAlert    getNextAlert    getNextAlert    getNextAlert    
//*************************************************************************

NextAlert GetEE(){
          int eeAddress = 0;
          //Get existing setting from EEProm
          EEPROM.get(eeAddress, nextAlertData);
          return nextAlertData;
          }

//*************************************************************************
//  END getNextAlert    END getNextAlert    END getNextAlert    END getNextAlert    
//*************************************************************************

//*************************************************************************
//  setNextAlert    setNextAlert   setNextAlert    setNextAlert    
//*************************************************************************

  void SetEE(int Day, int Month, int Year, int Hour, int Minute, int Partial){
          int eeAddress = 0;
          nextAlertData = {Day,Month,Year,Hour,Minute,Partial};
        
          //Put setting to EEProm
          EEPROM.put(eeAddress, nextAlertData);
          }
//*************************************************************************
//  END setNextAlert    END setNextAlert   END setNextAlert    END setNextAlert    
//*************************************************************************

//*************************************************************************
//  UPDATE CALENDAR   UPDATE CALENDAR   UPDATE CALENDAR   UPDATE CALENDAR   
//*************************************************************************
   void DisplayUpdateCalendarWarning(){
    display.setTextSize(2);
    display.setTextColor(WHITE);

    do{
    display.clearDisplay();
    display.display();
    digitalWrite(LEDPIN, HIGH);
    delay(1000);
    
    display.setCursor(12,18);
    display.println("OPPDATER");
    display.setCursor(12,40);
    display.println("KALENDER");
    display.display();
    digitalWrite(LEDPIN, LOW);
    delay(1000);

    
    if(Serial.available() > 0){
    String inString = "";
  
    while (Serial.available() > 0) {
      int inChar = Serial.read();
      inString += (char)inChar;
      if (inChar == '\n') {
        if(inString.substring(14,15) == "t"){
        rtc.adjust(DateTime(inString.substring(4,8).toInt(), inString.substring(2,4).toInt(), inString.substring(0,2).toInt(), inString.substring(8,10).toInt(), inString.substring(10,12).toInt(),inString.substring(12,14).toInt()));
        }
      }
    }
    }
    DateTime now = rtc.now();
    if(now.day() != 24){
      break;
    }
    }
    while (1 == 1);

   }
    
//*************************************************************************
//  END UPDATE CALENDAR   END UPDATE CALENDAR   END UPDATE CALENDAR   
//*************************************************************************

//*************************************************************************
//  SETUP     SETUP     SETUP     SETUP     SETUP     SETUP     SETUP     
//*************************************************************************

void setup () {
  // Init Display
  pinMode(LEDPIN, OUTPUT);
  pinMode(ALERT_ACK_PIN, INPUT_PULLUP);
  pinMode(DSTPIN, INPUT_PULLUP);
  digitalWrite(LEDPIN, LOW);
  
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(100);
  Serial.begin(9600);
  
  nextAlertData = GetEE();
  delay(1000);
 
    // calculate a date which is 7 days and 30 seconds into the future
    DateTime now = rtc.now();
    future = (now + TimeSpan(2,1,0,0));
}

//*************************************************************************
//  END SETUP       END SETUP       END SETUP       END SETUP       
//*************************************************************************

//*************************************************************************
//  LOOP      LOOP      LOOP      LOOP      LOOP      LOOP      LOOP      
//*************************************************************************
void loop () {
  String displayPrintString = "";
  String displayNextDateString = "";
  bool LED_Alert;
  static bool AlertAck;
  bool LED_ON;
  bool LED_STATE = false;
  static int AlertAckDay;
  static bool colon;
  static bool EEPROM_UPDATE_DONE;
  
// Get TimeData From Serial If Available
if(Serial.available() > 0){
  String inString = "";
  // Read serial input:
  while (Serial.available() > 0) {
    int inChar = Serial.read();
    //if (isDigit(inChar)) {
      // convert the incoming byte to a char and add it to the string:
      inString += (char)inChar;
    //}
    // if you get a newline, print the string, then the string's value:
    if (inChar == '\n') {
      if(inString.substring(13,14) == "n"){
        SetEE(inString.substring(0,2).toInt(), inString.substring(2,4).toInt(), inString.substring(4,8).toInt(), inString.substring(8,10).toInt(), inString.substring(10,12).toInt(), inString.substring(12,13).toInt());
        inString = "";
      }
      else if(inString.substring(14,15) == "t"){
         rtc.adjust(DateTime(inString.substring(4,8).toInt(), inString.substring(2,4).toInt(), inString.substring(0,2).toInt(), inString.substring(8,10).toInt(), inString.substring(10,12).toInt(),inString.substring(12,14).toInt()));
      }
// End Get TimeData From Serial
 

    }
  }
}

  
  
  
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(34,2);
    display.clearDisplay();
    DateTime now = rtc.now();
 
    if(now.day() < 10)displayPrintString += "0";
    displayPrintString += now.day();
    displayPrintString += ".";

    if(now.month() < 10) displayPrintString += "0";
    displayPrintString += now.month();
    displayPrintString += ".";
    
    displayPrintString += now.year();
    display.println(displayPrintString);
//    displayPrintString += " - ";
   
    
    displayPrintString = "";
    display.setCursor(35,11);
    display.setTextSize(2);
    if(now.hour() < 10) displayPrintString += "0";
    displayPrintString += now.hour();
    if(colon){
      displayPrintString += ":";
    }
    else{
      displayPrintString += " ";
    }
    colon = !colon;
    
    if(now.minute() < 10) displayPrintString += "0";
    displayPrintString += now.minute();
//    displayPrintString += ":";
    
//    if(now.second() < 10)displayPrintString += "0";
//    displayPrintString += now.second();
//    
    display.println(displayPrintString);

    if(nextAlertData.Day < 10) displayNextDateString += "0";
    displayNextDateString +=(nextAlertData.Day);
    displayNextDateString += ".";
    if(nextAlertData.Month < 10) displayNextDateString += "0";
    displayNextDateString += (nextAlertData.Month);
    displayNextDateString += ".";
    displayNextDateString += (nextAlertData.Year);
//    displayNextDateString += " - ";
//    if(nextAlertData.Hour < 10) displayNextDateString += "0";
//    displayNextDateString += (nextAlertData.Hour);
//    displayNextDateString += ":";
//    if(nextAlertData.Minute < 10) displayNextDateString += "0";
//    displayNextDateString += nextAlertData.Minute;// + ":");
      display.setTextSize(1);
      display.setCursor(25,30);
      display.println("Neste Bossdag");
      display.setTextSize(2);
      display.setCursor(4,40);
      display.println(displayNextDateString);
      display.setTextSize(1);
      display.setCursor(18,54);
      if(nextAlertData.Partial == 1){
        display.setCursor(24,56);
        display.println("Restavfall Mat");
      }
      else{
        display.setCursor(8,56);
        display.println("Rest Plast Papp Mat");
      }

      display.drawRect(24, 0, 79,27, WHITE);
      display.display();
    
  if((now.day() == nextAlertData.Day && now.hour() >= 6) || ((now.day() +4) == nextAlertData.Day && now.hour() >= 18) || ((now.day() +1) ==  nextAlertData.Day && now.hour() >= 18) && now.month() == nextAlertData.Month && now.year() == nextAlertData.Year){
    if (AlertAck == false){
         LED_Alert = true;
       }
      else{
        LED_Alert = false;
       }
  }
   else{
       LED_Alert = false;
      }
  if(LED_Alert == true){
    if(colon){
    digitalWrite(LEDPIN, HIGH);
    delay(50);
    digitalWrite(LEDPIN, LOW);
    if(nextAlertData.Partial == 0){
          delay(200);
          digitalWrite(LEDPIN, HIGH);
          delay(50);
          digitalWrite(LEDPIN, LOW);
          delay(200);
          digitalWrite(LEDPIN, HIGH);
          delay(50);
          digitalWrite(LEDPIN, LOW);
          }
    }
    }
    else{
      digitalWrite(LEDPIN, LOW);
      }
      if(digitalRead(ALERT_ACK_PIN) == LOW){
      AlertAck = true;
      AlertAckDay = now.day();
     }
     if(now.day() == nextAlertData.Day && AlertAck == true){
      DateTime future (now + TimeSpan(14,0,0,0));
        SetEE(future.day(), future.month(), future.year(),future.hour(),future.minute(),!nextAlertData.Partial);
     }
       if (now.day() == 24 && now.month() == 6 && now.year() == 2019){
        DisplayUpdateCalendarWarning();
      }
     if(now.day() == 27 && now.month() == 5 && now.year() == 2019 && AlertAck == true){
      if(!EEPROM_UPDATE_DONE){
          SetEE(6, 6, 2019,0,0,0);
          EEPROM_UPDATE_DONE = true;
      }
     }
     else if(now.day() == 6 && now.month() == 6 && now.year() == 2019 && AlertAck == true){
      if(!EEPROM_UPDATE_DONE){
        SetEE(24, 6, 2019,0,0,1);
        EEPROM_UPDATE_DONE = true;
      }
     }
     else{
        EEPROM_UPDATE_DONE = false;
     }
     
     if(AlertAck == true){

      if(AlertAckDay != now.day()){
        AlertAck = false;
        AlertAckDay = 0;
      }
      if(now.minute() == 59 && now.second() >= 55){
        AlertAck = false;
      }
     }
      

   

    delay(500);
    
}



//*************************************************************************
//  END LOOP    END LOOP    END LOOP    END LOOP    END LOOP    
//*************************************************************************
