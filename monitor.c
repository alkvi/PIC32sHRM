/* monitor.c

This file written 2017 by akv
Latest update 2017-11-18

*/

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include <math.h>
#include "projecthead.h"  /* function declarations! */

int rawVal = 0;
int prevRaw = 0;
int ewma = 0;
int prevEwma = 0;
double alpha = 0.05;
float bpm = 0;
int beats = 0;
int timerCount = 0;

int mode = 0;
int drawEwma = 0;

//Interrupt services
void user_isr( void ) {

	//handle interrupt from ADC -- get new value from ADC buffer, clear flag
	if ( (IFS(1) & 0x2) > 0 ){
		rawVal = ADC1BUF0;
	    IFSCLR(1) = (0x2);
  }

  	//handle interrupt from timer 2
  	if ( (IFS(0) & 0x100) > 0 ){
  		timerCount += 1;
	    IFSCLR(0) = (1 << 8);
  	}

  	//handle interrupt from switches (now only sw4)
  	if ( (IFS(0) & 0x80000) > 0 ){
  		drawEwma = 1 - drawEwma;
	    IFSCLR(0) = (1 << 19);
  	}

}

//should really be on a very short timer to blink properly via one call
void blinkLED(char leds){
	PORTESET = leds;
	PORTECLR = leds;
}

void initRate(void){
	ewma = rawVal;
	prevEwma = rawVal;
	prevRaw = rawVal;
}

void setupTimer(void){
	//clear TMR2
  	TMR2 = 0;
  	//set scaling to 256:1 (bits 4 to 6 on). 
  	T2CON = 0x70;
  	//set period to 6250 for 20 ms (with 80 MHz clock)
  	PR2 = 6250;

  	//clear interrupt flag (bit 8 in IFS0)
  	IFS(0) &= (0 << 8);
  	//set interrupt prios to highest (bits 2 3 4 in IPC2) (and IPC2 bits 0 and 1 for subprio)
  	IPC(2) |= 0x1f;
  	//enable timer 2 interrupt (bit 8 in IEC0)
  	IEC(0) |= 0x100;
}

//calculate and update the EWMA
void updateEWMA(void){
	ewma = alpha*rawVal + (1-alpha)*prevEwma;
}

void calcRate(void){
	updateEWMA();
	if (rawVal > ewma && prevRaw < ewma){
		blinkLED(0xff);
		beats += 1;
		//start a timer
		//turn T2 on (bit 15)
  		T2CONSET = (1 << 15);
	}
	if (beats == 2){
		beats = 0;
		//end the timer and check value
		T2CONCLR = (1 << 15);
		TMR2 = 0;
		bpm = 60 / (0.02 * (float)timerCount);
		bpm = round(bpm);
		timerCount = 0;
	}
	prevEwma = ewma;
	prevRaw = rawVal;

}


int readPin(char pin){
    AD1CHS = pin << 16;       		//select ADC pin in AD1CHS<16:19>
    AD1CON1SET = 0x0002;           	//sample by setting SAMP bit
    while( AD1CON1 & 0x0002 );      //wait until internal counter ends sampling
    while( !AD1CON1 & 0x0001 );    	//wait for auto convert
    return ADC1BUF0;                //get result
}

//draw the raw signal curve on screen
void drawSignal(void){
	
	draw_voltage(0,rawVal);
	if (drawEwma){
		draw_voltage(0,ewma);
	}

}

//write the heart rate in BPM on screen
void drawRate(void){

	char * rawConv = itoaconv(rawVal);
	display_string(0, rawConv);
	char * ewmaConv = itoaconv(ewma);
	display_string(1, ewmaConv);
	char * bpmConv = itoaconv(bpm);
	display_string(2, bpmConv);
	display_update();

}

void setupADCManual(void){

	AD1PCFGCLR = (1 << 4);	//set up A1 as analog, not digital
	TRISBSET = (1 << 4);	//initialize Port B so that bit 4 is set (use A1 as analog input)

	AD1CON1CLR = 0x8000;    // make sure ADC is off before messing with it
 
 	//AD1CON1<2>		0 - ASAM (sample bit manually set to start sampling)
	//AD1CON1<7:5>		111 - SSRC internal counter ends sampling and starts conversion (auto convert)
    AD1CON1 = 0x00e0;       
    AD1CON2 = 0;            // AD1CON2<15:13> set voltage reference to pins AVSS/AVDD
    AD1CON3 = 0x0f01;       // TAD = 4*TPB, acquisition time = 15*TAD 

    AD1CON1SET = 0x8000; 	//turn ADC back on 

}

void setupADCAuto(void){

	AD1PCFGCLR = (1 << 4);	//set up A1 as analog, not digital
	TRISBSET = (1 << 4);	//initialize Port B so that bit 4 is set (use A1 as analog input)

    AD1CON1CLR = 0x8000;    // make sure ADC is off before messing with it

    IPCSET(6) = (3 << 26);	//AD1IP
    IPCSET(6) = (1 << 24);	//AD1IS
    IFSCLR(1) = (0x2);		//AD1IF
    IECSET(1) = (0x2);		//AD1IE

	//AD1CON1<2>		1 - ASAM (sample bit automatically set)
	//AD1CON1<7:5>		111 - SSRC internal counter ends sampling and starts conversion (auto convert)
	AD1CON1SET = 0x00e4;

	//AD1CON2<1>, 		0 - BUFM Buffer configured as 16 bit buffer (only one)
    //AD1CON2<10>, 		0 - CSCNA  : Scan inputs
    AD1CON2 = 0x0000;

    //AD2CON2<5:2>		x - SMPI  : Interrupt flag set at after x completed conversions
    AD1CON2SET = (1 << 2);

    //AD1CON3<7:0> 		ADCS TAD = TPB * 2 * (ADCS<7:0> + 1) = 4 * TPB
    //AD1CON3<12:8> 	SAMC AD1CON3<12:8> * TAD = 15 * TAD
    AD1CON3 = 0x0f01;

    AD1CHS = 4 << 16;	//ADC1CHS

    AD1CON1SET = 0x8000; 	//turn ADC back on 

}

void setupIO(void){

	//initialize Port E so that bits 7 through 0 of Port E are set as outputs (i.e., the 8 least significant bits)
  	TRISECLR = 0xff;
  	//initialize the value to 0 on port E LEDs
  	PORTECLR = 0xff;
  	//initialize port D so that bits 11 through 5 of Port D are set as inputs (i.e switches + buttons)
  	TRISDSET = 0xfe0;

  	//clear interrupt flag for SW4, bit 19 in IFS(0)
	IFSCLR(0) = (1 << 19);
	//set up IPC4, bits 28 27 26 (main), 25 24 (sub)
	IPCSET(4) = (28 << 24);
	//also enable interrupts from SW4, which is INT4, and INT4IE is bit 19 in IEC0
	IECSET(0) = (1 << 19);

}

int getbtns(void){

	int btnvals = PORTD;
	btnvals &= 0xe0;
	btnvals >>= 5;
	return btnvals;

}

int getsw( void ){

	int switchvals = PORTD;
	switchvals &= 0xf00;
	switchvals >>= 8;
	return switchvals;

}

void monitorLoop(void){

	switch(mode) {
   	case 0 :
    	calcRate();
    	drawSignal();
    	break;
    case 1:
    	calcRate();
    	drawRate();
    	break;
    default :
    	calcRate();
    	drawSignal();
   }


	//poll buttons
  	int btnvals = getbtns();
  	if (btnvals > 0){
	    //go through bit by bit in btnvals
	    int bit1 = btnvals & 0x1;
	    int bit2 = btnvals & 0x2;
	    int bit3 = btnvals & 0x4;
	    
	    if (bit1 > 0){
	    	ewma = 0;
	    }
	    else if (bit2 > 0){
	    	clearScreen();
	     	mode = 1;
	    }
	    else if (bit3 > 0){
	    	clearScreen();
	     	mode = 0;
	    }
	}

}




