#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Replace with your network credentials
const char* ssid     = "******";
const char* password = "******";

// Define OLED specs
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// SDA PIN to 21
// SCL pin to22

// Enable buzzer connected to pin 32
#define Buzzer 32
// Enable builtin LED
#define LedBuiltIn 2

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
String IP;

// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;

// Store day and month names
const String Days[] = {"SUNDAY", "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY"}; 
const String Months[] = {"JANUARY", "FEBRUARY", "MARCH", "APRIL", "MAY", "JUNE", "JULY", "AUGUST", "SEPTEMBER", "OCTOBER", "NOVEMBER", "DECEMBER"};
int secondsIndex; // Stores seconds
int minutesIndex; // Stores minutes
int hoursIndex; // Stores hours
int dayIndex; // Stores day of week
int monthDay; // Stores day of month
int monthIndex; // Stores month
int year; // Stores year
// Variables to check passed time without using delay
long PassedTime;
// Initialize at non valid numbers so that the function updates their values on first call
int lastminute {61}; 
int lasthour {25};
int lastday;
int lastmonth;

// Functions
void Time(void);
int getMonth(); // returns month in integer form 1-12
int getMonthDay(); // returns day of the month in integer form 1 - 31
int getYear(); // returns year in integer form 2020+
void DefaultFrame(void);
// Draws line on x axis with animation at different heights 
// starting y position, Ending y position
void DrawLineAnimated(int j, int k);
//  Draws line on defined sSarting x, Starting y, Ending y positions
void DrawXLine (int i, int j, int k);

// General string display functions
// String to be displayed, X cursor, Y, cursor, Font size
void DisplayRawText(String s, int i, int j, int k);
void AutoTextDisplay(String s, int i, int j, int k);
// Buzz repeat times and duration in ms
void Buzz(unsigned int i, unsigned int j);

void setup() {
  pinMode(LedBuiltIn, OUTPUT);
  pinMode(Buzzer, OUTPUT);
  Buzz(1, 100);
  // Initialize Serial Monitor
  Serial.begin(115200);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.display();
  AutoTextDisplay("Connecting to: ", 0, 0, 1);
  AutoTextDisplay(ssid, 0, 8, 1);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  String ipaddress = WiFi.localIP().toString();
  // Print local IP address and start web server
  AutoTextDisplay("Wifi Connected!", 0, 16, 1);
  AutoTextDisplay("IP address: ", 0, 24, 1);
  AutoTextDisplay(ipaddress, 0, 32, 1);
  delay(1000);
  // Initialize a NTPClient to get time
  timeClient.begin();
  timeClient.setTimeOffset(7200);
  PassedTime = millis();
  Buzz(2, 100);
  DrawLineAnimated(9, 9);
}

void loop() {
  while(!timeClient.update()) { 
    timeClient.forceUpdate();
  }
  if (PassedTime <= millis() - 1000) { // update time once per second
    Time();
    display.clearDisplay();
    DefaultFrame();
  }
  if (WiFi.status() == WL_CONNECTED) DisplayRawText("WiFi", 0, 57, 1);
  display.display();
}

void DefaultFrame(void) {
  display.clearDisplay();
  DisplayRawText("NTP CLOCK", 38, 0, 1);
  DrawXLine(0, 9, 9);
  DisplayRawText(Days[dayIndex] + ", the ", 0, 12, 1);
  if (monthDay == 1) DisplayRawText(String(monthDay) + "st of " + Months[monthIndex-1] + " " + year, 0, 21, 1);
  else if (monthDay == 2) DisplayRawText(String(monthDay) + "nd of " + Months[monthIndex-1] + " " + year, 0, 21, 1);
  else if (monthDay == 3) DisplayRawText(String(monthDay) + "rd of " + Months[monthIndex-1] + " " + year, 0, 21, 1);
  else DisplayRawText(String(monthDay) + "th of " + Months[monthIndex-1] + " " + year, 0, 21, 1);
  DisplayRawText(timeStamp, 16, 33, 2);
  DrawXLine(0, 54, 54);
  //display.display();
}

void AutoTextDisplay(String s, int i, int j, int k) {
  DisplayRawText(s, i, j, k);
  display.display();
}

void DisplayRawText(String s, int i, int j, int k) {
  display.setTextSize(k); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(i, j);
  display.println(s);
}

void Buzz(unsigned int i, unsigned int j) {
  for (int k = 0; k <= i - 1; k++) {
    digitalWrite(Buzzer, HIGH);
    delay(j);
    digitalWrite(Buzzer, LOW);
    delay(j);
  }
}

void Time(void) {
  digitalWrite(LedBuiltIn, HIGH);
  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:13Z
  // We need to extract date and time
  formattedDate = timeClient.getFormattedDate();
  //secondsIndex = timeClient.getSeconds();
  minutesIndex = timeClient.getMinutes();
  if (lastminute != minutesIndex) {
    hoursIndex = timeClient.getHours();
    lastminute = minutesIndex;
    if (lasthour != hoursIndex) {
      dayIndex = timeClient.getDay();
      monthDay = getMonthDay();
      Buzz(2, 100);
      lasthour = hoursIndex;
      if (lastday != monthDay) {
        monthIndex = getMonth();
        lastday = monthDay;
        if (lastmonth != monthIndex) {
          year = getYear();
          lastmonth = monthIndex;
        }
      }
    }
  }
   
  // Extract date
  int splitT = formattedDate.indexOf("T");
  //dayStamp = formattedDate.substring(0, splitT);
  // Extract time
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
  PassedTime = millis();
  digitalWrite(LedBuiltIn, LOW);
}

int getMonthDay() {
  int monthday;
  time_t rawtime = timeClient.getEpochTime();
  struct tm * ti;
  ti = localtime (&rawtime);
  monthday = (ti->tm_mday) < 10 ? 0 + (ti->tm_mday) : (ti->tm_mday);
  return monthday;
}

int getMonth() {
  int month;
  time_t rawtime = timeClient.getEpochTime();
  struct tm * ti;
  ti = localtime (&rawtime);
  month = (ti->tm_mon + 1) < 10 ? 0 + (ti->tm_mon + 1) : (ti->tm_mon + 1);
  return month;
}

int getYear() {
  int year;
  time_t rawtime = timeClient.getEpochTime();
  struct tm * ti;
  ti = localtime (&rawtime);
  year = ti->tm_year + 1900;
  return year;
}

void DrawLineAnimated(int j, int k) {
  uint8_t i;
  display.clearDisplay(); 
  for(i = 0; i<display.width(); i+=2) {
    display.drawLine(0, j, i, k, SSD1306_WHITE);
    display.display(); // Update screen with each newly-drawn line
  }
}

void DrawXLine(int i, int j, int k){
  for(i; i<display.width(); i++) {
    display.drawLine(i, j, i, k, SSD1306_WHITE);
  }
}