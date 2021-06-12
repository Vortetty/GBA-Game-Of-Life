/*
 *
 * This file is licenced under the Apache 2.0 
 * https://www.apache.org/licenses/LICENSE-2.0.txt
 *
 * ©2021 Vortetty/Tæmt modʒiɹæ/Winter
 *
 */


#define DEBUG_DRAW false


#include <math.h>
#include <bitset>
#include <gba_input.h>
#include <bitset>
#include <climits>
#include <cstring>
#include <vector>
#include <fat.h>

#define WHITE 0xffff
#define BLACK 0x0000
#define CURSOR_ON RGBToColor(0x32cd32)
#define CURSOR_OFF RGBToColor(0x228b22)


struct Vector2 {
    uint16_t x;
    uint16_t y; 
};

unsigned short* buffer = (unsigned short*)0x06000000;
uint8_t* cartRam = (uint8_t*)0x0E000000;

struct sideTracker {
    bool l;
    bool r; 
    bool t;
    bool b; 
};

unsigned short randint(unsigned short a, unsigned short b){
    float random = ((float) rand()) / (float) RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return (unsigned short)(a + r);
}

unsigned short RGBToColor(unsigned short r, unsigned short g, unsigned short b){
    return b/8*1024 + g/8*32 + r/8;
}

unsigned short RGBToColor(int rgb){
    unsigned short r = (rgb >> 16) & 0xff;
    unsigned short g = (rgb >> 8) & 0xff;
    unsigned short b = rgb & 0xff;
    return b/8*1024 + g/8*32 + r/8;
}

void drawNxN(unsigned short xoffset, unsigned short yoffset, unsigned short width, unsigned short height, unsigned short color){
    for (unsigned short x = 0; x < width; x++){
        for (unsigned short y = 0; y < height; y++){
            buffer[(xoffset+x)+(yoffset+y)*240] = color;
        }
    }
}

bool isPressed(unsigned short keys, unsigned short key){
    return (keys & key) == key;
}

int main(){
    // ✧ Setup ✧
    // ------------------------------------------------------------------------
    *(unsigned short*)0x04000000 = 0x0403; // Set mode, idk why but only this one works


    drawNxN(0, 0, 240, 160, BLACK); // Clear Back

    Vector2 position = {0, 0}; // Vec2 for position
    bool shouldPlay=false, shouldSetBit; // Whether it should simulate, whether it should set a bit
    unsigned short keys; // Stores what keys are pressed

    
    std::bitset<240*160> resetState;
    std::bitset<240*160> fieldBuf;
    std::bitset<240*160> field;
    int bit, lastBit, cnt;
    sideTracker sides;
    // ------------------------------------------------------------------------

    // ✧ The game ✧
    // ------------------------------------------------------------------------
    while (true){ // Run until game should exit
        scanKeys(); // Populate keys pressed
        keys = keysDown(); // Gets pressed keys 
        if (isPressed(keys, KEYPAD_BITS::KEY_UP)) position.y--; // if up is pressed decrement y
        if (isPressed(keys, KEYPAD_BITS::KEY_DOWN)) position.y++; // if down is pressed increment y
        if (isPressed(keys, KEYPAD_BITS::KEY_LEFT)) position.x--; // if left is pressed decrement x
        if (isPressed(keys, KEYPAD_BITS::KEY_RIGHT)) position.x++; // if right is pressed increment x
        if (isPressed(keys, KEYPAD_BITS::KEY_A)) shouldSetBit = true; // if it should activate a cell
        if (isPressed(keys, KEYPAD_BITS::KEY_START)) shouldPlay = !shouldPlay; // if it play the game
        if (isPressed(keys, KEYPAD_BITS::KEY_B)) {
        }; // if b is pressed save current field, not implemented

        lastBit = bit;
        bit = position.x + position.y*240;

        if (shouldSetBit){
            fieldBuf.flip(bit);
            resetState.flip(bit);
            shouldSetBit = false;
        }

        if (shouldPlay){
            for (int x = 0; x < 240; x++) {
                for (int y = 0; y < 160; y++) {
                    cnt = 0;

                    sides.l = x == 0;
                    sides.r = x == 239;
                    sides.t = y == 0;
                    sides.b = y == 159;

                    if (!sides.l) cnt += fieldBuf[(y)*240 + (x-1)] ? 1 : 0;
                    if (!sides.r) cnt += fieldBuf[(y)*240 + (x+1)] ? 1 : 0;
                    if (!sides.t) cnt += fieldBuf[(y-1)*240 + (x)] ? 1 : 0;
                    if (!sides.b) cnt += fieldBuf[(y+1)*240 + (x)] ? 1 : 0;

                    if ((!sides.l) && (!sides.t)) cnt += fieldBuf[(y-1)*240 + (x-1)] ? 1 : 0;
                    if ((!sides.r) && (!sides.t)) cnt += fieldBuf[(y-1)*240 + (x+1)] ? 1 : 0;
                    if ((!sides.r) && (!sides.b)) cnt += fieldBuf[(y+1)*240 + (x+1)] ? 1 : 0;
                    if ((!sides.l) && (!sides.b)) cnt += fieldBuf[(y+1)*240 + (x-1)] ? 1 : 0;

                    if ((cnt < 2 || cnt > 3) && fieldBuf[y*240 + x]) {
                        field[y*240 + x] = 0;
                    } 
                    else if (cnt == 3) {
                        field[y*240 + x] = 1;
                    } 
                    else if (cnt == 2 && fieldBuf[y*240 + x]) {
                        field[y*240 + x] = 1;
                    }

                    if (DEBUG_DRAW){
                        if ((cnt < 2 || cnt > 3) && fieldBuf[y*240 + x]) {
                            buffer[y*240 + x] = RGBToColor(0xff0033);
                        } 
                        else if (cnt == 3) {
                            buffer[y*240 + x] = RGBToColor(0x00ff33);
                        } 
                        else if (cnt == 2 && fieldBuf[y*240 + x]) {
                            buffer[y*240 + x] = RGBToColor(0xffff00);
                        } 
                        else {
                            buffer[y*240 + x] = RGBToColor(0x333333);
                        }
                    }

                    scanKeys(); // Populate keys pressed
                    keys = keysDown(); // Gets pressed keys 
                    if (isPressed(keys, KEYPAD_BITS::KEY_SELECT)) { // Reset field
                        x = y = 500;
                        shouldPlay = false;
                        fieldBuf = std::bitset<240*160>(resetState);
                        field = std::bitset<240*160>();

                        for (int i = 0; i < 240*160; i++) {
                            buffer[i] = field[i] ? WHITE : BLACK;
                        }
                    }
                    if (isPressed(keys, KEYPAD_BITS::KEY_START)) { // Pause but preserve field
                        x = y = 500;
                        shouldPlay = false;
                    }
                }
            }

            for (int i = 0; i < 240*160; i++) {
                if (field[i] != fieldBuf[i] || DEBUG_DRAW) 
                    buffer[i] = field[i] ? WHITE : BLACK;
            }

            fieldBuf = std::bitset<240*160>(field);
        } else {
            if (bit != lastBit){
                buffer[lastBit] = fieldBuf[lastBit] ? WHITE : BLACK;
            }

            buffer[position.x + position.y*240] = fieldBuf[position.x + position.y*240] ? CURSOR_ON : CURSOR_OFF;
        }
    }
    // ------------------------------------------------------------------------

    return 0; // Have to return 0
}
