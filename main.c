/* mipslabmain.c

   This file written 2015 by Axel Isaksson,
   modified 2015, 2017 by F Lundevall
   modified 2017 by akv

   Latest update 2017-11-18 by akv

   For copyright and licensing, see file COPYING */

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include <string.h>
#include "projecthead.h"  /* function declarations! */


int main(void) {

    /* This will set the peripheral bus clock to the same frequency
	  as the sysclock. That means 80 MHz, when the microcontroller
	  is running at 80 MHz. Changed 2017, as recommended by Axel. */
	SYSKEY = 0xAA996655;  /* Unlock OSCCON, step 1 */
	SYSKEY = 0x556699AA;  /* Unlock OSCCON, step 2 */
	while(OSCCON & (1 << 21)); /* Wait until PBDIV ready */
	OSCCONCLR = 0x180000; /* clear PBDIV bit <0,1> */
	while(OSCCON & (1 << 21));  /* Wait until PBDIV ready */
	SYSKEY = 0x0;  /* Lock OSCCON */
	
	/* Set up output pins */
	AD1PCFG = 0xFFFF;
	ODCE = 0x0;
	TRISECLR = 0xFF;
	PORTE = 0x0;
	
	/* Output pins for display signals */
	PORTF = 0xFFFF;
	PORTG = (1 << 9);
	ODCF = 0x0;
	ODCG = 0x0;
	TRISFCLR = 0x70;
	TRISGCLR = 0x200;
	
	/* Set up input pins */
	TRISDSET = (1 << 8);
	TRISFSET = (1 << 1);
	
	/* Set up SPI as master */
	SPI2CON = 0;
	SPI2BRG = 4;
	/* SPI2STAT bit SPIROV = 0; */
	SPI2STATCLR = 0x40;
	/* SPI2CON bit CKP = 1; */
    SPI2CONSET = 0x40;
	/* SPI2CON bit MSTEN = 1; */
	SPI2CONSET = 0x20;
	/* SPI2CON bit ON = 1; */
	SPI2CONSET = 0x8000;
	
	display_init();
	//display_string(0, "rad 0");
	display_update();
		
	//initialize monitor things
	asm("ei");
	//setupADCManual();
	setupADCAuto();


  	//clear TMR2
  	TMR2 = 0;
  	//set scaling to 256:1 (bits 4 to 6 on). 
  	T2CON = 0x70;
  	//set period to 31 250 for 100 ms (with 80 MHz clock)
  	PR2 = 31250;

  	//clear interrupt flag (bit 8 in IFS0)
  	IFS(0) &= (0 << 8);
  	//set interrupt prios to highest (bits 2 3 4 in IPC2) (and IPC2 bits 0 and 1 for subprio)
  	IPC(2) |= 0x1f;
  	//enable timer 2 interrupt (bit 8 in IEC0)
  	IEC(0) |= 0x100;

  	//turn T2 on (bit 15)
  	//T2CON |= (1 << 15);

	while( 1 )
	{
		//do stuff
		monitor();
	}
	return 0;
}








