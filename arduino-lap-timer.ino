#include <LiquidCrystal.h>

// PIN CONFIGURATION
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
int laserPin = 13;
int buttonPin2 = 8;
int speakerPin = 9;
int ledPin3 = 10; // green

// States
const int STATE_PAUSE = 0;
const int STATE_COUNTDOWN = 1;
const int STATE_READY = 2;
const int STATE_RUNNING = 3;
const int STATE_ENDED = 4;

int state = STATE_PAUSE;

long lastHit = 0; //time of the last hit in the barrier (absolut)
boolean laserBlocked = false;
long laserBlockTime = 0; //Absolute

long startTime = 0; // Absolute
long totalTime = 0;
long lastTime = 0;

int laserLightLevelThreshold = 10;
int laserTimeSensitivity = 100;

// Button variables
int buttonState = 0;
int lastButtonState = 0;
long lastButtonPressTime = 0;
boolean buttonWasPressed = false;
boolean buttonIsHeld = false;

// LED Timer
long timeLEDGreen = 0;

void setup()
{
  pinMode(laserPin, OUTPUT);
  pinMode(buttonPin2, INPUT);
  pinMode(speakerPin, OUTPUT);
  pinMode(ledPin3, OUTPUT);
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);


  lcd.setCursor(0, 0);
  lcd.print("Arduino");
  lcd.setCursor(0, 1);
  lcd.print("Laser Lap Timer  ");
  
  delay(1500);
  
  lcd.setCursor(0, 0);
  lcd.print("Roadrunners RC");
  lcd.setCursor(0, 1);
  lcd.print("                ");

  delay(1500);
}

void loop()
{
  handleInput();

  switch (state){
    case STATE_PAUSE:
      statePaused(); 
      break;
    case STATE_COUNTDOWN: 
      stateCountdown(); 
      break;
    case STATE_READY:
      stateReady();
      break;
    case STATE_RUNNING:
      stateRunning();
      break;
    case STATE_ENDED:
      stateEnded();
      break;
  }
}

int getLightLevel()
{
  //mesasure light
  int lightLevel = analogRead(0);
  lightLevel = map(lightLevel, 0, 900, 255, 0);
  lightLevel = constrain(lightLevel, 0, 255);

  return lightLevel;
}

void statePaused()
{
  digitalWrite(ledPin3, LOW);
  digitalWrite(laserPin,LOW);
  lcd.setCursor(0, 0);
  lcd.print("Press button    ");
  lcd.setCursor(0, 1);
  lcd.print("to start        ");
}

void stateReady()
{
  lcd.setCursor(0, 0);
  lcd.print("Go...           ");
  lcd.setCursor(0, 1);
  lcd.print("                ");
    
  if (getLightLevel() < laserLightLevelThreshold) {
    if (!laserBlocked) {
      laserBlocked = true;
      laserBlockTime = millis(); 
    } else {
      if (millis() - laserBlockTime > laserTimeSensitivity) {
          // turn off all LED
          digitalWrite(ledPin3, LOW);
          state = STATE_RUNNING;
          lastTime = laserBlockTime;
          startTime = laserBlockTime;
      }
    }
  } else {
    laserBlocked = false;
  }
}

void stateRunning()
{
  // react to light
  if (getLightLevel() < laserLightLevelThreshold) {
    if (!laserBlocked) {
      laserBlocked = true;
      laserBlockTime = millis(); 
    } else {
      if (millis() - laserBlockTime > laserTimeSensitivity) {
          lastTime = laserBlockTime;
      }
    }
  } else {
    laserBlocked = false;
  }

  // Display the time and best / last round
  displaySecondLine();
  displayFirstLine();  
}

void stateEnded()
{
  lcd.setCursor(0, 0);
  lcd.print("Final ");
  lcd.print(lastTime - startTime);
  lcd.print("ms");
  lcd.print("                       ");  
  lcd.setCursor(0, 1);
  lcd.print("Hold to reset          ");
}

void stateCountdown()
{
  //switch laser on
  digitalWrite(laserPin,HIGH);

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Ready...          ");

  playNote('c',300);

  delay(1000);

  lcd.setCursor(0, 0);
  lcd.print("Set...          ");

  playNote('c',300);

  delay(1000);

  lcd.setCursor(0, 0);
  lcd.print("Go...          ");
  digitalWrite(ledPin3, HIGH);
  digitalWrite(laserPin,HIGH);

  playNote('g',600);

  state = STATE_READY;
}

void displayFirstLine()
{
  lcd.setCursor(0, 0);

  lcd.print(" ");
  lcd.print(millis() - startTime);
  lcd.print("ms");
  lcd.print("                       ");
}

void displaySecondLine()
{
  lcd.setCursor(0, 1);

  lcd.print("Last ");
  lcd.print(lastTime - startTime);
  lcd.print("ms");
  lcd.print("                       ");
}

void handleInput()
{
  buttonWasPressed = false;
  buttonIsHeld = false;
  
  buttonState = !digitalRead(buttonPin2);
  if(buttonState != lastButtonState) {
    if(buttonState == HIGH) {
      buttonWasPressed = true;
      lastButtonPressTime = millis();
    }
  }
  
  if(buttonState == HIGH && millis() - lastButtonPressTime > 2000) {
    buttonIsHeld = true;
  }
  
  lastButtonState = buttonState;
  
  if(buttonWasPressed) {
    if (state == STATE_PAUSE) {
      state = STATE_COUNTDOWN;
    } else if (state == STATE_RUNNING) {
      digitalWrite(laserPin,LOW);
      state = STATE_ENDED;
    }
  } else if (buttonIsHeld) {
    if (state == STATE_ENDED) {
        state = STATE_COUNTDOWN;
    }
  }
}

void playTone(int tone, int duration) 
{
  for (long i = 0; i < duration * 1000L; i += tone * 2) {
    digitalWrite(speakerPin, HIGH);
    delayMicroseconds(tone);
    digitalWrite(speakerPin, LOW);
    delayMicroseconds(tone);
  }
}

void playNote(char note, int duration)
{
  char names[] = { 
    'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C'   };
  int tones[] = { 
    1915, 1700, 1519, 1432, 1275, 1136, 1014, 956   };

  // play the tone corresponding to the note name
  for (int i = 0; i < 8; i++) {
    if (names[i] == note) {
      playTone(tones[i], duration);
    }
  }
}
