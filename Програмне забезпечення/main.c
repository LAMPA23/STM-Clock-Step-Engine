#include <stm32f407xx.h>

#define ticks_const 0x00F4C985
#define Period 		5000

volatile unsigned long delay_counter = 0;
volatile unsigned long scaller = 0;
volatile unsigned long clock_time = 00*60 + 0;
volatile unsigned long angle = 3000;
volatile unsigned long angle_buff = 0;
volatile unsigned long count_1 = 0;

void lcd_init(void);
void lcd_cmd(unsigned long cmd);
void lcd_data(unsigned long cmd);
void lcd_print_time(void);
void lcd_print_angle(void);
void lcd_print(void);

void interrupt_init(void);
void EXTI15_10_IRQHandler(void);
void EXTI9_5_IRQHandler(void);

void SysTick_Init (unsigned long ticks);
void SysTick_Handler(void);

void GPIO_init(void);
void my_delay(unsigned long delay);
void set_angle(void);


// #################################################
// #################################################
void SysTick_Init (unsigned long ticks)
{
	SysTick->CTRL = 0;
	SysTick->LOAD = ticks - 1;
	NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1);
	SysTick->VAL = 0;
	SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;
	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk; 
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;	
}

void SysTick_Handler(void)
{
	scaller++;
	if(scaller%60==0) 
	{
		clock_time++;
		if(clock_time >= 24*60) clock_time = 0;
		lcd_init();
		lcd_print();
	}
	
}



// #################################################
// #################################################
void interrupt_init(void)
{	
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	
	SYSCFG->EXTICR[2] |= SYSCFG_EXTICR3_EXTI11_PC;
	SYSCFG->EXTICR[2] |= SYSCFG_EXTICR3_EXTI8_PC;
	
	EXTI->IMR |= EXTI_IMR_IM11;
	EXTI->IMR |= EXTI_IMR_IM8;
	
	EXTI->FTSR |= EXTI_RTSR_TR11;
	EXTI->FTSR |= EXTI_RTSR_TR8;
	
	NVIC_SetPriority(EXTI15_10_IRQn,2);
	NVIC_SetPriority(EXTI9_5_IRQn,3);
	
	NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
	NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
	
	NVIC_EnableIRQ(EXTI15_10_IRQn);
	NVIC_EnableIRQ(EXTI9_5_IRQn);
	
	__enable_irq();
}

void EXTI15_10_IRQHandler(void)  //SWT1
{
	EXTI->IMR &= ~EXTI_IMR_IM11;
	
	clock_time++;
	if(clock_time >= 24*60) clock_time = 0;
	lcd_init();
	lcd_print();
	
	EXTI->PR|= EXTI_PR_PR11;
	EXTI->IMR |= EXTI_IMR_IM11;
}

void EXTI9_5_IRQHandler(void) //SWT5
{
	EXTI->IMR &= ~EXTI_IMR_IM8;
	
	clock_time--;
	if(clock_time >= 24*60) clock_time = 0;
	lcd_init();
	lcd_print();
	
	EXTI->PR|= EXTI_PR_PR8;
	EXTI->IMR |= EXTI_IMR_IM8;
}




// #################################################
// #################################################
void lcd_print(void)
{
	lcd_cmd(0x01);// clear
	lcd_print_time();
	lcd_print_angle();
}

void lcd_print_time(void)
{
	lcd_cmd(0x80);
	lcd_data('T');
	lcd_data('i');
	lcd_data('m');
	lcd_data('e');
	lcd_data(0x20);
	lcd_data(0x2D);
	lcd_data(0x20);
	lcd_data(0x30 + (((clock_time-clock_time%600)/600)%6));
	lcd_data(0x30 + (((clock_time-clock_time%60)/60)%10));
	lcd_data(0x3A);
	lcd_data(0x30 + ((clock_time%60-clock_time%10)/10));
	lcd_data(0x30 + (clock_time%10));
}

void lcd_print_angle(void)
{
	lcd_cmd(0xC0);
	lcd_data('A');
	lcd_data('n');
	lcd_data('g');
	lcd_data('l');
	lcd_data('e');
	lcd_data(0x20);
	lcd_data(0x2D);
	lcd_data(0x20);
	lcd_data(0x30 + (angle - (angle%1000))/1000);
	lcd_data(0x30 + ((angle - (angle%100))/100)%10);
	lcd_data(0x30 + ((angle - (angle%10))/10)%10);
	lcd_data(0x30 + (angle%10));
}

void lcd_init(void)
{
	lcd_cmd(0x28);// init
	lcd_cmd(0x01);// clear
	//lcd_cmd(0x0D);// cursor on
	lcd_cmd(0x0C);// cursor off
	my_delay(1000);
}

void lcd_cmd(unsigned long cmd)
{
	GPIOE->ODR &= ~GPIO_ODR_OD7;
	GPIOE->ODR &= ~GPIO_ODR_OD11;
	
	if(cmd&(1<<4)) GPIOE->ODR |= GPIO_ODR_OD12; 
		else GPIOE->ODR &= ~GPIO_ODR_OD12;
	if(cmd&(1<<5)) GPIOE->ODR |= GPIO_ODR_OD13; 
		else GPIOE->ODR &= ~GPIO_ODR_OD13;
	if(cmd&(1<<6)) GPIOE->ODR |= GPIO_ODR_OD14; 
		else GPIOE->ODR &= ~GPIO_ODR_OD14;
	if(cmd&(1<<7)) GPIOE->ODR |= GPIO_ODR_OD15; 
		else GPIOE->ODR &= ~GPIO_ODR_OD15;

	GPIOE->ODR |= GPIO_ODR_OD11;
	my_delay(40);
	GPIOE->ODR &= ~GPIO_ODR_OD11;
	my_delay(4000);
	
	if(cmd&(1<<0)) GPIOE->ODR |= GPIO_ODR_OD12; 
		else GPIOE->ODR &= ~GPIO_ODR_OD12;
	if(cmd&(1<<1)) GPIOE->ODR |= GPIO_ODR_OD13; 
		else GPIOE->ODR &= ~GPIO_ODR_OD13;
	if(cmd&(1<<2)) GPIOE->ODR |= GPIO_ODR_OD14; 
		else GPIOE->ODR &= ~GPIO_ODR_OD14;
	if(cmd&(1<<3)) GPIOE->ODR |= GPIO_ODR_OD15; 
		else GPIOE->ODR &= ~GPIO_ODR_OD15;
	
	GPIOE->ODR |= GPIO_ODR_OD11;
	my_delay(40);
	GPIOE->ODR &= ~GPIO_ODR_OD11;	
	my_delay(4000);
}

void lcd_data(unsigned long cmd)
{
	GPIOE->ODR |= GPIO_ODR_OD7;
	GPIOE->ODR &= ~GPIO_ODR_OD11;
	
	if(cmd&(1<<4)) GPIOE->ODR |= GPIO_ODR_OD12; 
		else GPIOE->ODR &= ~GPIO_ODR_OD12;
	if(cmd&(1<<5)) GPIOE->ODR |= GPIO_ODR_OD13; 
		else GPIOE->ODR &= ~GPIO_ODR_OD13;
	if(cmd&(1<<6)) GPIOE->ODR |= GPIO_ODR_OD14; 
		else GPIOE->ODR &= ~GPIO_ODR_OD14;
	if(cmd&(1<<7)) GPIOE->ODR |= GPIO_ODR_OD15; 
		else GPIOE->ODR &= ~GPIO_ODR_OD15;

	GPIOE->ODR |= GPIO_ODR_OD11;
	my_delay(40);
	GPIOE->ODR &= ~GPIO_ODR_OD11;
	my_delay(4000);
	
	if(cmd&(1<<0)) GPIOE->ODR |= GPIO_ODR_OD12; 
		else GPIOE->ODR &= ~GPIO_ODR_OD12;
	if(cmd&(1<<1)) GPIOE->ODR |= GPIO_ODR_OD13; 
		else GPIOE->ODR &= ~GPIO_ODR_OD13;
	if(cmd&(1<<2)) GPIOE->ODR |= GPIO_ODR_OD14; 
		else GPIOE->ODR &= ~GPIO_ODR_OD14;
	if(cmd&(1<<3)) GPIOE->ODR |= GPIO_ODR_OD15; 
		else GPIOE->ODR &= ~GPIO_ODR_OD15;
	
	GPIOE->ODR |= GPIO_ODR_OD11;
	my_delay(40);
	GPIOE->ODR &= ~GPIO_ODR_OD11;	
	my_delay(4000);
}




// #################################################
// #################################################
void set_angle(void)
{
	angle_buff = angle;
////	if( 07*60 + 30 <= clock_time && clock_time <= 12*60 + 00) angle = 1800;
////	else if( 12*60 + 01 <= clock_time && clock_time <= 16*60 + 00) angle = 1200;
////	else if( 16*60 + 01 <= clock_time && clock_time <= 22*60 + 00) angle = 2500;
////	else if( 22*60 + 01 <= clock_time || clock_time <= 07*60 + 29) angle = 600;
	
	if( 0 <= clock_time && clock_time <= 2 ) angle = 4200;
	else if( 3 <= clock_time && clock_time <= 5) angle = 1200;
	else if( 6 <= clock_time && clock_time <= 8) angle = 2500;
	else if( 9 <= clock_time && clock_time <= 12) angle = 600;
	
	
	if(angle != angle_buff)
	{
		lcd_init();
		lcd_print();
		count_1 = 0;
		while(count_1 != 100)
		{
			GPIOA->ODR |= GPIO_ODR_OD4;
			my_delay(angle);
			GPIOA->ODR &= ~GPIO_ODR_OD4;
			my_delay(Period - angle);
			count_1++;
		}
	}
	
}

void GPIO_init(void)
{
	// AHB1ENR
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
	
	// MODER A
	GPIOA->MODER |= ~GPIO_MODER_MODE2_0;
	GPIOA->MODER &= ~GPIO_MODER_MODE2_1;
	GPIOA->MODER |= ~GPIO_MODER_MODE4_0;
	GPIOA->MODER &= ~GPIO_MODER_MODE4_1;		
	
	
	// MODER C
	GPIOC->MODER &= ~GPIO_MODER_MODE8_0;
	GPIOC->MODER &= ~GPIO_MODER_MODE8_1;
	GPIOC->MODER &= ~GPIO_MODER_MODE11_0;
	GPIOC->MODER &= ~GPIO_MODER_MODE11_1;
		
	
	// MODER E
	GPIOE->MODER |= ~GPIO_MODER_MODE5_0;
	GPIOE->MODER &= ~GPIO_MODER_MODE5_1;
	GPIOE->MODER |= GPIO_MODER_MODE7_0;
	GPIOE->MODER &= ~GPIO_MODER_MODE7_1;
	GPIOE->MODER |= GPIO_MODER_MODE10_0;
	GPIOE->MODER &= ~GPIO_MODER_MODE10_1;
	GPIOE->MODER |= GPIO_MODER_MODE11_0;
	GPIOE->MODER &= ~GPIO_MODER_MODE11_1;
	GPIOE->MODER |= GPIO_MODER_MODE12_0;
	GPIOE->MODER &= ~GPIO_MODER_MODE12_1;
	GPIOE->MODER |= GPIO_MODER_MODE13_0;
	GPIOE->MODER &= ~GPIO_MODER_MODE13_1;
	GPIOE->MODER |= GPIO_MODER_MODE14_0;
	GPIOE->MODER &= ~GPIO_MODER_MODE14_1;
	GPIOE->MODER |= GPIO_MODER_MODE15_0;
	GPIOE->MODER &= ~GPIO_MODER_MODE15_1;
	
	
	// OTYPER E
	GPIOE->OTYPER &= ~GPIO_OTYPER_OT7;
	GPIOE->OTYPER &= ~GPIO_OTYPER_OT10;
	GPIOE->OTYPER &= ~GPIO_OTYPER_OT11;
	GPIOE->OTYPER &= ~GPIO_OTYPER_OT12;
	GPIOE->OTYPER &= ~GPIO_OTYPER_OT13;
	GPIOE->OTYPER &= ~GPIO_OTYPER_OT14;
	GPIOE->OTYPER &= ~GPIO_OTYPER_OT15;
	
	
	// ODR
	GPIOE->ODR &= ~GPIO_ODR_OD10;
}

void my_delay(unsigned long delay)
{	
	for(delay_counter=0;delay_counter<delay;delay_counter++);
}



// #################################################
// #################################################
int main()
{
	GPIO_init();
	interrupt_init();
	SysTick_Init(ticks_const);
	lcd_init();
	lcd_print();
	while(1)
	{
		set_angle();
	}
}
