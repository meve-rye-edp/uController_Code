#include "UART.h"
#include <stdio.h>
#include <stdlib.h>
#include "p33FJ256GP710.h"
#include "lcd.h"
#include "ADC.h"
#include "delay.h"

//ADC CHANNELS
// Each channel was selected such that a ground pin is right next to the input
// pin while the pin needed to be a ADC pin

#define Moduleinit 0x010B
#define B1ADC 3 //AN3 Selected for 'Battery Module' 1
#define B2ADC 0 //AN0 Selected for 'Battery Module' 2
#define B3ADC 1 //AN1 Selected for 'Battery Module' 3
#define B4ADC 8 //AN8 Selected for 'Battery Module' 4
#define multiplier 11
#define DC_OFFSET 8.228
#define G1 10.8703
#define G2 11.165
#define G3 11.55
#define G4 10.723

// Switches
#define SW1 PORTDbits.RD6
#define SW2 PORTDbits.RD7
#define SW3 PORTAbits.RA7
#define SW4 PORTDbits.RD13
#define LEDset TRISA
//LEDS
#define LED PORTA


//***************************************************************************************//

_FOSCSEL(FNOSC_FRC);			// Internal FRC oscillator
_FOSC(FCKSM_CSECMD & OSCIOFNC_OFF  & POSCMD_NONE);  
								// Clock Switching is enabled and Fail Safe Clock Monitor is disabled
								// OSC2 Pin Function: OSC2 is Clock Output
								// Primary Oscillator Mode: XT Crystanl
_FWDT(FWDTEN_OFF);									//Turn off WatchDog Timer
_FGS(GCP_OFF);										//Turn off code protect
_FPOR( FPWRT_PWR1 );	

void Update_LCD( void );
double voltage1=0;
double voltage2=0;
double voltage3=0;
double voltage4=0;

	double v1 =0;
	double v2=0;
	double v3=0;
	double v4=0;

//Four debouncer Switches
int	SW1Pressed=0;
int	SW2Pressed=0;
int SW3Pressed=0;
int	SW4Pressed=0;
double average = 4.15;
#define TRUE	1
#define FALSE	0
unsigned char S3Flag, S4Flag, S5Flag, S6Flag;
char result [24];
char status_result [24];



void InitPorts() {
	// S3 (portD Pin 6)
	// S6 (portD Pin 7)
	// S5 (portA Pin 7)
	// S4 (portD Pin 13)
	
	TRISD = 0x20C0;	// D6,7,13 inputs
	AD1PCFGHbits.PCFG23 = 1;	// This is important.  RA7 is muxxed with AN23,
		// So we need to config the pin as DIGITAL
	TRISA = 0x0080;	// only 0th bit needs be output. A7 is input
	S3Flag = S4Flag = S5Flag = S6Flag = 0;	// Some Debounce Flags
	PORTA= 0x00f0;
	LED=0;

}


void VoltageRead (){
	
	Delay(500);
	v1 = readADC(B1ADC); //Read ADC
	voltage1 =(v1/1024)*3.3*G1-DC_OFFSET;
	
	Delay(500);
	
	v2 = readADC(B2ADC); //Read ADC
	voltage2 = (v2/1024)*3.3*G2-DC_OFFSET;
	
	Delay(500);
	
	v3 = readADC(B3ADC); //Read ADC
	voltage3 = (v3/1024)*3.3*G3-DC_OFFSET;

	Delay(500);

	v4 = readADC(B4ADC); //Read ADC
	voltage4 = (v4/1024)*3.3*G4-DC_OFFSET;

}

void SoftwareDebounce() {

		if(PORTDbits.RD6 == FALSE) {
		while(!PORTDbits.RD6){}
	
		if(S3Flag == FALSE) {
			S3Flag = TRUE;
			
			Delay(1000);
			home_clr();
			VoltageRead();
			sprintf(result,"Voltage 1: %0.2fV",voltage1);
			puts_lcd(&result);
			check();
			rs232_snd_str(result);
			delimit();

			
			
		}
	}
	else {
		S3Flag = FALSE;
	}

	if(PORTDbits.RD7 == FALSE) {
	while(!PORTDbits.RD7){}
		if( S6Flag == FALSE ) {
			S6Flag = TRUE;

			home_clr();
		
			VoltageRead();
			sprintf(result,"Voltage 2: %0.2fV",voltage2);
			puts_lcd(&result);
			check();
			rs232_snd_str(result);
			delimit();	
			
		}
	}
	else {
		S6Flag = FALSE;
	}
	if(PORTAbits.RA7 == FALSE) {
		while(!PORTAbits.RA7){}
		if( S5Flag == FALSE ) {
			S5Flag = TRUE;
			home_clr();
		
			VoltageRead();
			sprintf(result,"Voltage 3: %0.2fV",voltage3);
			puts_lcd(&result);
			check();
			rs232_snd_str(result);
			delimit();	
			
		}
	}
	else {
		S5Flag = FALSE;
	}
	if(PORTDbits.RD13 == FALSE) {
		while(!PORTDbits.RD13){}
		if( S4Flag == FALSE ) {
			S4Flag = TRUE;
			home_clr();
		
			VoltageRead();
			
			sprintf(result,"Voltage 4: %0.2fV",voltage4);
			puts_lcd(&result);
			check();
			rs232_snd_str(result);
			rs232_snd_str(status_result);
			delimit();	
		}
	}
	else {
		S4Flag = FALSE;
	}
}
int main(void) {

		
	InitClock();	// This is the PLL settings

	InitUART2();	// Initialize UART2 for 9600,8,N,1 TX/RX

	InitPorts();	// LEDs outputs, Switches Inputs

	Init_LCD();


/* Welcome message */
	home_clr();
	puts_lcd("EDP 2013" );
	line_2();
	puts_lcd("MileStone 3");
	
	initADC(Moduleinit);
	AD1PCFGHbits.PCFG23 = 1;
	TRISB = Moduleinit; //Sets RB3/AN3 as input since AN3 is being used as analog Input
//	TRISA = 0x00FE;			//Make all PORTs all outputs

	while(1) {	// The ever versatile Infinite Loop!
		SoftwareDebounce();
	}
}

//This Function Compares all the voltages and if one is less or more then the average the battery module is unbalanced

void check (){

		if (voltage1<(average*0.8) || voltage2>(average*1.2)) {
			
				line_2();
			//	status_result[24] = "Battery 1 FAULTY";
				puts_lcd("Battery 1 FAULTY");
				LED=0x0001;
			}else if(voltage2<(average*0.8) || voltage2>(average*1.2)){
			
				line_2();
			//	status_result[24]={"Battery 2 FAULTY"};
				puts_lcd("Battery 2 FAULTY");
				LED=0x0003;
			}else if(voltage3<(average*0.8) || voltage3>(average*1.2)){
			
				line_2();
			//	status_result[24]={"Battery 3 FAULTY"};
				puts_lcd("Battery 3 FAULTY");
				LED=0x0007;

			}else if(voltage4<(average*0.8) | voltage4>(average*1.2)){
			
				line_2();
			//	status_result[24]={"Battery 4 FAULTY"};
				puts_lcd("Battery 4 FAULTY");
				LED=0x000f;
			}else{

				line_2();
			//	status_result[24] = {"Module Balanced"};
				puts_lcd("Module Balanced");
			LED=0;
			}

}

		/* if(!SW1){
		
			SW1Pressed=1;
			SW2Pressed=0;
			SW3Pressed=0;
			SW4Pressed=0;
		

		}else if (!SW2) {
		
			SW1Pressed=0;
			SW2Pressed=1;
			SW3Pressed=0;
			SW4Pressed=0;
		
		}else if (!SW3) {
		
			SW1Pressed=0;
			SW2Pressed=0;
			SW3Pressed=1;
			SW4Pressed=0;
			

		}else if (!SW4) {

			SW1Pressed=0;
			SW2Pressed=0;
			SW3Pressed=0;
			SW4Pressed=1;
		}

*/
// Interrupts for UART


void __attribute__ ((interrupt, no_auto_psv)) _U2RXInterrupt(void) {
	LATA = U2RXREG;
	IFS1bits.U2RXIF = 0;
}
void __attribute__ ((interrupt, no_auto_psv)) _U2TXInterrupt(void) {
	IFS1bits.U2TXIF = 0;
}
