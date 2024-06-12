#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <TM1637Display.h>


// ----------------------------------------------------------------- //
// -------------------------- GAME CONFIG -------------------------- //
// ----------------------------------------------------------------- //

//---------- DIGITAL PINS (buttons, buzzer, timer) ----------//
#define DISPLAY_TIMER_DIO 23
#define DISPLAY_TIMER_CLK 22
#define GAME_BTN_1 50
#define GAME_BTN_2 51
#define MOD_GEN_BTN_R 26
#define MOD_GEN_BTN_G 27
#define MOD_GEN_BTN_Y 28
#define MOD_GEN_BTN_P 29
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
#define MOD_PTB_LED_R A3
#define MOD_PTB_LED_G A4
#define MOD_PTB_LED_B A5
#define MOD_GEN_LED_OK A6
#define MOD_GEN_LED_P A7
#define MOD_GEN_LED_R A8
#define MOD_GEN_LED_G A9
#define MOD_GEN_LED_Y A10
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
int timer_numbers = 0;
String serial_number = "";
bool ptb_status = false;
bool gen_status = false;
bool mem_status = false;

//---------- BUZZER ----------//
long lastBuzzerChange = 0;
int buzzerState = 0;
long buzzerDurations[] = {600, 300, 100};
long buzzerInterval = 0;

//---------- MODULES ----------//
enum Color {
    RED,
    GREEN,
    BLUE,
    YELLOW
};


void setup() {
  pinMode(ERR_LED_1, OUTPUT);
  pinMode(ERR_LED_2, OUTPUT);
  pinMode(MOD_PTB_LED_OK, OUTPUT);
  pinMode(MOD_PTB_LED_R, OUTPUT);
  pinMode(MOD_PTB_LED_G, OUTPUT);
  pinMode(MOD_PTB_LED_B, OUTPUT);
  pinMode(MOD_GEN_LED_OK, OUTPUT);
  pinMode(MOD_GEN_LED_R, OUTPUT);
  pinMode(MOD_GEN_LED_Y, OUTPUT);
  pinMode(MOD_GEN_LED_G, OUTPUT);
  pinMode(MOD_GEN_LED_P, OUTPUT);
  pinMode(MOD_MEM_LED_OK, OUTPUT);
  pinMode(MOD_MEM_LED_1, OUTPUT);
  pinMode(MOD_MEM_LED_2, OUTPUT);
  pinMode(MOD_MEM_LED_3, OUTPUT);
  pinMode(MOD_MEM_LED_4, OUTPUT);
  pinMode(GAME_BTN_1, INPUT);
  pinMode(GAME_BTN_2, INPUT);
  pinMode(MOD_GEN_BTN_R, INPUT);
  pinMode(MOD_GEN_BTN_G, INPUT);
  pinMode(MOD_GEN_BTN_Y, INPUT);
  pinMode(MOD_GEN_BTN_P, INPUT);
  pinMode(MOD_PTB_BTN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(MOD_MEM_BTN_1, INPUT);
  pinMode(MOD_MEM_BTN_2, INPUT);
  pinMode(MOD_MEM_BTN_3, INPUT);
  pinMode(MOD_MEM_BTN_4, INPUT);

  startGame();
}

void loop() {
  if(serial_number != ""){
    //----------Game Running----------//
    updateTimer();
    updateBuzzer();
    modulePTBLoop();

    
    modulePTBLoop(serial_number);
    module1Loop(serial_number);
    module2Loop(serial_number);
    module3Loop(serial_number);
  } else {
    //----------Test Mode----------//
    int ptb_btn = digitalRead(MOD_PTB_BTN);
    int ptb_gen_r = digitalRead(MOD_GEN_BTN_R);
    int ptb_gen_g = digitalRead(MOD_GEN_BTN_G);
    int ptb_gen_y = digitalRead(MOD_GEN_BTN_Y);
    int ptb_gen_p = digitalRead(MOD_GEN_BTN_P);
    int mem_btn_1 = digitalRead(MOD_MEM_BTN_1);
    int mem_btn_2 = digitalRead(MOD_MEM_BTN_2);
    int mem_btn_3 = digitalRead(MOD_MEM_BTN_3);
    int mem_btn_4 = digitalRead(MOD_MEM_BTN_4);
    int game_btn_1 = digitalRead(GAME_BTN_1);
    int game_btn_2 = digitalRead(GAME_BTN_2);

    if(ptb_btn == LOW){
      digitalWrite(MOD_PTB_LED_R, HIGH);
      digitalWrite(MOD_PTB_LED_B, HIGH);
      digitalWrite(MOD_PTB_LED_G, HIGH);
    } else {
      digitalWrite(MOD_PTB_LED_R, LOW);
      digitalWrite(MOD_PTB_LED_B, LOW);
      digitalWrite(MOD_PTB_LED_G, LOW);
    }

    if(ptb_gen_r == LOW){
      digitalWrite(MOD_GEN_LED_R, HIGH);
    } else {
      digitalWrite(MOD_GEN_LED_R, LOW);
    }
    if(ptb_gen_g == LOW){
      digitalWrite(MOD_GEN_LED_G, HIGH);
    } else {
      digitalWrite(MOD_GEN_LED_G, LOW);
    }
    if(ptb_gen_y == LOW){
      digitalWrite(MOD_GEN_LED_Y, HIGH);
    } else {
      digitalWrite(MOD_GEN_LED_Y, LOW);
    }
    if(ptb_gen_p == LOW){
      digitalWrite(MOD_GEN_LED_P, HIGH);
    } else {
      digitalWrite(MOD_GEN_LED_P, LOW);
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

    if(game_btn_2 == HIGH){
      digitalWrite(ERR_LED_1, HIGH);
      digitalWrite(ERR_LED_2, HIGH); 
      digitalWrite(MOD_PTB_LED_OK, HIGH);
      digitalWrite(MOD_GEN_LED_OK, HIGH);
      digitalWrite(MOD_MEM_LED_OK, HIGH);
    } else {
      digitalWrite(ERR_LED_1, LOW);
      digitalWrite(ERR_LED_2, LOW);
      digitalWrite(MOD_PTB_LED_OK, LOW);
      digitalWrite(MOD_GEN_LED_OK, LOW);
      digitalWrite(MOD_MEM_LED_OK, LOW);
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

void addError(){
  errors++;
  if(errors == 1){
    digitalWrite(ERR_LED_1, HIGH);
  }
  else if (errors == 2) {
    digitalWrite(ERR_LED_2, HIGH);
  }
  else if (errors > 2) {
    gameOver();
  }
}

Color generateRandomColor(){
  int randomValue = random(4);
  return static_cast<Color>(randomValue);
}

bool checkSerialNumberHasVowel() {
  const char *ptr = serial_number.c_str();
  while (*ptr != '\0') {
    if (*ptr == 'A' || *ptr == 'E') {
      return true;
    }
    ptr++;
  }
  return false;
}

bool checkLastCharSerialNumberIsEvenNumber() {
  const char *ptr = serial_number.c_str();
  char lastChar = *(ptr + serial_number.length() - 1);
  return (lastChar == '0' || lastChar == '2' || lastChar == '4' || lastChar == '6' || lastChar == '8');
}

bool checkLastCharSerialNumberIsOddNumber(){
  const char *ptr = serial_number.c_str();
  char lastChar = *(ptr + serial_number.length() - 1);
  return (lastChar == '1' || lastChar == '3' || lastChar == '5' || lastChar == '7' || lastChar == '9');
}

int countSerialNumberEvens(){
  const char *ptr = serial_number.c_str();
  int count = 0;
  while (*ptr != '\0') {
    if (*ptr == '0' || *ptr == '2' || *ptr == '4' || *ptr == '6' || *ptr == '8') {
      count++;
    }
    ptr++;
  }
  return count;
}

int countSerialNumberOdds(){
  const char *ptr = serial_number.c_str();
  int count = 0;
  while (*ptr != '\0') {
    if (*ptr == '1' || *ptr == '3' || *ptr == '5' || *ptr == '7' || *ptr == '9') {
      count++;
    }
    ptr++;
  }
  return count;
}

bool timerHasDigit(char number) {
  char str[5];
  itoa(timer_numbers, str, 10);
  
  char *p = str;
  
  while (*p != '\0') {
    if (*p == number) {
      return true;
    }
    p++;
  }
  
  return false;
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
  timer_numbers = result;
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
        //tone(BUZZER_PIN, 2178);
        break;
      case 2:
        //tone(BUZZER_PIN, 1758);
        break;
    }
  }
}





// ------------------------------------------------------------------------------- //
// -------------------------- MODULE - PRESS THE BUTTON -------------------------- //
// ------------------------------------------------------------------------------- //

bool ptb_pressing = false;
Color ptb_color_stage_1, ptb_color_stage_2;

//Changes the color of the led to the desired color.
//The led needs to set 'LOW' to 'on' colors and 'HIGH' to 'off' colors.
void modulePTBChangeColor(Color color){
  switch (color) {
    case RED:
      digitalWrite(MOD_PTB_LED_R, LOW);
      digitalWrite(MOD_PTB_LED_B, HIGH);
      digitalWrite(MOD_PTB_LED_G, HIGH);
      break;
    case BLUE:
      digitalWrite(MOD_PTB_LED_R, HIGH);
      digitalWrite(MOD_PTB_LED_B, LOW);
      digitalWrite(MOD_PTB_LED_G, HIGH);
      break;
    case GREEN:
      digitalWrite(MOD_PTB_LED_R, HIGH);
      digitalWrite(MOD_PTB_LED_B, HIGH);
      digitalWrite(MOD_PTB_LED_G, LOW);
      break;
    case YELLOW:
      digitalWrite(MOD_PTB_LED_R, LOW);
      digitalWrite(MOD_PTB_LED_B, HIGH);
      digitalWrite(MOD_PTB_LED_G, LOW);
      break;
  }
}

//Starts the module by generating the colors randomly
void modulePTBStart(const char* serial_number){
  ptb_color_stage_1 = generateRandomColor();
  ptb_color_stage_2 = generateRandomColor();

  modulePTBChangeColor(ptb_color_stage_1);
}

//When button is pressed, starts stage 1 checker
//If the color is related to press only, it will automatically unlock the module
//If not, shows the color of stage 2 (releasing the button)
void modulePTBStage1() {
  bool endsWithEven = checkLastCharSerialNumberIsEvenNumber();
  bool endsWithOdd = checkLastCharSerialNumberIsOddNumber();
  int countEvens = countSerialNumberEvens();

  if (ptb_color_stage_1 == BLUE && endsWithOdd){
    modulePTBChangeColor(ptb_color_stage_2);
  }
  else if (ptb_color_stage_1 == GREEN && countEvens >= 2){
    ptb_status == true;
    digitalWrite(MOD_PTB_LED_OK, HIGH);
  }
  else if (ptb_color_stage_1 == YELLOW && endsWithEven){
    modulePTBChangeColor(ptb_color_stage_2);
  }
  else if (ptb_color_stage_1 == RED){
    ptb_status == true;
    digitalWrite(MOD_PTB_LED_OK, HIGH);
  }
  else {
    modulePTBChangeColor(ptb_color_stage_2);
  }
}

//When button is released, starts stage 2 checker
//If the button was released correctly, it will automatically unlock the module
//If not, will add an error to the player
void modulePTBStage2(int time_released) {
  if(ptb_color_stage_2 == BLUE){
    if(timerHasDigit('4')){
      ptb_status == true;
      digitalWrite(MOD_PTB_LED_OK, HIGH);
    } else {
      addError();
    }
  }
  else if(ptb_color_stage_2 == GREEN){
    if(timerHasDigit('1')){
      ptb_status == true;
      digitalWrite(MOD_PTB_LED_OK, HIGH);
    } else {
      addError();
    }
  }
  else if(ptb_color_stage_2 == YELLOW){
    if(timerHasDigit('5')){
      ptb_status == true;
      digitalWrite(MOD_PTB_LED_OK, HIGH);
    } else {
      addError();
    }
  }
  else{
    if(timerHasDigit('1')){
      ptb_status == true;
      digitalWrite(MOD_PTB_LED_OK, HIGH);
    } else {
      addError();
    }
  }
}

//Loop function that checks button status changes of the module
void modulePTBLoop(const char* serial_number) {
  if(ptb_status == true) return;

  int button_state = digitalRead(MOD_PTB_BTN);

  //Button changed to PRESSED
  if (button_state == HIGH && ptb_pressing == false){
    ptb_pressing = true;
    modulePTBStage1();
  }

  //Button changed to RELEASED
  if (button_state == LOW && ptb_pressing == true){
    ptb_pressing = false;
    modulePTBStage2();
  }
}




//Modulo 2
void module2Loop(const char* serial) {
  Color ledColor = generateRandomColor();
  if (ledColor == RED || ledColor == PURPLE || ledColor == GREEN) {
    if (hasVowel(serial)) {
      switch (ledColor) {
        case RED:
          digitalWrite(MOD_SMS_LED_R, HIGH);
          break;
        case PURPLE:
          digitalWrite(MOD_SMS_LED_P, HIGH);
          break;
        case GREEN:
          digitalWrite(MOD_SMS_LED_G, HIGH);
          break;
      }
    } else {
      switch (ledColor) {
        case RED:
          digitalWrite(MOD_SMS_LED_Y, HIGH);
          break;
        case PURPLE:
          digitalWrite(MOD_SMS_LED_Y, HIGH);
          break;
        case GREEN:
          digitalWrite(MOD_SMS_LED_R, HIGH);
          break;
      }
    }
    delay(1000);
    digitalWrite(MOD_SMS_LED_R, LOW);
    digitalWrite(MOD_SMS_LED_Y, LOW);
    digitalWrite(MOD_SMS_LED_P, LOW);
    digitalWrite(MOD_SMS_LED_G, LOW);
  }
}

//Modulo 3
void module3Loop(const char* serial) {
  srand(time(NULL));
  const int numStages = 5;
  const int numButtons = 4;

  for (int stage = 1; stage <= numStages; ++stage) {
    int randomNumber = generateRandomNumber();
    Serial.print("Número exibido: ");
    Serial.println(randomNumber);

    int buttonToPress;
    if (randomNumber == 1) {
      buttonToPress = (stage == 2 || stage == 4) ? serial[0] - '0' : serial[1] - '0';
    } else if (randomNumber == 2) {
      buttonToPress = (stage == 3 || stage == 5) ? serial[0] - '0' : serial[1] - '0';
    } else if (randomNumber == 3) {
      buttonToPress = (stage == 1 || stage == 4) ? serial[2] - '0' : 4;
    } else {
      buttonToPress = (stage == 1 || stage == 3) ? serial[2] - '0' : 4;
    }

    Serial.print("Pressione o botão com o número ");
    Serial.println(buttonToPress);
    delay(3000);

    int userInput = 0; // Simulando entrada do usuário (0 como valor padrão)
    if (digitalRead(buttonToPress) == HIGH) {
      userInput = buttonToPress;
    }

    if (userInput != buttonToPress) {
      Serial.println("Erro! Recomeçando o módulo no Estágio 1.");
      errors++;
      return;
    }
  }
  Serial.println("Parabéns! Você desarmou o módulo.");
}