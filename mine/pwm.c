#include "stm32f10x.h"
#include "pwm.h"

static void  TIM2_GPIO_Config(void)	 
{
  GPIO_InitTypeDef GPIO_InitStructure;
 
 RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);  //TIM2~7都挂载在APB1上


 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);	//GPIO都挂载在APB2上


 GPIO_InitStructure.GPIO_Pin=GPIO_Pin_1;
 GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;
 GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;

 GPIO_Init(GPIOA,&GPIO_InitStructure);

}


static void TIM2_Mode_Config(void)
{

u16 CCR1_Val=750;

TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	 /* Time base configuration */		 
  TIM_TimeBaseStructure.TIM_Period = 999;       //当定时器从0计数到999，即为1000次，为一个定时周期
  TIM_TimeBaseStructure.TIM_Prescaler = 0;	    //设置预分频：不预分频，即为72MHz
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1 ;	//设置时钟分频系数：不分频
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //向上计数模式

  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
   /* PWM1 Mode configuration: Channel2 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;	    //配置为PWM模式1
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;	
  TIM_OCInitStructure.TIM_Pulse = CCR1_Val;	   //设置跳变值，当计数器计数到这个值时，电平发生跳变
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;  //当定时器计数值小于CCR1_Val时为高电平
  TIM_OC2Init(TIM2, &TIM_OCInitStructure);	 //使能通道3


  TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);



	 
TIM_ARRPreloadConfig(TIM2, ENABLE);			 // 使能TIM2重载寄存器ARR

  /* TIM3 enable counter */
  TIM_Cmd(TIM2, ENABLE);                   //使能定时器2
}



 void TIM2_PWM_Init(void)
{
	TIM2_GPIO_Config();
	TIM2_Mode_Config();	
}

