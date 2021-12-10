#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
// #include <wire.h> //i2c

#define srClk 13 // serial clock
#define srDataIn 12	//serial data-in
#define rClk0 8 // register clock rank 0
#define rClk1 9 // register clock rank 1
#define rClk2 10 // register clock rank 2
#define rClk3 11 // register clock rank 3

#define IV9_B 0b00000001
#define IV9_C 0b00000010
#define IV9_A 0b00000100
#define IV9_F	0b00001000
#define IV9_G	0b00010000
#define IV9_D	0b00100000
#define IV9_E	0b01000000
#define IV9_DP	0b10000000

#define IV9_0 IV9_A+IV9_B+IV9_C+IV9_D+IV9_E+IV9_F
#define IV9_1 IV9_B+IV9_C
#define IV9_2 IV9_A+IV9_B+IV9_G+IV9_E+IV9_D
#define IV9_3 IV9_A+IV9_B+IV9_G+IV9_C+IV9_D
#define IV9_4 IV9_F+IV9_B+IV9_G+IV9_C
#define IV9_5 IV9_A+IV9_F+IV9_G+IV9_C+IV9_D
#define IV9_6 IV9_A+IV9_F+IV9_G+IV9_C+IV9_D+IV9_E
#define IV9_7 IV9_A+IV9_B+IV9_C
#define IV9_8 IV9_A+IV9_B+IV9_C+IV9_D+IV9_E+IV9_F+IV9_G
#define IV9_9 IV9_A+IV9_B+IV9_C+IV9_D+IV9_F+IV9_G

byte line0[] = {B00000,B00000,B00000,B00000,B00000,B00000,B00000,B00000};
byte line1[] = {B00000,B00000,B00000,B00000,B00000,B00000,B00000,B11111};
byte line2[] = {B00000,B00000,B00000,B00000,B00000,B00000,B11111,B11111};
byte line3[] = {B00000,B00000,B00000,B00000,B00000,B11111,B11111,B11111};
byte line4[] = {B00000,B00000,B00000,B00000,B11111,B11111,B11111,B11111};
byte line5[] = {B00000,B00000,B00000,B11111,B11111,B11111,B11111,B11111};
byte line6[] = {B00000,B00000,B11111,B11111,B11111,B11111,B11111,B11111};
byte line7[] = {B00000,B11111,B11111,B11111,B11111,B11111,B11111,B11111};
byte line8[] = {B11111,B11111,B11111,B11111,B11111,B11111,B11111,B11111};

uint8_t digits[] = {IV9_0, IV9_1, IV9_2, IV9_3, IV9_4, IV9_5, IV9_6, IV9_7, IV9_8, IV9_9};

static unsigned long timer = millis();

LiquidCrystal_I2C lcd(0x27,16,2);

void blink(void){
  if (millis() - timer > 500) {
    timer = millis();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
}

void createCharacters(){
  lcd.createChar(1, line1);
  lcd.createChar(2, line2);
  lcd.createChar(3, line3);
  lcd.createChar(4, line4);
  lcd.createChar(5, line5);
  lcd.createChar(6, line6);
  lcd.createChar(7, line7);
  lcd.createChar(8, line8);
}

void putChar(void){
  for(int k=1;k>=0;k--){
    for(int j=0;j<16;j++){
      for(int i=1;i<9;i++){
        lcd.setCursor(j,k);
        lcd.write(i);
        delay(25);
      }
    }
  }
  for(int k=0;k<2;k++){
    for(int j=15;j>=0;j--){
      for(int i=8;i>=1;i--){
        lcd.setCursor(j,k);
        lcd.write(i);
        delay(25);
      }
      lcd.setCursor(j,k);
      lcd.print(" ");
    }
  }
}

void putCharUP(void){
  for(int j=0;j<16;j++){
    for(int k=1;k>=0;k--){
      for(int i=1;i<9;i++){
        lcd.setCursor(j,k);
        lcd.write(i);
        delay(25);
      }
    }
  }
  for(int j=15;j>=0;j--){
    for(int k=0;k<2;k++){
      for(int i=8;i>=1;i--){
        lcd.setCursor(j,k);
        lcd.write(i);
        delay(25);
      }
      lcd.setCursor(j,k);
      lcd.print(" ");
    }
  }
}

void output7seg(uint8_t digit, uint8_t rank) {
	PORTC &= ~(1 << rank);

	for (int i = 0; i < 8; i++) {
		uint8_t dataBit = (((digit & (1 << i)) >> i));

		PORTB &= ~(1 << srClk);

		if (dataBit)
			PORTB |= (1 << srDataIn);
		else
			PORTB &= ~(1 << srDataIn);

		PORTB |= (1 << srClk);
		PORTB &= ~(1 << srDataIn);
	}
	PORTC |= (1 << rank);
}

void out(void){
  for (int i = 0; i < 10; i++) {
		    for(int j=0;j<16;j++){
		      for(int i=1;i<9;i++){
		        lcd.setCursor(j,0);
		        lcd.write(i);
		        delay(2);
		      }
		    }
		    for(int j=15;j>=0;j--){
		      for(int i=8;i>=1;i--){
		        lcd.setCursor(j,0);
		        lcd.write(i);
		        delay(2);
		      }
		      lcd.setCursor(j,0);
		      lcd.print(" ");
		    }
		lcd.setCursor(0,1);
		lcd.print(i);
    digitalWrite(rClk0, LOW);
    shiftOut(srDataIn, srClk, MSBFIRST, digits[i]);
    digitalWrite(rClk0, HIGH);
  }
}

void counter(void){
  for (int i=0; i<10; i++) {
  	output7seg(digits[i],rClk0);
  	for (int i=0; i<10; i++) {
  		output7seg(digits[i],rClk1);
  		for (int i=0; i<10; i++) {
  			output7seg(digits[i],rClk2);
  			for (int i=0; i<10; i++) {
  				output7seg(digits[i],rClk3);
  			}
  		}
  	}
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(srDataIn, OUTPUT);
  pinMode(srClk, OUTPUT);
  pinMode(rClk0, OUTPUT);
  pinMode(rClk1, OUTPUT);
  pinMode(rClk2, OUTPUT);
  pinMode(rClk3, OUTPUT);

	digitalWrite(rClk0, LOW);
	shiftOut(srDataIn, srClk, MSBFIRST, 0);
	digitalWrite(rClk0, HIGH);

  lcd.init();
  lcd.backlight();
  createCharacters();

  lcd.clear();
  Serial.println("start");
}

void loop() {
  blink();
  // putChar();
  // putCharUP();

  out();
}
