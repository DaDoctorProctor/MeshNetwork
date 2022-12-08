// C++ libraries
using namespace std;
#include <vector>
#include <string>
#include "OLED.h"
using namespace oled;

// Hardware libraries
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>

// Define different custom values
#define TFT_BROWN 0x38E0 // New background colour
#define WAIT 500 // Pause in milliseconds between screens, change to 0 to time font rendering



void writeParagraph(vector<string> text, int font_size, int space, TFT_eSPI tft){
    int y = 0;
    size_t length = text.size();
    for (int i = 0; i < length; i++) {
        tft.drawString(text[i].c_str(), 0, y, font_size);
        y += space;
    }
}

void OLED::lcdSetup(){
    TFT_eSPI tft = TFT_eSPI();  
    unsigned long targetTime = 0; 

    tft.init(); 
    tft.setRotation(3);
    targetTime = millis();

    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_RED, TFT_BLACK);

    vector<string> text { "Homero", "Perez", "Mata", "2022"};
    writeParagraph(text, 4, 26, tft);
}




