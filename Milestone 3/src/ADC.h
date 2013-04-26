#include "p33FJ256GP710.h"


// Start with one pin being used to read a single battery
// Next read all four batteries from battery circuit.


// initialize the ADC for single conversion, select Analog input pins

//MODULE 2 ***********************************************************************************************************************************
void initADC( int amask)
{
    AD1PCFGH = amask;      // select analog input pins
    AD1CON1 = 0x00E8;     // data output format-integer, auto convert after end of sampling.12bit Operation mode. No need to include a delay loop to provide time for completion of sampling
    // Was 0x00e0 for singe channel

	AD1CSSL = 0;          // no scanning required
    AD1CON3 = 0x1FFF ;     // Use internal clock, max sample time avail is 31Tad, conversion time Tad = 128*Tcy = 128*(2/Fosc) = 256/8MHz = 32us which is > (required)75ns ); 31 Tad = 31*32us = 992us is the sample time. About 100samples/sec
    //Possibly 1FFFF Check Both
	AD1CON2 = 0x0300;          // use MUXA, AVss and AVdd are used as Vref+/-
	// Was 0x0000
    AD1CON1bits.ADON = 1; // turn on the ADC
} //initADC


int readADC(int channel)
{
	AD1CHS0bits.CH0SA = channel;			// Select the channel to convert
	IFS0bits.AD1IF = 0; 					// Clear ADC interrupt flag
	AD1CON1bits.DONE=0;						// Clear Done Flag
	AD1CON1bits.SAMP=1; 					// Start sampling 

	while(IFS0bits.AD1IF == 0);				// Wait for conversion complete

	AD1CON1bits.ASAM = 0; 					// Then stop sample/convert...
	return(ADC1BUF0);
}


