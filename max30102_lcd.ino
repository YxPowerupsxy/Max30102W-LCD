#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const uint8_t I2C_ADDRESS =0x27;
const uint8_t LCD_CHAR= 16;
const uint8_t LCD_ROW= 2;
LiquidCrystal_I2C lcd(I2C_ADDRESS, LCD_CHAR,LCD_ROW);

#include "MAX30105.h"

#include "heartRate.h"

MAX30105 particleSensor;

const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred

float beatsPerMinute;
int beatAvg;
int IR = 7;

void setup()
{
    Wire.begin();
  lcd.begin();
  pinMode(IR, INPUT);

  // Turn on the blacklight and print a message.
  lcd.backlight();

  lcd.print("Initializing...");

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    lcd.print("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  lcd.clear();
  lcd.print("Place Finger.");

  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
  
}

void getBP()
{
  
long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true)
  { 
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }

  Serial.print("IR=");
  Serial.print(irValue);
  Serial.print(", BPM=");
  Serial.print(beatsPerMinute);
  Serial.print(", Avg BPM=");
  Serial.print(beatAvg);

    lcd.clear();
    lcd.print("PlaceYourFinger");
    lcd.setCursor(0, 1);
    lcd.print("Avg BPM= ");
    clearCharacters(1, 9, LCD_CHAR - 1 );
    lcd.setCursor (9, 1);
    lcd.print(beatAvg);
    delay(300);

  if (irValue < 50000)
  {
    Serial.print(" No finger?");

  Serial.println();
}
}
  void clearCharacters(uint8_t row,uint8_t start, uint8_t stop )
{
    for (int i=start; i<=stop; i++)
    {
    lcd.setCursor (i,row); //  
    lcd.write(254);
    } 

}//clearCharacters
void loop()
{ 
     getBP();
}
