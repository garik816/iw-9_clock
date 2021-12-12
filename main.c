#include <Arduino.h>

#define srClk 2 // serial clock
#define srDataIn 1  //serial data-in
#define rClk0 5 // register clock rank 0
#define rClk1 6 // register clock rank 1
#define rClk2 16 // register clock rank 2
#define rClk3 17 // register clock rank 3
#define LED_PIN 10 // DEBUG
#define BUTTON_1_PIN 18 // DEBUG
#define BUTTON_2_PIN 19 // DEBUG

#define IV9_B 0b00000001
#define IV9_C 0b00000010
#define IV9_A 0b00000100
#define IV9_F 0b00001000
#define IV9_G 0b00010000
#define IV9_D 0b00100000
#define IV9_E 0b01000000
#define IV9_DP  0b10000000

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

uint8_t digits[] = {IV9_0, IV9_1, IV9_2, IV9_3, IV9_4, IV9_5, IV9_6, IV9_7, IV9_8, IV9_9};
uint8_t sec = 0, min = 0, hour = 0;

bool buttonFlag = false;

static unsigned long timer = millis();
static unsigned long oneSecondTimer = millis();
static unsigned long buttonTimer = millis();

void blink(void){
  if (millis() - timer > 1000) {
    timer = millis();
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }
}

void putDigit(uint8_t digit, uint8_t row){
  digitalWrite(row, LOW);
  shiftOut(srDataIn, srClk, MSBFIRST, digits[digit]);
  digitalWrite(row, HIGH);
}



void Clock(void){
  if (millis() - oneSecondTimer > 2000) {
    oneSecondTimer = millis();
    sec++;
  }

  if (sec == 60){
    sec = 0;
    min++;
  }

  if (min == 60){
    min = 0;
    hour++;
  }

  if (hour == 24){
    hour = 0;
  }

  putDigit(min%10, rClk3);
  putDigit((min/10)%10, rClk2);
  putDigit(hour%10, rClk1);
  putDigit((hour/10)%10, rClk0);

}

// void counter(void){
//   for (int i = 0; i < 10000; i++) {
//     putDigit(i%10, rClk3);
//     putDigit((i/10)%10, rClk2);
//     putDigit((i/100)%10, rClk1);
//     putDigit((i/1000)%10, rClk0);
//     delay(100);
//   }
// }

void detectButtons(void){

  bool btn1State = !digitalRead(BUTTON_1_PIN);
  bool btn2State = !digitalRead(BUTTON_2_PIN);

  if (btn1State && !buttonFlag && millis() - buttonTimer > 100) {
    buttonFlag = true;
    buttonTimer = millis();
    hour++;
  }
  if (btn2State && !buttonFlag && millis() - buttonTimer > 100) {
    buttonFlag = true;
    buttonTimer = millis();
    min++;
  }
  if ((!btn1State || !btn2State) && buttonFlag && millis() - buttonTimer > 500) {
  buttonFlag = false;
  buttonTimer = millis();
  //Serial.println("release");
}

}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(srDataIn, OUTPUT);
  pinMode(srClk, OUTPUT);
  pinMode(rClk0, OUTPUT);
  pinMode(rClk1, OUTPUT);
  pinMode(rClk2, OUTPUT);
  pinMode(rClk3, OUTPUT);

  pinMode(BUTTON_1_PIN, INPUT);
  pinMode(BUTTON_2_PIN, INPUT);

  digitalWrite(rClk0, LOW);
  digitalWrite(rClk1, LOW);
  digitalWrite(rClk2, LOW);
  digitalWrite(rClk3, LOW);
  shiftOut(srDataIn, srClk, MSBFIRST, 0);
  digitalWrite(rClk0, HIGH);
  digitalWrite(rClk1, HIGH);
  digitalWrite(rClk2, HIGH);
  digitalWrite(rClk3, HIGH);
}

void loop() {
  blink();
  detectButtons();
  Clock();
  // counter();
}
