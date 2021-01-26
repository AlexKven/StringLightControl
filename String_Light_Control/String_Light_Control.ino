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
CRGB background[ledCount];

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

int power_cap = 85;

int brightness = 5;

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
#define modeBGCOLOR 2
#define modePATTERN 3
#define modeSPEED 4
#define modeINFO 5
#define max_mode 5

int curVal_info = 0;
int max_info = 1;
int info_fps = 0;
int info_power = 0;

int countDigits(int i)
{
  if (i < 0)
    return 1 + countDigits(-i);
  int result = 1;
  while (i > 10)
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

CRGB color_police(int led)
{
  return (led / 4) % 2 == 0 ? CRGB(255, 0, 0) : CRGB(0, 0, 255);
}

void pattern_solid(double time, bool cycleStart, bool reverse)
{
  if (reverse)
  {
    for (int i = 0; i < ledCount; i++)
    {
      leds[i] = background[i];
    }
  }
  return;
}

void pattern_pulse(double time, bool cycleStart, bool reverse)
{
  for (int i = 0; i < ledCount; i++)
  {
    CRGB color = leds[i];
    CRGB bgColor = background[i];
    if (reverse && i % 2 == 0)
    {
      if (time < 1)
      {
        color = multColor<double>(color, bgColor, 1, 0, 1 - time);
      }
      else
      {
        color = multColor<double>(color, bgColor, 1, 0, time - 1);
      }
    }
    else
    {
      if (time < 1)
      {
        color = multColor<double>(color, bgColor, 1, 0, time);
      }
      else
      {
        color = multColor<double>(color, bgColor, 1, 0, 2 - time);
      }
    }
    leds[i] = color;
  }
}

void pattern_slide_base(double time, bool cycleStart, bool reverse, int slideLength, double transition)
{
  double ledTime = 5.0 / ledCount;
  double pos = time / ledTime;
  for (int i = 0; i < ledCount; i++)
  {
    CRGB color = leds[i];
    CRGB bgColor = background[i];
    
    double dist = reverse ?
      pos - (ledCount - i - 1) :
      pos - i;
    if (dist < 0)
      dist += ledCount;

    if (ledCount - dist < transition)
    {
      color = multColor<double>(color, bgColor, transition, 0, transition - (ledCount - dist));
    }
    else
    {
      color = multColor<double>(color, bgColor, ledCount, ledCount - slideLength, ledCount - dist);
    }
    
    leds[i] = color;
  }
}

void pattern_slide_long(double time, bool cycleStart, bool reverse)
{
  pattern_slide_base(time, cycleStart, reverse, 40, 2.5);
}

void pattern_slide_medium(double time, bool cycleStart, bool reverse)
{
  pattern_slide_base(time, cycleStart, reverse, 20, 2);
}

void pattern_slide_short(double time, bool cycleStart, bool reverse)
{
  pattern_slide_base(time, cycleStart, reverse, 10, 1);
}

void pattern_scan(double time, bool cycleStart, bool reverse)
{
  pattern_slide_base(time, cycleStart, reverse, ledCount / 2.0, ledCount / 2.0);
}

void pattern_binary(double time, bool cycleStart, bool reverse)
{
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
      leds[i] = background[i];
    shValue = shValue >> 1;
  }
}

void setup()
{
  lcd.begin(16, 2);
  FastLED.addLeds<WS2812, 3>(leds, 50);
  randomSeed(analogRead(1));
  Serial.begin(9600);

  numVals_color = 40;
  int i = 0;
  vals_color = new optColor[numVals_color];
  vals_color[i++].constant<255, 255, 255>(F("White"));
  vals_color[i++].constant<128, 128, 128>(F("Light Grey"));
  vals_color[i++].constant<48, 48, 48>(F("Dark Grey"));
  vals_color[i++].constant<0, 0, 0>(F("Black"));
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

  numVals_pattern = 7;
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
  vals_pattern[i].name = F("Binary");
  vals_pattern[i].func = pattern_binary;
  vals_pattern[i++].time = 0.25;

  strClear = F("                ");
  updateDisplay();
}

int lastButton = btnNONE;
double curTime = 0;
unsigned long lastTick = 0;
unsigned long lastInfo = 0;
unsigned int frameCount = 0;
bool reverse = false;
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
  for (int i = 0; i < ledCount; i++)
  {
    CRGB color = vals_color[curVal_color].func(i);
    CRGB bgColor = vals_color[curVal_bgColor].func(i);
    leds[i] = color;
    background[i] = bgColor;
  }
  vals_pattern[curVal_pattern].func(curTime, cycleStart, reverse);
  
  unsigned long totalPwr = 765;
  totalPwr *= ledCount;
  unsigned long actualPwr = 0;
  for (int i = 0; i < ledCount; i++)
  {
    CRGB color = leds[i];
    color = multColor<int>(color, CRGB::Black, 10, 0, brightness);
    leds[i] = color;
    actualPwr += leds[i].r;
    actualPwr += leds[i].g;
    actualPwr += leds[i].b;
  }
  actualPwr *= 100;
  info_power = (int)(actualPwr / totalPwr);

  if (info_power > power_cap)
  {
    actualPwr = 0;
    for (int i = 0; i < ledCount; i++)
    {
      leds[i] = multColor<int>(leds[i], CRGB::Black, info_power, 0, power_cap);
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
    info_fps = (int)(1000000 * (double)frameCount / (double)diff);
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
            break;
          case modeCOLOR:
            increment(curVal_color, 0, numVals_color - 1);
            break;
          case modeBGCOLOR:
            increment(curVal_bgColor, 0, numVals_color - 1);
            break;
          case modePATTERN:
            curTime = 0;
            increment(curVal_pattern, 0, numVals_pattern - 1);
            break;
          case modeSPEED:
            increment(speed, min_speed, max_speed);
            break;
          case modeINFO:
            increment(curVal_info, 0, max_info);
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
          case modeBGCOLOR:
            decrement(curVal_bgColor, 0, numVals_color - 1);
            break;
          case modePATTERN:
            curTime = 0;
            decrement(curVal_pattern, 0, numVals_pattern - 1);
            break;
          case modeSPEED:
            decrement(speed, min_speed, max_speed);
            break;
          case modeINFO:
            decrement(curVal_info, 0, max_info);
            break;
        }
        break;
      case btnSELECT:
        reverse = !reverse;
        break;
      }
      updateDisplay();
    }
    lastButton = curButton;
  }
}
