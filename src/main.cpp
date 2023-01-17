#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>

#define SCREEN_WIDTH    240
#define SCREEN_HEIGHT   320
#define SCREEN_ROTATION 0

// #define TFT_CS D0  //for D1 mini or TFT I2C Connector Shield (V1.1.0 or later)
// #define TFT_DC D8  //for D1 mini or TFT I2C Connector Shield (V1.1.0 or later)
// #define TFT_RST -1 //for D1 mini or TFT I2C Connector Shield (V1.1.0 or later)
// #define TS_CS D3   //for D1 mini or TFT I2C Connector Shield (V1.1.0 or later)

#define TFT_CS  14  //for D32 Pro
#define TFT_DC  27  //for D32 Pro
#define TFT_RST 33  //for D32 Pro
#define TS_CS   12  //for D32 Pro

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// The TIRQ interrupt signal used for this example.
// #define TIRQ_PIN  15
// XPT2046_Touchscreen ts(TS_CS, TIRQ_PIN);    // using IRQ pin... make sure to solder wire from TFT PCB to D32 board
// or if not using interrupts
// XPT2046_Touchscreen ts(TS_CS); 

// typedef struct{
//   byte x;
//   int16_t y;
//   int16_t color = ILI9341_CYAN; // initial color
// } TouchScreenPoint; 

// TouchScreenPoint  currentPoint;
// TouchScreenPoint screenPoint[18100];

// void fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h,int16_t radius, uint16_t color);


// analogReadResolution(resolution): set the sample bits and resolution. It can be a value between 9 (0 – 511) and 12 bits (0 – 4095). Default is 12-bit resolution.
// analogSetWidth(width): set the sample bits and resolution. It can be a value between 9 (0 – 511) and 12 bits (0 – 4095). Default is 12-bit resolution.
// analogSetCycles(cycles): set the number of cycles per sample. Default is 8. Range: 1 to 255.
// analogSetSamples(samples): set the number of samples in the range. Default is 1 sample. It has an effect of increasing sensitivity.
// analogSetClockDiv(attenuation): set the divider for the ADC clock. Default is 1. Range: 1 to 255.
// analogSetAttenuation(attenuation): sets the input attenuation for all ADC pins. Default is ADC_11db. Accepted values:
//     ADC_0db: sets no attenuation. ADC can measure up to approximately 800 mV (1V input = ADC reading of 1088).
//     ADC_2_5db: The input voltage of ADC will be attenuated, extending the range of measurement to up to approx. 1100 mV. (1V input = ADC reading of 3722).
//     ADC_6db: The input voltage of ADC will be attenuated, extending the range of measurement to up to approx. 1350 mV. (1V input = ADC reading of 3033).
//     ADC_11db: The input voltage of ADC will be attenuated, extending the range of measurement to up to approx. 2600 mV. (1V input = ADC reading of 1575).
// analogSetPinAttenuation(pin, attenuation): sets the input attenuation for the specified pin. The default is ADC_11db. Attenuation values are the same from previous function.
// adcAttachPin(pin): Attach a pin to ADC (also clears any other analog mode that could be on). Returns TRUE or FALSE result.
// adcStart(pin), adcBusy(pin) and resultadcEnd(pin): starts an ADC convertion on attached pin’s bus. Check if conversion on the pin’s ADC bus is currently running (returns TRUE or FALSE). Get the result of the conversion: returns 16-bit integer.


// EZSBC ESP32
// ============
// ADC0 - IO36
// ADC3 - IO39
// ADC7 - IO35
// ADC6 - IO34
// ADC5 - IO33
// ADC4 - IO32
// ADC18 - IO25
// ADC19 - IO26
// ADC17 - IO27
// ADC13 - IO15
// ADC16 - IO14
// ADC14 - IO13
// ADC15 - IO12
// ADC10 - IO3
// ADC12 - IO2
// ADC11 - IO0

// DAC0 - IO25
// DAC1 - IO26

// 0-4095 reading from ADC
int aInputValue = 0;

int getAverageAnalogValue(){
  const int numberOfSamples = 1000; 
  const int aInputPin = 34; // ADC6

  for (int i=0; i<numberOfSamples; i++){
    aInputValue += analogRead(aInputPin);
  }
  return (aInputValue / numberOfSamples);   // average of n samples
}


void displayCurrentLabel(){
  // display Current divisions
  // clear bottom area of text, then write new text
  int textx = 0;
  int texty = 35;
  tft.fillRoundRect(textx, texty, SCREEN_WIDTH, 24, 1, ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(textx, texty);
  tft.println("0A                3A");
}

float adcToCurrentReading(int aIntputValue){
  float currentValue = 0;
  // convert aInputValue to calibrated current reading
  // we need to compute for different tiers because device is non-linear
  // Current calibration data 
  float Current0AmpADCReading = 2703;
  float Current1AmpADCReading = 2904;
  float Current2AmpADCReading = 3119;
  float Current3AmpADCReading = 3380;
  
  // 0 to 1Amp
  if (aInputValue >= Current0AmpADCReading && aInputValue <= Current1AmpADCReading) {  
    currentValue = (aInputValue - Current0AmpADCReading)/(Current1AmpADCReading-Current0AmpADCReading);
  };

  // 1Amp to 2Amp
  if (aInputValue > Current1AmpADCReading && aInputValue <= Current2AmpADCReading) {
    currentValue = (aInputValue - Current1AmpADCReading)/(Current2AmpADCReading-Current1AmpADCReading) + 1;
  };

  // 2Amp to 3Amp
  if (aInputValue > Current2AmpADCReading && aInputValue <= Current3AmpADCReading){
    currentValue = (aInputValue - Current2AmpADCReading)/(Current3AmpADCReading-Current2AmpADCReading) + 2;
  }

  return currentValue;
}

void displayCurrentValue(float currentValue){
  int textx = 0;
  int texty = 60;
  int barh = 25;
  int barradius = 1;
  tft.fillRoundRect(textx, texty, SCREEN_WIDTH, barh, barradius, ILI9341_BLACK);
  tft.setCursor(textx, texty);
  tft.println("Current: " + String(currentValue) + " A");
}

void updateBarGraph(int aInputValue){
  int barx = 0;
  int bary = 0;
  int barw = map(aInputValue, 2700, 3380, 0, SCREEN_WIDTH);
  int barh = 25;
  int barradius = 1;
  int barcolor = ILI9341_CYAN;
  int darkbarcolor = ILI9341_DARKCYAN;
  // draw new graph
  tft.fillRoundRect(barx, bary, barw, barh, barradius, barcolor);
  // clear from end of graph to end of screen
  tft.fillRoundRect(barw+1, bary, SCREEN_WIDTH-barw, barh, 1, darkbarcolor);
}

void displayADCRawValue(int aInputValue){
  // display raw value of AnalogRead
  // clear bottom area of text, then write new text
  int textx = 0;
  int texty = 290;
  tft.fillRoundRect(textx, texty, SCREEN_WIDTH, 24, 1, ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(textx, texty);
  tft.println("Raw Value:" + String(aInputValue));
}

void setup() {
  Serial.begin(115200);
  // ts.begin();
  // ts.setRotation(SCREEN_ROTATION);

  tft.begin();
  tft.setRotation(SCREEN_ROTATION);
  tft.fillScreen(ILI9341_BLACK);
}

void loop() {
  aInputValue = getAverageAnalogValue();
  // aInputValue = 2688;    // Testing
  float currentValue = adcToCurrentReading(aInputValue);
  displayCurrentLabel();
  displayCurrentValue(currentValue);
  updateBarGraph(aInputValue);
  displayADCRawValue(aInputValue);
  delay(50);
}