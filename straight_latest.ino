/*
 *  Arduino Morse Decoder 
 *  (c) Craig McIntyre 2023 
 *  Licensed under GPL 3.0
*/

#include <Keyboard.h>


int unit = 250;                               // dit unit. Dah is 3x, space is 7x - CHANGE THIS TO CHANGE OVERALL SPEED, LOWER IS FASTER


unsigned long signal_valid;
unsigned long timer1, timer2, timer3, timer4;   // timing variables
int inputPin = 4;                             // input pin for push button
int morsePin = 15;                             // output pin for speaker / piezo (used with tone)
int toneFreq = 700;                            // pitch frequency in Hz (standard Morse pitch)

String code = "";                             // string to hold letters
int space = true;                             // boolean to prevent multiple spaces
int first_space = true;                       // boolean to prevent leading space


void setup() {
  Keyboard.begin();
//  Serial.begin(9600); if needed for debugging
  pinMode(inputPin, INPUT_PULLUP);
  pinMode(morsePin, OUTPUT);
}

void loop() {

NextDotDash:

  timer3 = millis();                          // space timer checks time from last press
  
  // Wait for key press
  while (digitalRead(inputPin) == HIGH) {}
  delay(10); // Software Debounce = - if button triggers for less than n ms...
  if (digitalRead(inputPin) == HIGH) {     // ... it must be non-press, keep waiting

    goto NextDotDash; 
  }

  space = false;                            // reset to allow spaces to occur
  timer1 = millis();                        // letter timer for letter spacing
  
  // --- TONE START ---
  tone(morsePin, toneFreq);                 // start tone on key press

  // Wait for key release
  while (digitalRead(inputPin) == LOW) {}
  
  timer2 = millis();                            
  
  // --- TONE STOP ---
  noTone(morsePin);                         // stop tone on key release
  
  // Software debounce for release of key
  delay(10);
  
  signal_valid = timer2 - timer1;           // time for which button was pressed

  if (signal_valid > 20) {                  // ignore presses under 20ms
    code += morsify();                      // function to read dot or dash
  }

  while ((millis() - timer2) < unit * 3) {  // move to next letter if nothing received
    timer4 = millis();
    if ((timer4 - timer3) > unit * 7) {     // add a space if enough time has elapsed
      if (space == false && first_space == false) {
        Keyboard.print(" ");
        space = true;
      }
    }
    if (digitalRead(inputPin) == LOW) {
      goto NextDotDash;
    }
  }
  decoder();                                // function to decipher code into alphabet
}

char morsify() {
  if (signal_valid < unit) {
    return '0';                             // dot if signal is short
  } else {
    return '1';                             // dash if signal is long
  }
}

void decoder() {
  
  static String letters[] = {"01", "1000", "1010", "100", "0", "0010", "110",
                             "0000", "00", "0111", "101", "0100", "11", "10",
                             "111", "0110", "1101","010", "000", "1", "001", 
                             "0001", "011", "1001", "1011", "1100", "E"};

  static String numbers[] = {"11111", "01111", "00111", "00011", "00001",
                             "00000", "10000", "11000", "11100", "11110", "E"};
                             
  int i = 0;                                // counter for letters loop
  int j = 0;                                // counter for numbers loop

  if (code == "010101") {                   // manual checks for punctuation           
    Keyboard.print(".");
  }
  else if (code == "110011") {
    Keyboard.print(",");
  }
  else if (code == "001100") {
    Keyboard.print("?");
  }
  else if (code == "10010") {
    Keyboard.print("/");
  }
  else if (code == "011010") {
    Keyboard.print("@");
  }
  else {
    while (letters[i] != "E") {             // loops letter array for match
      if (letters[i] == code) {     
        Keyboard.print(char('A' + i));      // adds index of array to 'A' to give letter
        break;
      }
      i++;
    }
    while (numbers[j] != "E") {             // loops number array for match
      if (numbers[j] == code) {
        Keyboard.print(0 + j);              // adds index of array to 0 to give number
        break;
      }
      j++;
    }
    if (letters[i] == "E" && numbers[j] == "E") { // hash if character not found
      Keyboard.print("#");
    }
  }
  code = "";                                // empty the morse code variable
  first_space = false;                      // avoid space on startup
}
