/*
 * Modified example copied from FastLED 3.0 Branch - originally written by Daniel Garcia
 * This example shows how to use some of FastLED's functions with the SmartMatrix Library
 * using the SmartMatrix buffers directly instead of FastLED's buffers.
 * FastLED's dithering and color balance features can't be used this way, but SmartMatrix can draw in
 * 36-bit color and so dithering may not provide much advantage.
 *
 * This example requires FastLED 3.0 or higher.  If you are having trouble compiling, see
 * the troubleshooting instructions here:
 * https://github.com/pixelmatix/SmartMatrix/#external-libraries
 */

//#include <SmartLEDShieldV4.h>  // uncomment this line for SmartLED Shield V4 (needs to be before #include <SmartMatrix3.h>)
#include <SmartMatrix3.h>
#include <FastLED.h>

#define COLOR_DEPTH 24                  // This sketch and FastLED uses type `rgb24` directly, COLOR_DEPTH must be 24
const uint8_t kMatrixWidth = 64;        // known working: 32, 64, 96, 128
const uint8_t kMatrixHeight = 32;       // known working: 16, 32, 48, 64
const uint8_t kRefreshDepth = 24;       // known working: 24, 36, 48
const uint8_t kDmaBufferRows = 2;       // known working: 2-4, use 2 to save memory, more to keep from dropping frames and automatically lowering refresh rate
const uint8_t kPanelType = SMARTMATRIX_HUB75_32ROW_MOD16SCAN;   // use SMARTMATRIX_HUB75_16ROW_MOD8SCAN for common 16x32 panels
const uint8_t kMatrixOptions = (SMARTMATRIX_OPTIONS_NONE);      // see http://docs.pixelmatix.com/SmartMatrix for options
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);
const uint8_t kScrollingLayerOptions = (SM_SCROLLING_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);

uint8_t hue = 0;
uint8_t pulse = 0;

uint8_t distanceToCenterLookup[kMatrixWidth][kMatrixHeight];

void preCalcLookups() {
  for (int i=0; i<kMatrixWidth; i++){
    for (int j=0; j<kMatrixHeight; j++){
      distanceToCenterLookup[i][j] = distanceToCenter(i, j);
    }
  }
}

float distanceToCenter(int x, int y) {
  // equation for a circle centered at matrixwidth, matrixheight, solved for r
  return sqrt(pow(x-kMatrixWidth, 2) + pow(y-kMatrixHeight,2));
}

void setup() {
  Serial.println("resetting!");
  Serial.begin(115200);
    
  preCalcLookups();
  
  matrix.addLayer(&backgroundLayer);
//  matrix.setRotation(rotation270); // doesn't work for some reason?
  matrix.begin();

  backgroundLayer.setBrightness(25);
}

void loop() {
  // if sketch uses swapBuffers(false), wait to get a new backBuffer() pointer after the swap is done:
  while(backgroundLayer.isSwapPending());

  rgb24 *buffer = backgroundLayer.backBuffer();


  CRGB RGBcolor;
  CHSV HSVcolor;
  
  for(int i = 0; i < kMatrixWidth; i++) {
    for(int j = 0; j < kMatrixHeight; j++) {
      // how far are we from the center point?
      uint8_t distToCenter = distanceToCenterLookup[i][j];
      
      // generate our polar coordinate color
      HSVcolor = CHSV(hue + distToCenter,255,255);
            
      // default to black
      RGBcolor = CRGB::Black;

      // unless..
      if ((pulse <= distToCenter && distToCenter <= pulse+1+(20/distToCenter)) ||
         (((pulse-27)*1.5) <= distToCenter && distToCenter <= ((pulse-27)*1.5)+1+(20/distToCenter))) {
        hsv2rgb_rainbow(HSVcolor, RGBcolor);
//        hsv2rgb_spectrum(HSVcolor, RGBcolor);
      }
      // write the colors
      buffer[kMatrixWidth*j + i] = RGBcolor;
    }
  }

  // buffer is filled completely each time, use swapBuffers without buffer copy to save CPU cycles
  backgroundLayer.swapBuffers(false);
//  matrix.countFPS();      // print the loop() frames per second to Serial
  countFPS();

  EVERY_N_MILLISECONDS( 4 ) { hue -= 4; }
  EVERY_N_MILLISECONDS( 7 ) { pulse += 1; if (pulse > 96) pulse = 0;}
}

void countFPS() {
  static long loops = 0;
  static long lastMillis = 0;
  long currentMillis = millis();

  loops++;
  if(currentMillis - lastMillis >= 1000){
    if(Serial) {
        Serial.print("Loops last second:");
        Serial.println(loops);
    }
    lastMillis = currentMillis;
    loops = 0;
  }
}
