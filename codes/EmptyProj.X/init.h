/* 
 * File:   init.h
 * Author: Kevin De Cuyper
 *
 * Created on 29 janvier 2013, 14:34
 */


#ifndef INIT_H
#define	INIT_H


#define FCY 40000000

/**
 * Configure le dsPIC pour utiliser l'oscillateur externe avec un quartz à 8MHz
 * et la PLL pour obtenir FCY = 40MHz
 */
void oscillatorInit(void);


#endif	/* INIT_H */
