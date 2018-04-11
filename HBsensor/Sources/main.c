// filename ******** Main.C ************** 

//***********************************************************************
// Simple ADC example for the Technological Arts EsduinoXtreme board
// by Carl Barnes, 12/03/2014
//***********************************************************************

#include <hidef.h>      /* common defines and macros */
#include "derivative.h"  /* derivative information */
#include "SCI.h"

char string[20];
unsigned short val; 
unsigned int on=0;
unsigned int thr;
unsigned int abvthr;
unsigned int bpm;
unsigned int time;
unsigned int first_dig;
unsigned int sec_dig;
unsigned int third_dig;

void msDelay(unsigned int);
interrupt VectorNumber_Vtimch0 void ISR_Vtimch0(void);

void Out(void){
 PTJ ^=0x01; //Toggle PJ0

}

void OutCRLF(void){
  SCI_OutChar(CR);
  SCI_OutChar(LF);
}
 void SetClk12(void){
  
  CPMUREFDIV = 0x00;        //Using ref clock 1mhz, not oscill 
  CPMUSYNR = 0x0B;          //Set SYNDIV >> 11+1 dec>> 24=2*1mhz*12=24mhz
  CPMUPOSTDIV = 0x00;       //no post div
  CPMUCLKS = 0x80;          //Select PLL clock 24mhz/2=12mhz
  CPMUOSC = 0x00;           //turn off oscill frequency
  
  while(!(CPMUFLG & 0x08));   //wait for LOCK Flag    
  
  }

void main(void) {
 //Set Ports
  SetClk12();
  DDRJ = 0x01;      //LED
  DDR1AD=0xFF;      //AD
  DDRM=0xFF;        //M
  DDRP=0xFF;        //PP
  
 //DRJ |=0x01;
  
/*
 * The next six assignment statements configure the Timer Input Capture                                                   
 */           
  TSCR1 = 0x90;    //Timer System Control Register 1
                    // TSCR1[7] = TEN:  Timer Enable (0-disable, 1-enable)
                    // TSCR1[6] = TSWAI:  Timer runs during WAI (0-enable, 1-disable)
                    // TSCR1[5] = TSFRZ:  Timer runs during WAI (0-enable, 1-disable)
                    // TSCR1[4] = TFFCA:  Timer Fast Flag Clear All (0-normal 1-read/write clears interrupt flags)
                    // TSCR1[3] = PRT:  Precision Timer (0-legacy, 1-precision)
                    // TSCR1[2:0] not used

  TSCR2 = 0x00;    //Timer System Control Register 2
                    // TSCR2[7] = TOI: Timer Overflow Interrupt Enable (0-inhibited, 1-hardware irq when TOF=1)
                    // TSCR2[6:3] not used
                    // TSCR2[2:0] = Timer Prescaler Select: See Table22-12 of MC9S12G Family Reference Manual r1.25 (set for bus/1)
  
                    
  TIOS = 0xFE;     //Timer Input Capture or Output capture
                    //set TIC[0] and input (similar to DDR)
  PERT = 0x01;     //Enable Pull-Up resistor on TIC[0]

  TCTL3 = 0x00;    //TCTL3 & TCTL4 configure which edge(s) to capture
  TCTL4 = 0x02;    //Configured for falling edge on TIC[0]

/*
 * The next one assignment statement configures the Timer Interrupt Enable                                                   
 */           
   
  TIE = 0x01;      //Timer Interrupt Enable

/*
 * The next one assignment statement configures the ESDX to catch Interrupt Requests                                                   
 */           
  
	EnableInterrupts; //CodeWarrior's method of enabling interrupts


// Setup and enable ADC channel 0
// Refer to Chapter 14 in S12G Reference Manual for ADC subsystem details

	ATDCTL1 = 0x4F;		// set for 12-bit resolution
	ATDCTL3 = 0x88;		// right justified, one sample per sequence
	ATDCTL4 = 0x05;		// prescaler = 5 ATD clock = 12mhz /(2 * (5 + 1)) == 1.04MHz
	ATDCTL5 = 0x26;		// continuous conversion on channel 6
	                    


// Setup LED and SCI
  SCI_Init(9600);
  
  thr=400;
  abvthr=0;
  
  for (;;){
  
  if (on == 0){
    val = ATDDR0;  
    SCI_OutUDec(val);
    OutCRLF();
    
    
  if(val >= thr && abvthr == 0){
    abvthr = 1;
    bpm = 60000/time;
    OutCRLF();
    time = 0;
      
    first_dig = bpm%10;
    bpm /= 10;
    sec_dig = bpm%10;
    bpm /= 10;
    third_dig = bpm%10;
  
 
    PT1AD = (sec_dig & 0x07)*16; //A0-A3
    PT1AD += first_dig;
    PTM = third_dig & 0x01;     //PTM0
    PTP = (sec_dig & 0x08)*16;      //P0-P3 
    PTP+= sec_dig;
  }
    
  if (val < thr && abvthr == 1){
    abvthr = 0;
  }
    
  time += 50;
    
    if (time > 5000){
      time = 0;
      SCI_OutUDec(0);
      OutCRLF();
      PT1AD = 0x00;
      }
     msDelay(20);

  }
}
}
interrupt VectorNumber_Vtimch0 void ISR_Vtimch0(void){
 unsigned int temp;
 if(on==0){   
   on=1;
   Out();
   SCI_OutChar(CR);

 } else{
   on=0;
   Out();

 }     
 msDelay(20);
 temp=TC0;
 
}

void msDelay(unsigned int k)
{
 //x/68=12/6.25
  unsigned int i;
  unsigned int j;
  
  for(j=0; j<k;j++){
    for(i=0; i<131;i++){
      PTJ=PTJ;
      PTJ=PTJ;
      PTJ=PTJ;
      PTJ=PTJ;
      PTJ=PTJ;
    }
  } 
}