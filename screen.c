/*
   This file written 2017 by akv
   Some parts are original code written by F Lundevall, Axel Isaksson


   For copyright and licensing, see file COPYING */

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <stddef.h>
#include <math.h>
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "projecthead.h"  /* Declatations for these labs */

#define ZERO_ANY(T, a, n) do{\
   T *a_ = (a);\
   size_t n_ = (n);\
   for (; n_ > 0; --n_, ++a_)\
     *a_ = (T) { 0 };\
} while (0)

/* Declare a helper function which is local to this file */
static void num32asc( char * s, int ); 

#define DISPLAY_CHANGE_TO_COMMAND_MODE (PORTFCLR = 0x10)
#define DISPLAY_CHANGE_TO_DATA_MODE (PORTFSET = 0x10)

#define DISPLAY_ACTIVATE_RESET (PORTGCLR = 0x200)
#define DISPLAY_DO_NOT_RESET (PORTGSET = 0x200)

#define DISPLAY_ACTIVATE_VDD (PORTFCLR = 0x40)
#define DISPLAY_ACTIVATE_VBAT (PORTFCLR = 0x20)

#define DISPLAY_TURN_OFF_VDD (PORTFSET = 0x40)
#define DISPLAY_TURN_OFF_VBAT (PORTFSET = 0x20)

#define SIGAREA 128

char monBuf0[SIGAREA];
char monBuf1[SIGAREA];
char monBuf2[SIGAREA];
char monBuf3[SIGAREA];
int bufPos = 0;

//clear the whole screen by sending 0x0 and 0x10, plus spi 0 everything
void clearScreen(void){

	int i, j;

	for(i = 0; i < 4; i++) {

		DISPLAY_CHANGE_TO_COMMAND_MODE;

		spi_send_recv(0x22);
		spi_send_recv(i);

		spi_send_recv(0x0);
		spi_send_recv(0x10);
					
		DISPLAY_CHANGE_TO_DATA_MODE;
			
		for(j = 0; j < 128; j++)
			spi_send_recv(0);

	}

}

//given a 10 bit integer voltage value, draw a dot on the screen
void draw_voltage(int x, int val) {
	int i, j;
	char * pointy;

	//invert value since coordinates are flipped
	val = 1023 - val;
	val = val / 32;

	//figure out which page coordinate should fall in
	if (val < 8){
    	i = 0;
    	pointy = monBuf0;
    }
    else if (val < 16){
    	i = 1;
    	pointy = monBuf1;
    }
    else if (val < 24){
    	i = 2;
    	pointy = monBuf2;
    }
    else{
    	i = 3;
    	pointy = monBuf3;
    }

	DISPLAY_CHANGE_TO_COMMAND_MODE;

	spi_send_recv(0x22);
	spi_send_recv(i);
		
	spi_send_recv(x & 0xF);
	spi_send_recv(0x10 | ((x >> 4) & 0xF));
		
	DISPLAY_CHANGE_TO_DATA_MODE;

	//set bit 0 to 7 depending on position within page, to draw position
	int yPos = val - i*8;
    int yCoord = (1 << yPos);
    *(pointy+bufPos) = yCoord;
		
	for(j = 0; j < SIGAREA; j++)
		spi_send_recv(*(pointy+j));

	//go to next buffer position. if end of screen, reset to start of screen and clear
	bufPos += 1;
	if (bufPos > SIGAREA-1){
		bufPos = 0;
		ZERO_ANY(char,monBuf0,SIGAREA);
		ZERO_ANY(char,monBuf1,SIGAREA);
		ZERO_ANY(char,monBuf2,SIGAREA);
		ZERO_ANY(char,monBuf3,SIGAREA);
		clearScreen();
	}


}







