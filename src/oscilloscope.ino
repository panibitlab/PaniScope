#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <math.h>

// tft
#define TFT_CS  15
#define TFT_DC  2
#define TFT_RST 4

#define BG_COLOR      0x0010
#define GRID_COLOR    0x02A0
#define CENTER_COLOR  0x05A0
#define WAVE_COLOR    0x07E0
#define HEADER_COLOR  0x0120

#define HEADER_HEIGHT 20
#define MENU_HEIGHT 28
#define MENU_TOP 100

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

const int signalOut = 25;   // DAC
const int signalIn  = 32;   // ADC

const int waveButton  = 27;
const int paramButton = 14;

const int potPin = 33;

// wave 
#define TABLE_SIZE 64

uint8_t sineTable[TABLE_SIZE];

float waveFrequency = 1.0;
float amplitude = 1.0;
float offset = 1.65;


// parameters
enum Parameter {FREQ, AMP, OFFSET, DUTY};
float dutyCycle = 20.0;

Parameter currentParameter = FREQ;

enum Waveform {SINE, SQUARE, TRIANGLE, PULSE};

Waveform currentWave = SINE;


// sampling
const int sampleInterval = 20000;

unsigned long lastSample = 0;

int x = 0;
int lastY = 72;


// button
bool lastParamButtonState = HIGH;
unsigned long lastButtonTime = 0;

const int debounceTime = 200;


// sine table
void createSineTable() {

  for (int i = 0; i < TABLE_SIZE; i++) {

    float angle = 2.0 * PI * i / TABLE_SIZE;
    float value = (sin(angle) + 1.0) * 127.5;

    sineTable[i] = (uint8_t)value;
  }
}


// grid
void drawGrid() {

  tft.fillScreen(BG_COLOR);

  // header
  tft.fillRect(0, 0, 160, HEADER_HEIGHT, HEADER_COLOR);
  tft.drawFastHLine(0, HEADER_HEIGHT, 160, CENTER_COLOR);


  // Oscilloscope Grid
  for (int gx = 0; gx < 160; gx += 20) {
    tft.drawFastVLine(gx, HEADER_HEIGHT + 1, MENU_TOP - HEADER_HEIGHT - 1, GRID_COLOR);
  }
  for (int gy = HEADER_HEIGHT + 16; gy < MENU_TOP; gy += 16) {
    tft.drawFastHLine(0, gy, 160, GRID_COLOR);
  }

  // Center horizontal
  tft.drawFastHLine(0, 60, 160, CENTER_COLOR);
  // Center vertical
  tft.drawFastVLine(80, HEADER_HEIGHT + 1, MENU_TOP - HEADER_HEIGHT - 1, CENTER_COLOR);


  // Bottom Menu
  tft.fillRect(0, MENU_TOP, 160, MENU_HEIGHT, HEADER_COLOR);
  tft.drawFastHLine(0, MENU_TOP, 160, CENTER_COLOR);
}


// HEADER
void updateHeader() {

  tft.fillRect(0, 0, 160, HEADER_HEIGHT, HEADER_COLOR);

  tft.setTextColor(WAVE_COLOR);
  tft.setTextSize(1);

  tft.setCursor(3, 7);

  // Waveform
  if (currentWave == SINE) tft.print("SINE");
  else if (currentWave == SQUARE) tft.print("SQUARE");
  else if (currentWave == TRIANGLE) tft.print("TRI");
  else if (currentWave == PULSE) tft.print("PULSE");

  // Frequency
  tft.setCursor(48, 7);
  tft.print("F:");
  tft.print(waveFrequency, 1);
  tft.print("Hz");

  // Amplitude
  tft.setCursor(110, 7);
  tft.print("AMP:");
  tft.print(amplitude, 1);
}

void updateMenu() {

  tft.fillRect(0, MENU_TOP + 1, 160, MENU_HEIGHT - 1, HEADER_COLOR);

  tft.setTextColor(WAVE_COLOR);
  tft.setTextSize(1);

  // freq
  tft.setCursor(3, MENU_TOP + 8);

  if (currentParameter == FREQ) tft.print("> "); else tft.print("  ");

  tft.print("F:");
  tft.print(waveFrequency, 1);
  tft.print("Hz");

  // amp
  tft.setCursor(3, MENU_TOP + 19);

  if (currentParameter == AMP) tft.print("> "); else tft.print("  ");

  tft.print("A:");
  tft.print(amplitude, 1);
  tft.print("V");

  // offset
  tft.setCursor(83, MENU_TOP + 8);

  if (currentParameter == OFFSET) tft.print("> "); else tft.print("  ");

  tft.print("O:");
  tft.print(offset, 1);
  tft.print("V");

  // duty
  tft.setCursor(83, MENU_TOP + 19);

  if (currentParameter == DUTY) tft.print("> "); else tft.print("  ");

  tft.print("D:");
  tft.print(dutyCycle, 0);
  tft.print("%");
}


// BUTTON
void checkButtons() {

  // Parameter Button
  bool paramState = digitalRead(paramButton);

  if (lastParamButtonState == HIGH && paramState == LOW) {

    if (millis() - lastButtonTime > debounceTime) {

      lastButtonTime = millis();

      if (currentParameter == FREQ) {currentParameter = AMP;}
      else if (currentParameter == AMP) {currentParameter = OFFSET;}
      else if (currentParameter == OFFSET) {currentParameter = DUTY;}
      else {currentParameter = FREQ;}
    }
  }

  lastParamButtonState = paramState;

  // Wave Button
  static bool lastWaveButtonState = HIGH;

  bool waveState = digitalRead(waveButton);

  static unsigned long lastWaveButtonTime = 0;

  if (lastWaveButtonState == HIGH && waveState == LOW) {

    if (millis() - lastWaveButtonTime > debounceTime) {

      lastWaveButtonTime = millis();

      if (currentWave == SINE) {currentWave = SQUARE;}
      else if (currentWave == SQUARE) {currentWave = TRIANGLE;}
      else if (currentWave == TRIANGLE) {currentWave = PULSE;}
      else {currentWave = SINE;}
    }
  }

  lastWaveButtonState = waveState;
}


// potentiometer
void updateParameter() {

  int potValue = analogRead(potPin);

  if (currentParameter == FREQ) {waveFrequency = map(potValue, 0, 4095, 5, 200) / 10.0;}
  else if (currentParameter == AMP) {amplitude = map(potValue, 0, 4095, 0, 330) / 100.0;}
  else if (currentParameter == OFFSET) {offset = map(potValue, 0, 4095, 0, 330) / 100.0;}
  else if (currentParameter == DUTY) {dutyCycle = map(potValue, 0, 4095, 5, 95);}
}


// wave generator
uint8_t generateWave(int index) {

  float phase = (float)index / TABLE_SIZE;

  // sine
  if (currentWave == SINE) {return sineTable[index];}
  // square
  if (currentWave == SQUARE) {if (phase < dutyCycle / 100.0) return 255; else return 0;}
  // triangle
  if (currentWave == TRIANGLE) {
    if (phase < 0.5) {return phase * 2.0 * 255;}
    else {return (1.0 - (phase - 0.5) * 2.0) * 255;}
  }
  // pulse
  if (currentWave == PULSE) {if (phase < dutyCycle / 100.0) return 255; else return 0;}

  return 0;
}


// setuo
void setup() {

  Serial.begin(115200);

  // TFT
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);

  drawGrid();

  // Buttons
  pinMode(waveButton, INPUT_PULLUP);
  pinMode(paramButton, INPUT_PULLUP);

  // ADC
  pinMode(signalIn, INPUT);

  // DAC
  dacWrite(signalOut, 128);

  // Sine
  createSineTable();
}


// loop
void loop() {

  unsigned long now = micros();

  checkButtons();
  updateParameter();

  updateHeader();
  updateMenu();

  // generate sine
  static unsigned long lastDAC = 0;

  static int tableIndex = 0;

  float sampleRate = waveFrequency * TABLE_SIZE;

  unsigned long dacInterval = 1000000.0 / sampleRate;


  if (now - lastDAC >= dacInterval) {

    lastDAC = now;


    uint8_t rawWave = generateWave(tableIndex);

    float normalized = rawWave / 255.0;


    // Apply amplitude
    float value = offset + (normalized - 0.5) * amplitude;

    // Limit to DAC range
    value = constrain(value, 0.0, 3.3);

    // Convert to DAC 0-255
    int dacValue = value * 255.0 / 3.3;

    dacWrite(signalOut, dacValue);

    tableIndex++;
    if (tableIndex >= TABLE_SIZE) {tableIndex = 0;}
  }


  // oscillloscope 
  if (now - lastSample >= sampleInterval) {

    lastSample = now;

    int value = analogRead(signalIn);
    float voltage = value * 3.3 / 4095.0;

    // Convert to screen
    int y = map(value, 0, 4095, 127, HEADER_HEIGHT + 1);

    // Draw waveform
    if (x > 0) {tft.drawLine(x - 1, lastY, x, y, WAVE_COLOR);}

    lastY = y;
    x++;

    // Screen finished
    if (x >= 160) {

      x = 0;
      lastY = 72;

      drawGrid();
    }
  }
}
