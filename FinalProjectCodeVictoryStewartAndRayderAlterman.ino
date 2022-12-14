// Victory Stewart and Rayder Alterman
// Final Project

#define RDA 0x80
 #define TBE 0x20  
 volatile unsigned char *myUCSR0A = (unsigned char *)0x00C0;
 volatile unsigned char *myUCSR0B = (unsigned char *)0x00C1;
 volatile unsigned char *myUCSR0C = (unsigned char *)0x00C2;
 volatile unsigned int  *myUBRR0  = (unsigned int *) 0x00C4;
 volatile unsigned char *myUDR0   = (unsigned char *)0x00C6;

  
volatile unsigned char* my_ADMUX = (unsigned char*) 0x7C;
volatile unsigned char* my_ADCSRB = (unsigned char*) 0x7B;
volatile unsigned char* my_ADCSRA = (unsigned char*) 0x7A;
volatile unsigned int* my_ADC_DATA = (unsigned int*) 0x78;

volatile unsigned char *portDDRB = (unsigned char *) 0x24;
volatile unsigned char *portB =    (unsigned char *) 0x25;

volatile unsigned char* port_d  = (unsigned char*) 0x2B; 
volatile unsigned char* ddr_d  = (unsigned char*) 0x2A; 
volatile unsigned char* pin_d  = (unsigned char*) 0x29;

#include <LiquidCrystal.h>
#include "DHT.h"

#define DHTPIN 2     
#define DHTTYPE DHT11   
DHT dht(DHTPIN, DHTTYPE);

//initialize the LCD monitor values with the pin values on the arduino
const int rs = 22, en = 24, d4 = 34, d5 = 36, d6 = 38, d7 = 40, temp = 10;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

#include <Stepper.h>
#define STEPS 100
Stepper stepper(STEPS, 31, 27, 25, 23);
int previous = 0;
void setup() {
  // put your setup code here, to run once:
  *portDDRB |= 0x40;//Port 12/yellow LED Output
  *portB &= 0xBF;
  adc_init();  // setup the ADC
  *ddr_d |= 0xFF; //Port D/Green LED Output
  DDRB |= (1<<DDB5); //Port 11/Red LED Output
  pinMode(6, OUTPUT); //Blue LED
  pinMode(4, INPUT); //enable pin
  U0init(9600);
  dht.begin();
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  stepper.setSpeed(30);
  pinMode(7, INPUT);
}

void loop() {
  float f = dht.readTemperature(true);
  bool Enable;
  unsigned int WaterLevel = adc_read(0);
  int Button = digitalRead(4); //Enable disable switch
  if(Button == HIGH){
    if(Enable == 1){
      Enable = 0;
    }
    else{
      Enable = 1;
    }
    delay (500);
  }
  
//Disabled
  if(Enable == 1){
//add find/display time
  *portB |= 0x40; //Port 12 High
   *port_d &= 0xFE; //Port D Low
   PORTB &= ~(1<<PORTB5); //Port 11 Low
  digitalWrite(6, LOW);
  lcddisabled();
  }

//Idle
  if(f < 75 & WaterLevel > 100 & Enable == 0){
  *port_d |= 0x01; //Port D High
  *portB &= 0xBF; //Port 12 Low
  digitalWrite(6, LOW);
  PORTB &= ~(1<<PORTB5); //Port 11 Low
  lcdidle(f);
  stepperon();
  }

//Running
  if(f > 75 & WaterLevel > 100 & Enable == 0){
  *port_d &= 0xFE; //Port D Low
  *portB &= 0xBF; //Port 12 Low
  digitalWrite(6, HIGH);
  PORTB &= ~(1<<PORTB5); //Port 11 Low
  lcdidle(f);
  stepperon();
  }

//Error
  if(WaterLevel < 100 & Enable == 0){
  *port_d &= 0xFE; //Port D Low
  *portB &= 0xBF; //Port 12 Low
  digitalWrite(6, LOW);
  PORTB |=(1<< PORTB5); //Port 11 High
  lcderror();
  }
  
   
}



void U0init(unsigned long U0baud)
{

 unsigned long FCPU = 16000000;
 unsigned int tbaud;
 tbaud = (FCPU / 16 / U0baud - 1);
 // Same as (FCPU / (16 * U0baud)) - 1;
 *myUCSR0A = 0x20;
 *myUCSR0B = 0x18;
 *myUCSR0C = 0x06;
 *myUBRR0  = tbaud;
}

unsigned char U0kbhit()
{
  return *myUCSR0A & RDA;
}
unsigned char U0getchar()
{
  return *myUDR0;
}
void U0putchar(unsigned char U0pdata)
{
  while((*myUCSR0A & TBE)==0);
  *myUDR0 = U0pdata;
}
void printTemp(int ctemp, int ftemp){
  // set the cursor to column 0, line 1
  lcd.setCursor(0, 1);
  //display the temperatures in celcuis and ferinheit
  lcd.print(ctemp);
  lcd.print("*C ");
  lcd.print(ftemp);
  lcd.print("*F ") ;
}

void printHum(int hum){
  //set curser on first row
  lcd.setCursor(0, 0);
  //display humidity
  lcd.print("Humidity: ");
  lcd.print(hum);
  lcd.print("%");
}

void lcderror(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ERROR");
}

void lcddisabled(){
  lcd.clear();
}

void lcdidle(int f){
  // Wait a few seconds between measurements.

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit
  
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    lcd.setCursor(0, 0);
    lcd.clear();
    lcd.print("CANNOT READ");
    return;
  }
  else{
    printHum(h);
    printTemp(t, f);
  }
}

void adc_init()
{
  // setup the A register
  *my_ADCSRA |= 0b10000000; 
  *my_ADCSRA &= 0b11011111; 
  *my_ADCSRA &= 0b11110111; 
  *my_ADCSRA &= 0b11111000; 
  // setup the B register
  *my_ADCSRB &= 0b11110111; 
  *my_ADCSRB &= 0b11111000; 
  // setup the MUX Register
  *my_ADMUX  &= 0b01111111; 
  *my_ADMUX  |= 0b01000000; 
  *my_ADMUX  &= 0b11011111; 
  *my_ADMUX  &= 0b11100000; 
}

unsigned int adc_read(unsigned char adc_channel_num)
{
  // clear the channel selection bits (MUX 4:0)
  *my_ADMUX  &= 0b11100000;
  // clear the channel selection bits (MUX 5)
  *my_ADCSRB &= 0b11110111;
  // set the channel number
  if(adc_channel_num > 7)
  {
    // set the channel selection bits, but remove the most significant bit (bit 3)
    adc_channel_num -= 8;
    // set MUX bit 5
    *my_ADCSRB |= 0b00001000;
  }
  // set the channel selection bits
  *my_ADMUX  += adc_channel_num;
  // set bit 6 of ADCSRA to 1 to start a conversion
  *my_ADCSRA |= 0x40;
  // wait for the conversion to complete
  while((*my_ADCSRA & 0x40) != 0);
  // return the result in the ADC data register
  return *my_ADC_DATA;
}
void print_int(unsigned int out_num)
{
  // clear a flag (for printing 0's in the middle of numbers)
  unsigned char print_flag = 0;
  // if its greater than 1000
  if(out_num >= 1000)
  {
    // get the 1000's digit, add to '0' 
    U0putchar(out_num / 1000 + '0');
    // set the print flag
    print_flag = 1;
    // mod the out num by 1000
    out_num = out_num % 1000;
  }
  // if its greater than 100 or we've already printed the 1000's
  if(out_num >= 100 || print_flag)
  {
    // get the 100's digit, add to '0'
    U0putchar(out_num / 100 + '0');
    // set the print flag
    print_flag = 1;
    // mod the output num by 100
    out_num = out_num % 100;
  } 
  // if its greater than 10, or we've already printed the 10's
  if(out_num >= 10 || print_flag)
  {
    U0putchar(out_num / 10 + '0');
    print_flag = 1;
    out_num = out_num % 10;
  } 
  // always print the last digit (in case it's 0)
  U0putchar(out_num + '0');
  // print a newline
  U0putchar('\n');
}
void stepperon(){
  if(digitalRead(7) == HIGH){
    stepper.step(10);
  }
}
