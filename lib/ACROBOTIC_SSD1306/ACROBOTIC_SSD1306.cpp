/*
  06/01/2016
  Author: Makerbro
  Platforms: ESP8266
  Language: C++
  File: ACROBOTIC_SSD1306.cpp
  ------------------------------------------------------------------------
  Description: 
  SSD1306 OLED Driver Library.
  ------------------------------------------------------------------------
  Please consider buying products from ACROBOTIC to help fund future
  Open-Source projects like this! We'll always put our best effort in every
  project, and release all our design files and code for you to use. 
  https://acrobotic.com/
  ------------------------------------------------------------------------
  License:
  Released under the MIT license. Please check LICENSE.txt for more
  information.  All text above must be included in any redistribution. 
*/

#include "ACROBOTIC_SSD1306.h"
#include "gfxfont.h"
#include "../I2C/I2C.h"

#define FONT_PULL_UP 14
#define FONT_SIZE 30

static bool connected = false;


ACROBOTIC_SSD1306::ACROBOTIC_SSD1306() {
    I2c.begin();
    I2c.setSpeed(true);
}

void ACROBOTIC_SSD1306::init(bool flip) {
    sendCommand(0xAE);            //display off
    sendCommand(0xA6);            //Set Normal Display (default)
    sendCommand(0xAE);            //DISPLAYOFF
    sendCommand(0xD5);            //SETDISPLAYCLOCKDIV
    sendCommand(0x80);            // the suggested ratio 0x80
    sendCommand(0xA8);            //SSD1306_SETMULTIPLEX
    sendCommand(0x3F);
    sendCommand(0xD3);            //SETDISPLAYOFFSET
    sendCommand(0x0);             //no offset
    sendCommand(0x40 | 0x0);        //SETSTARTLINE
    sendCommand(0x8D);            //CHARGEPUMP
    sendCommand(0x14);
    sendCommand(0x20);            //MEMORYMODE
    sendCommand(0x00);            //0x0 act like ks0108
    if (flip){
        sendCommand(0xA0);            //SEGREMAP   Mirror screen horizontally (A0)
        sendCommand(0xC0);            //COMSCANDEC Rotate screen vertically (C0)
    }
     else {
        sendCommand(0xA1);            //SEGREMAP   Mirror screen horizontally (A0)
        sendCommand(0xC8);            //COMSCANDEC Rotate screen vertically (C0)
    }
    sendCommand(0xDA);            //0xDA
    sendCommand(0x12);            //COMSCANDEC
    sendCommand(0x81);            //SETCONTRAST
    sendCommand(0xCF);            //
    sendCommand(0xd9);            //SETPRECHARGE
    sendCommand(0xF1);
    sendCommand(0xDB);            //SETVCOMDETECT
    sendCommand(0x40);
    sendCommand(0xA4);            //DISPLAYALLON_RESUME
//    sendCommand(0xA6);            //NORMALDISPLAY
    sendCommand(0xA7);            //INVERSEDISPLAY
    clearDisplay();
    sendCommand(0x2E);            //Stop scroll
    sendCommand(0x20);            //Set Memory Addressing Mode
    sendCommand(0x00);            //Set Memory Addressing Mode ab Horizontal addressing mode
    setFont(font8x8);
}

bool ACROBOTIC_SSD1306::checkAndInit(bool flip) {
    if (!connected && checkConnected()) {
        oled.setGfxFont(&FreeSans12pt7b);
        init(flip);
        return true;
    } else {
        return false;
    }
}

bool ACROBOTIC_SSD1306::checkConnected() {
    connected = I2c.check(SSD1306_Address);
    return connected;
}

bool ACROBOTIC_SSD1306::getConnected() {
    return connected;
}


void ACROBOTIC_SSD1306::setFont(const uint8_t *font) {
    m_font = font;
    m_font_width = pgm_read_byte(&m_font[0]);
}

void ACROBOTIC_SSD1306::setGfxFont(const GFXfont *_gfxfont) {
    gfxfont = (GFXfont *) _gfxfont;
}

void ACROBOTIC_SSD1306::sendCommand(unsigned char command) {
    I2c.write((int8_t) SSD1306_Address, (int8_t) SSD1306_Command_Mode, (int8_t) command);
}

void ACROBOTIC_SSD1306::setBrightness(unsigned char Brightness) {
    sendCommand(SSD1306_Set_Brightness_Cmd);
    sendCommand(Brightness);
}

void ACROBOTIC_SSD1306::setHorizontalMode() {
    addressingMode = HORIZONTAL_MODE;
    sendCommand(0x20);                      //set addressing mode
    sendCommand(0x00);                      //set horizontal addressing mode
}

void ACROBOTIC_SSD1306::setPageMode() {
    addressingMode = PAGE_MODE;
    sendCommand(0x20);                      //set addressing mode
    sendCommand(0x02);                      //set page addressing mode
}

void ACROBOTIC_SSD1306::setTextXY(unsigned char row, unsigned char col) {
    sendCommand(0xB0 + row);                          //set page address
    sendCommand(0x00 + (m_font_width * col & 0x0F));    //set column lower addr
    sendCommand(0x10 + ((m_font_width * col >> 4) & 0x0F)); //set column higher addr
}

uint8_t inline reverse(uint8_t b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

void ACROBOTIC_SSD1306::outputLineGauge(uint8_t row, const uint8_t mark_col, const uint8_t limit1, const uint8_t limit2, boolean inverse) {
    setTextXY(row, 0);
    uint8_t mark = 0;
    uint8_t line = 0;
    for(uint8_t col = 0; col < 128; col++){
        line = static_cast<uint8_t>((col == 0 || col == limit1 || col == limit1 - 1 || col == limit1 + 1 || col == limit2 || col == limit2 + 1  || col == limit2 - 1  || col == 127 || col == 63) ? 0xF : 0x8);
        int8_t delta = mark_col - col;
        delta = delta >= 0 ? delta : -delta;
        mark = delta > 3 ? 0x0 : 0xF >> delta << (4 + delta);
        line = line | mark;
        sendData(inverse ? reverse(line) : line );
    }
}

void ACROBOTIC_SSD1306::outputTextXY(uint8_t row, uint8_t col, const char *_text, const bool centered,
                                     const bool _dither) {

    const uint8_t first = pgm_read_byte(&gfxfont->first);
    uint8_t yAdvance = pgm_read_byte(&gfxfont->yAdvance);
    yAdvance = FONT_SIZE;

    uint8_t pgh = (yAdvance + 7) / 8;

    setPageMode(SSD1306_Set_PageRows_Cmd, row, row + pgh - 1);

    static uint8_t lastWidth = lastWidth == 0 ? getTextWidth(_text) : lastWidth;
    uint8_t newWidth = getTextWidth(_text);
    cleanPages(pgh, col, lastWidth, newWidth, centered);
    lastWidth = newWidth;

    setPageMode(SSD1306_Set_PageRows_Cmd, row, row + pgh - 1);

    int8_t cursor = 0;
    uint8_t i = 0;

    while (_text[i] != '\0') {
        unsigned char cr = (unsigned char) _text[i];
        cr -= first;

        GFXglyph *glyph = &(((GFXglyph *) pgm_read_ptr(&gfxfont->glyph))[cr]);
        uint16_t bitmapOffset = pgm_read_word(&glyph->bitmapOffset);

        int8_t xAdvance = pgm_read_byte(&glyph->xAdvance);

        int8_t width = pgm_read_byte(&glyph->width);
        int8_t height = pgm_read_byte(&glyph->height);

        int8_t xOffset = pgm_read_byte(&glyph->xOffset);
        int8_t yOffset = pgm_read_byte(&glyph->yOffset);

        uint8_t *bitmap = (uint8_t *) pgm_read_ptr(&gfxfont->bitmap);

        int8_t c_width = xAdvance;
        int8_t c_height = pgh * 8 + yOffset - FONT_PULL_UP;

        int8_t col_x = col + (centered ? -(newWidth + 1) / 2 : 0) + cursor;

        setPageMode(SSD1306_Set_PageCols_Cmd, col_x,  col_x + c_width - 1);

        cursor += xAdvance;
        for (uint8_t pg = 0; pg < pgh; pg++) {
            for (uint8_t c = 0; c < c_width; c++) {
                uint8_t cl = 0;
                for (uint8_t r = 0; r < 8; r++) {
                    uint8_t abs_r = (pg * 8 + r);
                    if (c < xOffset || abs_r < c_height || c >= width + xOffset || abs_r >= (c_height + height)) {
                        cl &= ~(1 << r);
                    } else {
                        uint8_t c_g = c - xOffset;
                        uint8_t r_g = abs_r - c_height;
                        uint16_t bit_ofs = r_g * width + c_g;
                        uint16_t ofs = bit_ofs / 8;
                        uint8_t bt = pgm_read_byte(&bitmap[bitmapOffset + ofs]);
                        uint8_t set = (bt & (0x80 >> (bit_ofs % 8))) >> (7 - bit_ofs % 8);
                        cl |= (set << r);
                    }
                }
                sendData(cl & (!_dither ? 0xFF : (c % 2 == 0 ? 0xAA: 0x55)));
            }
        }
        i++;
    }
    setWindow(0, 7, 0, 127);
}

void ACROBOTIC_SSD1306::setPageMode(uint8_t command, uint8_t start, uint8_t end) {
    sendCommand((unsigned char) command);                       //set page address
    sendCommand((unsigned char) start);
    sendCommand((unsigned char) end);
}

uint8_t ACROBOTIC_SSD1306::getTextWidth(const char *_text) {
    const uint8_t first = pgm_read_byte(&gfxfont->first);
    uint8_t result = 0;
    uint8_t i = 0;
    while (_text[i] != '\0') {
        unsigned char cr = (unsigned char) _text[i];
        cr -= first;
        GFXglyph *glyph = &(((GFXglyph *) pgm_read_ptr(&gfxfont->glyph))[cr]);
        int8_t width = pgm_read_byte(&glyph->width);
        int8_t xOffset = pgm_read_byte(&glyph->xOffset);
        int8_t xAdvance = pgm_read_byte(&glyph->xAdvance);
        i++;
        result += (uint8_t) (xAdvance);
    }
    return result;
}

void ACROBOTIC_SSD1306::setWindow(uint8_t row_s, uint8_t row_e, uint8_t col_s, uint8_t col_e) {
    setPageMode(SSD1306_Set_PageRows_Cmd, row_s, row_e);
    setPageMode(SSD1306_Set_PageCols_Cmd, col_s, col_e);
}

void ACROBOTIC_SSD1306::cleanPages(const uint8_t rows, const uint8_t col, const uint8_t _l_width, uint8_t _n_width,
                                   const bool centered) {
    if (_l_width > _n_width) {
        if (!centered) {
            cleanPage(rows, col + _n_width, col + _l_width);
        } else {
            cleanPage(rows, col - (_l_width + 1) / 2, col - (_n_width + 1) / 2);
            cleanPage(rows, col + _n_width / 2, col + (_l_width + 1) / 2);
        }
    }

}

void ACROBOTIC_SSD1306::cleanPage(const uint8_t rows, const uint8_t s_col, uint8_t e_col) {
    setPageMode(SSD1306_Set_PageCols_Cmd, s_col, e_col);
    for (uint16_t k = 0; k < (e_col - s_col + 1) * rows; k++) {
        sendData(0);
    }
}


void ACROBOTIC_SSD1306::clearDisplay() {
    unsigned char i, j;
    sendCommand(SSD1306_Display_Off_Cmd);     //display off
    for (j = 0; j < 8; j++) {
        setTextXY(j, 0);
        {
            for (i = 0; i < 16; i++)  //clear all columns
            {
                putChar(' ');
            }
        }
    }
    sendCommand(SSD1306_Display_On_Cmd);     //display on
    setTextXY(0, 0);
}

void ACROBOTIC_SSD1306::sendData(unsigned char Data) {
    I2c.write((int8_t) SSD1306_Address, (int8_t) SSD1306_Data_Mode, (int8_t) Data);
}

bool ACROBOTIC_SSD1306::putChar(unsigned char ch) {
    if (!m_font) return 0;
    //Ignore non-printable ASCII characters. This can be modified for
    //multilingual font.  
    if (ch < 32 || ch > 132) {
        ch = ' ';
    }
    for (unsigned char i = 0; i < m_font_width; i++) {
        // Font array starts at 0, ASCII starts at 32
        sendData(pgm_read_byte(&m_font[(ch - 32) * m_font_width + m_font_offset + i]));
    }
}

void ACROBOTIC_SSD1306::putString(const char *string) {
    unsigned char i = 0;
    while (string[i]) {
        putChar(string[i]);
        i++;
    }
}

unsigned char ACROBOTIC_SSD1306::putNumber(long long_num) {
    unsigned char char_buffer[10] = "";
    unsigned char i = 0;
    unsigned char f = 0;

    if (long_num < 0) {
        f = 1;
        putChar('-');
        long_num = -long_num;
    } else if (long_num == 0) {
        f = 1;
        putChar('0');
        return f;
    }

    while (long_num > 0) {
        char_buffer[i++] = long_num % 10;
        long_num /= 10;
    }

    f = f + i;
    for (; i > 0; i--) {
        putChar('0' + char_buffer[i - 1]);
    }
    return f;

}

unsigned char ACROBOTIC_SSD1306::putFloat(float floatNumber, unsigned char decimal) {
    unsigned int temp = 0;
    float decy = 0.0;
    float rounding = 0.5;
    unsigned char f = 0;
    if (floatNumber < 0.0) {
        putString("-");
        floatNumber = -floatNumber;
        f += 1;
    }
    for (unsigned char i = 0; i < decimal; ++i) {
        rounding /= 10.0;
    }
    floatNumber += rounding;

    temp = floatNumber;
    f += putNumber(temp);
    if (decimal > 0) {
        putChar('.');
        f += 1;
    }
    decy = floatNumber - temp;//decimal part,
    for (unsigned char i = 0; i < decimal; i++)//4
    {
        decy *= 10;// for the next decimal
        temp = decy;//get the decimal
        putNumber(temp);
        decy -= temp;
    }
    f += decimal;
    return f;
}

unsigned char ACROBOTIC_SSD1306::putFloat(float floatNumber) {
    unsigned char decimal = 2;
    unsigned int temp = 0;
    float decy = 0.0;
    float rounding = 0.5;
    unsigned char f = 0;
    if (floatNumber < 0.0) {
        putString("-");
        floatNumber = -floatNumber;
        f += 1;
    }
    for (unsigned char i = 0; i < decimal; ++i) {
        rounding /= 10.0;
    }
    floatNumber += rounding;

    temp = floatNumber;
    f += putNumber(temp);
    if (decimal > 0) {
        putChar('.');
        f += 1;
    }
    decy = floatNumber - temp;//decimal part,
    for (unsigned char i = 0; i < decimal; i++)//4
    {
        decy *= 10;// for the next decimal
        temp = decy;//get the decimal
        putNumber(temp);
        decy -= temp;
    }
    f += decimal;
    return f;
}

void ACROBOTIC_SSD1306::drawBitmap(unsigned char *bitmaparray, int bytes) {
    char localAddressMode = addressingMode;
    if (addressingMode != HORIZONTAL_MODE) {
        //Bitmap is drawn in horizontal mode
        setHorizontalMode();
    }

    for (int i = 0; i < bytes; i++) {
        sendData(pgm_read_byte(&bitmaparray[i]));
    }

    if (localAddressMode == PAGE_MODE) {
        //If pageMode was used earlier, restore it.
        setPageMode();
    }

}

void ACROBOTIC_SSD1306::setHorizontalScrollProperties(bool direction, unsigned char startPage, unsigned char endPage,
                                                      unsigned char scrollSpeed) {
    if (Scroll_Right == direction) {
        //Scroll right
        sendCommand(0x26);
    } else {
        //Scroll left
        sendCommand(0x27);

    }
    sendCommand(0x00);
    sendCommand(startPage);
    sendCommand(scrollSpeed);
    sendCommand(endPage);
    sendCommand(0x00);
    sendCommand(0xFF);
}

void ACROBOTIC_SSD1306::activateScroll() {
    sendCommand(SSD1306_Activate_Scroll_Cmd);
}

void ACROBOTIC_SSD1306::deactivateScroll() {
    sendCommand(SSD1306_Dectivate_Scroll_Cmd);
}

void ACROBOTIC_SSD1306::setNormalDisplay() {
    sendCommand(SSD1306_Normal_Display_Cmd);
}

void ACROBOTIC_SSD1306::setInverseDisplay() {
    sendCommand(SSD1306_Inverse_Display_Cmd);
}

void ACROBOTIC_SSD1306::displayOff() {
    sendCommand(SSD1306_Display_Off_Cmd);
}

void ACROBOTIC_SSD1306::displayOn() {
    sendCommand(SSD1306_Display_On_Cmd);
}

ACROBOTIC_SSD1306 oled;  // Pre-instantiate object
