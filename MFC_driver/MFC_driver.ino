
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <avr/pgmspace.h>
#include "Arduino.h"
#include "Wire.h"
#define I2Caddress 0x48
Adafruit_SSD1306 display(128, 64, &Wire, -1);

#include <PID_v1.h>
double Setpoint, Input, Output;
//double aggKp=25, aggKi=40, aggKd=5; //aggressive values      <--------Rough tuning after addition of better vacuum plumbing
double aggKp=25, aggKi=50, aggKd=5; //aggressive values   <--------These were the original values 
double consKp=23, consKi=13, consKd=12.8; //conservative values

PID myPID(&Input, &Output, &Setpoint, aggKp, aggKi, aggKd, P_ON_M, DIRECT);

#include <Keypad.h>
const byte ROWS = 4; 
const byte COLS = 4; 
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6}; 
byte colPins[COLS] = {5, 4, 3, 2}; 
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

unsigned int readings[20] = {0};
unsigned char readCnt = 0;
float RawVoltage;
float AveragedVoltage;
float PiraniV;
float ObsmTorr;

byte Menu0=0;
byte Menu1=0;
byte Menu2=0;
byte Menu3=0;

byte count=0;
int digit[]={0,0,0,0,0,0};
int set[]={0,0,0,0,0,0};
double SETTING=0;
double SettingV=0;

int Digit;
int Set;
byte L=0;
byte GasType = 0;

const byte MfcValvePin = 11;
float MfcOutV;





void setup() {
Serial.begin(9600);
if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {}
  display.setTextColor(WHITE);
  display.setTextSize(1);

Wire.begin();

pinMode(MfcValvePin, OUTPUT);
Input = AveragedVoltage;
Setpoint = 0;
//myPID.SetSampleTime(20);
//myPID.SetSampleTime(40);
myPID.SetMode(AUTOMATIC);

}



void loop() {
  
display.clearDisplay();
  display.setCursor(0,0);
  display.print(F("Welcome, Ben"));
  display.setCursor(0,16);
  display.println(F("A - SET"));
  display.println(F("B - Set/Meas (mTorr)"));
  display.println(F("C - In/Out (Volt)"));
  display.print(F("D - Gas Type Setting"));
display.display();

char customKey = customKeypad.getKey();

Adc();
Controller();

//switches between menus
if (customKey == 'A'){Menu0=1; customKey = 0;}
if (customKey == 'B'){Menu1=1; customKey = 0;}
if (customKey == 'C'){Menu2=1; customKey = 0;}
if (customKey == 'D'){Menu3=1; customKey = 0;}


/////////////////////////////-----SET SCREEN------//////////////////////////////////////
  while (Menu0==1) {
    display.clearDisplay();
      display.setCursor(0, 0);
      display.println(F("Enter setting (mTorr)"));
      display.setCursor(0,55);
      display.println(F("#=Enter  *=Clear"));
     
  //calls for input from keypad key
  char customKey = customKeypad.getKey();

    //switches between menus
    if (customKey == 'A'){Menu0=0; Menu1=0; Menu2=0; Menu3=0;}
    if (customKey == 'B'){Menu0=0; Menu1=1; Menu2=0; Menu3=0;}
    if (customKey == 'C'){Menu0=0; Menu1=0; Menu2=1; Menu3=0;}
    if (customKey == 'D'){Menu0=0; Menu1=0; Menu2=0; Menu3=1;}

    Adc();
    Controller();
  //  if (SETTING!=0) {Controller();}

    //Inputs setting values from keypad
    switch(count) {
      case 0: 
        if (customKey=='*'){count=0; break;}
        digit[0] = customKey; 
        display.drawLine(0, 28, 3, 28, WHITE);
        if (digit[0] != 0){count=1; customKey=0; break;}
        break;
      case 1:
        digit[0]=0;
        if (customKey=='*'){count=0; break;}
        digit[1] = customKey; 
        display.drawLine(6, 28, 9, 28, WHITE);
        if (digit[1] != 0){count=2; customKey=0; break;}
        break;
      case 2:
        digit[1]=0;
        if (customKey=='*'){count=0; break;}
        digit[2] = customKey;
        display.drawLine(12, 28, 15, 28, WHITE);
        if (digit[2] != 0){count=3; customKey=0; break;} 
        break;
      case 3:
        digit[2]=0;
        if (customKey=='*'){count=0; break;}
        digit[3] = customKey;
        display.drawLine(18, 28, 21, 28, WHITE);
        if (digit[3] != 0){count=4; customKey=0; break;}
        break;
      case 4:
        digit[3]=0;
        if (customKey=='*'){count=0; break;}
        digit[4] = customKey;
        display.drawLine(24, 28, 27, 28, WHITE);
        if (digit[4] != 0){count=5; customKey=0; break;}
        break;
      case 5:
        digit[4]=0;
        if (customKey=='*'){count=0; break;}
        digit[5] = customKey;
        display.drawLine(30, 28, 33, 28, WHITE);
        if (digit[5] != 0){count=6; customKey=0; break;}
        break;
      }

    //clears setting  
    if (customKey=='*'){count=0;
      if(count==0){for (byte j=0; j<7; j++)
                    {digit[j]=0; set[j]=0;}}}
                    
    //converts char digit value to numerical value
    for (byte i=0; i<6; i++){                
      if (byte(digit[i])==48){set[i]=0;}
      if (byte(digit[i])==49){set[i]=1;}
      if (byte(digit[i])==50){set[i]=2;}
      if (byte(digit[i])==51){set[i]=3;}
      if (byte(digit[i])==52){set[i]=4;}
      if (byte(digit[i])==53){set[i]=5;}
      if (byte(digit[i])==54){set[i]=6;}
      if (byte(digit[i])==55){set[i]=7;}
      if (byte(digit[i])==56){set[i]=8;}
      if (byte(digit[i])==57){set[i]=9;}
    }
 
    //displays set values
    display.setCursor(0,20);
    for (byte j=0; j<6; j++){
      display.print(set[j]);}

    //Uses sum of each digit position to calculate final Setting
    if (customKey=='#'){ 
      SETTING=
        (set[0]*100000)+
        (set[1]*10000)+
        (set[2]*1000)+
        (set[3]*100)+
        (set[4]*10)+
        (set[5]);
      display.clearDisplay();
      display.setCursor(0,20);
      display.println(F("Your setting is:"));
      display.println(SETTING, 0);
      display.display();
      delay(1500);}


    display.setCursor(0,37);
    display.print(F("Gas: "));
    if (GasType == 0){display.print(F("Air"));}
    if (GasType == 1){display.print(F("Deuterium"));}
    if (GasType == 2){display.print(F("Argon"));}
    if (GasType == 3){display.print(F("Helium"));}
    
    display.display();

  }
    
  

////////////////////////////-----Obs/Meas (mTorr) screen--------/////////////////////////////
  while (Menu1 == 1) {

    display.clearDisplay();

    char customKey = customKeypad.getKey();
    
    if (customKey == 'A'){Menu0=1; Menu1=0; Menu2=0; Menu3=0;}
    if (customKey == 'B'){Menu0=0; Menu1=0; Menu2=0; Menu3=0;}
    if (customKey == 'C'){Menu0=0; Menu1=0; Menu2=1; Menu3=0;}
    if (customKey == 'D'){Menu0=0; Menu1=0; Menu2=0; Menu3=1;}

    Adc();
    Controller();
    //if (SETTING!=0) {Controller();}
     
    display.setCursor(0,0);
      display.print(F("(mTorr)"));  
    display.setCursor(0, 16);
      display.print(F("Set: "));
      display.println(SETTING, 0);
      display.setCursor(0, 28);
      display.println(F("Observed:"));
      display.setCursor(0, 40);
       display.setTextSize(3);
        if (ObsmTorr>=100){
      display.println(ObsmTorr, 0);}
        else if (ObsmTorr<100 && ObsmTorr>=1){
      display.println(ObsmTorr, 3);}
        else if (ObsmTorr<1){
      display.println(ObsmTorr, 6);}
      display.setTextSize(1);

    display.display();
   
    }


///////////////////////////------In/Out (Volt) screen-------//////////////////////////////
  while (Menu2>0) {
    
    display.clearDisplay();

    char customKey = customKeypad.getKey();
    
    if (customKey == 'A'){Menu0=1; Menu1=0; Menu2=0; Menu3=0;}
    if (customKey == 'B'){Menu0=0; Menu1=1; Menu2=0; Menu3=0;}
    if (customKey == 'C'){Menu0=0; Menu1=0; Menu2=0; Menu3=0;}
    if (customKey == 'D'){Menu0=0; Menu1=0; Menu2=0; Menu3=1;}

    Adc();
    Controller();
    //if (SETTING!=0) {Controller();}

    display.setCursor(0,0);
      display.println(F("(Volt)"));
    display.setCursor(0, 18);
      display.println(F("Pirani Input:"));
      display.println(PiraniV, 4);
      display.println(F("Output to MFC:"));
      display.print(MfcOutV, 4);

    display.display();

  }

///////////////////////////------Gas Setting Screen--------//////////////////////////////
  while (Menu3>0) {
    display.clearDisplay();
      display.setCursor(0, 0);
      display.println(F("Gas type setting:"));
      display.setCursor(0, 35);
      display.print(F("Air/N2--0"));
      display.print(F("   "));
      display.println(F("Ar---2"));
      display.print(F("D2------1"));
      display.print(F("   "));
      display.print(F("He---3"));
     
  //calls for input from keypad key
  char customKey = customKeypad.getKey();

    //switches between menus
    if (customKey == 'A'){Menu0=0; Menu1=0; Menu2=0; Menu3=0;}
    if (customKey == 'B'){Menu0=0; Menu1=1; Menu2=0; Menu3=0;}
    if (customKey == 'C'){Menu0=0; Menu1=0; Menu2=1; Menu3=0;}
    if (customKey == 'D'){Menu0=0; Menu1=0; Menu2=0; Menu3=1;}

    Adc();
    Controller();

    Digit = customKey;
         
    //converts char digit value to numerical value    
                   
      if (byte(Digit)==48){Set=0;}
      if (byte(Digit)==49){Set=1;}
      if (byte(Digit)==50){Set=2;}
      if (byte(Digit)==51){Set=3;}
      if (byte(Digit)==52){Set=4;}
      if (byte(Digit)==53){Set=5;}
      if (byte(Digit)==54){Set=6;}
      if (byte(Digit)==55){Set=7;}
      if (byte(Digit)==56){Set=8;}
      if (byte(Digit)==57){Set=9;}
       
    //displays set values
    display.setCursor(0,20);
      display.print(Set);
    display.drawLine(0, 28, 3, 28, WHITE);

    if (customKey=='#'){GasType = Set;}
    
    display.setCursor(50,20);
      display.print(F("Gas:"));    
        if (GasType == 0){display.print(F("Air"));}
        if (GasType == 1){display.print(F("Deuterium"));}
        if (GasType == 2){display.print(F("Argon"));}
        if (GasType == 3){display.print(F("Helium"));}
        if (GasType > 3){display.print(F("u wrong"));}
        
    display.setCursor(0,57);
      display.print(F("#=Enter"));
     
    display.display();

  }

}


////////////////////////////////////Controller////////////////////////////////
void Adc(){
  
///////////// Pirani Calculations//////////////////////////
Wire.beginTransmission(I2Caddress);
Wire.write(0b00000001);
Wire.write(0b01000000);
Wire.write(0b10000010);
Wire.endTransmission();
Wire.beginTransmission(I2Caddress);
Wire.write(0b00000000);
Wire.endTransmission();
Wire.requestFrom(I2Caddress, 2);
uint16_t convertedValue;
convertedValue = (Wire.read() << 8 | Wire.read());
readings[readCnt] = convertedValue;
  readCnt = readCnt == 9 ? 0 : readCnt + 1;         //readCnt == 19 (previous value)
unsigned long totalReadings = 0;
  for (unsigned char cnt = 0; cnt < 10; cnt++){      //cnt < 20 (previous value)
    totalReadings += readings[cnt];}
RawVoltage = ((5./26761.) * convertedValue);       
AveragedVoltage = (5./26761.) * (totalReadings / 10.);  //totalReadings / 20. (previous value)

//Each graduation is 0.0001868 volts resolution

PiraniV = 2 * AveragedVoltage;
//multiplied by 2 because Pirani gauge 1.9-9.1v output, ADC can only tolerate 5.5v, using a 2:1 voltage divider
//Also change 27180 to max voltage from voltage divider using (convertedValue) variable once Pirani gauge set up 

//P = (10^(V-6.125))*1000
ObsmTorr = (pow(10,(PiraniV - 6.125))*1000); //Gas is air
  if (GasType == 1){ObsmTorr = (ObsmTorr * 0.78);} //Gas is deuterium
  if (GasType == 2){ObsmTorr = (ObsmTorr * 1.7);} //Gas is Argon
  if (GasType == 3){ObsmTorr = (ObsmTorr * 1.1);} //Gas is Helium
//float ObsmTorrRAW;
//ObsmTorrRAW = (pow(10,(PidV - 6.125))*1000);

}

///////////// PID Calculations//////////////////////////
void Controller(){

  Input = PiraniV;

if (SETTING == 0)
   {   
      Setpoint = 0;
      myPID.SetTunings(1, 1000, 0);
      myPID.Compute(); 
      analogWrite(MfcValvePin, Output);
      MfcOutV = (5./255.) * Output;
   }

else {
  SettingV = log10(SETTING) + 3.125;
  Setpoint=SettingV;

  //If Setpoint is less than or equal to 99mTorr, conservative PID parameters are used
  if (SETTING <= 99)
  {  //Setpoint is greater than 19mTorr, use fusion tuning parameters
    myPID.SetTunings(consKp, consKi, consKd);
  }

  //If Setpoint is greater than 99mTorr, aggressive PID parameters are used
  if (SETTING > 99)
  {
     //Setpoint is greater than 150mTorr, use aggressive tuning parameters
     myPID.SetTunings(aggKp, aggKi, aggKd);
  }

  myPID.Compute();
  analogWrite(MfcValvePin, Output);
  MfcOutV = (5./255.) * Output;

    }

//For clearer chart when using Serial Plotter
Serial.print(0); // To freeze the lower limit 
Serial.print(" "); 
Serial.print(30); // To freeze the upper limit 
Serial.print(" ");
Serial.print(SETTING); // SETTING 
Serial.print(" "); 

Serial.println(ObsmTorr);


}
