/* monitor.c

This file written 2017 by akv
Latest update 2017-11-18

*/

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include <math.h>
#include "projecthead.h"  /* function declarations! */

int testval = 0;
int rawVal = 0;
float testval2 = 0;
float testsin = 0;


//Interrupt services
void user_isr( void ) {

	//handle interrupt from ADC -- get new value from ADC buffer, clear flag
	if ( (IFS(1) & 0x2) > 0 ){
		rawVal = ADC1BUF0;
	    IFSCLR(1) = (0x2);
  }

  	//handle interrupt from timer 2
  	if ( (IFS(0) & 0x100) > 0 ){
	    IFS(0) &= (0 << 8);
  	}

}


int readPin(char pin){
    AD1CHS = pin << 16;       		//select ADC pin in AD1CHS<16:19>
    AD1CON1SET = 0x0002;           	//sample by setting SAMP bit
    while( AD1CON1 & 0x0002 );      //wait until internal counter ends sampling
    while( !AD1CON1 & 0x0001 );    	//wait for auto convert
    return ADC1BUF0;                //get result
}


void monitor(void) {

	/*char * rawConv = itoaconv(rawVal);
	display_string(1, rawConv);
	display_update();*/
	
	/*testsin = 500 + sin(testval2)*500;
	testsin = round(testsin);
	draw_voltage(0,(int)testsin);
	testval2 += 0.07;*/

	draw_voltage(0,rawVal);

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


