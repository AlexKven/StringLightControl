//Sample using LiquidCrystal library
#include <LiquidCrystal.h>
#include "FastLED.h"
 
// select the pins used on the LCD panel
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
 
// define some values used by the panel and buttons
int lcd_key     = 0;
int adc_key_in  = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

#define STR const __FlashStringHelper*
STR strClear;
size_t strlen(STR ifsh)
{
  PGM_P p = reinterpret_cast<PGM_P>(ifsh);
  size_t n = 0;
  while (1) {
    unsigned char c = pgm_read_byte(p++);
    if (c == 0) break;
    n++;
  }
  return n;
}

#define ledCount 50
CRGB leds[ledCount];

// read the buttons
int read_LCD_buttons()
{
 adc_key_in = analogRead(0);      // read the value from the sensor
 // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
 // we add approx 50 to those values and check to see if we are close
 if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
 if (adc_key_in < 50)   return btnRIGHT; 
 if (adc_key_in < 195)  return btnUP;
 if (adc_key_in < 380)  return btnDOWN;
 if (adc_key_in < 555)  return btnLEFT;
 if (adc_key_in < 790)  return btnSELECT;  
 return btnNONE;  // when all others fail, return this...
}

template<typename TVal, typename TLevel>
TVal mult(TVal maxVal, TVal minVal, TLevel maxLevel, TLevel minLevel, TLevel level)
{
  if (level <= minLevel)
    return minVal;
  if (level >= maxLevel)
    return maxVal;
  double valVariance = (double)maxVal - (double)minVal;
  double levelVariance = (double)maxLevel - (double)minLevel;
  double frac = ((double)level - (double)minLevel) / levelVariance;
  return (TVal)((double)minVal + frac * valVariance);
}

template<typename TLevel>
CRGB multColor(CRGB to, CRGB from, TLevel maxLevel, TLevel minLevel, TLevel level)
{
  if (level <= minLevel)
    return from;
  if (level >= maxLevel)
    return to;

  int r = mult<int, TLevel>(to.r, from.r, maxLevel, minLevel, level);
  int g = mult<int, TLevel>(to.g, from.g, maxLevel, minLevel, level);
  int b = mult<int, TLevel>(to.b, from.b, maxLevel, minLevel, level);

  return CRGB(r,g,b);
}

int brightness = 10;

int speed = 0;
int min_speed = -12;
int max_speed = 12;

STR speed_name(int speed)
{
  switch (speed)
  {
    case 0:
      return F("Normal");
    case 1:
      return F("1.1X Faster");
    case 2:
      return F("1.25X Faster");
    case 3:
      return F("1.5X Faster");
    case 4:
      return F("1.75X Faster");
    case 5:
      return F("2X Faster");
    case 6:
      return F("2.5X Faster");
    case 7:
      return F("3X Faster");
    case 8:
      return F("4X Faster");
    case 9:
      return F("5X Faster");
    case 10:
      return F("6X Faster");
    case 11:
      return F("8X Faster");
    case 12:
      return F("10X Faster");
    case -1:
      return F("1.1X Slower");
    case -2:
      return F("1.25X Slower");
    case -3:
      return F("1.5X Slower");
    case -4:
      return F("1.75X Slower");
    case -5:
      return F("2X Slower");
    case -6:
      return F("2.5X Slower");
    case -7:
      return F("3X Slower");
    case -8:
      return F("4X Slower");
    case -9:
      return F("5X Slower");
    case -10:
      return F("6X Slower");
    case -11:
      return F("8X Slower");
    case -12:
      return F("10X Slower");
  }
}

double speed_factor(int speed)
{
  if (speed < 0)
    return 1 / speed_factor(-speed);
  switch (speed)
  {
    case 0:
      return 1;
    case 1:
      return 1.1;
    case 2:
      return 1.25;
    case 3:
      return 1.5;
    case 4:
      return 1.75;
    case 5:
      return 2;
    case 6:
      return 2.5;
    case 7:
      return 3;
    case 8:
      return 4;
    case 9:
      return 5;
    case 10:
      return 6;
    case 11:
      return 8;
    case 12:
      return 10;
  }
}

struct optColor
{
  STR name;
  CRGB(*func)(int);
};

optColor* vals_color;
int numVals_color;
int curVal_color;

struct optPattern
{
  STR name;
  void(*func)(double, bool, bool);
  double time;
};

optPattern* vals_pattern;
int numVals_pattern;
int curVal_pattern;

void increment(int& val, int min, int max)
{
  val++;
  if (val > max)
    val = min;
}

void decrement(int& val, int min, int max)
{
  val--;
  if (val < min)
    val = max;
}

int mode = 0;
#define modeBRIGHTNESS 0
#define modeCOLOR 1
#define modePATTERN 2
#define modeSPEED 3

void updateDisplay()
{
  lcd.setCursor(0,0);
  lcd.print(strClear);
  lcd.setCursor(0,1);
  lcd.print(strClear);
  
  lcd.setCursor(0,0);
  lcd.print(F("<"));
  lcd.setCursor(15,0);
  lcd.print(F(">"));

      Serial.println(mode);
  switch (mode)
  {
    case modeBRIGHTNESS:
    {
      lcd.setCursor(3,0);
      lcd.print(F("Brightness"));
      lcd.setCursor(7,1);
      lcd.print(brightness);
      break;
    }
    case modeCOLOR:
    {
      lcd.setCursor(5,0);
      lcd.print(F("Color"));
      STR colorName = vals_color[curVal_color].name;
      int colorLen = strlen(colorName);
      lcd.setCursor(8 - (colorLen + 1) / 2,1);
      lcd.print(colorName);
      break;
    }
    case modePATTERN:
    {
      lcd.setCursor(4,0);
      lcd.print(F("Pattern"));
      STR patternName = vals_pattern[curVal_pattern].name;
      int patternLen = strlen(patternName);
      lcd.setCursor(8 - (patternLen + 1) / 2,1);
      lcd.print(patternName);
      break;
    }
    case modeSPEED:
    {
      lcd.setCursor(5,0);
      lcd.print(F("Speed"));
      STR speedName = speed_name(speed);
      int speedLen = strlen(speedName);
      lcd.setCursor(8 - (speedLen + 1) / 2,1);
      lcd.print(speedName);
      break;
    }
  }
}

CRGB color_white(int led)
{
  return CRGB::White;
}

CRGB color_red(int led)
{
  return CRGB::Red;
}

CRGB color_green(int led)
{
  return CRGB::Green;
}

CRGB color_blue(int led)
{
  return CRGB::Blue;
}

CRGB color_yellow(int led)
{
  return CRGB(255, 255, 0);
}

CRGB color_orange(int led)
{
  return CRGB(255, 128, 0);
}

CRGB color_sunrise(int led)
{
  return CRGB(255, 192, 0);
}

CRGB color_lightBlue(int led)
{
  return CRGB(0, 192, 255);
}

CRGB color_skyBlue(int led)
{
  return CRGB(0, 255, 255);
}

CRGB color_cyan(int led)
{
  return CRGB(0, 255, 192);
}

CRGB color_teal(int led)
{
  return CRGB(0, 255, 128);
}

CRGB color_seahawks(int led)
{
  return led % 2 == 0 ? CRGB(128, 255, 0) : CRGB(0, 64, 255);
}

void pattern_solid(double time, bool cycleStart, bool actionPressed)
{
  if (actionPressed)
  {
    for (int i = 0; i < ledCount; i++)
    {
      leds[i] = CRGB::Black;
    }
  }
  return;
}

void pattern_pulse(double time, bool cycleStart, bool actionPressed)
{
  if (actionPressed)
    Serial.println(time);
  for (int i = 0; i < ledCount; i++)
  {
    CRGB color = leds[i];
    if (actionPressed && i % 2 == 0)
    {
      if (time < 1)
      {
        color = multColor<double>(color, CRGB::Black, 1, 0, 1 - time);
      }
      else
      {
        color = multColor<double>(color, CRGB::Black, 1, 0, time - 1);
      }
    }
    else
    {
      if (time < 1)
      {
        color = multColor<double>(color, CRGB::Black, 1, 0, time);
      }
      else
      {
        color = multColor<double>(color, CRGB::Black, 1, 0, 2 - time);
      }
    }
    leds[i] = color;
  }
}

void pattern_slide_base(double time, bool cycleStart, bool actionPressed, int slideLength, double transition)
{
  double ledTime = 5.0 / ledCount;
  double pos = time / ledTime;
  for (int i = 0; i < ledCount; i++)
  {
    CRGB color = leds[i];
    
    double dist = actionPressed ?
      pos - (ledCount - i - 1) :
      pos - i;
    if (dist < 0)
      dist += ledCount;
    if (i == 0)
      Serial.println(pos);

    if (ledCount - dist < transition)
    {
      color = multColor<double>(color, CRGB::Black, transition, 0, transition - (ledCount - dist));
    }
    else
    {
      color = multColor<double>(color, CRGB::Black, ledCount, ledCount - slideLength, ledCount - dist);
    }
    
    leds[i] = color;
  }
}

void pattern_slide_long(double time, bool cycleStart, bool actionPressed)
{
  pattern_slide_base(time, cycleStart, actionPressed, 40, 2.5);
}

void pattern_slide_medium(double time, bool cycleStart, bool actionPressed)
{
  pattern_slide_base(time, cycleStart, actionPressed, 20, 2);
}

void pattern_slide_short(double time, bool cycleStart, bool actionPressed)
{
  pattern_slide_base(time, cycleStart, actionPressed, 10, 1);
}

void pattern_scan(double time, bool cycleStart, bool actionPressed)
{
  pattern_slide_base(time, cycleStart, actionPressed, ledCount / 2.0, ledCount / 2.0);
}

void pattern_binary(double time, bool cycleStart, bool actionPressed)
{
  static unsigned long long value = 0;
  if (cycleStart)
  {
    if (actionPressed)
      value -= 1;
    else
      value += 1;
  }
  unsigned long long shValue = value;
  for (int i = 0; i < ledCount; i++)
  {
    if (shValue % 2 == 0)
      leds[i] = CRGB::Black;
    shValue = shValue >> 1;
  }
}

void setup()
{
  lcd.begin(16, 2);
  FastLED.addLeds<WS2812, 4>(leds, 50);
  randomSeed(analogRead(1));
  Serial.begin(9600);

  numVals_color = 12;
  vals_color = new optColor[numVals_color];
  vals_color[0].name = F("White");
  vals_color[0].func = color_white;
  vals_color[1].name = F("Red");
  vals_color[1].func = color_red;
  vals_color[2].name = F("Green");
  vals_color[2].func = color_green;
  vals_color[3].name = F("Blue");
  vals_color[3].func = color_blue;
  vals_color[4].name = F("Yellow");
  vals_color[4].func = color_yellow;
  vals_color[5].name = F("Orange");
  vals_color[5].func = color_orange;
  vals_color[6].name = F("Sunrise");
  vals_color[6].func = color_sunrise;
  vals_color[7].name = F("Light Blue");
  vals_color[7].func = color_lightBlue;
  vals_color[8].name = F("Sky Blue");
  vals_color[8].func = color_skyBlue;
  vals_color[9].name = F("Cyan");
  vals_color[9].func = color_cyan;
  vals_color[10].name = F("Teal");
  vals_color[10].func = color_teal;
  vals_color[11].name = F("Seahawks");
  vals_color[11].func = color_seahawks;

  numVals_pattern = 7;
  vals_pattern = new optPattern[numVals_pattern];
  vals_pattern[0].name = F("Solid");
  vals_pattern[0].func = pattern_solid;
  vals_pattern[0].time = 1;
  vals_pattern[1].name = F("Pulse");
  vals_pattern[1].func = pattern_pulse;
  vals_pattern[1].time = 2;
  vals_pattern[2].name = F("Slide Long");
  vals_pattern[2].func = pattern_slide_long;
  vals_pattern[2].time = 5;
  vals_pattern[3].name = F("Slide Medium");
  vals_pattern[3].func = pattern_slide_medium;
  vals_pattern[3].time = 5;
  vals_pattern[4].name = F("Slide Short");
  vals_pattern[4].func = pattern_slide_short;
  vals_pattern[4].time = 5;
  vals_pattern[5].name = F("Scan");
  vals_pattern[5].func = pattern_scan;
  vals_pattern[5].time = 5;
  vals_pattern[6].name = F("Binary");
  vals_pattern[6].func = pattern_binary;
  vals_pattern[6].time = 0.25;

  strClear = F("                ");
  updateDisplay();
}

int lastButton = btnNONE;
double curTime = 0;
unsigned long lastTick = 0;
void loop()
{
  unsigned long curTick = micros();
  unsigned long diff = curTick - lastTick;
  curTime += (double)diff * speed_factor(speed) / 1000000.0;
  double timeLength = vals_pattern[curVal_pattern].time;
  bool cycleStart = curTime == 0;
  while (curTime > timeLength)
  {
    cycleStart = true;
    curTime -= timeLength;
  }
  lastTick = curTick;
  int curButton = read_LCD_buttons();
  for (int i = 0; i < ledCount; i++)
  {
    CRGB color = vals_color[curVal_color].func(i);
    leds[i] = color;
  }
  vals_pattern[curVal_pattern].func(curTime, cycleStart, curButton == btnSELECT);
  for (int i = 0; i < ledCount; i++)
  {
    CRGB color = leds[i];
    color = multColor<int>(color, CRGB::Black, 10, 0, brightness);
    leds[i] = color;
  }

  FastLED.show();

  if (curButton != lastButton)
  {
    Serial.println(speed_factor(speed));
    Serial.println(curTime);
    if (lastButton == btnNONE)
    {
     switch (curButton)
     {
      case btnRIGHT:
        increment(mode, 0, 3);
        break;
      case btnLEFT:
        decrement(mode, 0, 3);
        break;
      case btnUP:
        switch (mode)
        {
          case modeBRIGHTNESS:
            increment(brightness, 0, 10);
            break;
          case modeCOLOR:
            increment(curVal_color, 0, numVals_color - 1);
            break;
          case modePATTERN:
            curTime = 0;
            increment(curVal_pattern, 0, numVals_pattern - 1);
            break;
          case modeSPEED:
            increment(speed, min_speed, max_speed);
            break;
        }
        break;
      case btnDOWN:
        switch (mode)
        {
          case modeBRIGHTNESS:
            decrement(brightness, 0, 10);
            break;
          case modeCOLOR:
            decrement(curVal_color, 0, numVals_color - 1);
            break;
          case modePATTERN:
            curTime = 0;
            decrement(curVal_pattern, 0, numVals_pattern - 1);
            break;
          case modeSPEED:
            decrement(speed, min_speed, max_speed);
            break;
        }
        break;
      }
      updateDisplay();
    }
    lastButton = curButton;
  }
}
