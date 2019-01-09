/*
AIM	    : PROGRAM TO IMPLEMENT IO-EXPANDER USING I2C PROTOCOL 
AUTHOR	    : PUNEET BANSAL
DATE	    : 11/09/2018
Course	    : EMBEDDED SYSTEM DESIGN
Compiler    : SDCC
Tools Used  : CODE BLOCKS, FLIP, PUTTY.
*/

#include <mcs51/8051.h>
#include <stdio.h>
#include <at89c51ed2.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#define SCL P1_2
#define SDA P1_3
#define ACK 0
#define NACK 1
#define GET_MSB 0x80

/*Function initializations*/

void putchar(char c);           //To print a character on the serial port
void serial_init();             //Function to initialise the serial communication
char getchar();                 //To take a character input via the serial communication
int getnumber();                //To take a number from the user
void start();                   //Start sequence for I2c
void stop();                    //Stop sequence for I2c
void delay();                   //to provide delay
void i2c_init();
void write(uint8_t);            //Write 8 bits to the SDA bus
void write_byte(uint8_t);
void read_byte();               //Write 8 bits from the SDA bus
int input_from_user();          //Function to convert the input from the user to HEX
void hex_dump();
void reset_eeprom();
//void block_fill();

/*Global Variables*/

uint8_t data_read;
uint8_t byte1;
uint8_t byte2;
uint8_t read_b1;
uint8_t read_b2;


/*Function to set the AUXR register bits*/

_sdcc_external_startup()
{
 AUXR |= 0x0C;
 AUXR &= 0xFD;
 //EX0=1;
 //EA=1;

 return 0;
}
void ExtInt_Init()		        /* External interrupt initialize */
{
	IT0 = 1;      		        /* Interrupt0 on falling edge */
	EX0 = 1;      		        /* Enable External interrupt0 */
	EA  = 1;		            /* Enable global interrupt */
	IP = 0x01;		            /* Set highest priority for Ext. interrupt0 */
}
void External0_ISR() interrupt 0
{
uint8_t data_wr;
printf_tiny("\n\rEntered isr");
read_byte();
printf_tiny("\n\rData read from pins is %x",read_b2);
data_wr=((read_b2)^0x7f);
data_wr|=0x80;
//data_wr = data_wr>>1;
write_byte(data_wr);
//read_byte();
//printf_tiny("\n\rData read from pins after toggling is %x",read_b2);

}

void main(void)
{

 ExtInt_Init();
 serial_init();
 i2c_init();
printf_tiny("\n\rChecking IO expander\n\r");

 while(1)
 {;
 }

}

/*Function to initialise the i2c communication i.e setting the data and clock bus to high*/

void i2c_init()
{
 SDA=1;
 delay();
 delay();
 SCL=1;
 delay();
 delay();
}

/*Funcion to provide delay*/

void delay()
{
int i;
for(i=0;i<300;i++);
}
/*------------------Start condition: When SCL is high, SDA goes High to Low----------------------*/
void start()
{
SDA=1;
SCL=1;
delay();
SDA=0;
delay();
SCL=0;
delay();

}
/*------------------Stop condition: When SCL is high, SDA goes Low to High----------------------*/
void stop()
{
SDA=0;
delay();
SCL=1;
delay();
SDA=1;
delay();
}

/*Function to read 8 bits from the SDA bus and returns uint8_t value*/
uint8_t read(void)
{
uint8_t i,d, rxdata=0;
for(i=0;i<8;i++)
{
SCL = 0;
delay();
SDA = 1;
delay();
SCL = 1;
delay();
delay();
d=SDA;
rxdata=rxdata|(d<<7-i);

SCL=0;
delay();
}
return rxdata;

}

/*-----------------For write operation: The data must be valid when the clock pulse is high----------------*/
void write(uint8_t data_in)
{
int i;
for(i=0;i<8;i++)
{
SCL=0;
delay();                            /*Changing the clock to 0 to manipulate SDA*/

SDA=(((data_in)&(GET_MSB)) >> 7);   /*Taking only the MSB and sending it to SDA*/
SCL=1;                              /*Since we have valid data on SDA, now turning the SCL=1*/
delay();
SCL=0;
delay();                            /*To get the data at the falling edge of clk*/
data_in = data_in << 1;             /*Left shifting the data byte by one, to get the next SDA bit*/
}

}

int input_from_user()
{
int numlen=0;
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



void write_byte(uint8_t data_1)
{

    byte2=0x40;

    start();

    write(byte2);                   //Write the address byte to set the pointer to point to the specific address

    /*----------Waiting for Ack-----------------*/
    SCL=1;
    delay();
    delay();

    while(SDA==1)
        {
        printf("\n\rNegative Acknowledgement");
        write(byte2);
        SCL=1;
        delay();
        delay();
        }

    SCL=0;
    delay();
    delay();
    /*------------------------------------------*/

    write(data_1);

    /*----------Waiting for Ack-----------------*/
    SCL=1;
    delay();
    delay();

    while(SDA==1)
        {
        printf("\n\rNegative Acknowledgement");
        write(data_1);
        SCL=1;
        delay();
        delay();

        }

    SCL=0;
    delay();
    delay();
    /*-----------------------------------------*/
    stop();

}

void hex_dump()
{

uint16_t add_1,add_2,i,j;
uint16_t diff;
uint8_t byte1,byte2,read_b,read_b2;

printf_tiny("\n\rEnter the address from which you want to read the data. Please enter in hex(000-7FF)\n\r->");
add_1=(uint16_t)input_from_user();

/*Preparing the byte---------|1|0|1|0|A2|A1|A0|0|--------  */
byte1=((0xA0)|((add_1 & 0x700)>>7)| (0x00));

/*Obtaining the least significant 8bits of the address*/
byte2=(add_1 & 0xff);

/*Preparing the byte---------|1|0|1|0|A2|A1|A0|1|--------  */
read_b2=((0xA0)|((add_1 & 0x700)>>7)| (0x01));

if((add_1<=2047))
{
printf_tiny("\n\rEnter the address till which you want to read the data. Please enter in hex(000-7FF)\n\r->");
add_2=(uint16_t)input_from_user();
printf_tiny("\n\r");

diff=(((add_2)-(add_1))+1);

    if(add_2<=2047)
        {
           start();
           write(byte1);
           /*----------Waiting for Ack-----------------*/
           SCL=1;
           delay();
           delay();
           while(SDA==1)
            {
            printf("\n\rNegative Acknowledgement");
            start();
            write(byte1);
            SCL=1;
            delay();
            delay();
            //delay();
            }

           SCL=0;
           delay();
           delay();
           /*-----------------------------------------*/

           write(byte2);

           /*----------Waiting for Ack-----------------*/
           SCL=1;
           delay();
           delay();

            while(SDA==1)
            {
            printf("\n\rNegative Acknowledgement");
            //start();
            write(byte2);
            SCL=1;
            //delay();
            delay();
            delay();
            }

           SCL=0;
           delay();
           delay();
           /*-----------------------------------------*/

           SDA=1;
           delay();
            /*After setting the address pointer, now sending the START bit, enabling the read operation and reading from SDA*/

           start();
           write(read_b2);

           SCL=1;
           delay();
           delay();
           SCL=0;

            for(i=0;i<=diff;i+=16)
            {
            printf_tiny("0X%x :",add_1+i);
            for(j=0; j<16 &&((i+j)<=diff);j++)
                {
                read_b=read();
                /* If reached at the end address, send stop, else send Ack*/
                if((i+j)==diff)
                    {
                     stop();
                    }
                else
                    {
                    /*Sending Ack after every 8 bits read*/
                    SDA=ACK;
                    delay();
                    delay();

                    SCL=1;
                    delay();
                    delay();

                    SCL=0;
                    delay();

                    delay();
                    printf_tiny("0x%x ",read_b);
                    }
                }
            printf_tiny("\n\r");

            }
        }
    else
        {
        printf("\n\rEntered address is not valid");
        }

}
else
    {
    printf("\n\rEntered address is not valid");
    }
}

void read_byte()
{
read_b1=0x41;
start();

write(read_b1);
SCL=1;
delay();
delay();
if(SDA==1)
        {
        printf("\n\rNegative Acknowledgement");
        write(read_b1);
        }
SCL=0;
delay();
delay();

read_b2=read();

stop();

}

/*Sending the Start bit, then making the SDA high for 9 clock cycles, then sending the start bit and the stop biy*/
void reset_eeprom()
{
int i;
start();

    SDA=1;
    delay();
    delay();

for(i=0;i<9;i++)
    {
    SCL=1;
    delay();
    delay();

    SCL=0;
    delay();
    delay();
    }
start();
stop();
}

/*void block_fill()
{

}
*/

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

