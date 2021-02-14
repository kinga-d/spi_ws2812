/*
 * USART.h
 *
 *  Created on: Feb 3, 2021
 *      Author: Kinia
 */

#ifndef SRC_USART_H_
#define SRC_USART_H_



void UART2Config (void)
{
	/************** STEPS TO FOLLOW AS IN THE REFERENCE MANUAL *************

	1. Enable the UART CLOCK and GPIO CLOCK (Rx and Tx)
	2. Configure the UART PINs for Alternate Functions
	3. Program the M bits in USART_CR1 to define the word length.
	4. Select the desired baud rate using the USART_BRR register.
	5. Program the number of stop bits in USART_CR2.
	6. Enable the USART by writing the UE bit in USART_CR1 register to 1.
	7. Set the TE bit in USART_CR1 to send an idle frame as first transmission.


	 ***********************************************************************/

	 //1. Enable the UART CLOCK and GPIO CLOCK (Rx and Tx)

	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

	 //2. Configure the UART PINs for Alternate Functions

	GPIOA->MODER |= (2<<4) | (2<<6); //alternate function for pin PA2 and PA3

	GPIOA->AFR[0] |= (7<<8); //bytes (11,10,9,8)=(0,1,1,1) AF7 for USART2 pin PA2
	GPIOA->AFR[0] |= (7<<12); //bytes (15,14,13,12)=(0,1,1,1) AF7 for USART2 pin PA3

	//3. Program the M bit in USART_CR1 to define the word length

	USART2->CR1 &= ~(1<<28); //8 bits
	USART2->CR1 &= ~(1<<12);

	//4. Select the desired baud rate using the USART_BRR register.

		//24 000 000 MHz / 115200 = 208(d) = D0(h)

	USART2->BRR |= 0xD0; //USARTDIV = (PCLK1 = 36MHz) / (Baud = 115200)

	//5. Program the number of stop bits in USART_CR2.

	USART2->CR2 |= (1<<13);

	//6. Enable the USART by writing the UE bit in USART_CR1 register to 1

	USART2->CR1 |= 0x00; //clear all
	USART2->CR1 |= (1<<0);

	//7. Set the TE bit in USART_CR1 to send an idle frame as first transmission.

	USART2->CR1 |= (1<<3); 
	USART2->CR1 |= (1<<2); // enable RE

}

void UART2_SendChar(uint8_t c) {


		/************** STEPS TO FOLLOW AS IN THE REFERENCE MANUAL *************

		8. Write the data to send in the USART_TDR register (this clears the TXE bit). Repeat this
			for each data to be transmitted in case of single buffer.
		9. After writing the last data into the USART_TDR register, wait until TC=1. This indicates
			that the transmission of the last frame is complete. This is required for instance when
			the USART is disabled or enters the Halt mode to avoid corrupting the last
			transmission.

		 ***********************************************************************/

	USART2->TDR = c; //load the data into TDR register
	while (!(USART2->ISR & (1<<6))); //wait till TC set

}
void UART2_SendString (char *string){
	while (*string) UART2_SendChar (*string++);
}

#endif /* SRC_USART_H_ */
