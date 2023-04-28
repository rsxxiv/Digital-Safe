#include <LiquidCrystal.h>
#include <Keypad.h>
#include <Servo.h>
#include "SafeState.h"
// #include "icons.h"
#include "pitches.h"


/* Locking mechanism definitions */
#define SERVO_PIN 6
#define SERVO_LOCK_POS   0
#define SERVO_UNLOCK_POS 90
Servo lockServo;



/* Display pin defination */
// LiquidCrystal Serial(12, 11, 10, 9, 8, 7);

// Buzzer and PIR Setup defination
#define MOTION_SENSOR_PIN 13 // Arduino pin connected to the OUTPUT pin of motion sensor
#define BUZZER_PIN 12 // Arduino pin connected to Buzzer's pin
int state = LOW;
int value = 0;


/* Keypad setup */
const byte KEYPAD_ROWS = 4;
const byte KEYPAD_COLS = 4;
byte rowPins[KEYPAD_ROWS] = {5, 4, 3, 2};
byte colPins[KEYPAD_COLS] = {A3, A2, A1, A0};
char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);

/* SafeState stores the secret code in EEPROM */
SafeState safeState;

void lock() {
  lockServo.write(SERVO_LOCK_POS);
  safeState.lock();
}

void unlock() {
  lockServo.write(SERVO_UNLOCK_POS);
}

void showStartupMessage() {
  // Serial.setCursor(2, 0);

  Serial.println("\n");
  Serial.println("\n");
  Serial.println("\n");
  Serial.println("\n");
  Serial.println("\n");
  Serial.println("Welcome MITS!");
  // delay(500);

  // Serial.setCursor(0, 2);
  Serial.println("Digital Safe v1.0");
  // String message = "Digital Safe v1.0";
  // for (byte i = 0; i < message.length(); i++) {
  //   Serial.println(message[i]);
  //   delay(100);
  // }
  delay(2000);
}

String inputSecretCode() {
  // Serial.setCursor(5, 1);
  Serial.println("[____]");
  // Serial.setCursor(6, 1);
  String result = "";
  while (result.length() < 4) {
    char key = keypad.getKey();
    if (key >= '0' && key <= '9') {
      Serial.println('*');
      result += key;
    }
  }
  return result;
}

// void showWaitScreen(int delayMillis) {
//   // Serial.setCursor(2, 1);
//   Serial.println("[..........]");
//   // Serial.setCursor(3, 1);
//   for (byte i = 0; i < 10; i++) {
//     delay(delayMillis);
//     Serial.println("=");
//   }
// }

bool setNewCode() {
  Serial.flush();
  // Serial.setCursor(0, 0);
  Serial.println("Enter new code:");
  String newCode = inputSecretCode();

  Serial.flush();
  // Serial.setCursor(0, 0);
  Serial.println("Confirm new code");
  String confirmCode = inputSecretCode();

  if (newCode.equals(confirmCode)) {
    safeState.setCode(newCode);
    return true;
  } 
  else {
    Serial.flush();
    // Serial.setCursor(1, 0);
    Serial.println("Code mismatch");
    // Serial.setCursor(0, 1);
    Serial.println("Safe not locked!");
    delay(2000);
    return false;
  }
}

void showUnlockMessage() {
  Serial.flush();
  // Serial.setCursor(0, 0);
  // Serial.print(ICON_UNLOCKED_CHAR);
  // Serial.setCursor(4, 0);
  Serial.println("Unlocked!");
  // Serial.setCursor(15, 0);
  // Serial.print(ICON_UNLOCKED_CHAR);
  delay(1000);
}

void safeUnlockedLogic() {
  
  Serial.flush();

  // Serial.setCursor(0, 0);
  // Serial.print(ICON_UNLOCKED_CHAR);
  // Serial.setCursor(2, 0);
  Serial.println(" # to lock");
  // Serial.setCursor(15, 0);
  // Serial.print(ICON_UNLOCKED_CHAR);

  bool newCodeNeeded = true;

  if (safeState.hasCode()) {
    // Serial.setCursor(0, 1);
    Serial.println("  A = new code");
    newCodeNeeded = false;
  }

  auto key = keypad.getKey();
  while (key != 'A' && key != '#') {
    key = keypad.getKey();
  }

  bool readyToLock = true;
  if (key == 'A' || newCodeNeeded) {
    readyToLock = setNewCode();
  }

  if (readyToLock) {
    Serial.flush();
    // Serial.setCursor(5, 0);
    // Serial.print(ICON_UNLOCKED_CHAR);
    Serial.println(" ");
    // Serial.print(ICON_RIGHT_ARROW);
    Serial.println(" ");
    // Serial.print(ICON_LOCKED_CHAR);

    safeState.lock();
    lock();
    // showWaitScreen(100);
  }
}


int melody[] = {
  NOTE_E5, NOTE_G5,
  NOTE_E5, NOTE_G5,
  NOTE_E5, NOTE_G5,
  NOTE_E5, NOTE_G5,
  NOTE_E5, NOTE_G5,
  NOTE_E5, NOTE_G5,
  NOTE_E5, NOTE_G5,
  NOTE_E5, NOTE_G5,
  NOTE_E5, NOTE_G5,
};

// note durations: 4 = quarter note, 8 = eighth note, etc, also called tempo:
int noteDurations[] = {
  4, 4,
  4, 4,
  4, 4,
  4, 4,
  4, 4,
  4, 4,
  4, 4,
  4, 4,
  4, 4,
};
int noteDurationsGood[] = {
  8, 16,  12,
  16, 8,  16,
  8, 16,  12,
};

int melodyGood[] = {
  NOTE_B7, NOTE_A3, NOTE_CS8,
  NOTE_B7, NOTE_A3, NOTE_CS8,
  NOTE_B7, NOTE_A3, NOTE_CS8,
};


void buzzer(){
  // iterate over the notes of the melody:
  int size = sizeof(noteDurations) / sizeof(int);

  for (int thisNote = 0; thisNote < size; thisNote++) {
    // to calculate the note duration, take one second divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(BUZZER_PIN, melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(BUZZER_PIN);
  }
}
void buzzerGood(){
  // iterate over the notes of the melody:
  int size = sizeof(noteDurationsGood) / sizeof(int);

  for (int thisNote = 0; thisNote < size; thisNote++) {
    // to calculate the note duration, take one second divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurationsGood[thisNote];
    tone(BUZZER_PIN, melodyGood[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(BUZZER_PIN);
  }
}

void safeLockedLogic() {
  Serial.flush();
  // Serial.setCursor(0, 0);
  // Serial.print(ICON_LOCKED_CHAR);
  Serial.println(" Safe Locked! ");
  Serial.println(" Enter Password To Unlock! ");
  // Serial.print(ICON_LOCKED_CHAR);

  String userCode = inputSecretCode();
  bool unlockedSuccessfully = safeState.unlock(userCode);
  // showWaitScreen(200);

  
    if (unlockedSuccessfully) {
      showUnlockMessage();
      buzzerGood();
      unlock();
    }  
    else if (userCode == "6666"){
      delay(2000);
      Serial.println("Motion Detected");
          safeState.lock();
          lock();
          buzzer();    
  }
    else{
      Serial.flush();
      // Serial.setCursor(0, 0);
      Serial.println("Access Denied!");
      buzzer();
      // showWaitScreen(1000);
  }
}


void setup() {
  // Serial.begin(16, 2);
  // init_icons(lcd);

  lockServo.attach(SERVO_PIN);

  /* Make sure the physical lock is sync with the EEPROM state */
  // Serial.begin(115200);
  Serial.begin(9600);                    // initialize serial
  Serial.flush();
  pinMode(MOTION_SENSOR_PIN, INPUT); // set arduino pin to input mode

  if (safeState.locked()) {
    lock();
  } else {
    unlock();
  }


  showStartupMessage();
}

void loop() {

  value = digitalRead(MOTION_SENSOR_PIN);  

  if (safeState.locked()) {
    safeLockedLogic();
  } else {
    safeUnlockedLogic();
  }
}
