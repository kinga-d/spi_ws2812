#include "main.h"
#include "stm32f303xe.h"
#include "RCCConfig.h"
#include "Delay.h"
#include "USART.h"
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <locale.h>


	/* ************

	 WS2812 configuration data - 1 bit duration

	 fCK = 12.8 MHz
	 SPI prescaler = 2
	 fSPI = 6.4 MHz
	 1 bit = 1/6.4 MHz = 0,156 µs
	 1 byte = 0,156 us * 8 = 1,25 µs

		1 bit – 0,156 µs
		2 bit – 0,312 µs
		3 bit – 0,468 µs
		4 bit – 0,624 µs
		5 bit – 0,780 µs
		6 bit – 0,936 µs
		7 bit – 1,092 µs
		8 bit – 1,248 µs

	according to datasheet of WS2812

	TOH between 0.2 µs - 0.5 µs
	T1H between 0.55 µs - 0.85 µs
	TOL between 0.65 µs - 0.95 µs
	T1L between 0.45 µs - 0.75 µs
	RES above 50 µs

	so the abovementioned values correspond to

	T0H -> 3 bits
	T1H -> 4 bits
	T0L -> 5 bits
	T1L -> 4 bits

	zero = TOH + T0L
	one = T1H + T1L

	 ************ */

uint8_t logicZero = 0;				// 0b00000000	zeros to reset leds (byte)
uint8_t ledLow = 0b11100000;  	// 0b11100000   logic "0" for WS2812b diode (byte)
uint8_t ledHigh = 0b11110000; 	// 0b11110000   logic "1" for WS2812b diode (byte)

#define resetNumber 50  				// Number of bytes to reset the LEDs >50us

uint8_t resetBuffer[50];

char blueBin[8];
uint8_t blue[8];
char greenBin[8];
uint8_t green[8];
char redBin[8];
uint8_t red[8];

void spiConfig(void){
	//1. Enable the SPI CLOCK and GPIO CLOCK (SCK, MISO, MOSI)

	RCC->APB2ENR |= (1<<12); //spi clock enable APB2 = 12.8 MHz
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN; //gpioa clock enable AHB = 12.8 MHz

	//2. Configure the SPI PINs for Alternate Functions

	GPIOA->MODER |= (2<<10) | (2<<12) | (2<<14); //alternate function for pin PA5 and PA6 and PA7

		//alternate function SPI1 is on AF5 which means 0101 = 5
	GPIOA->AFR[0] |= (5<<20); //bytes (23,22,21,20)=(0,1,0,1) AF5 for SPI1_SCK pin PA5
	GPIOA->AFR[0] |= (5<<24); //bytes (27,26,25,24)=(0,1,0,1) AF5 for SPI1_MISO pin PA6
	GPIOA->AFR[0] |= (5<<28); //bytes (31,30,29,28)=(0,1,0,1) AF5 for SPI1_MOSI pin PA7

	GPIOA->OSPEEDR |= (3<<10) | (3<<12) | (3<<14); //max speed

	//3. SPI configuration

		//SPI DIRECTION 2LINES
		//CPOL = 0 CPHA = 0
		//lSB first

	SPI1->CR1 |= SPI_CR1_SSM; //software slave management
	SPI1->CR1 |= SPI_CR1_SSI;
	SPI1->CR1 |= SPI_CR1_MSTR; //master configuration
	SPI1->CR1 |= SPI_CR1_LSBFIRST;
	SPI1->CR2 |= SPI_CR2_NSSP;

	//4. setup Baud Rate Control to fpCLK/2
	//SPI1->CR1 |= (2<<3); bytes BR[2:0] (5,4,3)=(0,0,0)

	//5. enable SPI1
	SPI1->CR1 |= SPI_CR1_SPE;

}

void spiSend(uint8_t data){
	while( !(SPI1->SR & SPI_SR_TXE));//wait till TC set
	SPI1->DR = data; //load the data into DR register
}

void gpioConfig (void){
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

	GPIOA->MODER |= (1<<10);

	GPIOA->OTYPER |= 0;
	GPIOA->OSPEEDR |= 0;
}

void setReset(void){
    for (int i=0; i<resetNumber; i++){
        resetBuffer[i]=logicZero;
    }
    for (int i=0; i<resetNumber; i++){
        spiSend(resetBuffer[i]);
    }
}

void setRed(char redDec){

	//set Red
	void leadingZerosRed(int num,int index)
	{
	    if (index >= 0){
	        leadingZerosRed(num/2, index-1);
	    redBin[index]=num%2;}
	}

	void decToBinRed(int num)
	{
	    leadingZerosRed(num, 7);
	}

	decToBinRed(redDec);

	for (int i=0, j = 0; i<8; i++){
    	if(redBin[i] == 1){
            red[j] = ledHigh;
    	}
        else red[j] = ledLow;
    	j++;
    }

}

void setGreen(char greenDec){

	//set Red
	void leadingZerosGreen(int num,int index)
	{
	    if (index >= 0){
	        leadingZerosGreen(num/2, index-1);
	    greenBin[index]=num%2;}
	}

	void decToBinGreen(int num)
	{
	    leadingZerosGreen(num, 7);
	}

	decToBinGreen(greenDec);

	for (int i=0, j = 0; i<8; i++){
    	if(greenBin[i] == 1){
            green[j] = ledHigh;
    	}
        else green[j] = ledLow;
    	j++;
    }

}
void setBlue(char blueDec){

	//set Red
	void leadingZerosBlue(int num,int index)
	{
	    if (index >= 0){
	        leadingZerosBlue(num/2, index-1);
	    blueBin[index]=num%2;}
	}

	void decToBinBlue(int num)
	{
	    leadingZerosBlue(num, 7);
	}

	decToBinBlue(blueDec);

	for (int i=0, j = 0; i<8; i++){
    	if(blueBin[i] == 1){
            blue[j] = ledHigh;
    	}
        else blue[j] = ledLow;
    	j++;
    }

}

void setColor(int RED, int GREEN, int BLUE){
	setRed(RED);
	setGreen(GREEN);
	setBlue(BLUE);
}

void sendColor(){
		for (int i=0;i<8;i++) {
			spiSend(green[i]);
		}

		for (int i=0;i<8;i++){
			spiSend(red[i]);
		}

		for (int i=0;i<8;i++){
			spiSend(blue[i]);
		}}

int main(void){

	sysClockConfig();

	TIM6Config();

	gpioConfig ();

	spiConfig();

	//for (int i=0; i<20; i++){
//}
	while(1)
	{
	setColor(0, 255, 0); //set RGB

	sendColor();

	sendColor();



	sendColor();

	for (int i=0;i<resetNumber;i++){
		spiSend(resetBuffer[i]);}
	//Delay_ms(200);

	//sendColor();
	//sendColor();

	//for (int i=0;i<resetNumber;i++){
		//spiSend(resetBuffer[i]);}
	//Delay_ms(200);
}}

