/*
AIM	     : PROGRAM TO INTERFACE LCD AND PERFORM THE FOLLOWING FUNCTIONS
              1) GOTO A SPECIFIC ADDRESS ON LCD DDRAM ENTERED BY USER
              2) GOTO A COORDINATE (X,Y) ENTERED BY USER
              3) PRINTING A CHARACTER ENTERED BY USER ON THE LCD
              4) PRINTING A STRING ON THE LCD
              5) CLEAR THE LCD
              6) STARTING THE TIMER
              7) STOPPING THE TIMER
              8) PERFORMING THE DUMP OF CGRAM AND DDRAM ADDRESSES
              9) CREATING A CUSTOM CHARACTER
              10) PRINTING THE CUSTOM CHARACTER ON THE LCD
              11) PERFORMING DIGITAL TO ANALOG CONVERSION VIA SPI PROTOCOL

AUTHOR	    : PUNEET BANSAL
DATE	    : 11/09/2018
Course	    : EMBEDDED SYSTEM DESIGN
Compiler    : SDCC
Tools Used  : CODE BLOCKS, FLIP, PUTTY.
References  : Code for SPI reference from Lecture Slides on SPI.
              https://learn.colorado.edu/d2l/le/content/255799/viewContent/3545454/View?ou=255799
*/
/* **********************Header Files********************************* */
#include <mcs51/8051.h>
#include <at89c51ed2.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define SETM_8BIT (uint8_t)0x30      //set LCD to 8 bit mode
#define POLL_BF (uint8_t)0x80        //polling the busy flag
#define FXN_SET (uint8_t)0x38        // DL=1,N=1,F=0
#define DISP_OFF (uint8_t)0x08       // Turning the display off D3=1
#define DISP_ON (uint8_t)0x0C        // Turning the LCD on
#define ENTRY_MODE (uint8_t)0x06
#define CLRSCR (uint8_t)0x01	
#define GET_ADD (uint8_t)0x7F
#define CS_DAC P1_5

//Using A14,A13 as RS, R/W
#define LCD_CMD ((volatile uint8_t xdata*)(0x8000)) //RS=0,RW=0
#define LCD_BF ((volatile uint8_t xdata*)(0xA000))  //RS=0 RW=1
#define LCD_WR ((volatile uint8_t xdata*)(0xC000))  //RS=1 RW=0
#define LCD_RD ((volatile uint8_t xdata*)(0xE000))  //RS=1 RW=1

uint8_t msec,sec,mins,hours;
int flag;
uint8_t spicounter=0;

/* **************Function declarations *************** */
void msdelay(unsigned int);
void lcdinit();
void lcdbusywait();
void lcdgotoaddr(unsigned char);
void lcdgotoxy(unsigned char,unsigned char);
void lcdputch(char);
void lcdputstr(char *);
void serial_init();
void putchar(char);
char getchar();
int getnumber();
int input_from_user();
void timer_init();                                  //for timer 0
void start_timer();
void stop_timer();
void LCD_Dump();
void custom_char_input();
void custom_char_print();
void spi_init();
char serial_data;char data_example=0x55;char data_save;bit transmit_completed= 0;

/* ********** Timer 0 Interrupt Handler ******************** */
void timer0_isr() interrupt 1
{
unsigned char cursor_add;
flag=flag+1;
TF0=0;
TL0=0xFC;
TH0=0x4B;
TR0=1;

if(flag==2)                     //Checking for flag =2, to get delay =100ms
            {
                cursor_add=((*LCD_BF) & GET_ADD);  //Store the address of the lcd pointer before entering the ISR
                msec=msec+1;
                if(msec==10)
                {
                sec=sec+1;
                msec=0;
                }
                if(sec==59)
                {
                mins=mins+1;
                sec=0;
                }
                flag=0;
                lcdgotoaddr(0x5f);
                lcdputch(msec+0X30);          //printing milliseconds
                lcdgotoaddr(0x5e);
                lcdputch('.');
                lcdgotoaddr(0x5d);
                lcdputch((sec%10)+0X30);      //printing one's place of seconds
                lcdgotoaddr(0x5c);
                lcdputch((sec/10)+0X30);      //printing ten's place of seconds
                lcdgotoaddr(0x5b);
                lcdputch(':');

                lcdgotoaddr(0x5a);
                lcdputch((mins%10)+0X30);     //printing one's place of minutes

                lcdgotoaddr(0x59);
                lcdputch((mins/10)+0X30);     //printing ten's place of minutes

                //msdelay(10);
                lcdgotoaddr(cursor_add);      //Going back to the address where LCD pointer was before entering the ISR
            }

}
void ISR_SPI(void) interrupt 9                 /* interrupt address is 0x004B */
{
    uint8_t read_status;
    spicounter++;
    printf_tiny("\n\rEntered ISR FOR SPI");
    //Procedure to clear the interrupt flag
    read_status=SPSTA;
    serial_data=SPDAT;
    if(spicounter==1)
    {
        SPDAT=0x50;                         //Sending the second byte of the data
    }
    else if(spicounter==2)
    {
    CS_DAC=1;                       //Turning CS high, after sending both the byte
    IEN1= 0x0;                      //Disabling interrupt
    }

}

void main()
{

 int input;
 uint8_t add;
 char ch;
 uint8_t x,y;
 flag=0;

 serial_init();         //Initialize serial communication
 lcdinit();             //Initialize LCD
 timer_init();          //Initialize Timer 0
 msec=0;
 sec=0;
 mins=0;
 hours=0;
 do
    {

    printf_tiny("\n\r\n\rWELCOME TO THE COMMAND LINE\n\r1)GOTO ADDRESS\n\r2)GOTO X,Y\n\r3)PUTCHAR\n\r4)PUTSTRING\n\r5)CLEAR\n\r6)START TIMER\n\r7)STOP TIMER\n\r8)LCD DUMP\n\r9)CUSTOM CHAR INPUT\n\r10)SEE CUSTOM CHAR\n\r11)DAC\n\r12)RESET TIMER\n\r13)RESTART TIMER\n\r14)EXIT\n\r->");
    input=getnumber();
    switch(input)
        {
        case 1:
            printf_tiny("\n\rEnter the DDRAM address at which you want to write on LCD\n\r->");
            add=input_from_user();
            if((add<=16) || ((add>=64) && (add<=79)) ||((add>=16)&& (add<=31)) || ((add>=80) && (add<=95)))
            lcdgotoaddr(add);
            else
            printf_tiny("\n\rInvalid address");
            break;

        case 2:
            printf_tiny("\n\rEnter the X cordinate at which you want to write on LCD\n\r->");
            x=getnumber();
            printf_tiny("\n\rEnter the y cordinate at which you want to write on LCD\n\r->");
            y=getnumber();
            if((x<=3 )&& (y<=15) )
            lcdgotoxy(x,y);
            else
            printf_tiny("Invalid coordinates");
            break;

        case 3:
            printf_tiny("\n\rEnter the character\n\r->");
            ch=getchar();
            putchar(ch);
            lcdputch(ch);
            break;

        case 4:
            printf_tiny("\n\rEnter the string\n\r->");
            lcdputstr("HAPPY DIWALI");
            break;
        case 5:
            *LCD_CMD=CLRSCR;
            break;
        case 6:
            start_timer();
            break;

        case 7:
            stop_timer();
            break;

        case 8:
            LCD_Dump();
            break;
        case 9:
            custom_char_input();
            break;
        case 10:
            custom_char_print();
            break;
        case 11:
            printf("Beginning SPI communication");
            spi_init();
            break;
        case 12:
            stop_timer();
            msec=0;
            sec=0;
            mins=0;
            hours=0;
            //start_timer();
            break;
        case 13:
            stop_timer();
            msec=0;
            sec=0;
            mins=0;
            hours=0;
            start_timer();
            break;

        }




    }while(input!=14);

}

void msdelay(unsigned int time)  // Function for creating delay in milliseconds.
{
    unsigned i,j ;
    for(i=0;i<time;i++)
    for(j=0;j<1275;j++);
}

void lcdinit()
{
    msdelay(50);
    *(LCD_CMD)=(uint8_t)SETM_8BIT;
    msdelay(10);
    *LCD_CMD=SETM_8BIT;
    msdelay(10);
    *LCD_CMD=SETM_8BIT;

    lcdbusywait();              //Polls BF to check BF=0 or not
    *LCD_CMD=FXN_SET;           //Function set command
    lcdbusywait();
    *LCD_CMD=DISP_OFF;          //Display Off Command
    lcdbusywait();
    *LCD_CMD=DISP_ON;           //Display On Command
    lcdbusywait();
    *LCD_CMD=ENTRY_MODE;         //Command to enter entry mode
    lcdbusywait();
    *LCD_CMD=CLRSCR;            //TO clear the screen and move to home location.
}

void lcdbusywait()
{
    while((*LCD_BF) & POLL_BF);
}

void lcdgotoaddr(unsigned char addr)
{
    //printf_tiny("\n\rAdd entered is %x",addr);
    lcdbusywait();
    *LCD_CMD= (addr | 0x80);
    //msdelay(10);

}

void lcdgotoxy(unsigned char row, unsigned char column)
{
unsigned char add;
add=0;
//printf_tiny("\n\rx: %x, y:%x",row,column);
if(row==0)
    {
    add=column;
    lcdgotoaddr(add);
    }
else if(row==1)
    {
    add=64+column;
    lcdgotoaddr(add);
    }
else if(row==2)
    {
    add=16+column;
    lcdgotoaddr(add);
    }
else if(row==3)
    {
    add=80+column;
    lcdgotoaddr(add);
    }
//printf_tiny("add : %x",add);
}

void lcdputch(char cc)
{
    lcdbusywait();
    *LCD_WR=cc;
    //msdelay(10);
}

void lcdputstr(char *ss)
{
uint8_t i=0;
uint8_t len;
len=strlen(ss);
while(i!=len)
    {
    if(((*LCD_BF) & GET_ADD)==0x0F)
        {
        lcdgotoaddr(0x40);
        lcdputch(ss[i++]);
        }
    else if(((*LCD_BF) & GET_ADD)==0x4F)
        {
        lcdgotoaddr(0x10);
        lcdputch(ss[i++]);

        }
    else if(((*LCD_BF) & GET_ADD)==0x1F)
        {
        lcdgotoaddr(0x50);
        lcdputch(ss[i++]);

        }
    else if(((*LCD_BF) & GET_ADD)==0x5F)
        {
        lcdgotoaddr(0x00);
        lcdputch(ss[i++]);
        }
    else
        {
         lcdputch(ss[i++]);
        }
    }
}

void serial_init()
    {
    SCON=0X50;                  // enabling 8 bit variable baud serial communication
    TMOD=0X20;                  // timer 1 in mode 2
    TH1 =0xFD;                  //for setting baud rate=9600
    TR1 =1;                     //start timer 1
    }
void putchar(char c)
    {
     SBUF=c;
     while(TI==0);
     TI=0;
     }
char getchar()
    {
     char c;
     while(RI==0);
     c=SBUF;
     RI=0;
     return c;
    }

int getnumber()                         //Function to take a number input from the user
{
    int numlen=0;
    int number=0;
    char str[10];
    char c;
    int i=0,j=0;
    c=getchar();
    putchar(c);
    while(c!=13 && c>=48 && c<=57)     //Check whether the entered character is between 0-9, till the time return is encountered.
        {
        str[i++]=c;                    //Store the characters in the string
        c=getchar();
        putchar(c);
        }
    str[i]='\0';                       //Append the null character
    i=0;
    numlen=strlen(str);
    number=atoi(str);                  //Function to change the string to an int.

return number;
}

int input_from_user()
{
int numlen;
int number=0;
char str[10];
char c;
int i=0,j=0;
c=getchar();
putchar(c);
while(c!=13 && (c>=48 && c<=57)||(c>=65 && c<=70)||(c>=97 && c<=102))     //Check whether the entered character is between 0-9, till the time return is encountered.
        {
        str[i++]=c;                                                       //Store the characters in the string
        c=getchar();
        putchar(c);
        }
    str[i]='\0';                                                          //Append the null character
    i=0;
    numlen=strlen(str);

    for(j=0;j<numlen;j++)
    {
    if(str[j]>=48 && str[j]<=57)                                          //Handling the number inputs
        {
        number=number +((str[j]-48)*(powf(16,(numlen-(j+1)))));
        }
    else if(str[j]>=65 && str[j]<= 70)                                    //Handling Captial letters A-F
        {
        number=number+ ((str[j]-55)*(powf(16,(numlen-(j+1)))));
        }
    else if(str[j]>=97 && str[j]<=102)                                    //Handling small letters a-f
        {
        number=number+ ((str[j]-87)*(powf(16,(numlen-(j+1)))));
        }
    }
                       //Function to change the string to an int.

    return number;
}

void timer_init()
{
    TMOD|=0X01;
    TL0=0XFC;
    TH0=0X4B;
    IE|=0X82;
}

void start_timer()
{
    TR0=1;
}

void stop_timer()
{
    TR0=0;

}

void LCD_Dump()
{
unsigned char addr[]={0x00,0x40,0x10,0x50};
unsigned char cgram_addr[]={0x40,0x48,0x50,0x58,0x60,0x68,0x70,0x78};
unsigned char data_rd;
unsigned char cgram_data_rd;
int i,j,k,l;
printf_tiny("\n\r********DDRAM DUMP********\n\r");
     for(i=0;i<4;i++)
            {
                if(i==0)
                    {
                    printf_tiny("0X%x  :",addr[i]);
                    }
                else
                    {
                    printf_tiny("0X%x :",addr[i]);
                    }
                for(j=0; j<16 ;j++)
                    {
                    lcdgotoxy(i,j);
                    data_rd= *(LCD_RD);
                    printf_tiny("0x%x ",data_rd);
                        /*if(data_rd==0x00)
                        {
                        putchar('_');
                        putchar(' ');
                        }
                        else
                        {
                        putchar(data_rd);
                        putchar(' ');
                        }*/

                    }
            printf_tiny("\n\r");
            }

printf_tiny("\n\r********CGRAM DUMP********\n\r");

for(k=0;k<8;k++)
    {
    *(LCD_CMD)=cgram_addr[k];
    printf_tiny("0x%x :",cgram_addr[k]);
    for(l=0;l<8;l++)
        {
        cgram_data_rd=*(LCD_RD);
        printf_tiny("0x%x ", cgram_data_rd);
        }
    printf_tiny("\n\r");
    }


}
void custom_char_input()
{
unsigned char cc_no;
unsigned char cc_data;
//unsigned char addr_cgram[8];
int i;

printf_tiny("\n\rEnter which custom character you want to create(0-7)\n\r->");
cc_no=getnumber();
for(i=0;i<8;i++)
    {
    *(LCD_CMD)=0x40 |((cc_no & 0x07)<<3);
    printf_tiny("\n\rEnter the data of %d row\n\r->",i);
    cc_data=input_from_user();
    *(LCD_WR)=(cc_data & 0x1F);
    }

}
void custom_char_print()
{
unsigned char cc_no;
unsigned char x,y;
printf_tiny("\n\rEnter the custom character number you want to see\n\r->");
cc_no=getnumber();

printf_tiny("\n\rEnter the X cordinate at which you want to see custom char on LCD\n\r->");
x=getnumber();
printf_tiny("\n\rEnter the y cordinate at which you want to see custom char on LCD\n\r->");
y=getnumber();
lcdgotoxy(x,y);
lcdputch(cc_no);
}

void spi_init()
{

SPCON |= 0x10;              /* master mode */
P1_1=1;                     /* enable master */
SPCON |= 0x82;              /* FclkPeriph/128 */
SPCON &= ~0x08;             /* CPOL=0; transmit mode example */
SPCON |= 0x04;              /* CPHA=1; transmit mode example */
IEN1 |= 0x04;               /* enable SPI interrupt */
SPCON |= 0x40;              /* run SPI */
EA=1;                       /* enable interrupts */
CS_DAC=0;
SPDAT= 0x35;
}
