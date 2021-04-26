//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//中景园电子
//店铺地址：http://shop73023976.taobao.com/?spm=2013.1.0.0.M4PqC2
//
//  文 件 名   : main.c
//  版 本 号   : v2.0
//  作    者   : HuangKai
//  生成日期   : 2018-0101
//  最近修改   : 
//  功能描述   :演示例程(STM32F4系列)
//              说明: 
//              ----------------------------------------------------------------
//              GND    电源地
//              VCC  接5V或3.3v电源
//              SCL   PG12（SCLK）
//              SDA   PD5（MOSI）
//              RES   PD4
//              DC    PD15
//              CS    PD1
//              BLK   PE8
//              ----------------------------------------------------------------
// 修改历史   :
// 日    期   : 
// 作    者   : HuangKai
// 修改内容   : 创建文件
//版权所有，盗版必究。
//  生成日期   : 2018-0101
//All rights reserved
//******************************************************************************/
#include "delay.h"
#include "sys.h"
#include "led.h"
#include "lcd_init.h"
#include "lcd.h"
//#include "pic.h"
#include "usart.h"
#include "adc.h"
#include "arm_math.h"
#include "math.h"
#include "main.h"
#include "arm_const_structs.h"
#define FFT_LENGTH 1024
extern u16 ADCdata[DataLenth];
float ADC_temp;
float Fs,FFT_DPI;
int max;
float fft_inputbuf[FFT_LENGTH*2] = {0};	//FFT输入数组
float fft_outputbuf[FFT_LENGTH] = {0};	//FFT输出数组


float F0,xiebo_shizhen,xiebo_shizhen1,chuxiang,xiangweicha;
u16 x; 
int i,j;
u32 fre_temp;
float Precent=0 ;
u8 N ,n,cnt;	
float mag[20];
float xiebo[20];
float xiebo_hanliang[5];
u32 fr[20];
u32 xiebo_fr[20];
int max1,max2;
int m,num;
u8 cnt,count,xiebo_cishu,xiebo_hlcs;
int ln,imax;
 extern __IO uint16_t ADC_ConvertedValue[4];
float FINAL_DHT;
float select_max(float *a);
 
int main(void)
{ 
	delay_init(168);
	LCD_Init();//LCD初始化
	LCD_Fill(0,0,LCD_W,LCD_H,BLACK);
	uart_init(115200);
	printf("LCD/USART Initialized！Waiting for input......\n");
	
		//LCD_ShowString(0,0,"Yu Tian's LCD",WHITE,BLACK,32,0);
		//LCD_ShowString(0,50,"Freq:",WHITE,BLACK,32,0);
		//LCD_ShowString(220,60,"Hz",WHITE,BLACK,16,0);
		//LCD_ShowString(0,130,"duty:",WHITE,BLACK,32,0);
	
	 NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	 Adc_Init();
	 TIM3_ConfigInit(20508);//默认Fs为4096Hz
	 Fs=84000000.0f/20508.0f;     //采样频率
	 FFT_DPI = Fs/FFT_LENGTH;    //傅里叶变换的分辨率是16HZ
	 TIM_Cmd(TIM3,ENABLE);
	
	//arm_cfft_radix4_init_f32(&scfft,FFT_LENGTH,0,1);//初始化scfft结构体，设定FFT相关参数
	 while(1)
	 {	
			for(i=0;i>FFT_LENGTH;i++)
		 {
			 fft_inputbuf[i] = 0;
			 fft_outputbuf[i] = 0;
		 }
		  for(i=0;i<FFT_LENGTH;i++)
				{ 
					ADC_temp=((float)ADCdata[i]*(3.3f/4095.0f));//*Blackman[i];
					
					fft_inputbuf[2*i]= ADC_temp;
					
					fft_inputbuf[2*i+1] = 0.0f;
				}
//				for(i=0;i<FFT_LENGTH;i++)//生成信号序列
//          {
//							fft_inputbuf[2*i]=10+4.5*arm_sin_f32(2*PI*i*200/FFT_LENGTH);
//              fft_inputbuf[2*i+1]=0.0f;//虚部全部为0
//          }
//				
				/*for(i=0;i<FFT_LENGTH;i++)
				{
					printf("%lf       \r\n",fft_inputbuf[i]);
				}*/
		 //LCD_ShowFloatNum1(0,200,InBufArray[0],28,WHITE,BLACK,16);
		// LCD_ShowIntNum(0,100,ADCdata[8],4,WHITE,BLACK,32);        //show ADC的数据
				
		 //arm_cfft_radix4_f32(&scfft,fft_inputbuf);
		// arm_cfft_f32()
		 //arm_cmplx_mag_f32(fft_inputbuf,fft_outputbuf,FFT_LENGTH);
		arm_cfft_f32(&arm_cfft_sR_f32_len1024, fft_inputbuf, 0, 1);
    arm_cmplx_mag_f32(fft_inputbuf, fft_outputbuf, FFT_LENGTH);
		FINAL_DHT=select_max(fft_outputbuf);
		LCD_ShowString(0,0,"DHT IS:",WHITE,BLACK,32,0);
		LCD_ShowFloatNum1(33,30,FINAL_DHT,5,WHITE,BLACK,32);
		LCD_ShowString(135,30,"%",WHITE,BLACK,32,0);
		delay_ms(50);
}
}

float select_max(float *a)
{
   int i,j;
   float k,k1,m,THD;
   float aMax =0.0,aSecondMax = 0.0,aThirdMax = 0.0,aFourthMax=0.0,aFifthMax=0.0;
   float fMax =0.0,fSecondMax = 0.0,fThirdMax = 0.0,fFourthMax=0.0,fFifthMax=0.0;
   int nMax=0,nSecondMax=0,nThirdMax=0,nFourthMax=0,nFifthMax=0;
   for ( i = 1; i<255; i++)//i必须是1，是0的话，会把直流分量加进去
    {
        if (a[i]>aMax)
        {
            aMax = a[i]; 
            nMax=i;
    //      fMax=f[nMax];
        }
    }
  for ( i=1; i<255; i++)
    {
        if (nMax == i)
       {
            continue;//跳过原来最大值的下标，直接开始i+1的循环
       }
        if (a[i]>aSecondMax&&a[i]>a[i+1]&&a[i]>a[i-1])
       {
            aSecondMax = a[i]; 
            nSecondMax=i;
      //    fSecondMax=f[nSecondMax];
       }
    }
  for ( i=1; i<255; i++)
    {
        if (nMax == i||nSecondMax==i)
       {
            continue;//跳过原来最大值的下标，直接开始i+1的循环
       }
        if (a[i]>aThirdMax&&a[i]>a[i+1]&&a[i]>a[i-1])
        {
            aThirdMax = a[i]; 
            nThirdMax=i;
     //     fThirdMax=f[nThirdMax];
        }
    }
  for ( i=1; i<255; i++)
    {
        if (nMax == i||nSecondMax==i||nThirdMax==i)
        {
            continue;//跳过原来最大值的下标，直接开始i+1的循环
        }
        if (a[i]>aFourthMax&&a[i]>a[i+1]&&a[i]>a[i-1])
        {
            aFourthMax = a[i]; 
            nFourthMax=i;
     //     fFourthMax=f[nFourthMax];
        }
    }
  for ( i=1; i<255; i++)
    {
        if (nMax == i||nSecondMax==i||nThirdMax==i||nFourthMax==i)
        {
            continue;//跳过原来最大值的下标，直接开始i+1的循环
        }
        if (a[i]>aFourthMax&&a[i]>a[i+1]&&a[i]>a[i-1])
        {
            aFifthMax = a[i]; 
            nFifthMax=i;
     //     fFifthMax=f[nFifthMax];
        }
    }
  fMax=aMax/128;
  fSecondMax = aSecondMax/128;
	fThirdMax = aThirdMax/128;
	fFourthMax=aFourthMax/128;
	fFifthMax=aFifthMax/128;
	k=sqrt(fSecondMax*fSecondMax+fThirdMax*fThirdMax+fFourthMax*fFourthMax+fFifthMax*fFifthMax);
	k1=fMax;
	m=k/k1;
	THD=m*100;
	
	return THD;
}					
					
					
					
					
					
		
		//printf("%f \r\n",fft_outputbuf[0]);

		 //LCD_ShowFloatNum1(0,200,fft_outputbuf[0],10,WHITE,BLACK,16);           //Show 傅里叶变换的数据、
		 //LCD_ShowIntNum(2,200,fft_outputbuf[0],8,WHITE,BLACK,32);
		 //LCD_ShowFloatNum1(0,250,fft_outputbuf[0]/FFT_LENGTH,6,WHITE,BLACK,32);  //F = 0的基波幅值
					/*******************************比较大小**************************/

// for(i=0;i<FFT_LENGTH;i++){
//   if(max<fft_outputbuf[i]){
//     max = fft_outputbuf[i];
//     num = i;
//     }
//    }
//		LCD_ShowIntNum(0,0,max,6,WHITE,BLACK,32);
//		LCD_ShowFloatNum1(50,100,fft_outputbuf[num],6,WHITE,BLACK,32);
//}
//}

	/*****************************************各次谐波分析***************************************/
//		for(i=0;i<FFT_LENGTH;i++)
//			{
//				if(max < fft_outputbuf[i])	 //找出Vmax 用于归一化处理
//				{
//					 max = fft_outputbuf[i];	
//				}
//			}
//			LCD_ShowFloatNum1(50,100,fft_outputbuf[max],6,RED,BLACK,32);
//			
//			cnt=0;xiebo_shizhen=0;
//			F0=Fs/FFT_LENGTH;//频率分辨率
//			N=FFT_LENGTH/256;//采样点数对256的商
//			n=FFT_LENGTH%256;//采样点数对256的余数
//			
//				if(n>20)N+=1;
//				 x=0;
//				for(i=0;i<FFT_LENGTH;i++)
//				{
//					Precent=fft_outputbuf[i]/max;	
//					if(i<(FFT_LENGTH/2+1))
//					{  
//						if(max1 < fft_outputbuf[i+2])	 //找出Vmax 用于归一化处理
//						{
//					 max1 = fft_outputbuf[i+2];	
//					 max2=i+2;
//						}
//						if(Precent>0.03f)//幅值占最大赋值的3%则认为该分量在信号中有作用,记录其频谱特性
//						{
//							if(cnt<20)
//							{
//								if(i==0)
//								{fr[cnt]=0;
//								mag[cnt]=(fft_outputbuf[i]/FFT_LENGTH)*2.381f;}
//								else
//								{fre_temp=(i*F0*10+5)/10;//4舍5入
//								fr[cnt]=fre_temp;
//								mag[cnt]=((fft_outputbuf[i]*2)/FFT_LENGTH)*2.381f;}	
//								cnt++;
//							}							
//						}					
//					}
//				}	
//			for(xiebo_cishu=0;xiebo_cishu<10;xiebo_cishu++) 				
//				 {
//							if(xiebo_cishu==0)
//							{xiebo_fr[0]=0;
//							xiebo[0]=(fft_outputbuf[0]/FFT_LENGTH)*2.381f;}
//							else
//							{
//								xiebo_fr[xiebo_cishu]=xiebo_cishu*max2;
//							xiebo[xiebo_cishu]=((fft_outputbuf[xiebo_fr[xiebo_cishu]]*2)/FFT_LENGTH)*2.381f;}						
//					}
//				 
//			for(xiebo_cishu=0;xiebo_cishu<10;xiebo_cishu++)
//				{	 
//					xiebo_shizhen+=(xiebo[xiebo_cishu+1]*xiebo[xiebo_cishu+1]);
//				}
//			xiebo_shizhen1=100*sqrt((xiebo_shizhen-(xiebo[1]*xiebo[1]))/(xiebo[1]*xiebo[1]));
//	for(xiebo_hlcs=1;xiebo_hlcs<6;xiebo_hlcs++)    // 谐波含量计算
//		{
//				xiebo_hanliang[xiebo_hlcs]=xiebo[xiebo_hlcs]*100/xiebo[1];
//		}
//		
//		//display_info();
//		
//		
//				
//	}
//	 
//}


//void display_info(void)
//{
//		u16 i,j,y;
//		float AM_temp;
//			LCD_ShowString(80,0,"FFT result",WHITE,BLACK,16,0);
//			j=10;
//			for(i=0;i<cnt;i++)//显示幅值信息
//			{
//				if(i>10)
//				{
//					j=150;
//					y=i-11;
//				}
//				else
//				{
//					y=i;
//				}
//				if(i==0)
//					LCD_ShowChar(j,20+y*16,'0',WHITE,BLACK,16,0);
//				else LCD_ShowFloatNum1(j,20+y*16,fr[i],5,WHITE,BLACK,16);
//				LCD_ShowString(j+40,20+y*16,"Hz:",WHITE,BLACK,16,0);
//				//AM_temp =mag[i];//得到幅度信息用于显示
//				AM_temp = xiebo_hanliang[i+1];//得到幅度信息用于显示
//				LCD_ShowFloatNum1(j+58,20+y*16,AM_temp,5,WHITE,BLACK,16);
//				//LCD_ShowString(j+90,20+y*16,16,12,12,"v");//显示小数点
//			} 
//}

//void display_xiebo(void)
//{
//		u16 i,j,y;
//		float AM_temp;
//			LCD_ShowString(150,10,60,20,12,"FFT_xiebo_result");
//			j=10;
//			for(i=0;i<xiebo_cishu;i++)//显示幅值信息
//			{
//				if(i>10)
//				{
//					j=150;
//					y=i-11;
//				}
//				else
//				{
//					y=i;
//				}
//				LCD_ShowIntNum(j,20+y*16,xiebo_fr[i],4,WHITE,BLACK,16);
//				LCD_ShowString(j+40,20+y*16,"Hz:",WHITE,BLACK,16,0);
//				AM_temp = xiebo[i];//得到幅度信息用于显示
//				LCD_ShowFloatNum1(j+58,20+y*16,AM_temp,5,WHITE,BLACK,16);
//				LCD_ShowString(j+90,20+y*16,"v",WHITE,BLACK,16,0);//显示小数点
//			}
//	  for(i=0;i<6;i++)//显示幅值信息
//			{
//					y=i;
//				LCD_ShowNum(j+100,290+y*16,i+1,6,12);
//				LCD_ShowString(j+140,290+y*16,16,12,12,"ci:");
//				AM_temp = xiebo_hanliang[i+1];//得到幅度信息用于显示
//				LCD_ShowFloatNum(j+158,290+y*16,AM_temp,3,12,0);
//				LCD_ShowString(j+190,290+y*16,16,12,12,"%");//显示小数点

//		
//}

//中断处理函数
