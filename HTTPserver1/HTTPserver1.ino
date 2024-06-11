#include <WiFi.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <HTTPClient.h>

const char* ssid = "YOUR SSID";
const char* password = "YOUR PASSWORD";

LiquidCrystal_I2C lcd(0x27, 16, 2);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);

const int oneWireBus = 4;
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(115200);

 
  lcd.init();
  lcd.clear();
  lcd.backlight(); 
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to Wi-Fi");

  timeClient.begin();
  sensors.begin();
  }
void loop() {
  timeClient.update();

  String formattedTime = timeClient.getFormattedTime(); 
  int offsetHours = 3; // Offset for Iran time zone
  int offsetMinutes = 30; // Offset for Iran time zone (considering daylight saving time)

// Extract current hour, minute, and second
  int currentHour = timeClient.getHours(); 
  int currentMinute = timeClient.getMinutes();
  int currentSecond = timeClient.getSeconds();

// Calculate Iran local time
  int hour = (currentHour + 4) % 24;
  int minute = (currentMinute + offsetMinutes) % 60;
  int second = currentSecond;

// Handle edge case where time exceeds 24 hours
  hour = (hour < 0) ? hour + 24 : hour;

// Format Iran local time with seconds
  String iranTime =  String(hour) + ":" + String(minute) + ":" + String(second);
  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);
  DynamicJsonDocument jsonDoc(1024);
  jsonDoc["temperature"] = temperatureC;
  jsonDoc["time"] = iranTime;
  String jsonString;
  serializeJson(jsonDoc, jsonString);

  if(WiFi.status() == WL_CONNECTED){
    HTTPClient http;
    http.begin("http://192.168.1.3:5000/data");
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(jsonString);

  if (httpResponseCode > 0){
    String response = http.getString();
    Serial.println(httpResponseCode);
    Serial.println(response);
  } else{
    Serial.print("Error: ");
    Serial.print(httpResponseCode);
  }
  http.end();
}  
  lcd.setCursor(0, 0);
  lcd.print("TEMP: ");
  lcd.print(temperatureC);
  lcd.print(" C");
  lcd.setCursor(0, 1);
  lcd.print(iranTime);
  delay(1000);   
}
