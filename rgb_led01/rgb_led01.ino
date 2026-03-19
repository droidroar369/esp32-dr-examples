/*
* Programa que parpadea un led RGB entre 3 colores ()
*/

#define RGB_LED 48
#define BRIGHTNESS 255

uint8_t col=0;

void setup() {
  pinMode(RGB_LED, OUTPUT);
}

void loop() {
  switch(col) {
    case 0: //Red
    rgbLedWrite(RGB_LED, BRIGHTNESS, 0, 0);
    break;
    case 1: //Green
    rgbLedWrite(RGB_LED, 0, BRIGHTNESS, 0);
    break;
    case 2: //Blue
    rgbLedWrite(RGB_LED, 0, 0, BRIGHTNESS);
    break;
  }
  col= (col+1)%3;
  delay(1000);
}
