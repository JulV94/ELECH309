#include "init.h"
#include "adc.h"
#include <xc.h>
#define D 0.1016f
#ifndef M_PI
    #define M_PI 3.141592653f
#endif
#define ACCEL 0.1
#define MAX_SPEED 0.4
#define KP_POS 8
#define KP_ROT 8




void encodersInit(){
    RPINR14bits.QEA1R = 19;//left encoder
    RPINR14bits.QEB1R = 20;
    
    RPINR16bits.QEA2R = 24;//right encoder
    RPINR16bits.QEB2R = 25;
    
    // Set QEI modes
    QEI1CONbits.QEIM = 0b111;
    QEI2CONbits.QEIM = 0b111;// x4 mode
    
    QEI1CONbits.SWPAB = 1; 
}

void motorsInit() {
    RPOR9bits.RP18R = 18; //Map output pin to OC1 (left)
    RPOR6bits.RP13R = 19; //Map output pin to OC2 (right)
    
    //Output compare 1
    OC1CONbits.OCTSEL = 0; //selectionne le timer 2
    OC1RS = 0*PR2; // nb d'instructions à l'état haut (Ton) = (Ton/T)*PR2
    OC1CONbits.OCM = 0b110; //fault pin disabled
    
    //Output compare 2
    OC2CONbits.OCTSEL = 0; //selectionne le timer 2
    OC2RS = 0*PR2; // nb d'instructions à l'état haut (Ton) = (Ton/T)*PR2
    OC2CONbits.OCM = 0b110; //fault pin disabled
}

float posReg(float reference)
{
    return KP_POS*(reference - (((int)POS1CNT + (int)POS2CNT)*M_PI*D/720));
}

float rotReg(float reference)
{
    return KP_ROT*(reference - ((int)POS1CNT - (int)POS2CNT));
}

void moveForward(float d, float speed){
    int phiRef = 360*d/(M_PI*D);
    float realSpeed = 0;
    float rapportTemp[2];
    float reg2;
    while (realSpeed < speed && (int)POS1CNT < phiRef/2)
    {
        rapportTemp[0] = (1.5+realSpeed*0.5)/10;
        rapportTemp[1] = (1.5-realSpeed*0.5)/10;
        realSpeed = (int)POS1CNT * ACCEL*M_PI*D/(360*MAX_SPEED*realSpeed);
        reg2 = (rapportTemp[1]-KP_STRAIGHT*((int)POS1CNT - (int)POS2CNT));
        if (reg2 > 0.15)
        {
            reg2 = 0.145;
        }
        else if (reg2 < 0.1)
        {
            reg2 = 0.1;
        }
        OC2RS = reg2*PR2;
        OC1RS = rapportTemp[0]*PR2;
    }
    rapportTemp[0] = (1.5+speed*0.5)/10;
    OC1RS = rapportTemp[0]*PR2;
    rapportTemp[1] = (1.5-speed*0.5)/10;
    while ((int)POS1CNT <= phiRef-(MAX_SPEED*MAX_SPEED*speed*speed/ACCEL))
    {
        reg2 = (rapportTemp[1]-KP_STRAIGHT*((int)POS1CNT - (int)POS2CNT));
        if (reg2 > 0.15)
        {
            reg2 = 0.145;
        }
        else if (reg2 < 0.1)
        {
            reg2 = 0.1;
        }
        OC2RS = reg2*PR2;
    }
    if (realSpeed > speed)
    {
        realSpeed = speed;
    }
    while ((int)POS1CNT <= phiRef)
    {
        rapportTemp[0] = (1.5+realSpeed*0.5)/10;
        rapportTemp[1] = (1.5-realSpeed*0.5)/10;
        realSpeed = (phiRef - (int)POS1CNT) * ACCEL*M_PI*D/(360*MAX_SPEED*realSpeed);
        reg2 = (rapportTemp[1]-KP_STRAIGHT*((int)POS1CNT - (int)POS2CNT));
        if (reg2 > 0.15)
        {
            reg2 = 0.145;
        }
        else if (reg2 < 0.1)
        {
            reg2 = 0.1;
        }
        OC2RS = reg2*PR2;
        OC1RS = rapportTemp[0]*PR2;
    }
    OC1RS = 0;
    OC2RS = 0;
    asm("nop");
    asm("nop");
    asm("nop");
}


int main(void)
{
    oscillatorInit();
    encodersInit();
    
    
    // Timer 2 at 10ms
    PR2 = 50000; // nb d'instructions pour une période de 10ms(T) 400000/8
    T2CONbits.TCKPS = 0b01; // Prescaler at 8
    T2CONbits.TON = 1; // Activate 16 bits timer
    //pas besoin d'interruption
    
    motorsInit();
    
    int phi1Mes = POS1CNT; //position angulaire en degrés°, fournie par l'encodeur
    int phi2Mes = POS2CNT; // 360 impulsions = 360° en mode x4
    
    int phiRef = 360; // position angulaire de référence
    int err; // erreur de position
    int dc; //rapport cyclique = fraction de la tension d'alimentation
    int Kp = 1/360;
    float rapportTemp; //rapport de temps Ton/T du pulse de commande moteur
    moveForward(1, 1);
	
	while(1) {
        //asm("nop");
       
        
        
        
	}
}


