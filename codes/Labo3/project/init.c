#include <xc.h>

//  BITS DE CONFIGURATION
///////////////////////////////////////////////////////////////////////////////
// disables the JTAG to prevent conflicts with TRISA
_FICD(ICS_PGD1 & JTAGEN_OFF);

// disables the watchdog
_FWDT(FWDTEN_OFF);

// The dsPIC will be clocked by the primary oscillator with a 10MHz crystal.
// We want to use the PLL to obtain Fosc = 80MHz ( <=> 40MIPS ).
// Problem : with a 10MHz crystal, PLL constraints are not met with the
// default parameter.
// Solution :
//	- boot using the internal FRC oscillator as clock source,
//	- set the right PLL parameters to obtain Fosc = 80MHz, without violating
//	  the PLL constraints,
//	- switch the clock source to the PLL
//	- wait for the end of the clock switch
//
// Select internal FRC oscillator as clock source
_FOSCSEL(FNOSC_FRC);
// enables clock switching and configure the primary oscillator for a 10MHz crystal
_FOSC(FCKSM_CSECMD & OSCIOFNC_OFF & POSCMD_XT);


void oscillatorInit(void) {
    PLLFBD = 30; // M=32
    CLKDIVbits.PLLPOST = 0; // N1=2
    CLKDIVbits.PLLPRE = 0; // N2=2
    __builtin_write_OSCCONH( 3 );
    __builtin_write_OSCCONL( 1 );
    // Wait for Clock switch to occur
    while (OSCCONbits.COSC != 0b011);
}
