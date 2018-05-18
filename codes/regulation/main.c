#include "init.h"
#include "adc.h"
#include <xc.h>
#include "math.h"
#define D 0.1016f
#define E 0.22
#ifndef M_PI
    #define M_PI 3.141592653f
#endif
#define ACCEL 0.3
#define MAX_SPEED 0.4
#define KP_POS 8.0/20
#define KP_ROT 2.18/20
#define TOL 0.002
#define ROT_TOL 2
#define DEAD_ZONE 0.005

#define THROTTLE 0.5
#define ANG_THROTTLE 0.5

typedef enum cmd {
    STRAIGHT = 0x00,
    RIGHT = 0x01,
    LEFT = 0x02
} cmd;

float floatAbs(float value)
{
    if (value < 0)
    {
        return -value;
    }
    return value;
}

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
    OC1R = 0*PR2; // nb d'instructions � l'�tat haut (Ton) = (Ton/T)*PR2
    OC1RS = 0;
    OC1CONbits.OCM = 0b110; //fault pin disabled

    //Output compare 2
    OC2CONbits.OCTSEL = 0; //selectionne le timer 2
    OC2R = 0*PR2; // nb d'instructions � l'etat haut (Ton) = (Ton/T)*PR2
    OC2RS = 0;
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
float posReg(float reference, int offset1, int offset2) {
    return KP_POS*(reference - (((int)POS1CNT + (int)POS2CNT - offset1 - offset2)*M_PI*D/720));
}
//regulateur de rotation
float rotReg(float reference, int offset1, int offset2) {
    return KP_ROT*(reference - (((int)POS1CNT - (int)POS2CNT - offset1 + offset2)*M_PI*D/360));
}

float genReference(float pos, float speed, float dt) {
    int sign = 1;
    if (pos < 0)
    {
        sign = -1;
    }
  if (sign*speed*speed/ACCEL < pos)
  {
    // Trapeze
    if (dt < speed/ACCEL)
    {
      // Acceleration part
      return sign*ACCEL*dt*dt/2;
    }
    else if (dt < sign*pos/speed)
    {
      // Flat speed part
      return sign*speed*dt - sign*speed*speed/(2*ACCEL);
    }
    else if(dt < sign*pos/speed + speed/ACCEL)//ajout de condition sur t3
    {
      // Deceleration part
      float newDt = dt - sign*pos/speed;
      return pos - sign*speed*speed/(2*ACCEL) + sign*speed*newDt - sign*ACCEL*newDt*newDt/2;
    }
    else{
        return pos;// at zero speed, the position is constant.
    }
  }
  else
  {
    // Triangle
    if (dt*dt < sign*pos/ACCEL)
    {
      // Acceleration part
      return sign*ACCEL*dt*dt/2;
    }
    else if (2*dt*dt < sign*pos/ACCEL)
    {
      // Deceleration part
      float newDt = dt - sqrt(sign*pos/ACCEL);
      return pos/2 + sign*ACCEL*sqrt(sign*pos/ACCEL)*newDt - sign*ACCEL*newDt*newDt/2;
    }
    else{
        return pos;// at zero speed, the position is constant.
    }
  }
}

//A l'aide du PWM, on envoie des commandes au moteur en choissisant T=10ms. Les moteurs sont alimentee soit en tension +ve ou -ve
void sendMotorCmd(float cmd1, float cmd2) {
    if (cmd1 < 0.15)
    {
        cmd1 -= DEAD_ZONE;
    }
    else if (cmd1 > 0.15)
    {
        cmd1 += DEAD_ZONE;
    }
    
    if (cmd1 < 0.1)
    {
        cmd1 = 0.1;
    }
    else if (cmd1 > 0.2)
    {
        cmd1 = 0.2;
    }
    
    if (cmd2 < 0.15)
    {
        cmd2 -= DEAD_ZONE;
    }
    else if (cmd2 > 0.15)
    {
        cmd2 += DEAD_ZONE;
    }
    
    if (cmd2 < 0.1)
    {
        cmd2 = 0.1;
    }
    else if (cmd2 > 0.2)
    {
        cmd2 = 0.2;
    }
//envoie de la tension aux moteurs si conditions ci dessus sont remplies
  OC1RS = cmd1*PR2;
  OC2RS = cmd2*PR2;
}
//fonction qui permet de r�gler la translation
void translate(float d, float speed) {
    float cmdPos, cmdRot;
    int offsetP1 = (int)POS1CNT, offsetP2 = (int)POS2CNT;
    float cnt = 0; //compteur temps
    T1CONbits.TON = 1; //activation timer
    do {
        if (IFS0bits.T1IF)
        {
            IFS0bits.T1IF = 0;
            cnt += 0.01; //+10ms
            cmdPos = posReg(genReference(d, speed*MAX_SPEED,cnt), offsetP1, offsetP2);
            cmdRot = rotReg(0, offsetP1, offsetP2);
            sendMotorCmd(0.15 + cmdPos + cmdRot, 0.15 - (cmdPos - cmdRot));
         }
    }while(floatAbs((((int)POS1CNT + (int)POS2CNT - offsetP1 - offsetP2)*M_PI*D/720.0) - d) > TOL);
    T1CONbits.TON = 0;
    OC1RS = 0;
    OC2RS = 0;
}
//fonction qui permet de r�gler la rotation du robot
void rotate(float theta, float angSpeed) {
    float cmdPos, cmdRot;
    int offsetP1 = (int)POS1CNT, offsetP2 = (int)POS2CNT;
    float cnt = 0; //compteur temps
    T1CONbits.TON = 1; //activation timer
    do {
        if (IFS0bits.T1IF)
        {
            IFS0bits.T1IF = 0;
            cnt += 0.01; //+10ms
            cmdPos = posReg(0, offsetP1, offsetP2);
            cmdRot = rotReg(genReference(theta*E/2, angSpeed*MAX_SPEED,cnt), offsetP1, offsetP2);
            sendMotorCmd(0.15 + cmdPos + cmdRot, 0.15 - (cmdPos - cmdRot));
        }
    }while(floatAbs(((int)POS1CNT - (int)POS2CNT - offsetP1 + offsetP2)*(D/(2*E)) - theta) > ROT_TOL);
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
    
    // Init UART1
    UART1Init();
    RPINR18bits.U1RXR = 6; // Configure RP6 as UART1 Rx
    RPOR3bits.RP7R = 3;    // Configure RP7 as UART1 Tx

    // TODO remove that
    //translate(0.1, 0.5);
    //rotate(2*M_PI, 0.5);

    cmd rxCmd;
    char rxParam;

    while(1)
    {
        if (U1STAbits.URXDA)
        {
            rxCmd = (cmd)(U1RXREG);
            while(!U1STAbits.URXDA);
            rxParam = U1RXREG;
            switch(rxCmd) {
                case STRAIGHT:
                    translate((float)(rxParam)/100.0, THROTTLE);  // To get cm
                    break;
                case RIGHT:
                    rotate((float)(rxParam), ANG_THROTTLE);
                    break;
                case LEFT:
                    rotate(-(float)(rxParam), ANG_THROTTLE);
                    break;
                default:
                    break;
            }
        }
	  }
}
