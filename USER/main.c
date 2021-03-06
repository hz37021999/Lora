#include "sys.h"
#include "delay.h"
#include "usart.h" 
#include "led.h" 		 	 
#include "lcd.h"  
#include "key.h"     
#include "usmart.h" 
#include "malloc.h"
#include "sdio_sdcard.h"  
#include "w25qxx.h"    
#include "ff.h"  
#include "exfuns.h"   
#include "text.h"		
#include "usart3.h"
#include "string.h"
#include "touch.h"
#include "lora_app.h"
#include "adc.h"
#include "I2C.h"



//主函数
int main(void)
{	 	 

	u8 key=0;
	u8 fontok=0; 
  	
	delay_init();	    	                        //延时函数初始化	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	uart_init(115200);	 	                        //串口初始化为115200
  LCD_Init();	 
	IIC_Init();
	tp_dev.init();				                    //初始化触摸屏
	LED_Init();
	KEY_Init();
  W25QXX_Init();			  	                    //初始化W25Q128
	Adc_Init();		  		//ADC初始化
	my_mem_init(SRAMIN);		                    //初始化内部内存池
	exfuns_init();				                    //为fatfs相关变量申请内存  
 	f_mount(fs[0],"0:",1); 	                        //挂载SD卡 
 	f_mount(fs[1],"1:",1); 	                        //挂载FLASH.
	

	if(PCin(2)==0)
	{
				PCout(3) = 0;
				delay_ms(100);
				PCout(3) = 1;		
	}   //电压传感器开始
		/*
	key=KEY_Scan(0);  
	if(key==KEY0_PRES&&((tp_dev.touchtype&0X80)==0))//强制校准
	{
		LCD_Clear(WHITE);		                    //清屏
		TP_Adjust();  			                    //屏幕校准 
		TP_Save_Adjdata();	  
		LCD_Clear(WHITE);		                    //清屏
	}
	fontok=font_init();		                        //检查字库是否OK
	if(fontok||key==KEY1_PRES)	                    //需要更新字库				 
	{
		LCD_Clear(WHITE);		                    //清屏
 		POINT_COLOR=RED;		                    //设置字体为红色	   	   	  
		LCD_ShowString(60,50,200,16,16,"ALIENTEK STM32");
		while(SD_Init())		                    //检测SD卡
		{
			LCD_ShowString(60,70,200,16,16,"SD Card Failed!");
			delay_ms(200);
			LCD_Fill(60,70,200+60,70+16,WHITE);
			delay_ms(200);		    
		}								 						    
		LCD_ShowString(60,70,200,16,16,"SD Card OK");
		LCD_ShowString(60,90,200,16,16,"Font Updating...");
		key=update_font(20,110,16,"0:");           //从SD卡更新
		while(key)                                 //更新失败		
		{			 		  
			LCD_ShowString(60,110,200,16,16,"Font Update Failed!");
			delay_ms(200);
			LCD_Fill(20,110,200+20,110+16,WHITE);
			delay_ms(200);		       
		} 		  
		LCD_ShowString(60,110,200,16,16,"Font Update Success!");
		delay_ms(1500);	
		LCD_Clear(WHITE);                         //清屏	       
	}  */
	Lora_Test();//主测试
	
}


