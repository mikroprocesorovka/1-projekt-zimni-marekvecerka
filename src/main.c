
#include "stm8s.h"
#include "milis.h"
#include "stm8_hd44780.h"
#include "stdio.h"


/*#include "delay.h"*/
/*#include "uart1.h"*/

#define _ISOC99_SOURCE
#define _GNU_SOURCE


void init_enc(void);
void process_enc(void);
void init_timer(void);
void display(void);
void rezim(void);
void minutky(void);
void pocitani(void);
void pricti_cas(void);
void odecti_cas(void);

uint8_t y=0;
uint16_t time, cas, doba, trvani, odpocet, led=0;
int16_t sekundy=0;
int16_t minuty=0; // proměnná jejíž hodnotu měníme enkodérem
uint8_t textove_pole[32];

int main(void){
CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1); // 16MHz
GPIO_Init(GPIOD,GPIO_PIN_6,GPIO_MODE_OUT_PP_LOW_SLOW);
init_milis(); 
init_enc();		// inicializace vstupu enkodéru
lcd_init();		// inicializace displeje
init_timer();	
enableInterrupts(); 

  while (1){
	doba=milis();
	rezim();
	minutky();
	display();

	}
}

void display(void){ //zobrazování čísel na displeji 
	lcd_gotoxy(10,0);//stará se zobrazení textu a symbolů na displej.
	lcd_puts(":");
	sprintf(textove_pole,"%02u",sekundy);
	lcd_gotoxy(11,0);
	lcd_puts(textove_pole);
	sprintf(textove_pole,"%02u",minuty);
	lcd_gotoxy(8,0);
	lcd_puts(textove_pole);
	lcd_gotoxy(4,0);
	lcd_puts("cas ");
}



void rezim(void){ //startuje odpočet minutek, po stisknutí tlačítka enkodéru

		if(GPIO_ReadInputPin(GPIOF, GPIO_PIN_5) == RESET && y==0) {
			y=1;
			odpocet=0;
			led=0;
	} if(GPIO_ReadInputPin(GPIOF, GPIO_PIN_5) == RESET && y==1){
			y=0;
			odpocet=1;
	}
}

void minutky(void){ //funkce odpočítávání
	if(led==1){ 			//blikání ledky po skončení odpočtu
			if(milis() - trvani >= 200){
				trvani = milis(); 
				GPIO_WriteReverse(GPIOD,GPIO_PIN_6); 
			}
			}else{
				GPIO_WriteLow(GPIOD,GPIO_PIN_6);
			}

	if (odpocet==1){
			if(doba - cas >= 1000){
						cas = milis();
						sekundy--;
				if(sekundy<0){
					sekundy=59;
					minuty--;
				}
				if(minuty<0){
					odpocet=0;
					minuty=0;
					sekundy=0;
					led=1;
				}
				
					display();
			}
	}
}


// rutina přerušení
INTERRUPT_HANDLER(TIM3_UPD_OVF_BRK_IRQHandler, 15)
 {
	 TIM3_ClearITPendingBit(TIM3_IT_UPDATE); // vyčistit vlajku (nutné vždy)
	 process_enc(); 
 }

// vyhodnocuje stav enkodéru
void process_enc(void){
	static minule=1; // pamatuje si minulý stav vstupu A
	if (odpocet==0){
	if(GPIO_ReadInputPin(GPIOF,GPIO_PIN_7) == RESET && minule==1){
		minule = 0; // nyní je pin v log.0
		// přečteme stav vstupu B
		if(GPIO_ReadInputPin(GPIOF,GPIO_PIN_6) == RESET){
			minuty++;
		}else{
			minuty--;
		}
			if(minuty>99){
					minuty=0;
		}
			if(minuty<0){
				 minuty=0;
		}
	}
}	
	if(GPIO_ReadInputPin(GPIOF,GPIO_PIN_7) != RESET){
			minule = 1;   // pokud je vstup A v log.1
		} 
}

void init_enc(void){
GPIO_Init(GPIOF,GPIO_PIN_7,GPIO_MODE_IN_PU_NO_IT);  // vstup, s vnitřním pullup rezistorem
GPIO_Init(GPIOF,GPIO_PIN_6,GPIO_MODE_IN_PU_NO_IT);
GPIO_Init(GPIOF,GPIO_PIN_5,GPIO_MODE_IN_PU_NO_IT);
}

void init_timer(void){
TIM3_TimeBaseInit(TIM3_PRESCALER_16,1999); 
TIM3_ITConfig(TIM3_IT_UPDATE, ENABLE); 
TIM3_Cmd(ENABLE); // spustit timer
}



/*-------------------------------  Assert -----------------------------------*/
#include "__assert__.h"
