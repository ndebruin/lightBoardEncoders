#include <Keypad.h>
#include "Keyboard.h"

const byte ROWS = 5; 
const byte COLS = 8; 
char hexaKeys[ROWS][COLS] = {
  {'g','/','Q','7','8','9','c','O'},
  {'q','d','t','4','5','6','T','@'},
  {'b','R','s','1','2','3','&','F'},
  {'h','o','o','-','0','+','r','o'},
  {'G','L','B','C','.','E','o','o'}
};
byte rowPins[ROWS] = {6, 5, 4, 3, 2}; 
byte colPins[COLS] = {A0, 16, 15, 14, 10, 9, 8, 7};
Keypad RFU = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

void setup() {
   Serial.begin(9600);
}

void loop() {
  char RFUKey = RFU.getKey();
  //All Key Commands, Left to Right, Top to Bottom
  if (RFUKey == 'g'){
    Keyboard.press(KEY_LEFT_CTRL);
    Keyboard.press('g');
    delay(10);
    Keyboard.releaseAll();
  }
  if (RFUKey == '/'){
    Keyboard.press('/');
    delay(10);
    Keyboard.releaseAll();
  }
  if (RFUKey == 'Q'){
    Keyboard.press('q');
    delay(10);
    Keyboard.releaseAll();
  }
  if (RFUKey == '7'){
    Keyboard.press('7');
    delay(10);
    Keyboard.releaseAll();
  }
  if (RFUKey == '8'){
    Keyboard.press('8');
    delay(10);
    Keyboard.releaseAll();
  }
  if (RFUKey == '9'){
    Keyboard.press('9');
    delay(10);
    Keyboard.releaseAll();
  }
  if (RFUKey == 'O'){
    Keyboard.press('g');
    delay(10);
    Keyboard.releaseAll();
  }
  if (RFUKey == 'q'){
    Keyboard.press(KEY_LEFT_CTRL);
    Keyboard.press('g');
    Keyboard.press('0');
    delay(5);
    Keyboard.press(KEY_RETURN);
    delay(10);
    Keyboard.releaseAll(); 
  }
  if (RFUKey == 'd'){
    Keyboard.press('h');
    delay(10);
    Keyboard.releaseAll();
  }
  if (RFUKey == 't'){
    Keyboard.press('i');
    delay(10);
    Keyboard.releaseAll();
  }
  if (RFUKey == '4'){
    Keyboard.press('4');
    delay(10);
    Keyboard.releaseAll();
  }
  if (RFUKey == '5'){
    Keyboard.press('5');
    delay(10);
    Keyboard.releaseAll();
  }
  if (RFUKey == '6'){
    Keyboard.press('6');
    delay(10);
    Keyboard.releaseAll();
  }
  if (RFUKey == 'T'){
    Keyboard.press('t');
    delay(10);
    Keyboard.releaseAll();
  }
  if (RFUKey == '@'){
    Keyboard.press('a');
    delay(10);
    Keyboard.releaseAll();
  }
  if (RFUKey == 'b'){
    Keyboard.press(KEY_LEFT_CTRL);
    Keyboard.press(' ');
    delay(10);
    Keyboard.releaseAll();
  }
  if (RFUKey == 'R'){
    Keyboard.press('r');
    delay(10);
    Keyboard.releaseAll();
  }
  if (RFUKey == 's'){
    Keyboard.press('s');
    delay(10);
    Keyboard.releaseAll();
  }
  if (RFUKey == '1'){
    Keyboard.press('1');
    delay(10);
    Keyboard.releaseAll();
  }
  if (RFUKey == '2'){
    Keyboard.press('2');
    delay(10);
    Keyboard.releaseAll();
  }
  if (RFUKey == '3'){
    Keyboard.press('3');
    delay(10);
    Keyboard.releaseAll();
  }
  if (RFUKey == '&'){
    Keyboard.press('=');
    delay(10);
    Keyboard.releaseAll();
  }
  if (RFUKey == 'F'){
    Keyboard.press('f');
    delay(10);
    Keyboard.releaseAll();
  }
  if (RFUKey == 'h'){
    Keyboard.press(KEY_LEFT_CTRL);
    Keyboard.press(' ');
    delay(10);
    Keyboard.releaseAll();
  }
  if (RFUKey == '-'){
    Keyboard.press('-');
    delay(10);
    Keyboard.releaseAll();
  }
  if (RFUKey == '0'){
    Keyboard.press('0');
    delay(10);
    Keyboard.releaseAll();
  }
  if (RFUKey == '+'){
    Keyboard.press('=');
    delay(10);
    Keyboard.releaseAll();
  }
  if (RFUKey == 'r'){
    Keyboard.press('n');
    Keyboard.press(KEY_RETURN);
    delay(10);
    Keyboard.releaseAll();
  }
  if (RFUKey == 'G'){
    Keyboard.press(' ');
    delay(10);
    Keyboard.releaseAll();
  }
  if (RFUKey == 'L'){
    Keyboard.press(KEY_F1);
    delay(10);
    Keyboard.releaseAll();
  }
  if (RFUKey == 'B'){
    Keyboard.press(KEY_F2);
    delay(10);
    Keyboard.releaseAll();
  }
  if (RFUKey == 'C'){
    Keyboard.press(KEY_BACKSPACE);
    delay(10);
    Keyboard.releaseAll();
  }
  if (RFUKey == '.'){
    Keyboard.press('.');
    delay(10);
    Keyboard.releaseAll();
  }
  if (RFUKey == 'E'){
    Keyboard.press(KEY_RETURN);
    delay(10);
    Keyboard.releaseAll();
  }
}
