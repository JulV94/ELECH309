#include "init.h"
#include "adc.h"
#include <xc.h>
#include "math.h"
#define D 0.1016f
#ifndef M_PI
    #define M_PI 3.141592653f
#endif
#define ACCEL 0.1
#define MAX_SPEED 0.4
#define KP_POS 8.0/20
#define KP_ROT 0.03/20

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
    OC1RS = 0*PR2; // nb d'instructions ï¿½ l'ï¿½tat haut (Ton) = (Ton/T)*PR2
    OC1CONbits.OCM = 0b110; //fault pin disabled

    //Output compare 2
    OC2CONbits.OCTSEL = 0; //selectionne le timer 2
    OC2RS = 0*PR2; // nb d'instructions ï¿½ l'ï¿½tat haut (Ton) = (Ton/T)*PR2
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

float genReference(float pos, float speed, float dt)
{
  if (speed*speed/ACCEL < pos)
  {
    // TrapÃ¨ze
    if (dt < speed/ACCEL)
    {
      // Acceleration part
      return ACCEL*dt*dt/2;
    }
    else if (dt < pos/speed)
    {
      // Flat speed part
      return speed*dt - speed*speed/(2*ACCEL);
    }
    else
    {
      // Deceleration part
      float newDt = dt - pos/speed;
      return pos - speed*speed/(2*ACCEL) + speed*newDt - ACCEL*newDt*newDt/2;
    }
  }
  else
  {
    // Triangle
    if (dt*dt < pos/ACCEL)
    {
      // Acceleration part
      return ACCEL*dt*dt/2;
    }
    else
    {
      // Deceleration part
      float newDt = dt - sqrt(pos/ACCEL);
      return pos/2 + ACCEL*sqrt(pos/ACCEL)*newDt - ACCEL*newDt*newDt/2;
    }
  }
}





void moveForward(float d, float speed){
    float cmdPos, cmdRot, cmd1, cmd2;
    float cnt = 0; //compteur temps
    T1CONbits.TON =1; //activation timer
    do {
        if(IFS0bits.T1IF){
            IFS0bits.T1IF = 0;
            cnt+=0.01; //+10ms
            cmdPos = posReg(genReference(d, speed*MAX_SPEED,cnt));
            cmdRot= 0;//rotReg(0);
            cmd1 = 0.15 + cmdPos + cmdRot;
            cmd2 = 0.15 - (cmdPos - cmdRot);
            
            if(cmd2>0.2){
                cmd2=0.2;
            }
            else if(cmd2 <0.1){
                cmd2 = 0.1;
            }
            if(cmd1<0.1){
                cmd1=0.1;
            }
            else if(cmd1 >0.2){
                cmd1 = 0.2;
            }
            OC1RS = cmd1*PR2;
            OC2RS = cmd2*PR2;
        }
    }while((((int)POS1CNT + (int)POS2CNT)*M_PI*D/720) < d);
    T1CONbits.TON =0;
    OC1RS=0;
    OC2RS=0;
    
    
    /*int phiRef = 360*d/(M_PI*D);
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
     */
}


int main(void)
{
    oscillatorInit();
    encodersInit();
    
    //Timer 1 at 100Hz frequence de régulation
    PR1= 50000;
    T1CONbits.TCKPS = 0b01;


    // Timer 2 at 10ms PWM moteur
    PR2 = 50000; // nb d'instructions pour une pï¿½riode de 10ms(T) 400000/8
    T2CONbits.TCKPS = 0b01; // Prescaler at 8
    T2CONbits.TON = 1; // Activate 16 bits timer
    //pas besoin d'interruption

    motorsInit();

    int phi1Mes = POS1CNT; //position angulaire en degrï¿½sï¿½, fournie par l'encodeur
    int phi2Mes = POS2CNT; // 360 impulsions = 360ï¿½ en mode x4

    int phiRef = 360; // position angulaire de rï¿½fï¿½rence
    int err; // erreur de position
    int dc; //rapport cyclique = fraction de la tension d'alimentation
    int Kp = 1/360;
    float rapportTemp; //rapport de temps Ton/T du pulse de commande moteur
    moveForward(1, 0.5);

	while(1) {
        //asm("nop");




	}
}
