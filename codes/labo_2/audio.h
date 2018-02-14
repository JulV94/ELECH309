/* 
 * File:   audio.h
 * Author: M. Osée
 *
 * Created on 11 janvier 2016, 14:10
 */

#ifndef AUDIO_H
#define	AUDIO_H


/**
 * En-tête de la librairie du DAC pour les labos d'ELEC-H310 (et ELEC-H309)
 * 
 * Ces fonctions utilisent le SPI1 du dsPIC de l'Explorer16 pour communiquer 
 * avec le DAC de la carte d'extension
 * RB2 est aussi utilisée (comme Chip Select du DAC)
 */


/**
 * Initialise le SPI1 pour communiquer avec le DAC
 */
void audioInit(void);

/**
 * Envoie une nouvelle valeur au DAC. 
 * 
 * @param sig : valeur à envoyer. C'est un DAC 12 bits => il attend des valeurs
 * comprises entre 0 et 4095.
 * Si sig n'est pas dans cette intervalle, il est saturé.
 */
void audioWrite(int sig);

#endif	/* AUDIO_H */
