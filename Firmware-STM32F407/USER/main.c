
//              说明: SPI显示屏接线如下
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
					
					
					
				
