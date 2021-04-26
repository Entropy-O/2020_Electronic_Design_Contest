#include "adc.h"
#include "delay.h"	
#include "lcd.h"

#include "arm_math.h"
u16 ADCdata[DataLenth];

u8 flag_adc=0;
u16 ADCFIFO[16];//ADC16个规则转换通道暂存
u8 RegularConvCnt=0;//规则转换计数器
u16 max_cnt=0;
u8 maxtemp_cnt;
/**********************************
初始化ADC	ADC采峰值
//15HZ到9.8M
//200mv到3v
注意ADC的DMA通道
**********************************/
void  Adc_Init(void)
{    
  GPIO_InitTypeDef  GPIO_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_InitTypeDef       ADC_InitStructure;
	DMA_InitTypeDef  DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//使能GPIOA时钟
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); //使能ADC1时钟	
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);//DMA2时钟使能 
		
  DMA_DeInit(DMA2_Stream0);
	
	while (DMA_GetCmdStatus(DMA2_Stream0) != DISABLE){}//等待DMA可配置 
	
  /* 配置 DMA Stream */
  DMA_InitStructure.DMA_Channel = DMA_Channel_0;  //通道选择
  DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&ADC1->DR;//DMA外设地址
  DMA_InitStructure.DMA_Memory0BaseAddr = (u32)ADCdata;//DMA 存储器0地址
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;//外设到存储器模式
  DMA_InitStructure.DMA_BufferSize =DataLenth;//数据传输量 
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设非增量模式
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//存储器增量模式
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//外设数据长度:16位
  DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_HalfWord;//存储器数据长度:16位
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;// 循环模式
  DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;//中等优先级
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;//存储器突发单次传输
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;//外设突发单次传输
	DMA_ITConfig(DMA2_Stream0,DMA_IT_TC,ENABLE);
  DMA_Init(DMA2_Stream0, &DMA_InitStructure);//初始化DMA Stream
		
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn;//中断通道DMA2数据流0
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;//子优先级1
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

  //先初始化ADC1通道5 IO口
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_6;//PA5 通道5
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;//模拟输入
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;//不带上下拉
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化  
 
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1,ENABLE);	  //ADC1复位
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1,DISABLE);	//复位结束	 
 
	
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;//独立模式
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;//两个采样阶段之间的延迟5个时钟
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled; //多ADC DMA数据传输失能
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;//预分频4分频。ADCCLK=PCLK2/4=84/4=21Mhz,ADC时钟最好不要超过36Mhz 
  ADC_CommonInit(&ADC_CommonInitStructure);//初始化
	
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;//12位模式
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;//非扫描模式	
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;//关闭连续转换
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;//禁止触发检测，使用软件触发
 	ADC_InitStructure.ADC_ExternalTrigConv =  ADC_ExternalTrigConv_T3_TRGO ;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//右对齐	
  ADC_InitStructure.ADC_NbrOfConversion = 2;//1个转换在规则序列中 也就是只转换规则序列1 
  ADC_Init(ADC1, &ADC_InitStructure);//ADC初始化
	
	ADC_DMARequestAfterLastTransferCmd(ADC1,ENABLE);//使能ADC在DMA传输完成后再次进行DMA请求
	ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 1, ADC_SampleTime_3Cycles );	//ADC1,ADC通道,480个周期,提高采样时间可以提高精确度	
	ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 2, ADC_SampleTime_3Cycles );	
	
 	//Tconv=Ts+12Tclk=28Tclk+12Tclk=40Tclk=492/21=1.9us,再加上两个转换之间的延时5Tclk =45Tclk/21=2.1us
	//所以在该设置下最短触发2.1us
	ADC_DMACmd(ADC1, ENABLE);//使能ADC1的DMA
	ADC_Cmd(ADC1, ENABLE);//开启AD转换器	
	DMA_Cmd(DMA2_Stream0, ENABLE);  //开启DMA传输 
	
	
	
	
}

void DMA2_Stream0_IRQHandler(void)
{ 
	u16 i;	

	TIM_Cmd(TIM3,DISABLE);
	DMA_ClearFlag(DMA2_Stream0,DMA_FLAG_TCIF0);	//清除DMA传输完成中断  
	flag_adc=1;
	for(i=0;i<DataLenth;i++)
	{
//		InBufArray[i] = ((signed short)(ADCdata[i])) << 16;  //左移16位，高16位存放实部
	
	}
		TIM_Cmd(TIM3,ENABLE);
}

//获得ADC值
//ch: @ref ADC_channels 
//通道值 0~16取值范围为：ADC_Channel_0~ADC_Channel_16
//返回值:转换结果

	 
 void TIM3_ConfigInit(u16 arr)
{	 
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
//	NVIC_InitTypeDef NVIC_InitStructure;
	//初始化定时器3	 
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
	TIM_TimeBaseStructure.TIM_Period = arr;  //50us触发一次        
	TIM_TimeBaseStructure.TIM_Prescaler = 0; 	//工作在84M，      
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;    
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);
	TIM_SelectOutputTrigger(TIM3,TIM_TRGOSource_Update); //使用TIM3事件更新作为ADC触发	
}
