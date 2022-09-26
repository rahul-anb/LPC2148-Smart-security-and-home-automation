#include <lpc214x.h>
#include<string.h>
#include "pass.c"
#define PLOCK 0x00000400
#define LED_OFF (IO0SET = 1U << 31)
#define LED_ON (IO0CLR = 1U << 31)
#define COL0 (IO1PIN & 1 <<19)
#define COL1 (IO1PIN & 1 <<18)
#define COL2 (IO1PIN & 1 <<17)
#define COL3 (IO1PIN & 1 <<16)
void uart_init(void);

unsigned char lookup_table[4][4]={ {'0', '1', '2','3'},
{'4', '5', '6','7'},
{'8', '9', 'a','b'},
{'c', 'd', 'e','f'}};
unsigned char rowsel=0,colsel=0;

void show(char* buf)
{
IO0DIR |= 1U << 31 | 0x00FF0000 ; // to set P0.16 to P0.23 as o/ps
IO1DIR |= 1U << 25; // to set P1.25 as o/p used for EN
// make D7 Led on off for testing
LED_ON; delay_ms(500);LED_OFF;delay_ms(500);
LCD_Reset();
LCD_Init();
delay_ms(100);
LCD_CmdWrite(0x80); LCD_DisplayString(buf);
while(1);
}
static void LCD_CmdWrite( unsigned char cmdByte)
{
LCD_SendHigherNibble(cmdByte);
LCD_SendCmdSignals();
cmdByte = cmdByte << 4;
LCD_SendHigherNibble(cmdByte);
LCD_SendCmdSignals();
}

int main( )
{
SystemInit();
uart_init();//initialize UART0 port
IO0DIR |= 1U << 31 | 0x00FF0000; // to set P0.16 to P0.23 as o/ps
char password[5]="";
do
{
while(1)
{	 int inp=5;
	
	while(inp--)
	{
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
	}
	//show(password);
}
while(1);
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