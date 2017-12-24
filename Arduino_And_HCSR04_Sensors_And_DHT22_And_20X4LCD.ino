/*
 HC-SR04 Ping distance_1 sensor]
 VCC to arduino 5v GND to arduino GND
 Echo to Arduino pin 13 Trig to Arduino pin 12
 Red POS to Arduino pin 11
 Green POS to Arduino pin 10
 560 ohm resistor to both LED NEG and GRD power rail
 More info at: http://goo.gl/kJ8Gl
 Original code improvements to the Ping sketch sourced from Trollmaker.com
 Some code and wiring inspired by http://en.wikiversity.org/wiki/User:Dstaub/robotcar
 */

 /* I2C Wiring for Arduino:
  *  https://www.arduino.cc/en/Reference/Wire
 */

 //http://www.json.org/
 //http://www.json.org/example.html

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <DHT.h>;

//Constants
#define DHTPIN 12     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino
#define trigPin_1 9
#define trigPin_2 2

//Variables
int chk;
float hum;  //Stores humidity value
float temp; //Stores temperature value

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

void setup() {
  Serial.begin (9600);
  dht.begin();
  lcd.init();
  lcd.backlight();
}

void loop() {
  long duration_1, distance_1;
  long duration_2, distance_2;
  String LCDDisplay;
  String JSONString;
  
  lcd.clear();
  //DHT Values
  hum = dht.readHumidity();
  temp= dht.readTemperature(true);
  lcd.setCursor(0,0);
  LCDDisplay = "Temp: ";
  LCDDisplay = LCDDisplay + temp;
  LCDDisplay = LCDDisplay + "f";
  lcd.print(LCDDisplay);
  lcd.setCursor(0,1);
  LCDDisplay = "Humidity: ";
  LCDDisplay = LCDDisplay + hum;
  lcd.print(LCDDisplay);
  JSONString = "{\"Sensor\":{\"Type\":\"Temperature\",\"ID\":\"Temp1\",\"MinValue\":-40,\"MaxValue\":176,\"CurValue\":";
  JSONString = JSONString + temp;
  JSONString = JSONString + ",\"Unit\":,\"Fahrenheit\"}}";
  Serial.println(JSONString);
  JSONString = "{\"Sensor\":{\"Type\":\"Humidity\",\"ID\":\"Humid1\",\"MinValue\":0,\"MaxValue\":100,\"CurValue\":";
  JSONString = JSONString + hum;
  JSONString = JSONString + ",\"Unit\":,\"Percent\"}}";
  Serial.println(JSONString);

  //Range Sensor 1 Values
  duration_1 = pingMilliseconds(trigPin_1);
  distance_1 = ((duration_1/2) / 29.1)/2.54;
  lcd.setCursor(0,2);
  //https://www.arduino.cc/en/Tutorial/StringAdditionOperator
  LCDDisplay = "Sensor 1 Inches: ";
  LCDDisplay = LCDDisplay + distance_1;
  lcd.print(LCDDisplay);
  JSONString = "{\"Sensor\":{\"Type\":\"Distance\",\"ID\":\"Range1\",\"MinValue\":0,\"MaxValue\":275,\"CurValue\":";
  JSONString = JSONString + distance_1;
  JSONString = JSONString + ",\"Unit\":,\"Inches\"}}";
  Serial.println(JSONString);

  //Range Sensor 2 Values
  duration_2 = pingMilliseconds(trigPin_2);
  distance_2 = ((duration_2/2) / 29.1)/2.54;
  lcd.setCursor(0,3);
  LCDDisplay = "Sensor 2 Inches: ";
  LCDDisplay = LCDDisplay + distance_2;
  lcd.print(LCDDisplay);
  JSONString = "{\"Sensor\":{\"Type\":\"Distance\",\"ID\":\"Range1\",\"MinValue\":0,\"MaxValue\":275,\"CurValue\":";
  JSONString = JSONString + distance_2;
  JSONString = JSONString + ",\"Unit\":,\"Inches\"}}";
  Serial.println(JSONString);
  
  JSONString = "{\"MCUHealth\":{\"ID\":\"FreeMemory\",\"CurValue\":";
  JSONString = JSONString + freeRam();
  JSONString = JSONString + ",\"Unit\":,\"kb?\"}}";
  Serial.println(JSONString);

  JSONString = "{\"MCUHealth\":{\"ID\":\"VCCmV\",\"CurValue\":";
  JSONString = JSONString + readVcc();
  JSONString = JSONString + ",\"Unit\":,\"mV\"}}";
  Serial.println(JSONString);
  
  delay(5000);
}

long pingMilliseconds(int sensorPin){
  pinMode(sensorPin, OUTPUT);
  digitalWrite(sensorPin, LOW);  // Added this line
  delayMicroseconds(2); // Added this line
  digitalWrite(sensorPin, HIGH);
  delayMicroseconds(10); // Added this line
  digitalWrite(sensorPin, LOW);
  pinMode(sensorPin, INPUT);
  return pulseIn(sensorPin, HIGH);
}

int freeRam () 
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

long readVcc() {
  //https://provideyourown.com/2012/secret-arduino-voltmeter-measure-battery-voltage/
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;

  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}

