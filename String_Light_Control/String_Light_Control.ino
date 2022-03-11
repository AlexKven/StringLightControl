//Sample using LiquidCrystal library
#include <EEPROM.h>
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

#define ledCount 100
float ledCountFloat = (float)ledCount;
CRGB leds[ledCount];
CRGB background[ledCount];
uint8_t roundingOrder[ledCount];

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

float mult(float maxVal, float minVal, float maxLevel, float minLevel, float level)
{
  if (level <= minLevel)
    return minVal;
  if (level >= maxLevel)
    return maxVal;
  float valVariance = maxVal - minVal;
  float levelVariance = maxLevel - minLevel;
  float frac = (level - minLevel) / levelVariance;
  float result = minVal + frac * valVariance;
  
  return result;
}

float roundByNumber(float number, float roundNumber)
{
  float rounded = (float)((int)number);
  float frac = number - rounded;
  if (frac > roundNumber)
    rounded += 1.0f;
  return rounded;
}

CRGB multColor(CRGB to, CRGB from, float maxLevel, float minLevel, float level, float bright = 1.0f, float roundNumber = 1.0f)
{
  if (level <= minLevel)
    return from;
  if (level >= maxLevel)
    return to;

  float rFloat = mult(to.r, from.r, maxLevel, minLevel, level) * bright;
  float gFloat = mult(to.g, from.g, maxLevel, minLevel, level) * bright;
  float bFloat = mult(to.b, from.b, maxLevel, minLevel, level) * bright;

  if (roundNumber < 1.0)
  {
    rFloat = roundByNumber(rFloat, roundNumber);
    gFloat = roundByNumber(gFloat, roundNumber);
    bFloat = roundByNumber(bFloat, roundNumber);
  }

  int r = (int)(rFloat);
  int g = (int)(gFloat);
  int b = (int)(bFloat);

  if (roundNumber < 1.0)
  {
    if (r > 255)
      r = 255;
    if (g > 255)
      g = 255;
    if (b > 255)
      b = 255;
  }
  
  return CRGB(r,g,b);
}


CRGB colorBright(CRGB color, float bright, float roundNumber = 1.0f)
{
  float rFloat = (float)color.r * bright;
  float gFloat = (float)color.g * bright;
  float bFloat = (float)color.b * bright;

  if (roundNumber < 1.0)
  {
    rFloat = roundByNumber(rFloat, roundNumber);
    gFloat = roundByNumber(gFloat, roundNumber);
    bFloat = roundByNumber(bFloat, roundNumber);
  }

  int r = (int)(rFloat);
  int g = (int)(gFloat);
  int b = (int)(bFloat);

  if (roundNumber < 1.0)
  {
    if (r > 255)
      r = 255;
    if (g > 255)
      g = 255;
    if (b > 255)
      b = 255;
  }
  
  return CRGB(r,g,b);
}

int colorPower(CRGB color)
{
  return color.r + color.g + color.b;
}

int power_cap = 70;

int brightness = 5;
float brightnessFloat = 0.5f;

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

float speed_factor(int speed)
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

  void set(CRGB(*_func)(int), STR _name)
  {
    name = _name;
    func = _func;
  }
  
  template<int R, int G, int B>
  void constant(STR _name)
  {
    name = _name;
    func = color_constant<R, G, B>;
  }
};

optColor* vals_color;
int numVals_color;
int curVal_color;
int curVal_bgColor = 3;

struct optPattern
{
  STR name;
  unsigned long(*func)(float, bool, bool);
  float time;
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
#define modeBGCOLOR 2
#define modePATTERN 3
#define modeSPEED 4
#define modeINFO 5
#define modePRESET 6
#define max_mode 6

int curVal_info = 0;
int max_info = 1;
int info_fps = 0;
int info_power = 0;

int curVal_preset = 0;
int max_preset = 40;

bool reverse = false;

int countDigits(int i)
{
  if (i < 0)
    return 1 + countDigits(-i);
  int result = 1;
  while (i >= 10)
  {
    i /= 10;
    result++;
  }
  return result;
}

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
    case modeBGCOLOR:
    {
      lcd.setCursor(1,0);
      lcd.print(F("Backgrnd Color"));
      STR colorName = vals_color[curVal_bgColor].name;
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
    case modeINFO:
    {
      lcd.setCursor(2,0);
      lcd.print(F("Information"));
      STR postfix;
      int num;
      if (curVal_info == 0)
      {
        postfix = F(" FPS");
        num = info_fps;
      }
      else
      {
        postfix = F("% Power");
        num = info_power;
      }
      int numLen = countDigits(num);
      int postfixLen = strlen(postfix);
      int start = 8 - (numLen + postfixLen + 1) / 2;
      lcd.setCursor(start, 1);
      lcd.print(num);
      start += numLen;
      lcd.setCursor(start, 1);
      lcd.print(postfix);
      break;
    }
    case modePRESET:
    {
      lcd.setCursor(5,0);
      lcd.print(F("Preset"));
      lcd.setCursor(7,1);
      lcd.print(curVal_preset);
      break;
    }
  }
}

template<int R, int G, int B>
CRGB color_constant(int led)
{
  return CRGB(R, G, B);
}

CRGB color_seahawks(int led)
{
  return led % 2 == 0 ? CRGB(128, 255, 0) : CRGB(0, 64, 255);
}

CRGB color_christmas(int led)
{
  return led % 2 == 0 ? CRGB(48, 255, 0) : CRGB(255, 0, 0);
}

CRGB color_usa(int led)
{
  return led % 3 == 0 ? CRGB(255, 0, 0) : led % 3 == 1 ? CRGB(255, 255, 255) : CRGB(0, 0, 255);
}


CRGB color_ukraine(int led)
{
  return led % 2 == 0 ? CRGB(0, 0, 255) : CRGB(255, 255, 0);
}

CRGB color_police(int led)
{
  return (led / 4) % 2 == 0 ? CRGB(255, 0, 0) : CRGB(0, 0, 255);
}

int convertColorComponent(int val)
{
  int result = (val + 1) * 32;
  if (result > 0)
    result--;
  return result;
}

CRGB color_random(int led)
{
  float factor = 512.0 / ledCountFloat;
  int clr = (int)((float)(roundingOrder[led] + 1) * factor) - 1;
  int r = convertColorComponent(clr / 64);
  int g = convertColorComponent((clr / 8) % 8);
  int b = convertColorComponent(clr % 8);
  return CRGB(r, g, b);
}

float roundNumberForLed(int led)
{
  int order = roundingOrder[led];
  return (float)order / ledCountFloat;
}

unsigned long pattern_solid(float time, bool cycleStart, bool reverse)
{
  unsigned long power = 0;
  for (int i = 0; i < ledCount; i++)
  {
    if (reverse)
    {
      leds[i] = colorBright(background[i], brightnessFloat);
    }
    else
    {
      leds[i] = colorBright(leds[i], brightnessFloat);
    }
    
    power += colorPower(leds[i]);
  }
  return power;
}

unsigned long pattern_pulse(float time, bool cycleStart, bool reverse)
{
  unsigned long power = 0;
  for (int i = 0; i < ledCount; i++)
  {
    CRGB color = leds[i];
    CRGB bgColor = background[i];
    float roundNumber = roundNumberForLed(i);
    if (reverse && i % 2 == 0)
    {
      if (time < 1)
      {
        color = multColor(color, bgColor, 1, 0, 1 - time, brightnessFloat, roundNumber);
      }
      else
      {
        color = multColor(color, bgColor, 1, 0, time - 1, brightnessFloat, roundNumber);
      }
    }
    else
    {
      if (time < 1)
      {
        color = multColor(color, bgColor, 1, 0, time, brightnessFloat, roundNumber);
      }
      else
      {
        color = multColor(color, bgColor, 1, 0, 2 - time, brightnessFloat, roundNumber);
      }
    }
    leds[i] = color;
    power += colorPower(leds[i]);
  }
  return power;
}

unsigned long pattern_slide_base(float time, bool cycleStart, bool reverse, int slideLength, int slideCount, float transition, float tail)
{
  unsigned long power = 0;
  float sectionLength = (float)ledCount / (float)slideCount;
  float ledTime = 5.0 / ledCount; 
  float pos = time / (ledTime * slideCount);
  while (pos > sectionLength)
    pos -= sectionLength;
  for (int i = 0; i < ledCount; i++)
  {
    float sI = i;
    while (sI >= sectionLength)
      sI -= sectionLength;
    CRGB color = leds[i];
    CRGB bgColor = background[i];
    float roundNumber = roundNumberForLed(i);
    float dist = reverse ?
      pos - (sectionLength - sI - 1) :
      pos - sI;
    if (dist < 0)
      dist += sectionLength;
//
//    while (dist >= sectionLength)
//      dist -= sectionLength;

    if (sectionLength - dist < transition)
    {
      color = multColor(color, bgColor, transition, 0, transition - (sectionLength - dist));
    }
    else
    {
      color = multColor(color, bgColor, sectionLength - (slideLength - tail), sectionLength - slideLength, sectionLength - dist);
    }

    color = colorBright(color, brightnessFloat, roundNumber);
    
    leds[i] = color;
    power += colorPower(color);
  }
  return power;
}

unsigned long pattern_slide_long(float time, bool cycleStart, bool reverse)
{
  return pattern_slide_base(time, cycleStart, reverse, 40, 1, 2.5, 40);
}

unsigned long pattern_slide_medium(float time, bool cycleStart, bool reverse)
{
  return pattern_slide_base(time, cycleStart, reverse, 20, 1, 2, 20);
}

unsigned long pattern_slide_short(float time, bool cycleStart, bool reverse)
{
  return pattern_slide_base(time, cycleStart, reverse, 10, 1, 1, 10);
}

unsigned long pattern_scan(float time, bool cycleStart, bool reverse)
{
  return pattern_slide_base(time, cycleStart, reverse, ledCount / 2.0, 1, ledCount / 2.0, ledCount / 2.0);
}

unsigned long pattern_scan2(float time, bool cycleStart, bool reverse)
{
  return pattern_slide_base(time, cycleStart, reverse, ledCount / 4.0, 2, ledCount / 4.0, ledCount / 4.0);
}

unsigned long pattern_scan4(float time, bool cycleStart, bool reverse)
{
  return pattern_slide_base(time, cycleStart, reverse, ledCount / 8.0, 4, ledCount / 8.0, ledCount / 8.0);
}

unsigned long pattern_stripes2(float time, bool cycleStart, bool reverse)
{
  return pattern_slide_base(time, cycleStart, reverse, ledCount / 2.0, 1, 1, 1);
}

unsigned long pattern_stripes4(float time, bool cycleStart, bool reverse)
{
  return pattern_slide_base(time, cycleStart, reverse, ledCount / 4.0, 2, 1, 1);
}

unsigned long pattern_stripes8(float time, bool cycleStart, bool reverse)
{
  return pattern_slide_base(time, cycleStart, reverse, ledCount / 8.0, 4, 1, 1);
}

unsigned long pattern_stripes16(float time, bool cycleStart, bool reverse)
{
  return pattern_slide_base(time, cycleStart, reverse, ledCount / 16.0, 8, 1, 1);
}

unsigned long pattern_stripesMini(float time, bool cycleStart, bool reverse)
{
  return pattern_slide_base(time, cycleStart, reverse, 2, ledCount / 4, 1, 1);
}

unsigned long pattern_binary(float time, bool cycleStart, bool reverse)
{
  unsigned long power = 0;
  static unsigned long long value = 0;
  if (cycleStart)
  {
    if (reverse)
      value -= 1;
    else
      value += 1;
  }
  unsigned long long shValue = value;
  for (int i = 0; i < ledCount; i++)
  {
    if (shValue % 2 == 0)
      leds[i] = colorBright(background[i], brightnessFloat);
    else
      leds[i] = colorBright(leds[i], brightnessFloat);
    shValue = shValue >> 1;
    
    power += colorPower(leds[i]);
  }
  return power;
}

void shuffle(bool init = false)
{
  if (init)
  {
    for (int i = 0; i < ledCount; i++)
    {
      roundingOrder[i] = i;
    }
  }

  for (int i = ledCount - 1; i >= 0; i--)
  {
    int index = random(0, i + 1);
    int temp = roundingOrder[i];
    roundingOrder[i] = roundingOrder[index];
    roundingOrder[index] = temp;
  }
}

unsigned int eeprom_length;
unsigned int eeprom_region_start = 0;

uint8_t readResilient(int index, uint8_t def = 0)
{
  int eepromInd = index * 3;
  uint8_t val1 = EEPROM.read(eepromInd);
  uint8_t val2 = EEPROM.read(eepromInd + 1);
  uint8_t val3 = EEPROM.read(eepromInd + 2);

  if (val1 == val2)
    return val1;
  if (val1 == val3)
    return val1;
  if (val2 == val3)
    return val2;
  return def;
}

void writeResilient(int index, uint8_t value)
{
  uint8_t cur = readResilient(index);
  Serial.println(cur);
  Serial.println(value);
  Serial.println();
  
  int eepromInd = index * 3;
  EEPROM.update(eepromInd, value);
  EEPROM.update(eepromInd + 1, value);
  EEPROM.update(eepromInd + 2, value);
}

void loadSettings(bool loadPreset = true)
{
  if (loadPreset)
  {
    curVal_preset = readResilient(0, 0);
    if (curVal_preset > max_preset)
      curVal_preset = max_preset;
  }

  int startInd = curVal_preset * 6 + 1;
  brightness = readResilient(startInd, 1);
  if (brightness > 10)
    brightness = 10;
  brightnessFloat = (float)brightness / 10.0f;
  
  curVal_color = readResilient(startInd + 1);
  if (curVal_color >= numVals_color)
    curVal_color = numVals_color - 1;
  
  curVal_bgColor = readResilient(startInd + 2, 3);
  if (curVal_bgColor >= numVals_color)
    curVal_bgColor = numVals_color - 1;
  
  curVal_pattern = readResilient(startInd + 3);
  if (curVal_pattern >= numVals_pattern)
    curVal_pattern = numVals_pattern - 1;
  
  speed = (int)readResilient(startInd + 4, 12) + min_speed;
  if (speed > max_speed)
    speed = max_speed;
  
  reverse = readResilient(startInd + 5);
  if (reverse > 1)
    reverse = 1;
}

void saveSettings(bool presetOnly = false, bool includePreset = true)
{
  if (includePreset)
    writeResilient(0, curVal_preset);

  if (!presetOnly)
  {
    int startInd = curVal_preset * 6 + 1;
    writeResilient(startInd, brightness);
    writeResilient(startInd + 1, curVal_color);
    writeResilient(startInd + 2, curVal_bgColor);
    writeResilient(startInd + 3, curVal_pattern);
    writeResilient(startInd + 4, (uint8_t)(speed - min_speed));
    writeResilient(startInd + 5, reverse);
  }
}

void initEeprom()
{
  eeprom_length = EEPROM.length();
  loadSettings();

  if (brightness < 2)
    brightness = 0;
  else
    brightness -= 2;
  brightnessFloat = (float)brightness / 10.0f;
    
  writeResilient(curVal_preset * 6 + 1, brightness);
}

void clearEeprom()
{
  writeResilient(0, 0);
  for (int i = 0; i <= max_preset; i++)
  {
    int startInd = i * 6 + 1;
    writeResilient(startInd, 5);
    writeResilient(startInd + 1, 0);
    writeResilient(startInd + 2, 3);
    writeResilient(startInd + 3, 0);
    writeResilient(startInd + 4, 12);
    writeResilient(startInd + 5, false);
  }
  loadSettings();
}

void setup()
{
  lcd.begin(16, 2);
  FastLED.addLeds<WS2812, 3>(leds, ledCount);
  randomSeed(analogRead(1));
  Serial.begin(9600);

  numVals_color = 46;
  int i = 0;
  vals_color = new optColor[numVals_color];
  vals_color[i++].constant<255, 255, 255>(F("White"));
  vals_color[i++].constant<128, 128, 128>(F("Light Grey"));
  vals_color[i++].constant<48, 48, 48>(F("Dark Grey"));
  vals_color[i++].constant<0, 0, 0>(F("Black"));
  vals_color[i++].constant<255, 145, 15>(F("Soft Yellow"));
  vals_color[i++].constant<240, 100, 0>(F("Dark Soft Yellow"));
  vals_color[i++].constant<255, 175, 30>(F("Light Sft Yellow"));
  vals_color[i++].constant<255, 0, 0>(F("Red"));
  vals_color[i++].constant<96, 0, 0>(F("Dark Red"));
  vals_color[i++].constant<255, 48, 48>(F("Pink"));
  vals_color[i++].constant<0, 255, 0>(F("Green"));
  vals_color[i++].constant<0, 96, 0>(F("Dark Green"));
  vals_color[i++].constant<128, 255, 32>(F("Light Green"));
  vals_color[i++].constant<0, 0, 255>(F("Blue"));
  vals_color[i++].constant<0, 0, 96>(F("Dark Blue"));
  vals_color[i++].constant<96, 96, 255>(F("Light Blue"));
  vals_color[i++].constant<255, 255, 0>(F("Yellow"));
  vals_color[i++].constant<128, 128, 0>(F("Dark Yellow"));
  vals_color[i++].constant<255, 255, 32>(F("Light Yellow"));
  vals_color[i++].constant<255, 128, 0>(F("Orange"));
  vals_color[i++].constant<96, 48, 0>(F("Dark Orange"));
  vals_color[i++].constant<255, 160, 48>(F("Light Orange"));
  vals_color[i++].constant<255, 64, 0>(F("Sunset"));
  vals_color[i++].constant<160, 255, 0>(F("Yellow Green"));
  vals_color[i++].constant<60, 96, 0>(F("Drk Yellow Green"));
  vals_color[i++].constant<224, 0, 255>(F("Magenta"));
  vals_color[i++].constant<84, 0, 96>(F("Dark Magenta"));
  vals_color[i++].constant<255, 96, 255>(F("Violet"));
  vals_color[i++].constant<128, 0, 255>(F("Purple"));
  vals_color[i++].constant<48, 0, 96>(F("Dark Purple"));
  vals_color[i++].constant<160, 32, 255>(F("Light Purple"));
  vals_color[i++].constant<0, 255, 192>(F("Cyan"));
  vals_color[i++].constant<0, 96, 96>(F("Dark Cyan"));
  vals_color[i++].constant<48, 255, 255>(F("Light Cyan"));
  vals_color[i++].constant<0, 255, 192>(F("Sky Blue"));
  vals_color[i++].constant<0, 96, 72>(F("Dark Sky Blue"));
  vals_color[i++].constant<32, 255, 224>(F("Light Sky Blue"));
  vals_color[i++].constant<0, 255, 96>(F("Teal"));
  vals_color[i++].constant<0, 96, 36>(F("Dark Teal"));
  vals_color[i++].constant<32, 255, 96>(F("Light Teal"));
  vals_color[i++].set(color_seahawks, F("Seahawks"));
  vals_color[i++].set(color_christmas, F("Christmas"));
  vals_color[i++].set(color_police, F("Police"));
  vals_color[i++].set(color_usa, F("USA"));
  vals_color[i++].set(color_ukraine, F("Ukraine"));
  vals_color[i++].set(color_random, F("Random"));

  numVals_pattern = 14;
  i = 0;
  vals_pattern = new optPattern[numVals_pattern];
  vals_pattern[i].name = F("Solid");
  vals_pattern[i].func = pattern_solid;
  vals_pattern[i++].time = 1;
  vals_pattern[i].name = F("Pulse");
  vals_pattern[i].func = pattern_pulse;
  vals_pattern[i++].time = 2;
  vals_pattern[i].name = F("Slide Long");
  vals_pattern[i].func = pattern_slide_long;
  vals_pattern[i++].time = 5;
  vals_pattern[i].name = F("Slide Medium");
  vals_pattern[i].func = pattern_slide_medium;
  vals_pattern[i++].time = 5;
  vals_pattern[i].name = F("Slide Short");
  vals_pattern[i].func = pattern_slide_short;
  vals_pattern[i++].time = 5;
  vals_pattern[i].name = F("Scan");
  vals_pattern[i].func = pattern_scan;
  vals_pattern[i++].time = 5;
  vals_pattern[i].name = F("Scan 2");
  vals_pattern[i].func = pattern_scan2;
  vals_pattern[i++].time = 5;
  vals_pattern[i].name = F("Scan 4");
  vals_pattern[i].func = pattern_scan4;
  vals_pattern[i++].time = 5;
  vals_pattern[i].name = F("2 Stripes");
  vals_pattern[i].func = pattern_stripes2;
  vals_pattern[i++].time = 5;
  vals_pattern[i].name = F("4 Stripes");
  vals_pattern[i].func = pattern_stripes4;
  vals_pattern[i++].time = 5;
  vals_pattern[i].name = F("8 Stripes");
  vals_pattern[i].func = pattern_stripes8;
  vals_pattern[i++].time = 5;
  vals_pattern[i].name = F("16 Stripes");
  vals_pattern[i].func = pattern_stripes16;
  vals_pattern[i++].time = 5;
  vals_pattern[i].name = F("Mini Stripes");
  vals_pattern[i].func = pattern_stripesMini;
  vals_pattern[i++].time = 5;
  vals_pattern[i].name = F("Binary");
  vals_pattern[i].func = pattern_binary;
  vals_pattern[i++].time = 0.25;

  strClear = F("                ");

  shuffle(true);

  initEeprom();
  
  updateDisplay();
}

int lastButton = btnNONE;
float curTime = 0;
unsigned long lastTick = 0;
unsigned long lastInfo = 0;
unsigned int frameCount = 0;
unsigned long holdStart = 0;
unsigned long saveStart = 0;
bool savePresetOnly = false;
void loop()
{
  unsigned long curTick = micros();
  unsigned long diff = curTick - lastTick;
  curTime += (float)diff * speed_factor(speed) / 1000000.0;
  float timeLength = vals_pattern[curVal_pattern].time;
  bool cycleStart = curTime == 0;
  while (curTime > timeLength)
  {
    cycleStart = true;
    curTime -= timeLength;
    shuffle();
  }
  lastTick = curTick;
  for (int i = 0; i < ledCount; i++)
  {
    CRGB color = vals_color[curVal_color].func(i);
    CRGB bgColor = vals_color[curVal_bgColor].func(i);
    leds[i] = color;
    background[i] = bgColor;
  }
  unsigned long actualPwr = vals_pattern[curVal_pattern].func(curTime, cycleStart, reverse);
  
  unsigned long totalPwr = 765;
  totalPwr *= ledCount;
  actualPwr *= 100;
  info_power = (int)(actualPwr / totalPwr);

  if (info_power > power_cap)
  {
    actualPwr = 0;
    for (int i = 0; i < ledCount; i++)
    {
      leds[i] = multColor(leds[i], CRGB::Black, info_power, 0, power_cap);
      actualPwr += leds[i].r;
      actualPwr += leds[i].g;
      actualPwr += leds[i].b;
    }
    actualPwr *= 100;
    info_power = (int)(actualPwr / totalPwr);
  }

  FastLED.show();

  diff = curTick - lastInfo;
  if (diff > 1000000)
  {
    info_fps = (int)(1000000 * (float)frameCount / (float)diff);
    frameCount = 0;
    lastInfo = curTick;
    if (mode == modeINFO)
      updateDisplay();
  }
  frameCount++;

  int curButton = read_LCD_buttons();
    
  if (curButton != lastButton)
  {
    if (lastButton == btnNONE)
    {
     holdStart = millis();
      
     switch (curButton)
     {
      case btnRIGHT:
        increment(mode, 0, max_mode);
        break;
      case btnLEFT:
        decrement(mode, 0, max_mode);
        break;
      case btnUP:
        switch (mode)
        {
          case modeBRIGHTNESS:
            increment(brightness, 0, 10);
            saveStart = millis();
            savePresetOnly = false;
            break;
          case modeCOLOR:
            increment(curVal_color, 0, numVals_color - 1);
            saveStart = millis();
            savePresetOnly = false;
            break;
          case modeBGCOLOR:
            increment(curVal_bgColor, 0, numVals_color - 1);
            saveStart = millis();
            savePresetOnly = false;
            break;
          case modePATTERN:
            curTime = 0;
            increment(curVal_pattern, 0, numVals_pattern - 1);
            saveStart = millis();
            savePresetOnly = false;
            break;
          case modeSPEED:
            increment(speed, min_speed, max_speed);
            saveStart = millis();
            savePresetOnly = false;
            break;
          case modeINFO:
            increment(curVal_info, 0, max_info);
            break;
          case modePRESET:
            saveSettings(false, false);
            saveStart = 0;
            increment(curVal_preset, 0, max_preset);
            loadSettings(false);
            saveStart = millis();
            savePresetOnly = true;
            break;
        }
        break;
      case btnDOWN:
        switch (mode)
        {
          case modeBRIGHTNESS:
            decrement(brightness, 0, 10);
            saveStart = millis();
            savePresetOnly = false;
            break;
          case modeCOLOR:
            decrement(curVal_color, 0, numVals_color - 1);
            saveStart = millis();
            savePresetOnly = false;
            break;
          case modeBGCOLOR:
            decrement(curVal_bgColor, 0, numVals_color - 1);
            saveStart = millis();
            savePresetOnly = false;
            break;
          case modePATTERN:
            curTime = 0;
            decrement(curVal_pattern, 0, numVals_pattern - 1);
            saveStart = millis();
            savePresetOnly = false;
            break;
          case modeSPEED:
            decrement(speed, min_speed, max_speed);
            saveStart = millis();
            savePresetOnly = false;
            break;
          case modeINFO:
            decrement(curVal_info, 0, max_info);
            saveStart = millis();
            savePresetOnly = false;
            break;
          case modePRESET:
            saveSettings(false, false);
            saveStart = 0;
            decrement(curVal_preset, 0, max_preset);
            loadSettings(false);
            saveStart = millis();
            savePresetOnly = true;
            break;
        }
        break;
      case btnSELECT:
        reverse = !reverse;
        saveStart = millis();
        break;
      }
      brightnessFloat = (float)brightness / 10.0f;
      updateDisplay();
    }
    else
    {
      holdStart = 0;
    }
    lastButton = curButton;
  }
  else if (holdStart != 0)
  {
    if (curButton == btnSELECT && millis() - holdStart > 10000)
    {
      clearEeprom();
      mode = 0;
      updateDisplay();
    }
  }

  if (saveStart != 0 && millis() - saveStart > 5000)
  {
      saveSettings(savePresetOnly);
      saveStart = 0;
      savePresetOnly = false;
  }
}
