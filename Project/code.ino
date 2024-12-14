#include <TimerOne.h>
#include <LiquidCrystal_I2C.h>

class CapacitiveLiqSensor
{
  public:
    CapacitiveLiqSensor(int outPin, int inPin);
    float CapacitorMeasure();               //capacitance in pF
    void CalibrateWaterLevel(float a0, float a1);
    void CalibrateInCapToGnd(float inCapToGnd); 
    float WaterLevelMeasure(); //Water Level in cm

  private:
    int _maxAdcValue = 1023;        //10 bit
    float _inCapToGnd;              // in pF      (AVR ATmega328P)
    int _outPin;                    
    int _inPin;
    float capacitance;
    float _a0;
    float _a1;
};

CapacitiveLiqSensor::CapacitiveLiqSensor(int outPin, int inPin)
{
    _outPin = outPin;
    _inPin = inPin;
    digitalWrite(_outPin, LOW);
    digitalWrite(_inPin, LOW);
    pinMode(_outPin, OUTPUT);
    pinMode(_inPin, OUTPUT);
}

void CapacitiveLiqSensor::CalibrateInCapToGnd(float inCapToGnd)
{
  _inCapToGnd = inCapToGnd;
}

float CapacitiveLiqSensor::CapacitorMeasure()
{
  float cap;

  pinMode(_inPin, INPUT);                 // Rising high edge on OUT_PIN
  digitalWrite(_outPin, HIGH);
  int val = analogRead(_inPin);
  digitalWrite(_outPin, LOW);
  pinMode(_inPin, OUTPUT);                // Clear everything for next measurement

  //Low value capacitor - calculate result
  cap = (float)val * _inCapToGnd / (float)(_maxAdcValue - val);
  capacitance = cap;
  return cap;
}
float CapacitiveLiqSensor::WaterLevelMeasure()
{
  float WaterLevel;
  float cap = CapacitorMeasure();
  WaterLevel = _a0*cap + _a1;

  return WaterLevel;
}
void CapacitiveLiqSensor::CalibrateWaterLevel(float a0, float a1)
{
  _a0 = a0;
  _a1 = a1;
}


LiquidCrystal_I2C lcd(0x27, 16, 2);         //Digital Display
CapacitiveLiqSensor sensor(7,A2);           //Define the sensor
float capacitance = 0;                      //Measured Capacitance 
float WaterLevel;                           //Measured Water Level
float Fs = 10;                               //Sampling Frequency
int flag = 0;                               //Data recieved ?

void setup() 
{ 
  //Setup LCD
  lcd.begin();
  lcd.backlight();

  //Timer interrupt for sampling data
  Timer1.initialize(1000000/Fs);
  Timer1.attachInterrupt(Sampling);

  //Setup the sensor
  sensor.CalibrateInCapToGnd(26.36);
  sensor.CalibrateWaterLevel(0.1450 , -5.09855);
}

void loop() 
{
  if(flag == 1)
  {
    flag = 0;
    lcd.clear();          //Update LCD
    lcd.setCursor(0, 0);
    lcd.print("H(mm):");
    lcd.print(WaterLevel* 10); //Water Level (in mm)
    lcd.setCursor(0, 1);
    lcd.print("C(pF):");
    lcd.print(capacitance); //Capacitance (in pF)
    delay(500);
  }
} 

void Sampling()
{
  //Calculate Capacitance
  capacitance = sensor.CapacitorMeasure();  
  //Calculate Water Level
  WaterLevel = sensor.WaterLevelMeasure();
  flag = 1;
}
