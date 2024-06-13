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
#define MOD_GEN_BTN_B 29
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
#define MOD_PTB_LED_B A4
#define MOD_PTB_LED_G A5
#define MOD_GEN_LED_OK A6
#define MOD_GEN_LED_B A7
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
bool mem_status = true;
int mod_gen_length = 4;

//---------- BUZZER ----------//
long lastBuzzerChange = 0;
int buzzerState = 0;
long buzzerDurations[] = {600, 300, 100};
long buzzerInterval = 0;

//---------- ENUMS ----------//
enum Color {
    RED,
    GREEN,
    BLUE,
    YELLOW
};






// -------------------------------------------------------------------------- //
// -------------------------- BASIC GAME FUNCTIONS -------------------------- //
// -------------------------------------------------------------------------- //

void startGame() {
  timer_ms = 0;
  start_ms = 0;
  timeMultiplier = 1;
  errors = 0;
  difficulty = 1;
  timer_numbers = 0;
  serial_number = "";
  ptb_status = false;
  gen_status = false;
  mem_status = true;
  mod_gen_length = 4;
  generateSerialNumber();
  modulePTBStart();
  moduleGENStart();
  moduleMEMStart();
  moduleMEMUnlockModule();
  startTimer();
  digitalWrite(MOD_MEM_LED_OK, HIGH);
}

void restartGame(){
    gameOver();
    startGame();
}

void gameOver() {
  serial_number = "";
  timer_ms = 0;
  start_ms = 0;

  int points = 0;

  if (ptb_status == true) points += 35;
  if (gen_status == true) points += 40;
  if (mem_status == true) points += 50;
  points += timer_numbers;

  char buffer[20];
  snprintf(buffer, sizeof(buffer), "Pontuacao: %d", points);

  display_main.setCursor(0,0);
  display_main.print("A Bomba Explodiu!");
  display_main.setCursor(0,1);
  display_main.print(buffer);

  timer_ms = 0;
  start_ms = 0;
  timeMultiplier = 1;
  errors = 0;
  difficulty = 1;
  timer_numbers = 0;
  serial_number = "";
  ptb_status = false;
  gen_status = false;
  mem_status = true;
  mod_gen_length = 4;
}

void gameWin() {
  serial_number = "";
  timer_ms = 0;
  start_ms = 0;

  int points = 0;

  if (ptb_status == true) points += 70;
  if (gen_status == true) points += 80;
  if (mem_status == true) points += 100;
  points += timer_numbers;

  char buffer[20];
  snprintf(buffer, sizeof(buffer), "Pontuacao: %d", points);

  display_main.setCursor(0,0);
  display_main.print("Bomba desarmada! <3");
  display_main.setCursor(0,1);
  display_main.print(buffer);
}

void checkModulesUnlocked(){
  if(ptb_status && gen_status){
    gameWin();
    int melody[] = {
      262, 294, 330, 349, 392, 440, 494, 523
    };
    int noteDurations[] = {
      500, 500, 500, 500, 500, 500, 500, 500
    };
    for (int i = 0; i < 8; i++) {
      tone(BUZZER_PIN, melody[i], noteDurations[i]);
      delay(noteDurations[i] * 1.3); // Pausa entre as notas
    }
    noTone(BUZZER_PIN);
  }
}

void gameChangeDifficulty(){
  if(difficulty == 1){
    difficulty++;
    mod_gen_length = 6;
    display_main.setCursor(0,1);
    display_main.print("Avancado        ");
  }
  else if(difficulty == 2){
    difficulty++;
    mod_gen_length = 8;
    display_main.setCursor(0,1);
    display_main.print("Profissional    ");
  }
  else if (difficulty == 3){
    difficulty = 1;
    mod_gen_length = 4;
    display_main.setCursor(0,1);
    display_main.print("Iniciante      ");
  }
}

void addError(){
  errors++;
  if(errors == 1){
    digitalWrite(ERR_LED_1, HIGH);
    tone(BUZZER_PIN, 500);
    delay(300);
    noTone(BUZZER_PIN);
  }
  else if (errors == 2) {
    digitalWrite(ERR_LED_2, HIGH);
    tone(BUZZER_PIN, 500);
    delay(300);
    noTone(BUZZER_PIN);
  }
  else if (errors > 2) {
    gameOver();
  }
}

Color generateRandomColor(){
  int randomValue = random(4);
  return static_cast<Color>(randomValue);
}

Color* generateRandomColorSequence(int length) {
  Color* sequence = (Color*)malloc(length * sizeof(Color));
  for (int i = 0; i < length; i++) {
    sequence[i] = random(4);
  }
  return sequence;
}

bool checkSerialNumberHasVowel() {
  const char *ptr = serial_number.c_str();
  while (*ptr != '\0') {
    if (*ptr == 'A' || *ptr == 'a' || *ptr == 'E' || *ptr == 'e') {
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

int findIndex(int array[], int size, int target) {
  for (int i = 0; i < size; i++) {
    if (array[i] == target) {
      return i;
    }
  }
  return -1;
}

void generateSerialNumber() {
  size_t const address {0};
  unsigned int seed {};
  EEPROM.get(address, seed);
  randomSeed(seed);
  EEPROM.put(address, seed + 1);

  String hexString = "";

  for (int i = 0; i < 8; i++) {
    int randomNumber = random(0, 16); 
    char hexDigit = (randomNumber < 10) ? (char)('0' + randomNumber) : (char)('A' + (randomNumber - 10));
    hexString += hexDigit;
  }
  serial_number = hexString;
  display_main.setCursor(0,0);
  display_main.print("Serial Number");
  display_main.setCursor(0,1);
  display_main.print("                ");
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
void modulePTBStart(){
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
    ptb_status = true;
    digitalWrite(MOD_PTB_LED_OK, HIGH);
  }
  else if (ptb_color_stage_1 == YELLOW && endsWithEven){
    modulePTBChangeColor(ptb_color_stage_2);
  }
  else if (ptb_color_stage_1 == RED){
    ptb_status = true;
    digitalWrite(MOD_PTB_LED_OK, HIGH);
  }
  else {
    modulePTBChangeColor(ptb_color_stage_2);
  }
}

//When button is released, starts stage 2 checker
//If the button was released correctly, it will automatically unlock the module
//If not, will add an error to the player
void modulePTBStage2() {
  if(ptb_color_stage_2 == BLUE){
    if(timerHasDigit('4')){
      ptb_status = true;
      digitalWrite(MOD_PTB_LED_OK, HIGH);
    } else {
      addError();
    }
  }
  else if(ptb_color_stage_2 == GREEN){
    if(timerHasDigit('1')){
      ptb_status = true;
      digitalWrite(MOD_PTB_LED_OK, HIGH);
    } else {
      addError();
    }
  }
  else if(ptb_color_stage_2 == YELLOW){
    if(timerHasDigit('5')){
      ptb_status = true;
      digitalWrite(MOD_PTB_LED_OK, HIGH);
    } else {
      addError();
    }
  }
  else{
    if(timerHasDigit('1')){
      ptb_status = true;
      digitalWrite(MOD_PTB_LED_OK, HIGH);
    } else {
      addError();
    }
  }
}

//Loop function that checks button status changes of the module
void modulePTBLoop() {
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





// --------------------------------------------------------------------- //
// -------------------------- MODULE - GENIUS -------------------------- //
// --------------------------------------------------------------------- //

Color* color_sequence_show;
Color* color_sequence_result;
int gen_current_stage = 0;
int gen_current_press = 0;
int gen_stage_last_blink = 0;
long gen_ms_last_blink = 0;
bool gen_btn_r = false;
bool gen_btn_g = false;
bool gen_btn_b = false;
bool gen_btn_y = false;

void moduleGENStart(){
  gen_current_stage = 0;
  gen_current_press = 0;
  gen_stage_last_blink = 0;
  gen_ms_last_blink = 0;
  gen_btn_r = false;
  gen_btn_g = false;
  gen_btn_b = false;
  gen_btn_y = false;
  color_sequence_show = generateRandomColorSequence(mod_gen_length);
  moduleGENGenerateResult();
}

void moduleGENLoop(){
  if(gen_status == true) return;

  moduleGENBlinkRunner();
  moduleGENButtonChecker();
}

void moduleGENButtonChecker(){
  int gen_btn_r_state = digitalRead(MOD_GEN_BTN_R);
  int gen_btn_g_state = digitalRead(MOD_GEN_BTN_G);
  int gen_btn_b_state = digitalRead(MOD_GEN_BTN_B);
  int gen_btn_y_state = digitalRead(MOD_GEN_BTN_Y);

  if (gen_btn_r_state == HIGH && gen_btn_r == false){
    gen_btn_r = true;
    moduleGENPressButon(RED);
  }
  if (gen_btn_r_state == LOW && gen_btn_r == true){
    gen_btn_r = false;
    moduleGENBlinkClear();
  }

  if (gen_btn_g_state == HIGH && gen_btn_g == false){
    gen_btn_g = true;
    moduleGENPressButon(GREEN);
  }
  if (gen_btn_g_state == LOW && gen_btn_g == true){
    gen_btn_g = false;
    moduleGENBlinkClear();
  }

  if (gen_btn_b_state == HIGH && gen_btn_b == false){
    gen_btn_b = true;
    moduleGENPressButon(BLUE);
  }
  if (gen_btn_b_state == LOW && gen_btn_b == true){
    gen_btn_b = false;
    moduleGENBlinkClear();
  }

  if (gen_btn_y_state == HIGH && gen_btn_y == false){
    gen_btn_y = true;
    moduleGENPressButon(YELLOW);
  }
  if (gen_btn_y_state == LOW && gen_btn_y == true){
    gen_btn_y = false;
    moduleGENBlinkClear();
  }
}

void moduleGENPressButon(Color color){
  moduleGENBlink(color);
  if(color_sequence_result[gen_current_press] == color){
    gen_ms_last_blink = millis();
    gen_current_press++;

    if(gen_current_press > gen_current_stage){
      gen_current_stage++;
      gen_stage_last_blink = gen_current_stage;
    }

    if(gen_current_stage >= mod_gen_length) {
      gen_status = true;
      digitalWrite(MOD_GEN_LED_OK, HIGH);
      delay(300);
      moduleGENBlinkClear();
    }
  } else {
    gen_current_press = 0;
    gen_current_stage = 0;
    gen_stage_last_blink = 0;
    addError();
    free(color_sequence_show);
    free(color_sequence_result);
    color_sequence_show = generateRandomColorSequence(mod_gen_length);
    moduleGENGenerateResult();
  }
}

void moduleGENBlinkRunner(){
  if ((gen_ms_last_blink + 5000) <= millis()){
    if(true){
      gen_ms_last_blink = millis();
      moduleGENBlink(color_sequence_show[0]);
      gen_stage_last_blink = 0;
      gen_current_press = 0;
    }
  } 
  else if((gen_ms_last_blink + 400) <= millis()){
    if(gen_current_stage > gen_stage_last_blink){
      gen_ms_last_blink = millis();
      moduleGENBlink(color_sequence_show[gen_stage_last_blink + 1]);
      gen_stage_last_blink++;
    }
  }
  else if((gen_ms_last_blink + 300) <= millis()){
    moduleGENBlinkClear();
  }
}

void moduleGENBlink(Color color){
  switch (color) {
    case RED:
      digitalWrite(MOD_GEN_LED_R, HIGH);
      tone(BUZZER_PIN, 1046);
      break;
    case BLUE:
      digitalWrite(MOD_GEN_LED_B, HIGH);
      tone(BUZZER_PIN, 1174);
      break;
    case GREEN:
      digitalWrite(MOD_GEN_LED_G, HIGH);
      tone(BUZZER_PIN, 1318);
      break;
    case YELLOW:
      digitalWrite(MOD_GEN_LED_Y, HIGH);
      tone(BUZZER_PIN, 1396);
      break;
  }
}

void moduleGENBlinkClear(){
  noTone(BUZZER_PIN);
  digitalWrite(MOD_GEN_LED_R, LOW);
  digitalWrite(MOD_GEN_LED_G, LOW);
  digitalWrite(MOD_GEN_LED_B, LOW);
  digitalWrite(MOD_GEN_LED_Y, LOW);
}

void moduleGENGenerateResult(){
  color_sequence_result = (Color*)malloc(mod_gen_length * sizeof(Color));
  bool serial_number_has_vowel = checkSerialNumberHasVowel();
  if(serial_number_has_vowel){
    for (int i = 0; i < mod_gen_length; i++){
      if(errors == 0){
        switch(color_sequence_show[i]){
          case RED:
            color_sequence_result[i] = BLUE;
            break;
          case GREEN:
            color_sequence_result[i] = YELLOW;
            break;
          case BLUE:
            color_sequence_result[i] = RED;
            break;
          case YELLOW:
            color_sequence_result[i] = GREEN;
            break;
        }
      }
      else if (errors == 1){
        switch(color_sequence_show[i]){
          case RED:
            color_sequence_result[i] = YELLOW;
            break;
          case GREEN:
            color_sequence_result[i] = BLUE;
            break;
          case BLUE:
            color_sequence_result[i] = GREEN;
            break;
          case YELLOW:
            color_sequence_result[i] = RED;
            break;
        }
      }
      else if (errors == 2){
        switch(color_sequence_show[i]){
          case RED:
            color_sequence_result[i] = GREEN;
            break;
          case GREEN:
            color_sequence_result[i] = YELLOW;
            break;
          case BLUE:
            color_sequence_result[i] = RED;
            break;
          case YELLOW:
            color_sequence_result[i] = BLUE;
            break;
        }
      }
    }
  }
  else {
    for (int i = 0; i < mod_gen_length; i++){
      if(errors == 0){
        switch(color_sequence_show[i]){
          case RED:
            color_sequence_result[i] = BLUE;
            break;
          case GREEN:
            color_sequence_result[i] = GREEN;
            break;
          case BLUE:
            color_sequence_result[i] = YELLOW;
            break;
          case YELLOW:
            color_sequence_result[i] = RED;
            break;
        }
      }
      else if (errors == 1){
        switch(color_sequence_show[i]){
          case RED:
            color_sequence_result[i] = RED;
            break;
          case GREEN:
            color_sequence_result[i] = YELLOW;
            break;
          case BLUE:
            color_sequence_result[i] = BLUE;
            break;
          case YELLOW:
            color_sequence_result[i] = GREEN;
            break;
        }
      }
      else if (errors == 2){
        switch(color_sequence_show[i]){
          case RED:
            color_sequence_result[i] = YELLOW;
            break;
          case GREEN:
            color_sequence_result[i] = BLUE;
            break;
          case BLUE:
            color_sequence_result[i] = GREEN;
            break;
          case YELLOW:
            color_sequence_result[i] = RED;
            break;
        }
      }
    }
  }
}





// --------------------------------------------------------------------- //
// -------------------------- MODULE - MEMORY -------------------------- //
// --------------------------------------------------------------------- //
int mem_stages_labels[5][4];
int mem_stages_numbers[5];
int mem_stages_correct_positions[5];
int mem_current_stage = 0;
bool mem_btn_1 = false;
bool mem_btn_2 = false;
bool mem_btn_3 = false;
bool mem_btn_4 = false;

void moduleMEMStart(){
  moduleMEMGenerateRandomLabels();
  moduleMEMGenerateRandomNumbers();
  moduleMEMGenerateCorrectPositions();
  moduleMEMDisplay(0);
}

void moduleMEMLoop(){
  if(mem_status == true) return;

  moduleMEMCheckButton();
}

void moduleMEMUnlockModule(){
  mem_status = true;
  digitalWrite(MOD_MEM_LED_OK, HIGH);
}

void moduleMEMCheckButton(){
  int mem_btn_1_state = digitalRead(MOD_MEM_BTN_1);
  int mem_btn_2_state = digitalRead(MOD_MEM_BTN_2);
  int mem_btn_3_state = digitalRead(MOD_MEM_BTN_3);
  int mem_btn_4_state = digitalRead(MOD_MEM_BTN_4);

  if (mem_btn_1_state == HIGH && mem_btn_1 == false){
    mem_btn_1 = true;
    if(mem_stages_correct_positions[mem_current_stage] == 0){
      mem_current_stage++;
      digitalWrite(MOD_MEM_LED_1, HIGH);
      if(mem_current_stage == 5){
        moduleMEMUnlockModule();
        return;
      }
      moduleMEMDisplay(mem_current_stage);
    } else {
      Serial.println("Falha no botao 1");
      mem_current_stage = 0;
      moduleMEMDisplay(0);
      addError();
      digitalWrite(MOD_MEM_LED_1, LOW);
      digitalWrite(MOD_MEM_LED_2, LOW);
      digitalWrite(MOD_MEM_LED_3, LOW);
      digitalWrite(MOD_MEM_LED_4, LOW);
    }
  }
  if (mem_btn_1_state == LOW && mem_btn_1 == true){
    mem_btn_1 = false;
  }

  if (mem_btn_2_state == HIGH && mem_btn_2 == false){
    mem_btn_2 = true;
    if(mem_stages_correct_positions[mem_current_stage] == 1){
      mem_current_stage++;
      digitalWrite(MOD_MEM_LED_2, HIGH);
      if(mem_current_stage == 5){
        moduleMEMUnlockModule();
        return;
      }
      moduleMEMDisplay(mem_current_stage);
    } else {
      Serial.println("Falha no botao 2");
      mem_current_stage = 0;
      moduleMEMDisplay(0);
      addError();
      digitalWrite(MOD_MEM_LED_1, LOW);
      digitalWrite(MOD_MEM_LED_2, LOW);
      digitalWrite(MOD_MEM_LED_3, LOW);
      digitalWrite(MOD_MEM_LED_4, LOW);
    }
  }
  if (mem_btn_2_state == LOW && mem_btn_2 == true){
    mem_btn_2 = false;
  }

  if (mem_btn_3_state == HIGH && mem_btn_3 == false){
    mem_btn_3 = true;
    if(mem_stages_correct_positions[mem_current_stage] == 2){
      moduleMEMDisplay(mem_current_stage);
      mem_current_stage++;
      if(mem_current_stage == 5){
        moduleMEMUnlockModule();
        return;
      }
      digitalWrite(MOD_MEM_LED_3, HIGH);
    } else {
      Serial.println("Falha no botao 3");
      mem_current_stage = 0;
      moduleMEMDisplay(0);
      addError();
      digitalWrite(MOD_MEM_LED_1, LOW);
      digitalWrite(MOD_MEM_LED_2, LOW);
      digitalWrite(MOD_MEM_LED_3, LOW);
      digitalWrite(MOD_MEM_LED_4, LOW);
    }
  }
  if (mem_btn_3_state == LOW && mem_btn_3 == true){
    mem_btn_3 = false;
  }

  if (mem_btn_4_state == HIGH && mem_btn_4 == false){
    mem_btn_4 = true;
    if(mem_stages_correct_positions[mem_current_stage] == 3){
      mem_current_stage++;
      digitalWrite(MOD_MEM_LED_4, HIGH);
      if(mem_current_stage == 5){
        moduleMEMUnlockModule();
        return;
      }
      moduleMEMDisplay(mem_current_stage);
    } else {
      Serial.println("Falha no botao 4");
      mem_current_stage = 0;
      moduleMEMDisplay(0);
      addError();
      digitalWrite(MOD_MEM_LED_1, LOW);
      digitalWrite(MOD_MEM_LED_2, LOW);
      digitalWrite(MOD_MEM_LED_3, LOW);
      digitalWrite(MOD_MEM_LED_4, LOW);
    }
  }
  if (mem_btn_4_state == LOW && mem_btn_4 == true){
    mem_btn_4 = false;
  }
}

void moduleMEMDisplay(int stage){
  char bufferNumber[16];
  char bufferLabels[16];

  snprintf(bufferNumber, sizeof(bufferNumber), "       %d    ", mem_stages_numbers[stage]);
  snprintf(bufferLabels, sizeof(bufferLabels), "%d   %d    %d    %d", mem_stages_labels[stage][0], mem_stages_labels[stage][1], mem_stages_labels[stage][2], mem_stages_labels[stage][3]);

  display_mem.setCursor(0,0);
  display_mem.print(bufferNumber);

  display_mem.setCursor(0,1);
  display_mem.print(bufferLabels);
}

void moduleMEMGenerateRandomLabels(){
  int randomNumber;
  for (int i = 0; i < 5; i++) {
    bool usedNumbers[4] = {false, false, false, false};
    for (int j = 0; j < 4; j++){
      do {
        randomNumber = random(1, 5);
      } while (usedNumbers[randomNumber - 1]);
      mem_stages_labels[i][j] = randomNumber;
      usedNumbers[randomNumber - 1] = true;
    }
  }
}

void moduleMEMGenerateRandomNumbers(){
  for (int i = 0; i < 5; i++) {
    mem_stages_numbers[i] = random(1, 5);
  }
}

void moduleMEMGenerateCorrectPositions(){
  //Stage 0
  switch(mem_stages_numbers[0]){
    case 1:
      mem_stages_correct_positions[0] = 1;
      break;
    case 2:
      mem_stages_correct_positions[0] = 1;
      break;
    case 3:
      mem_stages_correct_positions[0] = 2;
      break;
    case 4:
      mem_stages_correct_positions[0] = 3;
      break;
  }

  //Stage 1
  switch(mem_stages_numbers[1]){
    case 1:
      mem_stages_correct_positions[1] = findIndex(mem_stages_labels[1], 4, 4);
      break;
    case 2:
      mem_stages_correct_positions[1] = mem_stages_correct_positions[0];
      break;
    case 3:
      mem_stages_correct_positions[1] = 0;
      break;
    case 4:
      mem_stages_correct_positions[1] = mem_stages_correct_positions[0];
      break;
  }

  //Stage 2
  switch(mem_stages_numbers[2]){
    case 1:
      mem_stages_correct_positions[2] = findIndex(mem_stages_labels[2], 4, mem_stages_labels[1][mem_stages_correct_positions[1]]);
      break;
    case 2:
      mem_stages_correct_positions[2] = findIndex(mem_stages_labels[2], 4, mem_stages_labels[0][mem_stages_correct_positions[0]]);
      break;
    case 3:
      mem_stages_correct_positions[2] = 2;
      break;
    case 4:
      mem_stages_correct_positions[2] = findIndex(mem_stages_labels[2], 4, 4);
      break;
  }

  //Stage 3
  switch(mem_stages_numbers[3]){
    case 1:
      mem_stages_correct_positions[3] = mem_stages_correct_positions[0];
      break;
    case 2:
      mem_stages_correct_positions[3] = 0;
      break;
    case 3:
      mem_stages_correct_positions[3] = mem_stages_correct_positions[1];
      break;
    case 4:
      mem_stages_correct_positions[3] = mem_stages_correct_positions[1];
      break;
  }

  //Stage 4
  switch(mem_stages_numbers[4]){
    case 1:
      mem_stages_correct_positions[4] = mem_stages_correct_positions[0];
      break;
    case 2:
      mem_stages_correct_positions[4] = mem_stages_correct_positions[1];
      break;
    case 3:
      mem_stages_correct_positions[4] = mem_stages_correct_positions[3];
      break;
    case 4:
      mem_stages_correct_positions[4] = mem_stages_correct_positions[2];
      break;
  }
}





// ----------------------------------------------------------------------------- //
// -------------------------- ARDUINO BASIC FUNCTIONS -------------------------- //
// ----------------------------------------------------------------------------- //
bool game_btn_1_status = false;
bool game_btn_2_status = false;

void setup() {
  Serial.begin(9600);

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
  pinMode(MOD_GEN_LED_B, OUTPUT);
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
  pinMode(MOD_GEN_BTN_B, INPUT);
  pinMode(MOD_PTB_BTN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(MOD_MEM_BTN_1, INPUT);
  pinMode(MOD_MEM_BTN_2, INPUT);
  pinMode(MOD_MEM_BTN_3, INPUT);
  pinMode(MOD_MEM_BTN_4, INPUT);

  display_main.init();
  display_main.backlight();
  display_main.setCursor(0,0);
  display_main.print("Nova Bomba");
  display_main.setCursor(0,1);
  display_main.print("Iniciante");

  display_mem.init();
  display_mem.backlight();
}

void loop() {
  if(serial_number != ""){
    //----------Game Running----------//
    updateTimer();
    updateBuzzer();
    modulePTBLoop();
    moduleGENLoop();
    checkModulesUnlocked();
    moduleMEMLoop();
    digitalWrite(MOD_MEM_LED_OK, HIGH);
  } else {
    //----------Game Not Running----------//
    if(game_btn_1_status == false && digitalRead(GAME_BTN_1) == HIGH){
      game_btn_1_status = true;
      startGame();
    }
    else if(game_btn_1_status == true && digitalRead(GAME_BTN_1) == LOW){
      game_btn_1_status = false;
    }

    if(game_btn_2_status == false && digitalRead(GAME_BTN_2) == HIGH){
      game_btn_2_status = true;
      gameChangeDifficulty();
    }
    else if(game_btn_2_status == true && digitalRead(GAME_BTN_2) == LOW){
      game_btn_2_status = false;
    }
  }
}