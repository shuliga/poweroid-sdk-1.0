//
// Created by SHL on 30.09.2019.
//

#ifndef BOARDS_H
#define BOARDS_H

#define STRINGIZE_NX(A) #A
#define STRINGIZE(A) STRINGIZE_NX(A)

#define PWR23 23
#define PWR20 20

#ifndef PWR_BOARD_VERSION
#define PWR_BOARD_VERSION PWR23
#endif

//#define SPI

#if PWR_BOARD_VERSION == PWR23


//Encoder pins
#define ENC2_PIN 2
#define ENC1_PIN 3
#define ENC_BTN_PIN 4

//Discrete signal IN pins
#define IN1_PIN 5
#define IN2_PIN 6
#ifndef MINI
#define IN3_PIN 7
#endif  //MINI

//Relay pins
#define PWR1_PIN 8
#ifndef MINI
#define PWR2_PIN 9
#endif  //MINI

#undef FACTORY_RESET_PIN

//Indicator pins
#ifndef SPI
#define IND1_PIN 10
#define IND2_PIN 11
#ifndef MINI
#define IND3_PIN 12
#endif  //MINI

#define LED_PIN 13
#endif

//Analogue signal IN pins
#define INA1_PIN 14
#define INA2_PIN 15
#ifndef MINI
#define INA3_PIN 16
#define INA4_PIN 17
#endif  //MINI

#ifdef SPI
#define SPI_SS_PIN 10
#endif

#endif // PWR23

#if PWR_BOARD_VERSION == PWR20

//Discrete signal IN pins
#define IN1_PIN 2
#define IN2_PIN 4
#define IN3_PIN 6

//Relay pins
#define PWR1_PIN 10
#define PWR2_PIN 11

#define LED_PIN 13

//Analogue signal IN pins
#define INA1_PIN 14
#define INA2_PIN 15
#define INA3_PIN 16

#endif // PWR20



#endif //BOARDS_H
