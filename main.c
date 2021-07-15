/**
  ******************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2018-xx-xx
  * @brief   USART��USART1�ӷ�����
  ******************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ�� STM32H750 ������ 
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :http://firestm32.taobao.com
  *
  ******************************************************************
  */  
#include "stm32h7xx.h"
#include "main.h"
#include "./led/bsp_led.h"
#include "./delay/core_delay.h" 
#include "./mpu/bsp_mpu.h" 
#include "./usart/bsp_debug_usart.h"

volatile uint8_t Rxflag=0;
uint8_t ucTemp;
struct dataMsg
{
	unsigned short c_angle;
	signed short x_angle;
	signed short y_angle;
	signed short z_angle;
};
float x,y,z;
/**
  * @brief  ������
  * @param  ��
  * @retval ��
  */
  uint8_t ucaRxBuf[256],sum;
	uint8_t ucaTxBuf[20]={0xAA,0x01,0x01,0xAC,0x00};
	uint16_t usRxCount=0; 
	struct dataMsg nMsg;
	uint32_t wait,tickstart;
int main(void)
{  
	/* ϵͳʱ�ӳ�ʼ����400MHz */
	SystemClock_Config();
  
 
	SCB_EnableICache();    // ʹ��ָ�� Cache
	SCB_EnableDCache();    // ʹ������ Cache
  
	/* LED �˿ڳ�ʼ�� */
	LED_GPIO_Config();	
    /* ���ڳ�ʼ�� */
    DEBUG_USART_Config();
	
	tickstart = HAL_GetTick();
	while (1)
	{
		/* 
			����DEBUG_USART�ڵ����ݣ����������� 
			���Խ��˶δ����װΪһ�����������������������̵���
		*/
		if(Rxflag)
		{
			if (usRxCount < sizeof(ucaRxBuf))
			{
				ucaRxBuf[usRxCount++] = ucTemp;
			}
			else
			{
				usRxCount = 0;
			}
			
			/* �򵥵�ͨ��Э�飬�����س����з���Ϊ1������w֡�������м������ж�ʵ���Զ������� */
			/* ���������ַ�����Ϊ���յ�һ������ */
			if (usRxCount == ucaRxBuf[1]+3)	/* �����ַ� */
			{	
				sum=0;
				for(usRxCount=0;usRxCount<ucaRxBuf[1]+2;usRxCount++)
				{
					sum+=ucaRxBuf[usRxCount];
				}
				if(sum==ucaRxBuf[11] && ucaRxBuf[2]==1)
				{
					nMsg.c_angle= (uint16_t)(ucaRxBuf[3]<<8)| ucaRxBuf[4];
					nMsg.x_angle=((int16_t)(ucaRxBuf[5]<<8)| ucaRxBuf[6]);
					nMsg.y_angle=((int16_t)(ucaRxBuf[7]<<8)| ucaRxBuf[8]);
					nMsg.z_angle=((int16_t)(ucaRxBuf[9]<<8)| ucaRxBuf[10]);
					x=nMsg.x_angle;
					x=x/10;
					y=nMsg.y_angle;
					y=y/10;
					z=nMsg.z_angle;
					z=z/10;
					usRxCount = 0;
				}
			}
			Rxflag=0;
		}
		if((HAL_GetTick() - tickstart) >= 100)
		{
			LED1_TOGGLE;
			tickstart = HAL_GetTick();
			HAL_UART_Transmit( &UartHandle,ucaTxBuf ,4,1000);
		}
	}
}

/**
  * @brief  System Clock ����
  *         system Clock ��������: 
	*            System Clock source  = PLL (HSE)
	*            SYSCLK(Hz)           = 400000000 (CPU Clock)
	*            HCLK(Hz)             = 200000000 (AXI and AHBs Clock)
	*            AHB Prescaler        = 2
	*            D1 APB3 Prescaler    = 2 (APB3 Clock  100MHz)
	*            D2 APB1 Prescaler    = 2 (APB1 Clock  100MHz)
	*            D2 APB2 Prescaler    = 2 (APB2 Clock  100MHz)
	*            D3 APB4 Prescaler    = 2 (APB4 Clock  100MHz)
	*            HSE Frequency(Hz)    = 25000000
	*            PLL_M                = 5
	*            PLL_N                = 160
	*            PLL_P                = 2
	*            PLL_Q                = 4
	*            PLL_R                = 2
	*            VDD(V)               = 3.3
	*            Flash Latency(WS)    = 4
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  HAL_StatusTypeDef ret = HAL_OK;
  
  /*ʹ�ܹ������ø��� */
  MODIFY_REG(PWR->CR3, PWR_CR3_SCUEN, 0);

  /* ��������ʱ��Ƶ�ʵ������ϵͳƵ��ʱ����ѹ���ڿ����Ż����ģ�
		 ����ϵͳƵ�ʵĵ�ѹ����ֵ�ĸ��¿��Բο���Ʒ�����ֲᡣ  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}
 
  /* ����HSE������ʹ��HSE��ΪԴ����PLL */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
  RCC_OscInitStruct.CSIState = RCC_CSI_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;

  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 160;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
 
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
//  if(ret != HAL_OK)
//  {

//    while(1) { ; }
//  }
  
	/* ѡ��PLL��Ϊϵͳʱ��Դ����������ʱ�ӷ�Ƶ�� */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK  | \
																 RCC_CLOCKTYPE_HCLK    | \
																 RCC_CLOCKTYPE_D1PCLK1 | \
																 RCC_CLOCKTYPE_PCLK1   | \
                                 RCC_CLOCKTYPE_PCLK2   | \
																 RCC_CLOCKTYPE_D3PCLK1);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;  
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2; 
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2; 
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2; 
  ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4);
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }
}
/****************************END OF FILE***************************/
