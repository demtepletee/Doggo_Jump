// Libraries
#include <xc.h>           // processor SFR definitions
#include <sys/attribs.h>  // __ISR macro
#include <stdio.h>
#include <stdlib.h>
#include <time.h> 
#include "animations.h"

// Defines
#define FRAME_DELAY .05 // delay between each frame

// PIC32 Settings
// DEVCFG0
#pragma config DEBUG = OFF // disable debugging
#pragma config JTAGEN = OFF  // disable jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // disable flash write protect
#pragma config BWP = OFF // disable boot write protect
#pragma config CP = OFF // disable code protect

// DEVCFG1
#pragma config FNOSC = PRIPLL // use primary oscillator with pll
#pragma config FSOSCEN = OFF // disable secondary oscillator
#pragma config IESO = OFF // disable switching clocks
#pragma config POSCMOD = HS // high speed crystal mode
#pragma config OSCIOFNC = OFF // disable clock output
#pragma config FPBDIV = DIV_1 // divide sysclk freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // disable clock switch and FSCM
#pragma config WDTPS = PS1048576 // use largest wdt
#pragma config WINDIS = OFF // use non-window mode wdt
#pragma config FWDTEN = OFF // wdt disabled
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz crystal
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz

// DEVCFG3
#pragma config USERID = 0xABBA // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations

// Function Prototypes
void wait(float time);

char doggo_height;  // used to check collisions
char game_over;     // triggered when collision detected

// Main
int main() {

    __builtin_disable_interrupts(); // disable interrupts while initializing things

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;

    // do your TRIS and LAT commands here
    TRISAbits.TRISA4 = 0; // A4 set as test output (LED)
    TRISBbits.TRISB4 = 1; // B4 set as jump button
    
    i2c_master_setup(); // set up i2c communications
    ssd1306_setup();    // set up OLED screen
    srand(time(0));     // random seed used to place obstacles
    
    LATAbits.LATA4 = 0; // initialize LED off
    
    char frame = 0;
    char i;
    game_over = 0;
    
    __builtin_enable_interrupts();
    
    // Infinite While Loop
    while (1) {
        // title screen
        ssd1306_drawString("D O G G O  J U M P !",1,3);  // print title
        print_pbtp();  // print "press button to play"
        ssd1306_update();
        while (1) {
            if (PORTBbits.RB4 == 0) { // check for button press
                ssd1306_clear();
                break;
            }
        }
        
        // intro animation
        ground_move(); // display ground
        for (i=0;i<12;i++) {
            doggo_wag();
            ssd1306_update();
            wait(FRAME_DELAY);
        }
        for (i=0;i<10;i++) {
            doggo_wag();
            doggo_woof();
            ssd1306_update();
            wait(FRAME_DELAY);
        }
        
        // game
        while (1) {
            for (frame=0; frame<6; frame++) {
                doggo_run(frame);   // play next frame in run animation
                ground_move();      // play next frame in ground move
                obstacles();        // move existing obstacles and create new ones
                ssd1306_update();
                wait(FRAME_DELAY);
                ssd1306_clear();
                if (PORTBbits.RB4 == 0) {  // poll for button press
                    // Jump
                    for (i=0;i<12;i++) {  // play 12 frames of jump animation
                        if (game_over) {  // if game_over triggered, leave loop early
                            break;
                        }
                        doggo_jump(i);  // play next frame of jump
                        doggo_height = jump_parabola[i];  // send height to doggo_height for collision detection
                        ground_move();
                        obstacles();
                        ssd1306_update();
                        wait(FRAME_DELAY);
                        ssd1306_clear();
                    }
                }
                if (game_over) {
                    break;
                }
            }
            if (game_over) {
                game_over = 0;  // reset game_over
                wait(2);
                break;
            }
        }
        
    }
}

// Function Definitions
void wait(float time) {
	_CP0_SET_COUNT(0);
	while (_CP0_GET_COUNT() < 24000000*time) {;}
}