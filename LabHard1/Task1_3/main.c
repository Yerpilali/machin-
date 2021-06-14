#include "main.h"

int main(void) 
{
	uint32_t vivi;
	int32_t i=0;
	RCC->AHBENR|=RCC_AHBENR_GPIOBEN;
	GPIOB->OSPEEDR|=0X5555; //все порты на скорость в 10 мгц - 0b0101010101010101
	GPIOB->MODER|=GPIO_MODER_MODER0_0 | GPIO_MODER_MODER1_0 | GPIO_MODER_MODER2_0 |
								GPIO_MODER_MODER3_0 | GPIO_MODER_MODER4_0 | GPIO_MODER_MODER5_0 |
								GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0 | GPIO_MODER_MODER9_0;
	GPIOB->MODER&=~( GPIO_MODER_MODER12 | GPIO_MODER_MODER13 | GPIO_MODER_MODER11 | GPIO_MODER_MODER10);
	GPIOB->ODR|=0x200;
	unsigned reg[13] = {0x0000023F, 0x00000206, 0x0000025B, 0x0000024F, 0x00000266, 0x0000026D, 0x0000027D, 0x00000207, 0x0000027F, 0x0000026F, 0x00000277, 0x0000027C, 0x00000239};
	unsigned namber[4] = {0, 0, 0, 0};
	while(1)
	{
		namber[0] = 0;
		namber[1] = 0;
		namber[2] = 0;
		namber[3] = 0;
		
		i=0;
		vivi=(((GPIOB->IDR)&0x8000)>>15)*128+(((GPIOB->IDR)&0x4000)>>14)*64+(((GPIOB->IDR)&0x2000)>>13)*16+(((GPIOB->IDR)&0x1000)>>12)*2+1+8;
		while(vivi!=0)
		{
			namber[i]=vivi%13;
			vivi=vivi/13;
			i=i+1;
		}
		for (int32_t s=3;s>=0;s--)
		{
				if(s<i)
				{
					GPIOB->ODR=reg[namber[s]];
					if(s==0)
					{
						GPIOB->ODR|=0x280;
					}
					delay(500000);
					GPIOB->ODR=0xffff0000;
					delay(50000);
				}
		}
	}
}
void delay(uint32_t count)
{
	for (uint32_t i=0;i<count;i++);
}
