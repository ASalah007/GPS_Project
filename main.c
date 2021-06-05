#include "tm4c123gh6pm.h"
#include "stdint.h"
#include <stdlib.h>
#include <math.h>

//Signatures
void SSD_init(void);
void LED_init(void);
void dist_to_display(uint16_t dist);


//LCD FUNCTIONS
void lcd_command(uint8_t comm);
void lcd_data(uint8_t data);
void delay(void);
void lcd_display(uint16_t data);
void lcd_init(void);

/////////////Global Variables/////////////
double distance=0;
double oldX=0, oldY=0;
double newX=0, newY=0;
uint8_t latitude[20];
uint8_t longitude[20];
/////////////Global Variables/////////////

void CalculateDistance(double newX,double newY ){
	if(oldX == 0.0 || oldY == 0.0){
				oldX=newX;
        oldY=newY;
	}
	else{
    double xsquare= (oldX-newX)*(oldX-newX);
    double ysquare= (oldY-newY)*(oldY-newY);
    double summing =  xsquare + ysquare;

    distance+= sqrt(summing)* 100000;
        oldX=newX;
        oldY=newY;
		}
}


void UART_init(void){
SYSCTL_RCGCUART_R |=0x0020;  // enable clock for UART5
SYSCTL_RCGCGPIO_R |=0x0010;   // activate port E
 while ((SYSCTL_RCGCUART_R&0x0020)==0){};
 while ((SYSCTL_PRGPIO_R&0x0010)==0){};
	 
	 //UART5 initialization
UART5_CTL_R =0;      //disable UART
UART5_IBRD_R=104;
// IBRD=int(1000000/9600)=int(104.16667)	 
UART5_FBRD_R=11;
//FBRD=round(0.16667*64)=11;
 UART5_LCRH_R |=	0x0070 ; //8-bit length ,enable FIFO
 UART5_CTL_R |=0x0201;  //enable RXE,URT
	 
//portE initailization
	 GPIO_PORTE_DIR_R &= ~0x10;
	 GPIO_PORTE_CR_R |= 0x10;        //unlock commit register
	 GPIO_PORTE_AFSEL_R |= 0x10;    //use PE4 alternating function
	 GPIO_PORTE_PCTL_R |= 0x10000;  // configure PE4 for UART
	 GPIO_PORTE_DEN_R |= 0x10;      //set PE4 as digital
	 GPIO_PORTE_AMSEL_R &= ~0x10;   //turn off analoge function
	  
}


uint8_t UART5_Receiver(void){
	  while((UART5_FR_R&0x10)!=0){} //wait when data are available (RXFE is 0)
    return ((uint8_t)(UART5_DR_R&0xFF)); 
}


void GPS_Coordinates(){
	uint8_t c;
	uint8_t i = 0;
	double temp;

	c = UART5_Receiver();

	if(c == '$')
	{
		c = UART5_Receiver();
		if(c == 'G')
		{
			c = UART5_Receiver();
			if(c == 'P')
			{
				c = UART5_Receiver();
				if(c == 'G')
				{
					c = UART5_Receiver();
					if(c == 'L')
					{
						c = UART5_Receiver();
						if(c == 'L')
						{
							c = UART5_Receiver(); /* For comma */
							c = UART5_Receiver(); /* For 3 */
							c = UART5_Receiver(); /* For 0 */

							c = UART5_Receiver();
							while(c != ',')
							{
								latitude[i] = c;
								i++;
								c = UART5_Receiver();
							}
							latitude[i] = '\0';

							temp = atof(latitude);
							temp /= 60;
							temp += 30;
							sprintf(latitude, "%.8f", temp);

							c = UART5_Receiver(); /* for N */
							c = UART5_Receiver(); /* for , */
							c = UART5_Receiver(); /* for 3 */
							c = UART5_Receiver(); /* for 1 */

							/* longitude */
							i=0;
							c = UART5_Receiver();
							while(c != ',')
							{
								longitude[i] = c;
								i++;
								c = UART5_Receiver();
							}
							longitude[i] = '\0';

							temp = atof(longitude);
							temp /= 60;
							temp += 31;
							sprintf(longitude, "%.8f", temp);
						}
					}
				}
			}
		}

	}
}







void SystemInit(void){
	LED_init();
	lcd_init();
}
int main (void)
{	
	while (1) 
		{  
			UART_init();
			GPS_Coordinates();
			
			GPIO_PORTF_DATA_R |= 0x02; // turn on red LED
		}
}

//initialization functions

// 1st SSD will take pins PD0 - PD6  			HUNDREDS
// 2nd SSD will take pins PB0 - PB6  			TENS
// 3rd SSD will take pins PA2 - pA7,PD7  	ONES
void SSD_init(void){
	SYSCTL_RCGCGPIO_R |= 0x1B;   // Enable port A,B,D,E;
	while((SYSCTL_PRGPIO_R & 0x1B) == 0);

	GPIO_PORTA_LOCK_R = 0x4C4F434B;
	GPIO_PORTA_CR_R |= 0xFC;                    //UNCLOCKING PORT A PINS: 2-7

	GPIO_PORTB_LOCK_R = 0x4C4F434B;
	GPIO_PORTB_CR_R |= 0x7F;                    //UNCLOCKING PORT B PINS: 0-6

	GPIO_PORTD_LOCK_R = 0x4C4F434B;
	GPIO_PORTD_CR_R |= 0xFF;                    //UNCLOCKING PORT D PINS: 0-7

	GPIO_PORTA_DIR_R |= 0xFC;
	GPIO_PORTA_DEN_R |= 0xFC;
	GPIO_PORTA_AMSEL_R &= ~0xFC;            //SET PINS 2-7 AS DIGITAL OUTPUT

	GPIO_PORTB_DIR_R |= 0x7F;
	GPIO_PORTB_DEN_R |= 0x7F;
	GPIO_PORTB_AMSEL_R &= ~0x7F;            //SET PINS 2-7 AS DIGITAL OUTPUT

	GPIO_PORTD_DIR_R |= 0xFF;
	GPIO_PORTD_DEN_R |= 0xFF;
	GPIO_PORTD_AMSEL_R &= ~0xFF;            //SET PINS 0-7 AS DIGITAL OUTPUT


	GPIO_PORTA_AFSEL_R &= ~0xFC;
	GPIO_PORTA_PCTL_R  &= ~0xFFFFFF00;

	GPIO_PORTB_AFSEL_R &= ~0x7F;
	GPIO_PORTB_PCTL_R   &= ~0x0FFFFFFF;

	GPIO_PORTD_AFSEL_R &= ~0xFF;
	GPIO_PORTD_PCTL_R  &= ~0xFFFFFFFF;

}

void LED_init(void){SYSCTL_RCGCGPIO_R |= 0x20;
    while((SYSCTL_PRGPIO_R & 0x20) == 0) ;

    GPIO_PORTF_LOCK_R = 0x4C4F434B;
    GPIO_PORTF_CR_R |= 0x0E;                    //UNCLOCKING PORT F

    GPIO_PORTF_DIR_R |= 0x0E;
    GPIO_PORTF_DEN_R |= 0x0E;
    GPIO_PORTF_AMSEL_R &= ~0x0E;            //SET PINS 1,2,3 AS DIGITAL OUTPUT

    GPIO_PORTF_AFSEL_R &= ~0x0E;
    GPIO_PORTF_PCTL_R &= ~0x0000FFF0; //SET PINS 1,2,3 AS GPIOs

}

// 7-segment-display functions
uint8_t decimal_to_BCD(uint8_t num){switch(num){
        case 0:
            return 0b00111111;
        case 1:
            return 0b00000110;
        case 2:
            return 0b00011011;
        case 3:
            return 0b01001111;
        case 4:
            return 0b01100110;
        case 5:
            return 0b01101101;
        case 6:
            return 0b01111101;
        case 7:
            return 0b00000111;
        case 8:
            return 0b01111111;
    }
    return 0b01101111;
	
}

void dist_to_display(uint16_t dist){
	
}

void delay_milli(int n){
int i,j;
for(i=0;i<n;i++){
for(j=0;j<3180;j++)
{}
}
}

void delay_micro(int n){
int i,j;
for(i=0;i<n;i++){
for(j=0;j<3;j++)
{}
}
}
//LCD functions
void lcd_command(unsigned char command){
	GPIO_PORTE_DATA_R &= ~0x0E; // RS,R/W => 0  TO SELECT COMMAND AND WRITE
	GPIO_PORTB_DATA_R  = command;  // put command in the data
	GPIO_PORTE_DATA_R |= 0x08;  // Enable is set
	delay_micro(0);
	GPIO_PORTE_DATA_R &= ~0x0E;
	if (command < 4){
		delay_milli(2); // command 1 and 2 needs up to 1.64ms //
	}		
	else{
		delay_micro(40); /* all others 40 us */	
	}
}
void lcd_data(unsigned char data){
	
	GPIO_PORTE_DATA_R |= 0x02;  // RS => 1  TO SELECT Data
	GPIO_PORTB_DATA_R  = data;  // put command in the data
	GPIO_PORTE_DATA_R |= 0x08;  // ENABLE IS SET TO 1
	delay_micro(40);
	GPIO_PORTE_DATA_R &= ~0x0E; // ENABLE IS SET TO 0
}
void lcd_display(uint16_t data){
	uint8_t hundreds = data/100;
	uint8_t tens=data/10 - hundreds*10 ;
	uint8_t ones=data-hundreds*100 - tens*10;
	lcd_data(hundreds +'0');
	lcd_data(tens +'0');
	lcd_data(ones +'0');
}
void lcd_init(void){
		SYSCTL_RCGCGPIO_R |= 0x12; // Enable clock for PORTB and PORTE
    while((SYSCTL_PRGPIO_R & 0x12) == 0){} ;

    //GPIO_PORTB_LOCK_R = 0x4C4F434B;
    GPIO_PORTE_LOCK_R = 0x4C4F434B;
    GPIO_PORTB_CR_R |= 0xFF;            // Unlocking PORTB PIN 1...8;
    GPIO_PORTE_CR_R |= 0x0E;            // Unlocking PORTE PIN 1 2 3;

    GPIO_PORTB_DEN_R  |= 0xFF;
    GPIO_PORTB_AMSEL_R &= ~0xFF;
    GPIO_PORTE_DEN_R  |= 0x0E;
    GPIO_PORTE_AMSEL_R &= ~0x0E;

    GPIO_PORTB_DIR_R  |= 0xFF;   // Configure PORTB Pin 1~8 digital output pins
    GPIO_PORTE_DIR_R  |= 0x0E;  // Configure PORTF Pin 1, 2 and 3 digital input pins


    GPIO_PORTB_AFSEL_R  = 0x00;
    GPIO_PORTE_AFSEL_R &= ~0x0E;
    GPIO_PORTB_PCTL_R   = 0x00000000;
    GPIO_PORTE_PCTL_R  &= ~0x0000FFF0;
			
		delay_milli(20); /* initialization sequence */
		lcd_command(0x30);
		delay_milli(5);
		lcd_command(0x30);
		delay_micro(100);
		lcd_command(0x30);

    lcd_command(0x38); //8-bits,2 display lines, 5x7 font

    lcd_command(0x06); //increments automatically

    lcd_command(0x0F); //Turn on display

    lcd_command(0x01); //clear display
}