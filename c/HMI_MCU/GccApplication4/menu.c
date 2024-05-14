
#include "config.h"
#include "display.h"
#include "i2c.h"
#include "usart.h"
#include "variables.h"
#include <avr/cpufunc.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
const uint8_t addr = 0x27;
volatile uint8_t selected_menu_index=0;
volatile uint8_t selected_menuitem=0;
volatile uint8_t max_menu_index = 3;
volatile uint8_t show_0_3=1;
volatile uint8_t show_1_4=0;
volatile uint8_t show_2_5=0;


char buffer[20];
char returnstatus[] = "Return";

/*		Main menu items			*/
char mainmenuindex0[] = "Show data";
char mainmenuindex1[] = "Regulatormode";
char mainmenuindex2[] = "IO settings";
char mainmenuindex3[] = "Generator";

/*		Info menu items		*/
char infoindex0[] = "Rpm";
char infoindex1[] = "BackEMF[V]";
char infoindex2[] = "Current[mA]";
char infoindex3[] = "Power[W]";
char rpm_string[] = "[N/A]";
char backemf_string[] = "[N/A]";
char current_string[] = "[N/A]";
char dutycycle_string[] = "[N/A]";

/*		IO settings menu items		*/
char iomenuindex0[] = "Digital in";
char iomenuindex1[] = "Digital out";
char iomenuindex2[] = "Analog in";
char iomenuindex3[] = "Analog out";
char* iomenuindex4 = returnstatus;

char statusOn[] = " [On]";
char statusOff[] = "[Off]";

char menuitemstatuson[]= " [ON]";
char menuitemstatusoff[]= "[OFF]";

char* iomenuindex0status = statusOff;
char* iomenuindex1status = statusOff;
char* iomenuindex2status = statusOff;
char* iomenuindex3status = statusOff;

// Regulator menu items	
char regulatorindex0[] = "Enable";
char regulatorindex1[] = "Setpoint";
char regulatorindex2[] = "KP";
char regulatorindex3[] = "KI";
char regulatorindex4[] = "KD";
char regulatorindex5[] = "Return";

char *regulatorstatusindex0;
char regulatorstatusindex1[]="[N/A]";
char regulatorstatusindex2[]="[N/A]";
char regulatorstatusindex3[]="[N/A]";
char regulatorstatusindex4[]="[N/A]";

// Generator menu items
char generatorindex0[]="Enable";
char generatorindex1[]="Control";
char generatorindex2[]="Load";
char generatorindex3[]="Return";
char statusUART[]="UART";
char statusADC[]="ADC";
char* generatorindex0status=statusOff;
char* generatorindex1status=statusUART;
char generatorindex2status[]="0";	



char* menuitem0status;
char* menuitem1status;
char* menuitem2status;
char* menuitem3status;

/*		General menuitems		*/
char* menuitem0 = mainmenuindex0;
char* menuitem1 = mainmenuindex1;
char* menuitem2 = mainmenuindex2;
char* menuitem3 = mainmenuindex3;



//selected index of menu selected
volatile uint8_t indexselected = 0;

//flag for selected menu
volatile uint8_t menuselected = 1; 
// mainmenu = 0 infomenu = 1 
//regulatormode=2 iosettingsmenu = 3 generator = 4

volatile uint8_t rpm_value_write;
volatile uint8_t regulatormodestatus = 0;
volatile uint16_t adjustsetpoint = 2000;
volatile uint16_t adjustsetpointstep = 100;
volatile uint8_t adjustmode=0;
volatile uint16_t adjustkp=0;
volatile uint16_t adjustki=0;
volatile uint16_t adjustkd=0;
volatile uint16_t adjustkpstep=5;
volatile uint16_t adjustkistep=5;
volatile uint16_t adjustkdstep=5;
volatile uint16_t adjustsetpointmax=4000;
volatile uint16_t adjustkpmax=1000;
volatile uint16_t adjustkimax=1000;
volatile uint16_t adjustkdmax=1000;
volatile uint16_t adjustgenerator=0;
volatile uint16_t adjustgenerator_old=1024;
volatile uint16_t adjustgeneratorstep=5;
volatile uint16_t adjustgeneratormax = 100;

volatile uint8_t generatorenable = 0;
volatile uint8_t generatorADCenable = 0;

volatile uint8_t refresh_screen=0;

volatile uint8_t io_changed=0;
volatile uint8_t generator_changed=0;
volatile uint8_t regulatormode_changed=0;

void Menu_Intro(void){
	HD44780_PCF8574_DisplayClear(addr);
	_delay_ms(500);
	HD44780_PCF8574_PositionXY(addr, 0,0);
	HD44780_PCF8574_DrawString(addr, "O");
	HD44780_PCF8574_PositionXY(addr, 0,3);
	HD44780_PCF8574_DrawString(addr, "O");

	for (uint8_t i=1;i<19;i++){
		HD44780_PCF8574_PositionXY(addr, i,0);
		HD44780_PCF8574_DrawString(addr, "-");
		_delay_ms(30);
		HD44780_PCF8574_PositionXY(addr, i,3);
		HD44780_PCF8574_DrawString(addr, "-");
		_delay_ms(30);
	}
	HD44780_PCF8574_PositionXY(addr, 19,0);
	HD44780_PCF8574_DrawString(addr, "0");
	HD44780_PCF8574_PositionXY(addr, 19,3);
	HD44780_PCF8574_DrawString(addr, "0");
	
	HD44780_PCF8574_PositionXY(addr, 0,1);
	HD44780_PCF8574_DrawString(addr, "|");
	HD44780_PCF8574_PositionXY(addr, 19,1);
	HD44780_PCF8574_DrawString(addr, "|");
	HD44780_PCF8574_PositionXY(addr, 0,2);
	HD44780_PCF8574_DrawString(addr, "|");
	HD44780_PCF8574_PositionXY(addr, 19,2);
	HD44780_PCF8574_DrawString(addr, "|");
	HD44780_PCF8574_PositionXY(addr, 2,1);
	HD44780_PCF8574_DrawString(addr, "This ");
	_delay_ms(800);
	HD44780_PCF8574_DrawString(addr, "is");
	_delay_ms(800);
	HD44780_PCF8574_PositionXY(addr, 9,2);
	HD44780_PCF8574_DrawString(addr, "THE RIGG");
	
	_delay_ms(3000);
	refresh_screen=1;
}

void ButtonUpPressed(void){	
	if (indexselected > 0 && adjustmode != 1){	
		indexselected--;
		if (indexselected==1){	//scroll menu up
			show_0_3=0;
			show_1_4=1;
			show_2_5=0;
		}
		else if (indexselected==0){	//scroll menu up
			show_0_3=1;
			show_1_4=0;
			show_2_5=0;
		}
	}
	else if(adjustmode==1){
		if (menuselected==2){
			if (indexselected == 1){	//selected setpoint
				if (adjustsetpoint<=(adjustsetpointmax-adjustsetpointstep)){//limit upper value of setpoint
					adjustsetpoint = adjustsetpoint+adjustsetpointstep;
				}
				
			}
			else if (indexselected==2){	//selected KP
				if (adjustkp<=(adjustkpmax-adjustkpstep)){	//limit upper value of KP
					adjustkp=adjustkp+adjustkpstep;
				}
			}
			else if (indexselected==3){	//selected KI
				if (adjustki<=(adjustkimax-adjustkistep)){	//limit upper value of KI
					adjustki=adjustki+adjustkistep;
				}
			}
			else if (indexselected==4){	//selected KD
				if (adjustkd<=(adjustkdmax-adjustkdstep)){	//limit upper value of KD
					adjustkd=adjustkd+adjustkdstep;
				}
			}
		}
		else if(menuselected=4){	//generator menu selected
			if (indexselected == 2){	//load selected
				if (adjustgenerator <adjustgeneratormax){
					adjustgenerator=adjustgenerator+adjustgeneratorstep;
					if (adjustgenerator>= 100){
						adjustgenerator=100;
					}
				}
			}
		}
		
	}
	refresh_screen=1;
}

void ButtonDownPressed(void){	
	if (indexselected < max_menu_index && adjustmode != 1){
		indexselected++;
		if (indexselected==4){		//scroll down menu
			show_0_3=0;
			show_1_4=1;
			show_2_5=0;		
		}
		else if (indexselected==5){	//scroll down menu
			show_0_3=0;
			show_1_4=0;
			show_2_5=1;
		}
	}
	else if(adjustmode==1){
		if (menuselected==2){	// Regulator menu selected
			if (indexselected == 1){	//Setpoint marked
				adjustsetpoint = adjustsetpoint-adjustsetpointstep;
				if(adjustsetpoint<=100){
					adjustsetpoint=100;
				}
			}
			else if (indexselected==2){	//KP marked
				
				if (adjustkp!=0){
					adjustkp=adjustkp-adjustkpstep;
				}
			}
			else if (indexselected==3){	//KI marked
				if (adjustki!=0){
					adjustki=adjustki-adjustkistep;
				}
			}
			else if (indexselected==4){	//KD marked
				
				if (adjustkd!=0){
					adjustkd=adjustkd-adjustkdstep;
				}
			}
		}
		else if(menuselected=4){	//Generator menu selected
			if (indexselected == 2){	//Load marked
				if (adjustgenerator !=0){
					adjustgenerator=adjustgenerator-adjustgeneratorstep;
					if (adjustgenerator <=0){
						adjustgenerator=0;
					}
				}
			}			
		}
		
	}
	refresh_screen=1;
}

void ButtonEnterPressed(void){
	if (menuselected == 0){		//main menu
		if (indexselected == 0){	//info screen selected
			menuselected = 1;
			max_menu_index=0;
			indexselected=0;
		}
		else if (indexselected == 1){	//regulatormode
			menuselected = 2;
			indexselected=0;
			max_menu_index=5;
		}
		else if (indexselected == 2){	//io menu
			menuselected = 3;
			max_menu_index=4;
			indexselected=0;
		}
		else if (indexselected == 3){	//generator menu
			menuselected = 4;
			max_menu_index=3;
			indexselected=0;
		}
	}
	
	
	else if (menuselected==1){	//info screen
		menuselected=0;
		max_menu_index=3;
		indexselected=0;
	}
	else if(menuselected ==2){	//regulatormode		
		regulatormode_changed=1;
		if (indexselected==0){	//toggle activate
			if (regulatormodestatus==0){	
				regulatormodestatus=1;
				digital_in_status=0;
				analog_in_status=0;
			}
			else if (regulatormodestatus==1){
				regulatormodestatus=0;
			}
		}
		else if (indexselected==1 && adjustmode==0){	//enter adjustmode for setpoint
			adjustmode=1;
		}
		else if (indexselected==1 && adjustmode==1){	//exit adjustmode
			adjustmode=0;
		}
		else if (indexselected==2 && adjustmode==0){	//enter adjustmode for KP
			adjustmode=1;
		}
		else if (indexselected==2 && adjustmode==1){	//exit adjustmode
			adjustmode=0;
		}
		else if (indexselected==3 && adjustmode==0){	//enter adjustmode for KI
			adjustmode=1;
		}
		else if (indexselected==3 && adjustmode==1){	//exit adjustmode
			adjustmode=0;
		}
		else if (indexselected==4 && adjustmode==0){	//enter adjustmode for KD
			adjustmode=1;
		}
		else if (indexselected==4 && adjustmode==1){	//exit adjustmode
			adjustmode=0;
		}
		else if (indexselected==5){	//return to main menu
			menuselected=0;			
			max_menu_index=3;
			indexselected=0;
			show_0_3=1;
			show_1_4=0;
			show_2_5=0;
		}
	}
	else if(menuselected ==3){	//io menu
		io_changed=1;
		if (indexselected==0){	//digital in
			if (digital_in_status ==0 ){	
				digital_in_status=1;
				iomenuindex0status = statusOn;
			}
			else if (digital_in_status==1){
				digital_in_status=0;
				iomenuindex0status = statusOff;
			}
		}
		else if (indexselected==1){	//digital out
			if (digital_out_status == 0){
				digital_out_status=1;
				iomenuindex1status = statusOn;
			}
			else if (digital_out_status == 1){
				digital_out_status=0;
				iomenuindex1status = statusOff;
			}
		}
		
		if (indexselected==2){	//analog in
			if (analog_in_status ==0 ){
				analog_in_status=1;
				iomenuindex2status = statusOn;
			}
			else if (analog_in_status==1){
				analog_in_status=0;
				iomenuindex2status = statusOff;
			}
		}
		else if (indexselected==3){	//analog out
			if (analog_out_status == 0){
				analog_out_status=1;
				iomenuindex3status = statusOn;
			}
			else if (analog_out_status == 1){
				analog_out_status=0;
				iomenuindex3status = statusOff;
			}
		}
		else if (indexselected==4){	//return
			menuselected=0;
			max_menu_index=3;
			indexselected=0;
			show_0_3=1;
			show_1_4=0;
			show_2_5=0;
		}
	}
	else if(menuselected ==4){//generator menu
		generator_changed=1;
		if(indexselected==0){//enable
			if (generatorenable == 0){
				generatorenable=1;
				generatorindex0status = statusOn;
			}
			else if (generatorenable == 1){
				generatorenable=0;
				adjustgenerator=0;
				generatorindex0status = statusOff;
			}
		}
		else if(indexselected==1){//toggle control by USART or HMI
			if (generatorADCenable == 0){
				generatorADCenable=1;
				generatorindex1status = statusADC;
			}
			else if (generatorADCenable == 1){
				generatorADCenable=0;
				generatorindex1status = statusUART;
			}
		}
		else if (indexselected==2){
			if (generatorADCenable==0){
				if ( adjustmode==0){	//enter adjustmode for setpoint
					adjustmode=1;
				}
				else if (adjustmode==1){	//exit adjustmode
					adjustmode=0;
				}
			}
			
		}
		else if (indexselected==3){	//return
			menuselected=0;
			max_menu_index=3;
			indexselected=0;
			show_0_3=1;
			show_1_4=0;
			show_2_5=0;
		}
	}	
	refresh_screen=1;
}

void Menu_handler(void){
	
	if (refresh_screen==1){	
	refresh_screen=0;
	HD44780_PCF8574_DisplayClear(addr);
	if (menuselected==0){	//draw main menu
		HD44780_PCF8574_PositionXY(addr, 1,0);
		HD44780_PCF8574_DrawString(addr, mainmenuindex0);
		HD44780_PCF8574_PositionXY(addr, 1,1);
		HD44780_PCF8574_DrawString(addr, mainmenuindex1);
		HD44780_PCF8574_PositionXY(addr, 1,2);
		HD44780_PCF8574_DrawString(addr, mainmenuindex2);
		HD44780_PCF8574_PositionXY(addr, 1,3);
		HD44780_PCF8574_DrawString(addr, mainmenuindex3);
		
		
		if (indexselected==0){
			HD44780_PCF8574_PositionXY(addr, 0,0);
			HD44780_PCF8574_DrawString(addr, ">");
			HD44780_PCF8574_PositionXY(addr, 0,0);
			if (adjustmode==1){
				HD44780_PCF8574_PositionXY(addr, 13,0);
				HD44780_PCF8574_DrawString(addr, ">");
				HD44780_PCF8574_PositionXY(addr, 13,0);
			}
		}
		else if (indexselected==1){
			HD44780_PCF8574_PositionXY(addr, 0,1);
			HD44780_PCF8574_DrawString(addr, ">");
			HD44780_PCF8574_PositionXY(addr, 0,1);
			if (adjustmode==1){
				HD44780_PCF8574_PositionXY(addr, 13,1);
				HD44780_PCF8574_DrawString(addr, ">");
				HD44780_PCF8574_PositionXY(addr, 13,1);
			}
		}
		else if (indexselected==2){
			HD44780_PCF8574_PositionXY(addr, 0,2);
			HD44780_PCF8574_DrawString(addr, ">");
			HD44780_PCF8574_PositionXY(addr, 0,2);
			if (adjustmode==1){
				HD44780_PCF8574_PositionXY(addr, 13,2);
				HD44780_PCF8574_DrawString(addr, ">");
				HD44780_PCF8574_PositionXY(addr, 13,2);
			}
		}
		else if (indexselected==3){
			HD44780_PCF8574_PositionXY(addr, 0,3);
			HD44780_PCF8574_DrawString(addr, ">");
			HD44780_PCF8574_PositionXY(addr, 0,3);
		}
	}
	
	else if (menuselected==1){	//Draw info screen
		HD44780_PCF8574_PositionXY(addr, 0,0);
		HD44780_PCF8574_DrawString(addr, infoindex0);
		HD44780_PCF8574_PositionXY(addr, 0,1);
		HD44780_PCF8574_DrawString(addr, infoindex1);
		HD44780_PCF8574_PositionXY(addr, 0,2);
		HD44780_PCF8574_DrawString(addr, infoindex2);
		HD44780_PCF8574_PositionXY(addr, 0,3);
		HD44780_PCF8574_DrawString(addr, infoindex3);
		HD44780_PCF8574_PositionXY(addr, 14,0);
		itoa(rpm_value,rpm_string,10);
		HD44780_PCF8574_DrawString(addr, rpm_string);
		HD44780_PCF8574_PositionXY(addr, 14,1);
		itoa(backemf_value,backemf_string,10);
		HD44780_PCF8574_DrawString(addr, backemf_string);
		HD44780_PCF8574_PositionXY(addr, 14,2);
		itoa(current_value,current_string,10);
		HD44780_PCF8574_DrawString(addr, current_string);
		HD44780_PCF8574_PositionXY(addr, 14,3);
		itoa(dutycycle_value,dutycycle_string,10);
		HD44780_PCF8574_DrawString(addr, dutycycle_string);
		HD44780_PCF8574_PositionXY(addr, 19,3);
		
	}
	else if (menuselected==2){	//Draw regulator screen
		
		itoa(adjustsetpoint,regulatorstatusindex1,10);
		itoa(adjustkp,regulatorstatusindex2,10);
		itoa(adjustki,regulatorstatusindex3,10);
		itoa(adjustkd,regulatorstatusindex4,10);
		
		
		
		if(show_0_3==1){
			HD44780_PCF8574_PositionXY(addr, 1,0);
			HD44780_PCF8574_DrawString(addr, regulatorindex0);
			HD44780_PCF8574_PositionXY(addr, 1,1);
			HD44780_PCF8574_DrawString(addr, regulatorindex1);
			HD44780_PCF8574_PositionXY(addr, 1,2);
			HD44780_PCF8574_DrawString(addr, regulatorindex2);
			HD44780_PCF8574_PositionXY(addr, 1,3);
			HD44780_PCF8574_DrawString(addr, regulatorindex3);
			HD44780_PCF8574_PositionXY(addr, 14,0);
			if (regulatormodestatus==1){
			HD44780_PCF8574_DrawString(addr, statusOn);
			}
			else if (regulatormodestatus==0){
				HD44780_PCF8574_DrawString(addr, statusOff);
			}
			HD44780_PCF8574_PositionXY(addr, 14,1);
			HD44780_PCF8574_DrawString(addr, regulatorstatusindex1);
			HD44780_PCF8574_PositionXY(addr, 14,2);
			HD44780_PCF8574_DrawString(addr, regulatorstatusindex2);
			HD44780_PCF8574_PositionXY(addr, 14,3);
			HD44780_PCF8574_DrawString(addr, regulatorstatusindex3);
			
			
			if (indexselected==0){
				HD44780_PCF8574_PositionXY(addr, 0,0);
				HD44780_PCF8574_DrawString(addr, ">");
				HD44780_PCF8574_PositionXY(addr, 0,0);	
				if (adjustmode==1){
					HD44780_PCF8574_PositionXY(addr, 13,0);
					HD44780_PCF8574_DrawString(addr, ">");
					HD44780_PCF8574_PositionXY(addr, 13,0);
				}			
			}
			else if (indexselected==1){
				HD44780_PCF8574_PositionXY(addr, 0,1);
				HD44780_PCF8574_DrawString(addr, ">");
				HD44780_PCF8574_PositionXY(addr, 0,1);
				if (adjustmode==1){
					HD44780_PCF8574_PositionXY(addr, 13,1);
					HD44780_PCF8574_DrawString(addr, ">");
					HD44780_PCF8574_PositionXY(addr, 13,1);
				}
			}
			else if (indexselected==2){
				HD44780_PCF8574_PositionXY(addr, 0,2);
				HD44780_PCF8574_DrawString(addr, ">");
				HD44780_PCF8574_PositionXY(addr, 0,2);
				if (adjustmode==1){
					HD44780_PCF8574_PositionXY(addr, 13,2);
					HD44780_PCF8574_DrawString(addr, ">");
					HD44780_PCF8574_PositionXY(addr, 13,2);
				}
			}
			else if (indexselected==3){
				HD44780_PCF8574_PositionXY(addr, 0,3);
				HD44780_PCF8574_DrawString(addr, ">");
				HD44780_PCF8574_PositionXY(addr, 0,3);
				if (adjustmode==1){
					HD44780_PCF8574_PositionXY(addr, 13,3);
					HD44780_PCF8574_DrawString(addr, ">");
					HD44780_PCF8574_PositionXY(addr, 13,3);
				}
			}
		}
		else if (show_1_4==1){
			HD44780_PCF8574_PositionXY(addr, 1,0);
			HD44780_PCF8574_DrawString(addr, regulatorindex1);
			HD44780_PCF8574_PositionXY(addr, 1,1);
			HD44780_PCF8574_DrawString(addr, regulatorindex2);
			HD44780_PCF8574_PositionXY(addr, 1,2);
			HD44780_PCF8574_DrawString(addr, regulatorindex3);
			HD44780_PCF8574_PositionXY(addr, 1,3);
			HD44780_PCF8574_DrawString(addr, regulatorindex4);
			HD44780_PCF8574_PositionXY(addr, 14,0);
			HD44780_PCF8574_DrawString(addr, regulatorstatusindex1);
			HD44780_PCF8574_PositionXY(addr, 14,1);
			HD44780_PCF8574_DrawString(addr, regulatorstatusindex2);
			HD44780_PCF8574_PositionXY(addr, 14,2);
			HD44780_PCF8574_DrawString(addr, regulatorstatusindex3);
			HD44780_PCF8574_PositionXY(addr, 14,3);
			HD44780_PCF8574_DrawString(addr, regulatorstatusindex4);
				if (indexselected==1){
					HD44780_PCF8574_PositionXY(addr, 0,0);
					HD44780_PCF8574_DrawString(addr, ">");
					HD44780_PCF8574_PositionXY(addr, 0,0);
					if (adjustmode==1){
						HD44780_PCF8574_PositionXY(addr, 13,0);
						HD44780_PCF8574_DrawString(addr, ">");
						HD44780_PCF8574_PositionXY(addr, 13,0);
					}
				}
				else if (indexselected==2){
					HD44780_PCF8574_PositionXY(addr, 0,1);
					HD44780_PCF8574_DrawString(addr, ">");
					HD44780_PCF8574_PositionXY(addr, 0,1);
					if (adjustmode==1){
						HD44780_PCF8574_PositionXY(addr, 13,1);
						HD44780_PCF8574_DrawString(addr, ">");
						HD44780_PCF8574_PositionXY(addr, 13,1);
					}
				}
				else if (indexselected==3){
					HD44780_PCF8574_PositionXY(addr, 0,2);
					HD44780_PCF8574_DrawString(addr, ">");
					HD44780_PCF8574_PositionXY(addr, 0,2);
					if (adjustmode==1){
						HD44780_PCF8574_PositionXY(addr, 13,2);
						HD44780_PCF8574_DrawString(addr, ">");
						HD44780_PCF8574_PositionXY(addr, 13,2);
					}
				}
				else if (indexselected==4){
					HD44780_PCF8574_PositionXY(addr, 0,3);
					HD44780_PCF8574_DrawString(addr, ">");
					HD44780_PCF8574_PositionXY(addr, 0,3);
					if (adjustmode==1){
						HD44780_PCF8574_PositionXY(addr, 13,3);
						HD44780_PCF8574_DrawString(addr, ">");
						HD44780_PCF8574_PositionXY(addr, 13,3);
					}
				}				
		}
		else if (show_2_5==1){
			HD44780_PCF8574_PositionXY(addr, 1,0);
			HD44780_PCF8574_DrawString(addr, regulatorindex2);
			HD44780_PCF8574_PositionXY(addr, 1,1);
			HD44780_PCF8574_DrawString(addr, regulatorindex3);
			HD44780_PCF8574_PositionXY(addr, 1,2);
			HD44780_PCF8574_DrawString(addr, regulatorindex4);
			HD44780_PCF8574_PositionXY(addr, 1,3);
			HD44780_PCF8574_DrawString(addr, regulatorindex5);
			HD44780_PCF8574_PositionXY(addr, 14,0);
			HD44780_PCF8574_DrawString(addr, regulatorstatusindex2);
			HD44780_PCF8574_PositionXY(addr, 14,1);
			HD44780_PCF8574_DrawString(addr, regulatorstatusindex3);
			HD44780_PCF8574_PositionXY(addr, 14,2);
			HD44780_PCF8574_DrawString(addr, regulatorstatusindex4);
			
			
				if (indexselected==2){
					HD44780_PCF8574_PositionXY(addr, 0,0);
					HD44780_PCF8574_DrawString(addr, ">");
					HD44780_PCF8574_PositionXY(addr, 0,0);
					if (adjustmode==1){
						HD44780_PCF8574_PositionXY(addr, 13,0);
						HD44780_PCF8574_DrawString(addr, ">");
						HD44780_PCF8574_PositionXY(addr, 13,0);
					}
				}
				else if (indexselected==3){
					HD44780_PCF8574_PositionXY(addr, 0,1);
					HD44780_PCF8574_DrawString(addr, ">");
					HD44780_PCF8574_PositionXY(addr, 0,1);
					if (adjustmode==1){
						HD44780_PCF8574_PositionXY(addr, 13,1);
						HD44780_PCF8574_DrawString(addr, ">");
						HD44780_PCF8574_PositionXY(addr, 13,1);
					}
				}
				else if (indexselected==4){
					HD44780_PCF8574_PositionXY(addr, 0,2);
					HD44780_PCF8574_DrawString(addr, ">");
					HD44780_PCF8574_PositionXY(addr, 0,2);
					if (adjustmode==1){
						HD44780_PCF8574_PositionXY(addr, 13,2);
						HD44780_PCF8574_DrawString(addr, ">");
						HD44780_PCF8574_PositionXY(addr, 13,2);
					}
				}
				else if (indexselected==5){
					HD44780_PCF8574_PositionXY(addr, 0,3);
					HD44780_PCF8574_DrawString(addr, ">");
					HD44780_PCF8574_PositionXY(addr, 0,3);
				}
		}
		
		
	}
		else if (menuselected==3){
			if (show_0_3==1){	
				HD44780_PCF8574_PositionXY(addr, 1,0);
				HD44780_PCF8574_DrawString(addr, iomenuindex0);
				HD44780_PCF8574_PositionXY(addr, 1,1);
				HD44780_PCF8574_DrawString(addr, iomenuindex1);
				HD44780_PCF8574_PositionXY(addr, 1,2);
				HD44780_PCF8574_DrawString(addr, iomenuindex2);
				HD44780_PCF8574_PositionXY(addr, 1,3);
				HD44780_PCF8574_DrawString(addr, iomenuindex3);
				HD44780_PCF8574_PositionXY(addr, 14,0);
				HD44780_PCF8574_DrawString(addr, iomenuindex0status);
				HD44780_PCF8574_PositionXY(addr, 14,1);
				HD44780_PCF8574_DrawString(addr, iomenuindex1status);
				HD44780_PCF8574_PositionXY(addr, 14,2);
				HD44780_PCF8574_DrawString(addr, iomenuindex2status);
				HD44780_PCF8574_PositionXY(addr, 14,3);
				HD44780_PCF8574_DrawString(addr, iomenuindex3status);
			
				if (indexselected==0){
					HD44780_PCF8574_PositionXY(addr, 0,0);
					HD44780_PCF8574_DrawString(addr, ">");
					HD44780_PCF8574_PositionXY(addr, 0,0);
				}
				else if (indexselected==1){
					HD44780_PCF8574_PositionXY(addr, 0,1);
					HD44780_PCF8574_DrawString(addr, ">");
					HD44780_PCF8574_PositionXY(addr, 0,1);
				}
				else if (indexselected==2){
					HD44780_PCF8574_PositionXY(addr, 0,2);
					HD44780_PCF8574_DrawString(addr, ">");
					HD44780_PCF8574_PositionXY(addr, 0,2);
				}
				else if (indexselected==3){
					HD44780_PCF8574_PositionXY(addr, 0,3);
					HD44780_PCF8574_DrawString(addr, ">");
					HD44780_PCF8574_PositionXY(addr, 0,3);
				}
			}
			else if(show_1_4==1){
				HD44780_PCF8574_PositionXY(addr, 1,0);
				HD44780_PCF8574_DrawString(addr, iomenuindex1);
				HD44780_PCF8574_PositionXY(addr, 1,1);
				HD44780_PCF8574_DrawString(addr, iomenuindex2);
				HD44780_PCF8574_PositionXY(addr, 1,2);
				HD44780_PCF8574_DrawString(addr, iomenuindex3);
				HD44780_PCF8574_PositionXY(addr, 1,3);
				HD44780_PCF8574_DrawString(addr, iomenuindex4);
				HD44780_PCF8574_PositionXY(addr, 14,0);
				HD44780_PCF8574_DrawString(addr, iomenuindex1status);
				HD44780_PCF8574_PositionXY(addr, 14,1);
				HD44780_PCF8574_DrawString(addr, iomenuindex2status);
				HD44780_PCF8574_PositionXY(addr, 14,2);
				HD44780_PCF8574_DrawString(addr, iomenuindex3status);			
			
				if (indexselected==1){
					HD44780_PCF8574_PositionXY(addr, 0,0);
					HD44780_PCF8574_DrawString(addr, ">");
					HD44780_PCF8574_PositionXY(addr, 0,0);
				}
				else if (indexselected==2){
					HD44780_PCF8574_PositionXY(addr, 0,1);
					HD44780_PCF8574_DrawString(addr, ">");
					HD44780_PCF8574_PositionXY(addr, 0,1);
				}
				else if (indexselected==3){
					HD44780_PCF8574_PositionXY(addr, 0,2);
					HD44780_PCF8574_DrawString(addr, ">");
					HD44780_PCF8574_PositionXY(addr, 0,2);
				}
				else if (indexselected==4){
					HD44780_PCF8574_PositionXY(addr, 0,3);
					HD44780_PCF8574_DrawString(addr, ">");
					HD44780_PCF8574_PositionXY(addr, 0,3);
				}
			}
		
		}
		else if (menuselected==4){//generatormenu
			itoa(adjustgenerator,generatorindex2status,10);
			HD44780_PCF8574_PositionXY(addr, 1,0);
			HD44780_PCF8574_DrawString(addr, generatorindex0);
			HD44780_PCF8574_PositionXY(addr, 1,1);
			HD44780_PCF8574_DrawString(addr, generatorindex1);
			HD44780_PCF8574_PositionXY(addr, 1,2);
			HD44780_PCF8574_DrawString(addr, generatorindex2);
			HD44780_PCF8574_PositionXY(addr, 1,3);
			HD44780_PCF8574_DrawString(addr, generatorindex3);/*
			HD44780_PCF8574_PositionXY(addr, 14,0);
			HD44780_PCF8574_DrawString(addr, generatorindex0status);
			HD44780_PCF8574_PositionXY(addr, 14,1);
			HD44780_PCF8574_DrawString(addr, generatorindex1status);
			HD44780_PCF8574_PositionXY(addr, 14,2);
			HD44780_PCF8574_DrawString(addr, generatorindex2status);
			HD44780_PCF8574_DrawString(addr, "%");
		
			if (indexselected==0){
				HD44780_PCF8574_PositionXY(addr, 0,0);
				HD44780_PCF8574_DrawString(addr, ">");
				HD44780_PCF8574_PositionXY(addr, 0,0);
			}
			else if (indexselected==1){
				HD44780_PCF8574_PositionXY(addr, 0,1);
				HD44780_PCF8574_DrawString(addr, ">");
				HD44780_PCF8574_PositionXY(addr, 0,1);
			}
			else if (indexselected==2){
				HD44780_PCF8574_PositionXY(addr, 0,2);
				HD44780_PCF8574_DrawString(addr, ">");
				HD44780_PCF8574_PositionXY(addr, 0,2);
			}
			else if (indexselected==3){
				HD44780_PCF8574_PositionXY(addr, 0,3);
				HD44780_PCF8574_DrawString(addr, ">");
				HD44780_PCF8574_PositionXY(addr, 0,3);
			}
			if (adjustmode==1){
				HD44780_PCF8574_PositionXY(addr, 13,2);
				HD44780_PCF8574_DrawString(addr, ">");
				HD44780_PCF8574_PositionXY(addr, 13,2);
			}*/
		
		}
	}
	Menu_update_infoscreen();
	Menu_update_generatorscreen();
}




void  Menu_update_infoscreen(void){
	if (menuselected==1){		
		HD44780_PCF8574_PositionXY(addr, 14,0);
		itoa(rpm_value,rpm_string,10);				
		HD44780_PCF8574_DrawString(addr, "      ");
		HD44780_PCF8574_PositionXY(addr, 14,0);
		if (strlen(rpm_string)==2){
			HD44780_PCF8574_DrawString(addr, "  ");
		}
		else if (strlen(rpm_string)==3){
			HD44780_PCF8574_DrawString(addr, " ");
		}
		HD44780_PCF8574_DrawString(addr, rpm_string);
		HD44780_PCF8574_PositionXY(addr, 14,1);
		itoa(backemf_value,backemf_string,10);
		char tmp[6];
		if (strlen(backemf_string)==1){
			tmp[0]=' ';
			tmp[1]='0';
			tmp[2]='.';
			tmp[3]='0';
			tmp[4]=backemf_string[0];
			tmp[5]='\0';
		}
		else if (strlen(backemf_string)==2){
			tmp[0]=' ';
			tmp[1]='0';
			tmp[2]='.';
			tmp[3]=backemf_string[0];
			tmp[4]=backemf_string[1];
			tmp[5]='\0';
		}
		else if (strlen(backemf_string)==3){
			tmp[0]=' ';
			tmp[1]=backemf_string[0];
			tmp[2]='.';
			tmp[3]=backemf_string[1];
			tmp[4]=backemf_string[2];
			tmp[5]='\0';
		}
		else if (strlen(backemf_string)==4){
			tmp[0]=backemf_string[0];
			tmp[1]=backemf_string[1];
			tmp[2]='.';
			tmp[3]=backemf_string[2];
			tmp[4]=backemf_string[3];
			tmp[5]='\0';
		}
		HD44780_PCF8574_DrawString(addr, "      ");
		HD44780_PCF8574_PositionXY(addr, 14,1);
		HD44780_PCF8574_DrawString(addr, tmp);
		HD44780_PCF8574_PositionXY(addr, 14,2);
		itoa(current_value,current_string,10);
		if (strlen(current_string)==2){
			HD44780_PCF8574_DrawString(addr, "  ");
		}
		else if (strlen(current_string)==3){
			HD44780_PCF8574_DrawString(addr, " ");
		}
		HD44780_PCF8574_DrawString(addr, "      ");
		HD44780_PCF8574_PositionXY(addr, 14,2);
		HD44780_PCF8574_DrawString(addr, current_string);
		HD44780_PCF8574_PositionXY(addr, 14,3);
		itoa(dutycycle_value,dutycycle_string,10);
		if (strlen(dutycycle_string)==2){
			HD44780_PCF8574_DrawString(addr, "  ");
		}
		else if (strlen(dutycycle_string)==3){
			HD44780_PCF8574_DrawString(addr, " ");
		}
		HD44780_PCF8574_DrawString(addr, "      ");
		HD44780_PCF8574_PositionXY(addr, 14,3);
		HD44780_PCF8574_DrawString(addr, dutycycle_string);
	}
}

void Menu_update_generatorscreen(void){
	if (menuselected==4){
		if(generatorenable==1){
			generatorindex0status=statusOn;
		}
		else if (generatorenable==0){
			generatorindex0status=statusOff;
		}		
		
		itoa(adjustgenerator,generatorindex2status,10);	
		HD44780_PCF8574_PositionXY(addr, 14,0);
		HD44780_PCF8574_DrawString(addr, "      ");
		HD44780_PCF8574_PositionXY(addr, 14,0);
		HD44780_PCF8574_DrawString(addr, generatorindex0status);	
		HD44780_PCF8574_PositionXY(addr, 14,1);	
		HD44780_PCF8574_DrawString(addr, "      ");
		HD44780_PCF8574_PositionXY(addr, 14,1);	
		if(generatorADCenable==1){
			HD44780_PCF8574_DrawString(addr, statusADC);
		}
		else if(generatorADCenable==0){
			HD44780_PCF8574_DrawString(addr, statusUART);
		}
		HD44780_PCF8574_PositionXY(addr, 14,2);	
		HD44780_PCF8574_DrawString(addr, "      ");
		HD44780_PCF8574_PositionXY(addr, 14,2);
		HD44780_PCF8574_DrawString(addr, generatorindex2status);	
		HD44780_PCF8574_DrawString(addr, "%");
	
		if (indexselected==0){
			HD44780_PCF8574_PositionXY(addr, 0,0);
			HD44780_PCF8574_DrawString(addr, ">");
			HD44780_PCF8574_PositionXY(addr, 0,0);
		}
		else if (indexselected==1){
			HD44780_PCF8574_PositionXY(addr, 0,1);
			HD44780_PCF8574_DrawString(addr, ">");
			HD44780_PCF8574_PositionXY(addr, 0,1);
		}
		else if (indexselected==2){
			HD44780_PCF8574_PositionXY(addr, 0,2);
			HD44780_PCF8574_DrawString(addr, ">");
			HD44780_PCF8574_PositionXY(addr, 0,2);
		}
		else if (indexselected==3){
			HD44780_PCF8574_PositionXY(addr, 0,3);
			HD44780_PCF8574_DrawString(addr, ">");
			HD44780_PCF8574_PositionXY(addr, 0,3);
		}
		if (adjustmode==1){
			HD44780_PCF8574_PositionXY(addr, 13,2);
			HD44780_PCF8574_DrawString(addr, ">");
			HD44780_PCF8574_PositionXY(addr, 13,2);
		}
	}
	
}


void Menu_placemarker(void){
	if(menuselected != 1){
		if(show_0_3){
			if (indexselected==0){
				HD44780_PCF8574_PositionXY(addr, 0,0);
				HD44780_PCF8574_DrawString(addr, ">");
				HD44780_PCF8574_PositionXY(addr, 0,0);
			}
			else if (indexselected==1){
				HD44780_PCF8574_PositionXY(addr, 0,1);
				HD44780_PCF8574_DrawString(addr, ">");
				HD44780_PCF8574_PositionXY(addr, 0,1);
			}
			else if (indexselected==2){
				HD44780_PCF8574_PositionXY(addr, 0,2);
				HD44780_PCF8574_DrawString(addr, ">");
				HD44780_PCF8574_PositionXY(addr, 0,2);
			}
			else if (indexselected==3){
				HD44780_PCF8574_PositionXY(addr, 0,3);
				HD44780_PCF8574_DrawString(addr, ">");
				HD44780_PCF8574_PositionXY(addr, 0,3);
			}	
		}
		else if(show_1_4){
			if (indexselected==1){
				HD44780_PCF8574_PositionXY(addr, 0,0);
				HD44780_PCF8574_DrawString(addr, ">");
				HD44780_PCF8574_PositionXY(addr, 0,0);
			}
			else if (indexselected==2){
				HD44780_PCF8574_PositionXY(addr, 0,1);
				HD44780_PCF8574_DrawString(addr, ">");
				HD44780_PCF8574_PositionXY(addr, 0,1);
			}
			else if (indexselected==3){
				HD44780_PCF8574_PositionXY(addr, 0,2);
				HD44780_PCF8574_DrawString(addr, ">");
				HD44780_PCF8574_PositionXY(addr, 0,2);
			}
			else if (indexselected==4){
				HD44780_PCF8574_PositionXY(addr, 0,3);
				HD44780_PCF8574_DrawString(addr, ">");
				HD44780_PCF8574_PositionXY(addr, 0,3);
			}	
			
		}
		else if(show_2_5){
			if (indexselected==2){
				HD44780_PCF8574_PositionXY(addr, 0,0);
				HD44780_PCF8574_DrawString(addr, ">");
				HD44780_PCF8574_PositionXY(addr, 0,0);
			}
			else if (indexselected==3){
				HD44780_PCF8574_PositionXY(addr, 0,1);
				HD44780_PCF8574_DrawString(addr, ">");
				HD44780_PCF8574_PositionXY(addr, 0,1);
			}
			else if (indexselected==4){
				HD44780_PCF8574_PositionXY(addr, 0,2);
				HD44780_PCF8574_DrawString(addr, ">");
				HD44780_PCF8574_PositionXY(addr, 0,2);
			}
			else if (indexselected==5){
				HD44780_PCF8574_PositionXY(addr, 0,3);
				HD44780_PCF8574_DrawString(addr, ">");
				HD44780_PCF8574_PositionXY(addr, 0,3);
			}
			
		}
	}
}