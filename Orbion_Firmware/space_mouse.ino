#include "U8glib.h"
#include <Adafruit_NeoPixel.h>
#include "icon.h"
#include <EEPROM.h>
#include <Mouse.h>
#include <Keyboard.h>
#include <Wire.h>
#include <Encoder.h>
#define ENCODER_A 10
#define ENCODER_B 9
#define NEO_L 5
#define NEO_R 11

U8GLIB_SH1106_128X64 disp(U8G_I2C_OPT_NO_ACK);
Adafruit_NeoPixel stripL = Adafruit_NeoPixel(3, NEO_L, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel stripR = Adafruit_NeoPixel(3, NEO_R, NEO_GRB + NEO_KHZ800);
/////////////// Encoder ////////////////////
Encoder myEnc(ENCODER_A, ENCODER_B);
long oldScroll = -999;
int encBut = 12;    //Pin encoder Button                    <------------
int encBefClick = 0;

/////////////// JoyStick ///////////////////

int horzPin = A6;         // Pin Analog output of X        <------------
int vertPin = A7 ;        // Pin Analog output of Y        <------------
int joyButt = 7;         // Pin Joystick Button           <------------

int moved = 0;
int YZero, XZero;
int YValue, XValue;
int sens = 0;
int used = 0;
int lastused = 0;
int offsetJoyX = 15;    // set this value if the joystick moves by itself
int offsetJoyY = 15;    // set this value if the joystick moves by itself

unsigned long tim, h, tim1, h1, tim2, h2, scrollTimeOld;
long scroll;
/////////////// Rear Button  ///////////////

int butFun = 8;   // Pin rear button                      <------------
int butFunBef = 0;
char arButt [36] = {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'z', 'x', 'c', 'v', 'b', 'n', 'm', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0'};
char cb;

/////////////// Mouse Axis ///////////////////////

int invertMouseX = -1;
int invertMouseY = 1;
int inv = 0;
int smooth = 0;
/////////////// MENU ////////////////////////

int reset = LOW;
int sel = 0;
int oldSel = -99;
int oldT = 0;
int exi = LOW;
int first = LOW;
int item = 0;
int epr = 0;
uint32_t menuColor = stripL.Color(0, 80, 255);

void setup()
{

  //////////////////////////////////////////////////////////////////////////// PIN //////////////////////////////////////

  pinMode(horzPin, INPUT_PULLUP);
  pinMode(vertPin, INPUT_PULLUP);
  pinMode(encBut, INPUT_PULLUP);
  pinMode(butFun, INPUT_PULLUP);
  pinMode(joyButt, INPUT_PULLUP);

  YZero = analogRead(vertPin);
  XZero = analogRead(horzPin);

  //////////////////////////////////////////////////////////////////////////// INITIALIZE ///////////////////////////////

  Mouse.begin();
  Keyboard.begin();
  disp.setRot180();
  disp.setFont(u8g_font_fub14r);
  Serial.begin(9600);
  stripL.begin();
  stripR.begin();
  stripL.fill(stripL.Color(0, 255, 0), 0, 3);
  stripL.show();
  stripR.fill(stripR.Color(0, 255, 0), 0 , 3);
  stripR.show();
  delay(800);
  neoStart();
}

void loop()
{
  //selSense(&sens);
  sens = (EEPROM.read(0)) * 10;
  YValue = analogRead(vertPin) - YZero;
  XValue = analogRead(horzPin) - XZero;


  //////////////////////////////////////////////////////////////////////////// SCROLL ///////////////////////////////////

  scroll = myEnc.read();
  if (scroll != oldScroll) {
    if (scroll < oldScroll)
    {
      if (EEPROM.read(10) == 4)
      {
        Keyboard.press(KEY_LEFT_ALT);
      }
      Mouse.move(0, 0, 1);
    }
    else
    {
      if (EEPROM.read(10) == 4)
      {
        Keyboard.press(KEY_LEFT_ALT);
      }
      Mouse.move(0, 0, -1);
    }
    oldScroll = scroll;
  }
  //////////////////////////////////////////////////////////////////////////// REAR BUTTON ///////////////////////////////

  if ((digitalRead(butFun) == 0) && (butFunBef == 0))
  {
    butFunBef = 1;
    if (EEPROM.read(20) <= 35)
    {
      Keyboard.press(arButt [EEPROM.read(20)]);
    }
    if (EEPROM.read(20) >= 36)
    {
      epr = 20;
      selButt(&epr);
    }
  }
  else if (digitalRead(butFun) && (butFunBef))
  {
    butFunBef = 0;
    tim2 = millis() - h2;
    if (tim2 > 50)
    {
      h2 = millis();
      Mouse.release(MOUSE_MIDDLE);
      Keyboard.releaseAll();
    }
  }

  //////////////////////////////////////////////////////////////////////////// CENTRAL BUTTON ///////////////////////////////

  if ((digitalRead(encBut) == 0) && (encBefClick == 0))
  {
    encBefClick = 1;
    if (EEPROM.read(30) <= 35)
    {
      Keyboard.press(arButt [EEPROM.read(30)]);
    }
    if (EEPROM.read(30) >= 36)
    {
      epr = 30;
      selButt(&epr);
    }
  }
  else if (digitalRead(encBut) && (encBefClick))
  {
    encBefClick = 0;
    tim2 = millis() - h2;
    if (tim2 > 50)
    {
      h2 = millis();
      Mouse.release(MOUSE_MIDDLE);
      Keyboard.releaseAll();
    }
  }

  //////////////////////////////////////////////////////////////////////////// ORBIT & PAN //////////////////////////////
  if (EEPROM.read(10) == 0)
  {
    smooth = 0;
  }
  else
  {
    smooth = 30;
  }

  tim = millis() - h;
  if (tim > smooth)
  {
    h = millis();
    if ((YValue > offsetJoyY) || (YValue < (-offsetJoyY)))
    {
      selModes();
      Mouse.move(0, (invertMouseY * (YValue / sens)), 0);
      moved = 1;
    }

    if ((XValue > offsetJoyX) || (XValue < (-offsetJoyX)))
    {
      selModes();
      Mouse.move((invertMouseX * (XValue / sens)), 0, 0);
      moved = 1;
    }

    if ( (YValue <= offsetJoyY) && (YValue >= (-offsetJoyY))  &&  (XValue <= offsetJoyX) && (XValue >= (-offsetJoyX)))
    {
      if ( (digitalRead(encBut) == 1) && (digitalRead(butFun) == 1) || (EEPROM.read(10) == 4) )
      {
        Keyboard.releaseAll();
        tim1 = millis() - h1;
        if (tim1 > 120)
        {
          h1 = millis();
          Mouse.release(MOUSE_MIDDLE);
          Mouse.release(MOUSE_LEFT);
        }
      }

    }
  }


  //////////////////////////////////////////////////////////////////////////// MAIN /////////////////////////////////////


  if (first == LOW)
  {
    start(&first);
  }
  if (digitalRead(joyButt) == LOW)
  {
    delay(300);
    Keyboard.releaseAll();
    Mouse.release(MOUSE_MIDDLE);
    Mouse.release(MOUSE_LEFT);
    Mouse.release(MOUSE_RIGHT);
    do
    {
      rotaryMenu(&sel, 0, 4, 1, 150, 3);
      menu(&sel, &exi, &first);
    }
    while (exi == LOW);
    delay(300);
  }
  exi = LOW;
  sel = 0;
}

void start(int* f)
{
  disp.firstPage();
  do {
    disp.drawBitmapP( 0, 0, 128 / 8, 64, faq);
  } while ( disp.nextPage() );
  *f = 1;
}

///////////////////////////////////////////////////////////////////////////  MENU /////////////////////////////////////

void rotaryMenu(int* s, int minV, int maxV, int increment, int wait, int error)
{
  scroll = myEnc.read();

  if (scroll != oldScroll) {
    if (scroll > oldScroll + error  && (millis() - scrollTimeOld > wait)) {
      scrollTimeOld = millis();
      oldScroll = scroll;
      if ((*s - increment) >= minV)
        *s = *s - increment;
    }
    else if (scroll + error < oldScroll  && (millis() - scrollTimeOld > wait)) {
      scrollTimeOld = millis();
      oldScroll = scroll;
      if ((*s + increment) <= maxV)
        *s = *s + increment;
    }
  }
}

void menu(int* s, int* e, int* f)
{
  if (*s != oldSel) {
    stripR.fill(stripR.Color(0, 0, 0), 0 , 3);
    stripL.fill(stripL.Color(0, 0, 0), 0 , 3);
    switch (*s)
    {
      case 0:
        disp.firstPage();
        do {
          disp.drawStr(13, 28, "Joy Sense");
          disp.drawStr(107, 55, ">");
        } while (disp.nextPage());
        stripL.setPixelColor(2, menuColor);
        break;

      case 1:
        disp.firstPage();
        do {
          disp.drawStr(16, 28, "Joy Mode");
          disp.drawStr(0, 55, "<");
          disp.drawStr(107, 55, ">");
        } while (disp.nextPage());
        stripL.setPixelColor(0, menuColor);
        break;

      case 2:
        disp.firstPage();
        do {
          disp.drawStr(13, 28, "Knob Push");
          disp.drawStr(0, 55, "<");
          disp.drawStr(107, 55, ">");
        } while (disp.nextPage());
        stripR.setPixelColor(0, menuColor);
        break;

      case 3:
        disp.firstPage();
        do {
          disp.drawStr(17, 28, "Rear Push");
          disp.drawStr(0, 55, "<");
          disp.drawStr(107, 55, ">");
        } while (disp.nextPage());
        stripR.setPixelColor(2, menuColor);
        break;

      case 4:
        disp.firstPage();
        do {
          disp.drawStr(42, 28, "Exit");
          disp.drawStr(0, 55, "<");
        } while (disp.nextPage());
        stripL.fill(stripL.Color(180, 0, 255), 0, 3);
        stripR.fill(stripR.Color(180, 0, 255), 0, 3);
        break;
      default:
        break;
    }
    stripL.show();
    stripR.show();
    oldSel = *s;
  }
  if (digitalRead(butFun) == LOW) {
    switch (*s)
    {
      case 0:
        oldT = 999;
        joySens();
        *s = 0;
        oldSel = 999;
        delay(200);
        break;

      case 1:
        oldT = 999;
        joyMode();
        *s = 1;
        oldSel = 999;
        delay(200);
        break;

      case 2:
        oldT = 999;
        epr = 30;
        buttMode(&epr);
        *s = 2;
        oldSel = 999;
        delay(200);
        break;

      case 3:
        oldT = 999;
        epr = 20;
        buttMode(&epr);
        *s = 3;
        oldSel = 999;
        delay(200);
        break;

      case 4:
        *e = HIGH;
        break;
      default:
        break;
    }
  }
  *f = LOW;
}


/////////////////////////////////////////////////////////////////////////// JOY MODES /////////////////////////////////

void joyMode()
{
  int t = EEPROM.read(10);
  int ex = LOW;
  delay(150);
  do
  {
    if (t != oldT) {
      stripR.fill(stripR.Color(0, 0, 0), 0 , 3);
      stripL.fill(stripL.Color(0, 0, 0), 0 , 3);
      if (t == 0) {
        disp.firstPage();
        do {
          disp.drawStr(16, 15, "Joy Mode");
          disp.drawStr(35, 40, "Mouse");
          disp.drawStr(107, 60, ">");
        } while (disp.nextPage());
        stripL.fill(menuColor, 0, 3);
      }
      if (t == 1) {
        disp.firstPage();
        do {
          disp.drawStr(16, 15, "Joy Mode");
          disp.drawStr(18, 40, "Autodesk");
          disp.drawStr(0, 60, "<");
        } while (disp.nextPage());
        stripR.fill(menuColor, 0, 3);
      }
      oldT = t;
      stripL.show();
      stripR.show();
    }
    rotaryMenu(&t, 0, 1, 1, 150, 3);
    if (digitalRead(butFun) == LOW)
    {
      ex = HIGH;
      EEPROM.update(10, t);
    }
  }
  while (ex == LOW);
  delay(150);
}

void selModes()
{
  switch (EEPROM.read(10))
  {
    case 0:
      //mouse mode
      break;

    case 1:
      //Autodesk
      Keyboard.press(KEY_LEFT_SHIFT);
      Mouse.press(MOUSE_MIDDLE);
      break;
  }
}

/////////////////////////////////////////////////////////////////////////// JOY SENSE /////////////////////////////////

void joySens()
{
  int ex2 = LOW;
  int t = (EEPROM.read(0)) * 10;
  delay(300);
  stripR.fill(menuColor, 0 , 3);
  stripL.fill(menuColor, 0 , 3);
  do
  {
    if (t != oldT) {
      disp.firstPage();
      do {
        disp.drawStr(13, 15, "Joy Sense");
        if (t > 99)
          disp.setPrintPos(45, 45);
        else
          disp.setPrintPos(52, 45);
        disp.print(t);
      } while (disp.nextPage());
      oldT = t;
      stripR.setBrightness(map(t, 10, 450, 255, 15));
      stripL.setBrightness(map(t, 10, 450, 255, 15));
      stripR.show();
      stripL.show();
    }

    rotaryMenu(&t, 10, 450, 10, 10, 1);
    if (digitalRead(butFun) == LOW)
    {
      ex2 = HIGH;
      EEPROM.update(0, t / 10);
    }
  }
  while (ex2 == LOW);
  stripL.setBrightness(255);//TODO const maxBrightness, maybe menu entry for changing + color??
  stripR.setBrightness(255);
  delay(200);
}

/////////////////////////////////////////////////////////////////////////// BUTTON MODE /////////////////////////////////

void buttMode(int *e)
{
  stripR.fill(menuColor, 0 , 3);
  stripL.fill(menuColor, 0 , 3);
  stripR.show();
  stripL.show();
  int ex = LOW;
  int t = EEPROM.read(*e);
  delay(120);
  do
  {
    if (t != oldT) {
      disp.firstPage();
      do {
        disp.drawStr(10, 15, "SELECT FN");
        if (t <= 35)
        {
          disp.setPrintPos(59, 45);
          disp.print(arButt[t]);
        }
        if (t == 36)
          disp.drawStr(35, 45, "Space");

        if (t == 37)
          disp.drawStr(38, 45, "CTRL");
        if (t == 38)
          disp.drawStr(40, 45, "Shift");
        if (t == 39)
          disp.drawStr(48, 45, "Alt");
        if (t == 40)
          disp.drawStr(45, 45, "Tab");
        if (t == 41)
          disp.drawStr(48, 45, "Esc");
        if (t == 42)
          disp.drawStr(48, 45, "Del");
        if (t == 43)
          disp.drawStr(18, 45, "Scroll btn");
      } while (disp.nextPage());
      oldT = t;
    }

    rotaryMenu(&t, 0, 43, 1, 10, 1);
    if (digitalRead(butFun) == LOW)
    {
      ex = HIGH;
      EEPROM.update(*e, t);
    }
  }
  while (ex == LOW);
  delay(120);
}

void selButt(int *e)
{
  switch (EEPROM.read(*e))
  {
    case 36:
      Keyboard.press(32);
      break;

    case 37:
      Keyboard.press(KEY_LEFT_CTRL);
      break;

    case 38:
      Keyboard.press(KEY_LEFT_SHIFT);
      break;

    case 39:
      Keyboard.press(KEY_LEFT_ALT);
      break;

    case 40:
      Keyboard.press(KEY_TAB);
      break;

    case 41:
      Keyboard.press(KEY_ESC);
      break;

    case 42:
      Keyboard.press(KEY_DELETE);
      break;

    case 43:
      Mouse.press(MOUSE_MIDDLE);
      break;
  }
}
void neoStart() {
  for (int i = 0; i < 3; i++) {
    stripR.setPixelColor(i, stripR.Color(180, 0, 255));
    stripL.setPixelColor(i, stripL.Color(180, 0, 255));
    stripR.show();
    stripL.show();
    delay(370);
  }
}

