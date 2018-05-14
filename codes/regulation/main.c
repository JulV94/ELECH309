#include "init.h"
#include "adc.h"
#include <xc.h>
#include "math.h"
#define D 0.1016f
#define E 0.22 // TODO to be defined
#ifndef M_PI
    #define M_PI 3.141592653f
#endif
#define ACCEL 0.3
#define ANG_ACCEL ACCEL
#define MAX_SPEED 0.4
#define MAX_ANG_SPEED MAX_SPEED*2/E
#define KP_POS 8.0/20
#define KP_ROT 10.9/(20*5)
#define TOL -0.001

void encodersInit() {
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
    OC1R = 0*PR2; // nb d'instructions ï¿½ l'ï¿½tat haut (Ton) = (Ton/T)*PR2
    OC1CONbits.OCM = 0b110; //fault pin disabled

    //Output compare 2
    OC2CONbits.OCTSEL = 0; //selectionne le timer 2
    OC2R = 0*PR2; // nb d'instructions ï¿½ l'etat haut (Ton) = (Ton/T)*PR2
    OC2CONbits.OCM = 0b110; //fault pin disabled
}

void timersInit() {
  //Timer 1 at 100Hz frequence de regulation
  PR1 = 50000;
  T1CONbits.TCKPS = 0b01;

  // Timer 2 at 10ms PWM moteur
  PR2 = 50000; // nb d'instructions pour une periode de 10ms(T) 400000/8
  T2CONbits.TCKPS = 0b01; // Prescaler at 8
  T2CONbits.TON = 1; // Activate 16 bits timer
  //pas besoin d'interruption
}

void UART1Init()
{
    U1BRG = 21;  // baudrate of 115200
    U1MODEbits.PDSEL = 0; // 8 bits no parity
    U1MODEbits.STSEL = 0; // 1 stop bit
    U1MODEbits.URXINV = 0; // Idle state is high state
    U1MODEbits.BRGH = 0; // Standard mode (not high baudrate)
    U1MODEbits.UARTEN = 1; // Activate UART
    U1STAbits.UTXEN = 1;  // activate transmission
}
//regulateur de translation
float posReg(float reference) {
    return KP_POS*(reference - (((int)POS1CNT + (int)POS2CNT)*M_PI*D/720));
}
//regulateur de rotation
float rotReg(float reference) {
    //float tmp;
    
    //tmp = ((int)POS1CNT - (int)POS2CNT);
    //if ((tmp < -50) || (tmp > 50))
        //tmp += 1;
    return KP_ROT*(reference - (((int)POS1CNT - (int)POS2CNT)*(D*M_PI)/(180)));
}

float genReference(float pos, float speed, float dt) {
  if (speed*speed/ACCEL < pos)
  {
    // Trapeze
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
    else if(dt < pos/speed + speed/ACCEL)//ajout de condition sur t3
    {
      // Deceleration part
      float newDt = dt - pos/speed;
      return pos - speed*speed/(2*ACCEL) + speed*newDt - ACCEL*newDt*newDt/2;
    }
    else{
        return pos;// at zero speed, the position is constant.
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

float genRotReference(float pos, float speed, float dt) {
  if (speed*speed/ANG_ACCEL < pos)
  {
    // Trapeze
    if (dt < speed/ANG_ACCEL)
    {
      // Acceleration part
      return ANG_ACCEL*dt*dt/2;
    }
    else if (dt < pos/speed)
    {
      // Flat speed part
      return speed*dt - speed*speed/(2*ANG_ACCEL);
    }
    else if(dt < pos/speed + speed/ANG_ACCEL)//ajout de condition sur t3
    {
      // Deceleration part
      float newDt = dt - pos/speed;
      return pos - speed*speed/(2*ANG_ACCEL) + speed*newDt - ANG_ACCEL*newDt*newDt/2;
    }
    else{
        return pos;// at zero speed, the position is constant.
    }
  }
  else
  {
    // Triangle
    if (dt*dt < pos/ANG_ACCEL)
    {
      // Acceleration part
      return ANG_ACCEL*dt*dt/2;
    }
    else
    {
      // Deceleration part
      float newDt = dt - sqrt(pos/ANG_ACCEL);
      return pos/2 + ANG_ACCEL*sqrt(pos/ANG_ACCEL)*newDt - ANG_ACCEL*newDt*newDt/2;
    }
  }
}



//A l'aide du PWM, on envoie des commandes au moteur en choissisant T=10ms. Les moteurs sont alimentee soit en tension +ve ou -ve
void sendMotorCmd(float cmd1, float cmd2) {
  if (cmd1 < 0.1)
  {
      cmd1 = 0.1;
  }
  else if (cmd1 > 0.2)
  {
      cmd1 = 0.2;
  }

  if (cmd2 > 0.2)
  {
      cmd2 = 0.2;
  }
  else if (cmd2 < 0.1)
  {
      cmd2 = 0.1;
  }
//envoie de la tension aux moteurs si conditions ci dessus sont remplies
  OC1RS = cmd1*PR2;
  OC2RS = cmd2*PR2;
}
//fonction qui permet de régler la translation
void translate(float d, float speed) {
    float cmdPos, cmdRot;
    float cnt = 0; //compteur temps
    T1CONbits.TON = 1; //activation timer
    do {
        if (IFS0bits.T1IF)
        {
            IFS0bits.T1IF = 0;
            cnt += 0.01; //+10ms
            cmdPos = posReg(genReference(d, speed*MAX_SPEED,cnt));
            cmdRot = rotReg(0);
            sendMotorCmd(0.15 + cmdPos + cmdRot, 0.15 - (cmdPos - cmdRot));
         }
    }while((((int)POS1CNT + (int)POS2CNT)*M_PI*D/720) - d < TOL);
    T1CONbits.TON = 0;
    OC1RS = 0;
    OC2RS = 0;
}
//fonction qui permet de régler la rotation du robot
void rotate(float theta, float angSpeed) {
    float cmdPos, cmdRot;
    float cnt = 0; //compteur temps
    T1CONbits.TON = 1; //activation timer
    do {
        if (IFS0bits.T1IF)
        {
            IFS0bits.T1IF = 0;
            cnt += 0.01; //+10ms
            cmdPos = posReg(0);
            cmdRot = rotReg(genReference(theta*E/2, angSpeed*MAX_ANG_SPEED*E/2,cnt));
            sendMotorCmd(0.15 + cmdPos + cmdRot, 0.15 - (cmdPos - cmdRot));
        }
    }while(((int)POS1CNT - (int)POS2CNT)*(D*M_PI/360*E) - theta < -0.08);
    T1CONbits.TON = 0;
    OC1RS = 0;
    OC2RS = 0;
}

int main(void) {
    oscillatorInit();
    AD1PCFGL = 0xFFFF; // Necessary for the UART to work ??
    encodersInit();
    timersInit();
    motorsInit();
    UART1Init();

    // TODO remove that
    //translate(1, 0.5);
    rotate(2*M_PI, 0.5);

    char rxReg;

    while(1)
    {
        if (U1STAbits.URXDA)
        {
            rxReg = U1RXREG;
            // TODO process data received
            // TODO perform asked task
        }
	  }
}
