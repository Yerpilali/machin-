/*---------------------------------------------------------------------------------------------------------------
**Проект: "Сервер удаленного контроля светодиодов".
**Назначение программы: принять по USART пакеты данных от ПК, запустить сведтодиоды и вернуть данные о их состоянии
**Разработчик: Нестеров Даниил Александрович - 1191б
**Цель: создать сервер удаленного контроля светодиодов на стенде STM_01
**Решаемые задачи:
**				1. Конфигурирование линий GPIO на выполнение альтернативных функций;
**				2. Конфигурирование USART;
**				3. Получение данных о нажатой клавише
**				4. Включение/отключение светодиода в указанной позиции
**				5. Вывод сообщения о сотоянии светодиода
**-------------------------------------------------------------------------------------------------------------*/

#include "main.h"	//Заголовчоный файл с описанием подключаемых библиотечных модулей

int main(void)
{
	InitUSART1();																								//Инициализация USART1 лаборатоного стенда
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;													//Включение тактирования порта B: RCC_AHBENR_GPIOBEN=0x00040000																																			
	GPIOB->MODER |= GPIO_MODER_MODER0_0 | GPIO_MODER_MODER1_0 	//Переключение линий 0-8 порта B в режим "Output"
								| GPIO_MODER_MODER2_0 | GPIO_MODER_MODER7_0 
								| GPIO_MODER_MODER6_0 | GPIO_MODER_MODER5_0 
								| GPIO_MODER_MODER4_0	| GPIO_MODER_MODER3_0 
								| GPIO_MODER_MODER8_0;
	//Объявление переменных
	unsigned reg[8] = {0x1, 0x2, 0x4, 0x80, 0x40, 0x20, 0x10, 0x8};  																											//Регистры для запуска светодиодов
	unsigned input [9][9] = {{0x31,0}, {0x32,1}, {0x33,2}, {0x34,3}, {0x35,4}, {0x36,5}, {0x37,6}, {0x38,7}, {0x30,8}};		//Двумерный массив (Вхлдящее число в hex и его соответсявие в данной программе)
	uint16_t check[8] = {0,0,0,0,0,0,0,0};											//Масиив состояний светодиодов (0 - выключен, 1 - включён)
	unsigned elem = 0;																					//Номер нажатаой клавиши
	uint8_t d;																									//Данные из USART 
	GPIOB->BSRR |= 0x100;																				//Включение линии 8 порта B
	
	while(1){
		//Получение данных из USART
		while ((USART1->ISR & USART_ISR_RXNE) == 0) { } 					//Чтение регистра состояния ISR и анализ флага RXNE (выставляется в 1, когда новый пакет данных получен приёмником Rx и скопирован в RDR)
		d = (uint8_t)USART1->RDR;																	//Копирование данных из USART (регистр RDR) в програмную переменную.
		// Чтение регистра RDR приводит к сбросу флага RXNE = 0
		
		//Сопоставление нажатой клавиши с номером
		for (uint16_t i = 0; i < 9; i++){
			if (d == input[i][0]){																	//Если номер нажатой клавиши равен номеру первого элемента массива input
				elem = input[i][1];																			//переменной elem присваивается номер второго элемента массива
			}
		}
		
		//Вывод состояний всех светодиодов при нажатии на "9"
		if (d == 0x39){
			for (uint16_t i=0; i<8; i++){														//Перебор всех светодиодов
				msg((uint8_t)input[i][0],check[i]);										//Вызов функции отправки сообщения, выводится состояние светодиода
			}
			continue;																								//Переход к следующей иттерации цикла while(1)
		}
		
		//Отключение всех светодиодов и вывод их состояний при нажатии на "0"
		if (elem == 8){
			for (int16_t i = 0; i<8; i++){													//Перебор светодиодов
				GPIOB->BSRR = reg[i] << 16;														//Отключение светодиода 
				check[i] = 0;																					//Присваивание массиву состояний 0, светодиод отключён
				msg((uint8_t)input[i][0],check[i]);										//Вызов функции отправки сообщения, выводится состояние светодиода
			}
			continue;																								//Переход к следующей иттерации цикла while(1)
		}
		
		//Включение, отключение светодиода 
		if (check[elem] == 0){																		//Если светодиод отключен
			GPIOB->BSRR |= reg[elem];																	//включается свтодиод в позиции указанной в переменной elem
			check[elem] = 1;																					//Присваивание массиву состояний 1, светодиод включён
			msg(d, check[elem]);																			//Вывод сообщения о сотоянии светодиода
			continue;																									//Переход к следующей иттерации цикла while(1)
		}
		else{																											//Если сетодиод включен
			GPIOB->BSRR |= reg[elem]<<16;															//отключение свтодиода в позиции указанной в переменной elem
			check[elem] = 0;																					//Присваивание массиву состояний 0, светодиод отключён
			msg(d, check[elem]);																			//Вывод сообщения о сотоянии светодиода
		  continue;																									//Переход к следующей иттерации цикла while(1)
		}
	}
}

//Функция ввыода сообщения (number - номер светодиода, flag - его состояние (0 - отключен, 1 - включен))
void msg (uint8_t number, unsigned flag){
	uint8_t textDiod [5] = {0x44, 0x69, 0x6F, 0x64,0x20};					//Массив содержащий сообщение "Diod "
	uint8_t textOn[5] = {0x20, 0x6f, 0x6e, 0x0D, 0x0A};						//Массив содержащий сообщение " on" + переход на новую строку + сдвиг каретки
	uint8_t textOff[6] = {0x20, 0x6f, 0x66,0x66, 0x0D, 0x0A}; 		//Массив содержащий сообщение " off" + переход на новую строку + сдвиг каретки

		//Вывод сообщения "Diod "
		for (uint16_t i=0; i<5; i++){
			while ((USART1->ISR & USART_ISR_TXE) == 0) { }						//Чтение регистра состояния ISR и анализ флага TXE(выставляется в 1 копирование из TDR в сдвиговый регистр завершено)
			USART1->TDR = textDiod[i];																//Передача данных из програмной переменной в USART(регистр TDR)
			//Запись в регистр TDR приводит к сбросу флага TXE = 0
		}
		while ((USART1->ISR & USART_ISR_TXE) == 0) { }							//Чтение регистра состояния ISR и анализ флага TXE(выставляется в 1 копирование из TDR в сдвиговый регистр завершено)
		USART1->TDR = number;																				//Передача данных из програмной переменной(номер светодиода) в USART(регистр TDR)
		//Запись в регистр TDR приводит к сбросу флага TXE = 0
		
		//Вывод сообщения при включённом светодиоде 
		if (flag == 1){
			//Вывод сообщения " on"
			for (uint16_t i=0; i<5; i++){
			while ((USART1->ISR & USART_ISR_TXE) == 0) { } 						//Чтение регистра состояния ISR и анализ флага TXE(выставляется в 1 копирование из TDR в сдвиговый регистр завершено)
			USART1->TDR = textOn[i];																	//Передача данных из програмной переменной в USART(регистр TDR)
			//Запись в регистр TDR приводит к сбросу флага TXE = 0
			}
		}
		//Вывод сообщения при выключенном светодиоде 
		else {
			//Вывод сообщения " off"
			for (uint16_t i=0; i<6; i++){
			while ((USART1->ISR & USART_ISR_TXE) == 0) { }						//Чтение регистра состояния ISR и анализ флага TXE(выставляется в 1 копирование из TDR в сдвиговый регистр завершено)			
			USART1->TDR = textOff[i];				   												//Передача данных из програмной переменной в USART(регистр TDR)
			//Запись в регистр TDR приводит к сбросу флага TXE = 0
			}
		}
}

//Функция инициализации USART лабораторного комплекса STM_01
void InitUSART1(){
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;													//Включение тактирования USART1
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;														//Включение тактирования порта А
	
	//Настройка линий порта А: РА9(ТХ_1) - выход передатчика; PA10(RX_1) - вход приёмника
	GPIOA->MODER |= 0x00280000;																		//Перевести линии РА9 и РА10 в режим альтернативной функции
	GPIOA->AFR[1] |= 0x00000110;																	//Включить на линиях РА9 и РА10 альтернативную функцию AF1
	
	//Настройка линии передатчика Тх (РА9)
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT_9;														//Сбросить 9 бит GPIOA->OTYPER - переключение в режим push-pull для линии РА9 (активный выход) 
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR9;														//Отключение подтягивающих резисторов для линии РА9 
	GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEEDR9;											//Установка высокой скорости синхронизации линии РА9
	
	//Настройка линии приемника Rx (РА10)
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR10;													//Сброс режима подтягивающих резисторов для линии РА10
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR10_0;													//Включение подтягивающего резистора pull-up на входной линии РА10 (вход приемника Rx)
	
	//Конфигурирование USART
	USART1->CR1 &= ~USART_CR1_UE;																	//Запрещение работы модуля USART1 для изменения параметров его конфигурации
	USART1->BRR=69;																								/*Настройка делителя частоты, тактирующего USART и задающего скорость приема и передачи данных на уровне 115200 бит/с: 
																																	Частота тактирующего генератора = 8 МГц 
																																	Скорость обмена по USART - 115200 бит/с; коэффициент деления - 8000000 / 115200 - 69,4444(4); Округленное значение - 69*/
	USART1->CR1 = USART_CR1_TE | USART_CR1_RE;										/*Разрешить работу приемника и передатчика USART. Остальные биты этого регистра сброшены, что обеспечивает: 
																																	количество бит данных в пакете 8;
																																	контроль четности - отключен; 
																																	прерывания по любым флагам USART - запрещены;
																																	состояние USART - отключен*/
	USART1->CR2 = 0;																							//Количество стоповых бит - 1
	USART1->CR3 = 0;																							//DMA1 - отключен
	USART1->CR1 |= USART_CR1_UE;																	//По завершении конфигурирования USART разрешить его работу (биту UE регистра CR1 присвоить 1)
}

/*---------------------------------------------------------------------------------------------------------------
**Руководство пользователя:
**		1. Запустите программу на лабораторном комплексе STM_01;
**		2. На компьютере запустите любую программу для мониторинга последовательных портов ПК (Terminal, PuTTY или др.) и подключитесь к соответствующему COM-порту на скорости 115200 бит/с;
**		3. С помощью программы мониторинга отправьте на лабораторный стенд цифры от 0 до 9 
**		4. Клавиши 1-8 включают/отключают соответствующие светодиоды, 9 - вывод сотояния всех светодиодов, 0 - отключение всех светодиодов
**-------------------------------------------------------------------------------------------------------------*/
