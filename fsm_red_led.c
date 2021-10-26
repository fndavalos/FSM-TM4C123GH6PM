/*
Written by: Francisco Davalos
Date: 10/25/2021
Hardware: Tiva LaunchPad TM4C123GH6PM
Description: A finite state machine that turns on and off the
red LED (PF1) based on the state of SW2 (PF4) and SW1(PF2)
*/

#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "driverlib/sysctl.h"

#define SYSCTL_RCC2_80MHZ 0x01000000
#define STATE_A &FSM[0]
#define STATE_B &FSM[1]
#define GPIO_PF0 0x01
#define GPIO_PF1 0x02
#define GPIO_PF4 0x10

void TurnOnRedLed(void);
void TurnOffRedLed(void);
void PPL_Init(void);
void PortConfig_Init(void);
void Nothing(void);

struct State{
	void *Out[4];
	const struct State *Next[4];
};
typedef const struct State StateType;

StateType FSM[2]={
		{{(void*)&Nothing, (void*)&TurnOffRedLed, (void*)&TurnOffRedLed,(void*)&Nothing}, //State A
	    {STATE_B, STATE_A, STATE_B, STATE_A}},
		{{(void*)&TurnOnRedLed, (void*)&Nothing, (void*)&Nothing,(void*)&TurnOnRedLed},   //State B
	   {STATE_A, STATE_B, STATE_B, STATE_B}}
};


int main()
{
	
	PPL_Init();					// Initialize clock to run at 80Mhz
	PortConfig_Init();	// Initialize Ports
	
	StateType *pt;
	uint32_t input;	
	pt = STATE_A;				// Set Initial State

	while(1)
	{
		input = (GPIO_PORTF_DATA_R&0x01) | ((GPIO_PORTF_DATA_R&0x10) >>3);//Cobines FP0 and PF4 to form input with range 0-3
		
		((void(*)(void))pt->Out[input])(); // void(*)(void) --> is  a pointer to a function taking a pointer to void and returning nothing
		pt = pt->Next[input];
	}

}
void PPL_Init(void)
{
	SYSCTL_RCC2_R	|= SYSCTL_RCC2_USERCC2; 														// Use RCC2
	SYSCTL_RCC2_R |= SYSCTL_RCC2_BYPASS2; 														// PPL Bypass 2
	SYSCTL_RCC_R	= (SYSCTL_RCC_R &~0x000007C0)+SYSCTL_RCC_XTAL_16MHZ;// Use 16Mhz crystal
	SYSCTL_RCC2_R &= ~SYSCTL_RCC2_OSCSRC2_M; 													// Selects Main Oscillator
	SYSCTL_RCC2_R &= ~SYSCTL_RCC2_PWRDN2;															// Power up PPL
	SYSCTL_RCC2_R |= SYSCTL_RCC2_DIV400;															// Divide PPL as 400 MHz
	SYSCTL_RCC2_R =	(SYSCTL_RCC2_R&~0X1FC00000)+(SYSCTL_RCC2_80MHZ);	// Set clock to 80MHz
	while((SYSCTL_RIS_R&0x00000040)==0){};														// Wait for the PPL to lock
	SYSCTL_RCC2_R &= ~SYSCTL_RCC2_BYPASS2; 														// Clear PPL Bypass 2
}

void PortConfig_Init(void)
{
	volatile uint32_t ui32Loop;
	
	SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOF; 															// Initialize GPIOF CLock
	ui32Loop = SYSCTL_RCGC2_R;					 															// Dummy Read
	GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;	 															// Unlock PF0
	GPIO_PORTF_CR_R |= GPIO_PF0;         															// allow changes to PF0
	GPIO_PORTF_DIR_R &= ~(GPIO_PF0|GPIO_PF4); 												// Set PF0 and PF4 as Inputs
	GPIO_PORTF_DIR_R |= GPIO_PF1;																			// Set PF1 as an output
	GPIO_PORTF_DEN_R |= (GPIO_PF0|GPIO_PF1|GPIO_PF4); 								//Enable PF0, PF1, and PF4 as Digital
	GPIO_PORTF_PUR_R |= GPIO_PF0|GPIO_PF4; 														//Enable pull-up resistors on PF0 and PF4
	
}

void TurnOnRedLed(void)
{
	GPIO_PORTF_DATA_R = GPIO_PF1;
}
void TurnOffRedLed(void)
{
	GPIO_PORTF_DATA_R &= ~GPIO_PF1;
}

void Nothing(void)
{
	//Does nothing
}