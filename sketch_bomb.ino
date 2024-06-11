#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <TM1637Display.h>


// ----------------------------------------------------------------- //
// -------------------------- GAME CONFIG -------------------------- //
// ----------------------------------------------------------------- //

//---------- DIGITAL PINS (buttons, buzzer, timer) ----------//
#define DISPLAY_TIMER_DIO 22
#define DISPLAY_TIMER_CLK 23
#define GAME_BTN_1 2
#define GAME_BTN_2 3
#define MOD_SMS_BTN_R 26
#define MOD_SMS_BTN_G 27
#define MOD_SMS_BTN_Y 28
#define MOD_SMS_BTN_P 29
#define MOD_PTB_BTN 8
#define BUZZER_PIN 9
#define MOD_MEM_BTN_1 40
#define MOD_MEM_BTN_2 41
#define MOD_MEM_BTN_3 42
#define MOD_MEM_BTN_4 43

//---------- ANALOG PINS (leds) ----------//
#define ERR_LED_1 A0
#define ERR_LED_2 A1
#define MOD_PTB_LED_OK A2
#define MOD_PTB_LED_C A3
#define MOD_PTB_LED_M A4
#define MOD_PTB_LED_Y A5
#define MOD_SMS_LED_OK A6
#define MOD_SMS_LED_R A7
#define MOD_SMS_LED_Y A8
#define MOD_SMS_LED_G A9
#define MOD_SMS_LED_P A10
#define MOD_MEM_LED_OK A11
#define MOD_MEM_LED_1 A12
#define MOD_MEM_LED_2 A13
#define MOD_MEM_LED_3 A14
#define MOD_MEM_LED_4 A15

//---------- COMPONENTS ----------//
LiquidCrystal_I2C display_main(0x27, 16, 2);
LiquidCrystal_I2C display_mem(0x23, 16, 2);
TM1637Display display_timer(DISPLAY_TIMER_CLK, DISPLAY_TIMER_DIO);

//---------- TIMER ----------//
long timer_ms = 0;
long start_ms = 0;
float timeMultiplier = 1;

//---------- GAME ----------//
int errors = 0;
int difficulty = 1;
String serial_number = "";

//---------- BUZZER ----------//
long lastBuzzerChange = 0;
int buzzerState = 0;
long buzzerDurations[] = {600, 300, 100};
long buzzerInterval = 0;


void setup() {
  pinMode(ERR_LED_1, OUTPUT);
  pinMode(ERR_LED_2, OUTPUT);
  pinMode(MOD_PTB_LED_OK, OUTPUT);
  pinMode(MOD_PTB_LED_C, OUTPUT);
  pinMode(MOD_PTB_LED_M, OUTPUT);
  pinMode(MOD_PTB_LED_Y, OUTPUT);
  pinMode(MOD_SMS_LED_OK, OUTPUT);
  pinMode(MOD_SMS_LED_R, OUTPUT);
  pinMode(MOD_SMS_LED_Y, OUTPUT);
  pinMode(MOD_SMS_LED_G, OUTPUT);
  pinMode(MOD_SMS_LED_P, OUTPUT);
  pinMode(MOD_MEM_LED_OK, OUTPUT);
  pinMode(MOD_MEM_LED_1, OUTPUT);
  pinMode(MOD_MEM_LED_2, OUTPUT);
  pinMode(MOD_MEM_LED_3, OUTPUT);
  pinMode(MOD_MEM_LED_4, OUTPUT);
  pinMode(GAME_BTN_1, INPUT);
  pinMode(GAME_BTN_2, INPUT);
  pinMode(MOD_SMS_BTN_R, INPUT);
  pinMode(MOD_SMS_BTN_G, INPUT);
  pinMode(MOD_SMS_BTN_Y, INPUT);
  pinMode(MOD_SMS_BTN_P, INPUT);
  pinMode(MOD_PTB_BTN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(MOD_MEM_BTN_1, INPUT);
  pinMode(MOD_MEM_BTN_2, INPUT);
  pinMode(MOD_MEM_BTN_3, INPUT);
  pinMode(MOD_MEM_BTN_4, INPUT);

  //startGame();
}

void loop() {
  if(serial_number != ""){
    updateTimer();
    updateBuzzer();
    modulePTBLoop();
  } else {
    int ptb_btn = digitalRead(MOD_PTB_BTN);
    int ptb_sms_r = digitalRead(MOD_SMS_BTN_R);
    int ptb_sms_g = digitalRead(MOD_SMS_BTN_G);
    int ptb_sms_y = digitalRead(MOD_SMS_BTN_Y);
    int ptb_sms_p = digitalRead(MOD_SMS_BTN_P);
    int mem_btn_1 = digitalRead(MOD_MEM_BTN_1);
    int mem_btn_2 = digitalRead(MOD_MEM_BTN_2);
    int mem_btn_3 = digitalRead(MOD_MEM_BTN_3);
    int mem_btn_4 = digitalRead(MOD_MEM_BTN_4);



    if(ptb_sms_r == LOW){
      digitalWrite(MOD_SMS_LED_R, HIGH);
    } else {
      digitalWrite(MOD_SMS_LED_R, LOW);
    }
    if(ptb_sms_g == LOW){
      digitalWrite(MOD_SMS_LED_G, HIGH);
    } else {
      digitalWrite(MOD_SMS_LED_G, LOW);
    }
    if(ptb_sms_y == LOW){
      digitalWrite(MOD_SMS_LED_Y, HIGH);
    } else {
      digitalWrite(MOD_SMS_LED_Y, LOW);
    }
    if(ptb_sms_p == LOW){
      digitalWrite(MOD_SMS_LED_P, HIGH);
    } else {
      digitalWrite(MOD_SMS_LED_P, LOW);
    }

    if(mem_btn_1 == HIGH){
      digitalWrite(MOD_MEM_LED_1, HIGH);
    } else {
      digitalWrite(MOD_MEM_LED_1, LOW);
    }
    if(mem_btn_2 == HIGH){
      digitalWrite(MOD_MEM_LED_2, HIGH);
    } else {
      digitalWrite(MOD_MEM_LED_2, LOW);
    }
    if(mem_btn_3 == HIGH){
      digitalWrite(MOD_MEM_LED_3, HIGH);
    } else {
      digitalWrite(MOD_MEM_LED_3, LOW);
    }
    if(mem_btn_4 == HIGH){
      digitalWrite(MOD_MEM_LED_4, HIGH);
    } else {
      digitalWrite(MOD_MEM_LED_4, LOW);
    }

    if(ptb_btn == HIGH){
      digitalWrite(ERR_LED_1, HIGH);
      digitalWrite(ERR_LED_2, HIGH); 
      digitalWrite(MOD_PTB_LED_OK, HIGH);
      digitalWrite(MOD_PTB_LED_C, HIGH);
      digitalWrite(MOD_SMS_LED_OK, HIGH);
      digitalWrite(MOD_SMS_LED_R, HIGH);
      digitalWrite(MOD_SMS_LED_Y, HIGH);
      digitalWrite(MOD_SMS_LED_G, HIGH);
      digitalWrite(MOD_SMS_LED_P, HIGH);
      digitalWrite(MOD_MEM_LED_OK, HIGH);
      digitalWrite(MOD_MEM_LED_1, HIGH);
      digitalWrite(MOD_MEM_LED_2, HIGH);
      digitalWrite(MOD_MEM_LED_3, HIGH);
      digitalWrite(MOD_MEM_LED_4, HIGH);
    } else {
      digitalWrite(ERR_LED_1, LOW);
      digitalWrite(ERR_LED_2, LOW);
      digitalWrite(MOD_PTB_LED_OK, LOW);
      digitalWrite(MOD_PTB_LED_C, LOW);
      digitalWrite(MOD_SMS_LED_OK, LOW);
      digitalWrite(MOD_SMS_LED_R, LOW);
      digitalWrite(MOD_SMS_LED_Y, LOW);
      digitalWrite(MOD_SMS_LED_G, LOW);
      digitalWrite(MOD_SMS_LED_P, LOW);
      digitalWrite(MOD_MEM_LED_OK, LOW);
      digitalWrite(MOD_MEM_LED_1, LOW);
      digitalWrite(MOD_MEM_LED_2, LOW);
      digitalWrite(MOD_MEM_LED_3, LOW);
      digitalWrite(MOD_MEM_LED_4, LOW);
    }
  }
}




// -------------------------------------------------------------------------- //
// -------------------------- BASIC GAME FUNCTIONS -------------------------- //
// -------------------------------------------------------------------------- //

void startGame() {
  generateSerialNumber();
  startTimer();
}

void gameOver() {
  serial_number = "";
  timer_ms = 0;
  start_ms = 0;
}

void gameWin() {
  
}





// ----------------------------------------------------------------------------- //
// -------------------------- SERIAL NUMBER FUNCTIONS -------------------------- //
// ----------------------------------------------------------------------------- //

void generateSerialNumber() {
  size_t const address {0};
  unsigned int seed {};
  EEPROM.get(address, seed);
  randomSeed(seed);
  EEPROM.put(address, seed + 1);

  String hexString = "";

  display_main.init();
  display_main.backlight();

  for (int i = 0; i < 8; i++) {
    int randomNumber = random(0, 16); 
    char hexDigit = (randomNumber < 10) ? (char)('0' + randomNumber) : (char)('A' + (randomNumber - 10));
    hexString += hexDigit;
  }
  serial_number = hexString;

  display_main.setCursor(0,0);
  display_main.print("Serial Number");
  display_main.setCursor(0,1);
  display_main.print(serial_number);
}







// ---------------------------------------------------------------------- //
// -------------------------- TIMER AND BUZZER -------------------------- //
// ---------------------------------------------------------------------- //

void startTimer() {
  switch (difficulty) {
    case 2:
      timer_ms = 240000;
      break;
    case 3:
      timer_ms = 180000;
      break;
    default:
      timer_ms = 300000;
      break;
  }

  start_ms = millis();
  display_timer.setBrightness(0x0f);
  updateTimer();
}

void updateTimer() {
  long ms_left = timer_ms - millis() + start_ms;

  int total_seconds = ms_left / 1000;
    
  int minutes = total_seconds / 60;
  int seconds = total_seconds % 60;

  if (minutes > 99) {
    minutes = 99;
  }
  if (seconds > 99) {
    seconds = 99;
  }
    
  int result = minutes * 100 + seconds;

  display_timer.showNumberDecEx(result, 0b01000000, true);
}

void updateBuzzer() {
  long currentMillis = millis();

  if (currentMillis - lastBuzzerChange >= buzzerDurations[buzzerState]) {
    lastBuzzerChange = currentMillis;
    buzzerState = (buzzerState + 1) % 3;

    switch (buzzerState) {
      case 0:
        noTone(BUZZER_PIN);
        break;
      case 1:
        tone(BUZZER_PIN, 2178);
        break;
      case 2:
        tone(BUZZER_PIN, 1758);
        break;
    }
  }
}





// ------------------------------------------------------------------------------- //
// -------------------------- MODULE - PRESS THE BUTTON -------------------------- //
// ------------------------------------------------------------------------------ //

void modulePTBLoop() {
  int buttonState = digitalRead(MOD_PTB_BTN);

  if(buttonState == HIGH){
    digitalWrite(ERR_LED_1, HIGH);
  } else {
    digitalWrite(ERR_LED_1, LOW);
  }
}


