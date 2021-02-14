/*
 * Delay.h
 *
 *  Created on: Feb 3, 2021
 *      Author: Kinia
 */

#ifndef SRC_DELAY_H_
#define SRC_DELAY_H_


void TIM6Config (void){

	/******************* STEPS TO FOLLOW ********

	 1. Enable Timer Clock
	 2. Set the prescaler and the ARR
	 3. Enable the Timer, and wait for the update Flag to set

	 ********************************************/
	RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;

	TIM6->PSC |= 47; // (12.8MHz/12+1) = 1Mhz ~ 1us
	TIM6->ARR |= 0xffff; //MAX ARR value

	TIM6->CR1 |= (1<<0); //enable the counter
	while (!(TIM6->SR & (1<<0))); //update interrupt flag

}

void Delay_us (uint16_t us)
{

	/******************* STEPS TO FOLLOW ********

	 1. RESET the counter
	 2. Wait for the counter to reach the entered value. As each count
	 will take 1 us, the total waiting time will be the required us delay.

	 ********************************************/
	TIM6->CNT = 0;
	while (TIM6->CNT < us);

}

void Delay_ms (uint16_t ms)
{
	for (uint16_t i=0; i<ms; i++){
		Delay_us (1000);
	}

}





#endif /* SRC_DELAY_H_ */
