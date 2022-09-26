#include <lpc214x.h>
#include <stdio.h>
#include<string.h>
#define PLOCK 0x00000400
#define LED_OFF (IO0SET = 1U << 31)
#define LED_ON (IO0CLR = 1U << 31)
#define COL0 (IO1PIN & 1 <<19)
#define COL1 (IO1PIN & 1 <<18)
#define COL2 (IO1PIN & 1 <<17)
#define COL3 (IO1PIN & 1 <<16)
#define PLOCK 0x00000400
#define LED_OFF (IO0SET = 1U << 31)
#define LED_ON (IO0CLR = 1U << 31)
#define RS_ON (IO0SET = 1U << 20)
#define RS_OFF (IO0CLR = 1U << 20)
#define EN_ON (IO1SET = 1U << 25)
#define EN_OFF (IO1CLR = 1U << 25)
#define SW2 (IO0PIN & (1 << 14))
#define SW3 (IO0PIN & (1 << 15))
#define SW4 (IO1PIN & (1 << 18))
#define SW5 (IO1PIN & (1 << 19))
#define SW6 (IO1PIN & (1 << 20))

void SystemInit(void);
void uart_init(void);
static void delay_ms(unsigned int j);//millisecond delay
static void delay_us(unsigned int count);//microsecond delay
static void LCD_SendCmdSignals(void);
static void LCD_SendDataSignals(void);
static void LCD_SendHigherNibble(unsigned char dataByte);
static void LCD_CmdWrite( unsigned char cmdByte);
static void LCD_DataWrite( unsigned char dataByte);
static void LCD_Reset(void);
void alphadisp7SEG(char *buf);
unsigned int adc(int no,int ch);
static void LCD_Init(void);
void show(char*,int);
void LCD_DisplayString(const char *ptr_stringPointer_u8);

char PASSWORD[9] = "12345678";
unsigned char lookup_table[4][4]={ {'0', '1', '2','3'},
																										{'4', '5', '6','7'},
																										{'8', '9', 'a','b'},
																										{'c', 'd', 'e','f'}};
unsigned char rowsel=0,colsel=0;
int doorLocked=0;
int entered=0;
unsigned int x=0; 
int dutyCycle;
void runDCMotor(int,int);

__irq void Timer0_ISR(void) 
{
			x = x ^ 1;
			if (x) 
						IOSET0 = 1U <<31;
			else 
						IOCLR0 = 1U <<31;
			int v = adc(1,3);
			if(entered==0){
						alphadisp7SEG("n00ne");
			} else {
			if(v>500){
						alphadisp7SEG("li-0n");
						IOSET0 |= 1U<<11;
			}
			else{
						alphadisp7SEG("li-0f");
						IOCLR0 |= 1U<<11;
			}	
						if(!SW2) dutyCycle = 0;
						if(!SW3) dutyCycle = 25;
						if(!SW4) dutyCycle = 50;
						if(!SW5) dutyCycle = 75;
						if(!SW6) dutyCycle = 100;
						runDCMotor(2,dutyCycle);
			}
			T0IR = 0x01; 
			VICVectAddr = 0x00000000 ; 
}

void runDCMotor(int direction,int dutycycle)
{
			IO0DIR |= 1U << 28;
			PINSEL0 |= 2 << 18;
			if (direction == 1)
						IO0SET = 1 << 28;
			else
						IO0CLR = 1 << 28;
			PWMPCR = (1 << 14);
			PWMMR0 = 10000;
			PWMMR6 = (10000U*dutycycle)/100;
			PWMTCR=0X00000009;
			PWMLER=0X70;
}

unsigned char getAlphaCode(unsigned char alphachar)
{
			switch (alphachar)
			{
			case 'f':return 0x8e;
			case 'i':return 0xf9;
			case 'r':return 0xce;
			case 'e':return 0x86; // 1000 0110
			case 'h':return 0x89;
			case 'l':return 0xc7;
			case 'n' : return 0xc8;
			case 'p':return 0x8c;
			case '0': return 0xc0;//11000000
			case '1': return 0xf9;
			case '-' : return 0xbf;
			case '2': return 0xa4;
			case '3':return 0xb0;
			case '4':return 0x99;
			case '5':return 0x92;
			case '6': return 0x02; //00000010
			case '7': return 0xf8; //11111000
			case '8': return 0x00;
			case '9': return 0x98;//00010000

			case ' ': return 0xff;
			default : break;
			}
			return 0xff;
}

void alphadisp7SEG(char *buf)
{
			unsigned char i,j;
			unsigned char seg7_data,temp=0;
			for(i=0;i<5;i++)
			{
						seg7_data = getAlphaCode(*(buf+i));

						for (j=0 ; j<8; j++)
						{
									temp = seg7_data & 0x80;
									if(temp == 0x80)
												IOSET0 |= 1 << 19;
									else
												IOCLR0 |= 1 << 19;
									IOSET0 |= 1 << 20; //IOSET0 | 0x00100000;
									delay_ms(1);
									IOCLR0 |= 1 << 20; //IOCLR0 | 0x00100000;

									seg7_data = seg7_data << 1; // get next bit into D7 position

						}
			}

			IOSET0 |= 1 << 30; //IOSET0 | 0x40000000;
			delay_ms(1); //nop();
			IOCLR0 |= 1 << 30; //IOCLR0 | 0x40000000;
			return;
}

int main(void)
{
			IODIR0 = 0x80000000;
			T0TCR = 0x00;
			T0MCR= 0x0003; 
			T0TC = 0x00; 
			T0MR0 = 15000000; 
			VICVectAddr4 = (unsigned long)Timer0_ISR; 
			VICVectCntl4 = 0x0000024; 
			VICIntEnable = 0x0000010; 
			T0TCR = 0x01; 
			SystemInit();
			int dutyCycle=0;
			uart_init();//initialize UART0 port
			IO0DIR |= 1U << 31 | 1U << 19 | 1U << 20 | 1U << 30 |1U<<11; // to set as o/ps
			IO0DIR |= 1U << 31 | 0x00FF0000; // to set P0.16 to P0.23 as o/ps
			IO1DIR |= 1U << 25; // to set P1.25 as o/p used for EN
			PINSEL1 |= 0x00080000;
			LCD_Reset();
			LCD_Init();
			char password[9]="";
			char hidden[9]="";
			while(1)
			{
						do
						{
									DACR = ((1<<16)|(1023<<6));
									show("Enter Password",0);
									show(PASSWORD,1);
									while(entered == 0)
									{
												int inp=8;
												while(inp--){
															while(1){
																		//check for keypress in row0,make row0 '0',row1=row2=row3='1'
																		rowsel=0;IO0SET = 0X000F0000;IO0CLR = 1 << 16;
																		if(COL0==0){colsel=0;break;}
																		if(COL1==0){colsel=1;break;}
																		if(COL2==0){colsel=2;break;}
																		if(COL3==0){colsel=3;break;}
																		//check for keypress in row1,make row1 '0'

																		rowsel=1;IO0SET = 0X000F0000;IO0CLR = 1 << 17;
																		if(COL0==0){colsel=0;break;}if(COL1==0){colsel=1;break;}
																		if(COL2==0){colsel=2;break;}if(COL3==0){colsel=3;break;}
																		//check for keypress in row2,make row2 '0'
																		rowsel=2;IO0SET = 0X000F0000;IO0CLR = 1 << 18;//make row2 '0'
																		if(COL0==0){colsel=0;break;}if(COL1==0){colsel=1;break;}
																		if(COL2==0){colsel=2;break;}if(COL3==0){colsel=3;break;}
																		//check for keypress in row3,make row3 '0'
																		rowsel=3;IO0SET = 0X000F0000;IO0CLR = 1 << 19;//make row3 '0'
																		if(COL0==0){colsel=0;break;}if(COL1==0){colsel=1;break;}
																		if(COL2==0){colsel=2;break;}if(COL3==0){colsel=3;break;}
															};
															delay_ms(50); //allow for key debouncing
															while(COL0==0 || COL1==0 || COL2==0 || COL3==0);//wait for key release
															delay_ms(50); //allow for key debouncing
															IO0SET = 0X000F0000; //disable all the rows
															char temp = lookup_table[rowsel][colsel];
															strcat(password,&temp);
															strcat(hidden,"*");
															show(hidden,2);
												}
												if(strcmp(password,PASSWORD)==0){
															show("Correct Password",3);
															entered=1;
															int no_of_steps_clk = 100;
															do{
																		IO0CLR=0x000F0000; IO0SET=0x00010000;delay_ms(10);if(--no_of_steps_clk==0){ break;}
																		IO0CLR=0x000F0000; IO0SET=0x00020000;delay_ms(10);if(--no_of_steps_clk==0){ break;}
																		IO0CLR=0x000F0000; IO0SET=0x00040000;delay_ms(10);if(--no_of_steps_clk==0){ break;}
																		IO0CLR=0x000F0000; IO0SET=0x00080000;delay_ms(10);if(--no_of_steps_clk==0){ break;}
															}while(1);
															no_of_steps_clk = 100;
															delay_ms(1000);
															do{
																		IO0CLR=0x000F0000; IO0SET=0x00080000;delay_ms(10);if(--no_of_steps_clk==0){ break;}
																		IO0CLR=0x000F0000; IO0SET=0x00040000;delay_ms(10);if(--no_of_steps_clk==0){ break;}
																		IO0CLR=0x000F0000; IO0SET=0x00020000;delay_ms(10);if(--no_of_steps_clk==0){ break;}
																		IO0CLR=0x000F0000; IO0SET=0x00010000;delay_ms(10);if(--no_of_steps_clk==0){ break;}
															}while(1);
												}
												else{
															show("Incorrect Password",3);
															DACR = ((1<<16)|(0<<6));
															delay_ms(500);
															DACR = ((1<<16)|(1023<<6));
															show("        ",2);
												}
									strcpy(password,"");
									strcpy(hidden,"");
									}
						}while(1);
			}
}

void uart_init(void)
{
			//configurations to use serial port
			PINSEL0 |= 0x00000005; // P0.0 & P0.1 ARE CONFIGURED AS TXD0 & RXD0
			U0LCR = 0x83; /* 8 bits, no Parity, 1 Stop bit */
			U0DLM = 0; U0DLL = 8; // 115200 baud rate
			U0LCR = 0x03; /* DLAB = 0 */
			U0FCR = 0x07; /* Enable and reset TX and RX FIFO. */
}

void show(char* buf,int line)
{
			delay_ms(100);
			char lineCmd;
			switch(line){
						case 0: lineCmd = 0x80; break;
						case 1:	lineCmd = 0xc0; break;
						case 2: lineCmd = 0x94; break;
						case 3: lineCmd = 0xd4; break;
			}
			LCD_CmdWrite(lineCmd); LCD_DisplayString(buf);
}

unsigned int adc(int no,int ch)
{
				unsigned int val;
				PINSEL0 |= 0x0F300000;
				switch (no)
				{
							case 0: AD0CR = 0x00200600 | (1<<ch);
							AD0CR |= (1<<24) ;
							while ( ( AD0GDR & ( 1U << 31 ) ) == 0);
							val = AD0GDR;
							break;
							case 1: AD1CR = 0x00200600 | ( 1 << ch );
							AD1CR |= ( 1 << 24 ) ;
							while ( ( AD1GDR & (1U << 31) ) == 0);
							val = AD1GDR;
							break;
				}
				val = (val >> 6) & 0x03FF;
				return val;
}

static void LCD_CmdWrite( unsigned char cmdByte)
{
				LCD_SendHigherNibble(cmdByte);
				LCD_SendCmdSignals();
				cmdByte = cmdByte << 4;
				LCD_SendHigherNibble(cmdByte);
				LCD_SendCmdSignals();
}

static void LCD_DataWrite( unsigned char dataByte)
{
				LCD_SendHigherNibble(dataByte);
				LCD_SendDataSignals();
				dataByte = dataByte << 4;
				LCD_SendHigherNibble(dataByte);
				LCD_SendDataSignals();
}
static void LCD_Reset(void)
{
				/* LCD reset sequence for 4-bit mode*/
				LCD_SendHigherNibble(0x30);
				LCD_SendCmdSignals();
				delay_ms(100);
				LCD_SendHigherNibble(0x30);
				LCD_SendCmdSignals();
				delay_us(200);
				LCD_SendHigherNibble(0x30);
				LCD_SendCmdSignals();
				delay_us(200);
				LCD_SendHigherNibble(0x20);
				LCD_SendCmdSignals();
				delay_us(200);
}
static void LCD_SendHigherNibble(unsigned char dataByte)
{
				//send the D7,6,5,D4(uppernibble) to P0.16 to P0.19
				IO0CLR = 0X000F0000;IO0SET = ((dataByte >>4) & 0x0f) << 16;
}
static void LCD_SendCmdSignals(void)
{
				RS_OFF; // RS - 1
				EN_ON;delay_us(100);EN_OFF; // EN - 1 then 0
}
static void LCD_SendDataSignals(void)
{
				RS_ON;// RS - 1
				EN_ON;delay_us(100);EN_OFF; // EN - 1 then 0
}
static void LCD_Init(void)
{
				delay_ms(100);
				LCD_Reset();
				LCD_CmdWrite(0x28u); //Initialize the LCD for 4-bit 5x7 matrix type
				LCD_CmdWrite(0x0Eu); // Display ON cursor ON
				LCD_CmdWrite(0x01u); //Clear the LCD
				LCD_CmdWrite(0x80u); //go to First line First Position
}


void LCD_DisplayString(const char *ptr_string)
{
				// Loop through the string and display char by char
				while((*ptr_string)!=0)
				LCD_DataWrite(*ptr_string++);
}
static void delay_us(unsigned int count)
{
				unsigned int j=0,i=0;
				for(j=0;j<count;j++)
				{
								for(i=0;i<10;i++);
				}
}

void SystemInit(void)
{
				PLL0CON = 0x01;
				PLL0CFG = 0x24;
				PLL0FEED = 0xAA;
				PLL0FEED = 0x55;
				while( !( PLL0STAT & PLOCK ))
				{ ; }
				PLL0CON = 0x03;
				PLL0FEED = 0xAA; // lock the PLL registers after setting the required PLL
				PLL0FEED = 0x55;
				VPBDIV = 0x01; // PCLK is same as CCLK i.e 60Mhz
}
void delay_ms(unsigned int j)
{
				unsigned int x,i;
				for(i=0;i<j;i++)
				{
							for(x=0; x<10000; x++);
				}
}