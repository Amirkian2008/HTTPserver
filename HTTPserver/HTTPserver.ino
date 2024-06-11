#include <WiFi.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
//#include <OneWire.h>
//#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <HTTPClient.h>
#define ADC_VREF_mV    3300.0 // in millivolt
#define ADC_RESOLUTION 4096.0
#define PIN_LM35       35 // ESP32 pin GPIO35(ADC0) connected to LM35

const char* ssid = "Nikan";
const char* password = "M158304h";

LiquidCrystal_I2C lcd(0x27, 16, 2);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);

/*const int oneWireBus = 4;
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);
*/
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
  //sensors.begin();
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
   int adcVal = analogRead(PIN_LM35);
  // convert the ADC value to voltage in millivolt
  float milliVolt = adcVal * (ADC_VREF_mV / ADC_RESOLUTION);
  // convert the voltage to the temperature in °C
  float tempC = milliVolt / 10;
  // convert the °C to °F
  float tempF = tempC * 9 / 5 + 32;
  //sensors.requestTemperatures();
 // float temperatureC = sensors.getTempCByIndex(0);
  DynamicJsonDocument jsonDoc(1024);
  jsonDoc["temperature"] = tempC;
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
  lcd.print(tempC);
  lcd.print(" C");
  lcd.setCursor(0, 1);
  lcd.print(iranTime);
  delay(1000);   
}
