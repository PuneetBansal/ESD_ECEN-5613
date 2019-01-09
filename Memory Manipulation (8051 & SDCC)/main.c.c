/*
AIM	    : PROGRAM TO IMPLEMENT MEMORY MANIPULATION USING 8051 AND SDCC
AUTHOR	    : PUNEET BANSAL
DATE	    : 10/19/2018
Course	    : EMBEDDED SYSTEM DESIGN
Compiler    : SDCC
Tools Used  : CODE BLOCKS, FLIP, PUTTY.
*/

#include <mcs51/8051.h>
#include <at89c51ed2.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <malloc.h>
#define  HEAP_SIZE 3200
#include <stdint.h>
#define DEBUG(x) ((*(xdata unsigned char *)0xefff)=x)


unsigned char xdata heap[HEAP_SIZE];

// Function prototypes of all the functions used.

void putstring(char *str);      //To print a string on the serial port
void allocate();                //To allocate a block of memory specified by the user
void putchar(char c);           //To print a character on the serial port
void serial_init();             //Function to initialise the serial communication
char getchar();                 //To take a character input via the serial communication
int getnumber();                //To take a number from the user
int input();
int plus();                     //Function to allocate memory blocks if user enters "+"
int minus();                    //Function to delete a specific buffer (specified by the user ) if the user enters "-"
int question();
int equal();                    //Function to display the contents of buffer 0
void attherate();               //Function to free all the allocated buffers

uint8_t *buffer0;               //Pointer pointing to the starting address of buffer 0

uint8_t *buffer1;               //Pointer pointing to the starting address of buffer 1
uint8_t char_input;
uint8_t *buffer_arr[20];        // Array to store all the pointers to the allocated buffer.

int buffer0_NOE;                //Counter to count the total number of elements in buffer 0
int elements_question;          //Counter to keep a track of the total number of elements entered after ?
uint16_t buffer0_size;          //To store the size of buffer0 entered by the user
int buffercount;                //To keep a track of the buffer in the buffer array
int memsize;                    //Keeps a track of the total memory available
int elements_total;             //Keeps a track of the total number of elements input regardless of it being command or storage character
int question_status;            //To check whether the question() has been called or not
uint8_t attherate_status;       //To check whether the question() has been called or not

uint16_t add0,add1;
uint16_t add[15];               //Array to store the starting addresses of all the allocated buffers
uint16_t add_l[15];             //Array to store the ending addresses of all the allocated buffers
uint16_t tot_size[15];          //Array to store the total size of of all the allocated buffers
uint8_t status[15];             //Array to store the status of the buffers. 0 means free and 1 means present.
uint8_t *buffer0_end;           //A pointer pointing to the last location to of  buffer0;
uint16_t *extdata;

void _sdcc_external_startup()   // Function to set the AUXR register bits
    {
      AUXR |= 0x0C;
      AUXR &= 0xFD;
    }

void main()
    {
        uint16_t buffer_NOE1;
        char ch_in;
        DEBUG(1);                       //To check the Software debug functionality.
        question_status=0;
        elements_question=0;
        buffer0_NOE=0;
        elements_total=0;
        serial_init();                  //Initialize Serial Communication
        _sdcc_external_startup();       //Calling the sdcc external startup function.
        init_dynamic_memory((MEMHEADER xdata *)heap, HEAP_SIZE);    //Initialize heap of size 6000 bytes.

        label1:
        printf_tiny("\n\rHello..%c",'a');
        allocate();

       do{
        buffer_NOE1=(uint16_t)buffer0_NOE;

        ch_in=getchar();
        putchar(ch_in);
        elements_total+=1;                      //Keeps a track of the total number of elements entered throughout the program.
        elements_question+=1;                   //Keeps a track of the total number of elements entered after the last question was encountered.

        if(ch_in>=97 && ch_in<=122 && buffer_NOE1<tot_size[0])  //Checking whether the entered character is lowercase and whether there is space in buffer 0.
            {
                        if(question_status==1)                  //If a question mark is encountered, then point the buffer0 pointer back to the starting address of the buffer0
                            {
                            buffer0=buffer_arr[0];
                            question_status=0;
                            }

            *buffer0=ch_in;                                     //Store the lowercase character in buffer 0.
             buffer0_NOE+=1;                                    //Increment the counter to count number of elements in buffer 0.
             buffer0+=1;                                        //Increment the buffer0 counter to point to the next address.
            }

        else if(ch_in=='+')                   //if the character entered is '+' implement the plus function
            {
            plus();
            }
        else if(ch_in=='-')                   //if the character entered is '-' implement the minus function
            {
            minus();
            }
        else if(ch_in=='?')                   //if the character entered is '?' implement the question function
            {
            question();
            elements_question=0;
            }
        else if(ch_in=='=')                   //if the character entered is '=' implement the equal function
            {
            equal();
            }
        else if(ch_in=='@')                   //if the character entered is '@' implement the attherate function
            {
            attherate();
            goto label1;
            }
        else
            {
             printf_tiny("\n\r Character not entered in buffer");
            }
        }while(ch_in!='q');

    }


void allocate()
    {
    buffercount=0;

    printf_tiny("\n\rEnter the buffer size b/w 16 and 3200 bytes. The number should be divisible by 16\r\n");
    buffer0_size=(uint16_t)getnumber();                         //Storing the buffer size entered by user in the buffer0_size variable

    printf_tiny("\n\rThe number entered is %u \r\n",buffer0_size);

    if(buffer0_size<=HEAP_SIZE && buffer0_size%16==0)             //Check if the buffer size entered by user is within the bounds and is divisible by 16
        {
        buffer_arr[buffercount]=(uint8_t *)malloc(buffer0_size);  //allocate the buffer, and store the pointer pointing to the first location of the buffer in the pointer array buffer_arr[]
        add[buffercount]=(uint16_t)buffer_arr[buffercount];       //Store the starting address of the buffer in the array add[]
        add_l[buffercount]=add[0]+buffer0_size;                   //Store the ending address of the buffer in add_l[]
        buffer0=buffer_arr[0];                                    //Initialize another pointer pointing to buffer 0, for manipulation.
        tot_size[buffercount]=buffer0_size;                       //Store the size of the allocate buffer in tot_size[]
        status[buffercount]=1;                                    //To know whether the buffer exists or not. 1-exists, 0- freed.
        buffercount+=1;                                           //Increment the counter to the buffer_arr[]
        printf_tiny("Buffer 0 starting address(in allocate) %x\n\r",add[0]);
        printf_tiny("\r\nBuffer0 allocated");
        memsize=HEAP_SIZE-buffer0_size;

            if(buffer0_size<=memsize)
            {
            buffer_arr[buffercount]=(uint8_t *)malloc(buffer0_size);   //Allocate buffer 1
            add[buffercount]=(uint16_t)buffer_arr[1];
            add_l[buffercount]=add[1]+buffer0_size;
            tot_size[buffercount]=buffer0_size;
            status[buffercount]=1;
            buffercount+=1;

            printf_tiny("\n\rBuffer1 allocated\n\r");
            printf_tiny("Buffer 1 starting address(in allocate) %x\n\r",add[1]);
            memsize=memsize-buffer0_size;
            }
            else
            {
            free(buffer_arr[0]);
            printf_tiny("\nPlease enter a valid buffer size\n Freeing buffer 0");   //if buffer 1 size is not within the limits, then freeing buffer 0 as well.
            allocate();
            }
        }
    else
        {
            printf_tiny("\nPlease enter a valid buffer size");
            allocate();
        }

    }

void serial_init()
    {
    SCON=0X50;          // enabling 8 bit variable baud serial communication
    TMOD=0X20;          // timer 1 in mode 2
    TH1 =0xFD;          //for setting baud rate=9600
    TR1 =1;             //start timer 1
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


void putstring(char *str)
    {
    int i;
    for(i=0;i<strlen(str);i++)
        {
        putchar(str[i]);
        }
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
    number=atoi(str);
    return number;

    }

int plus()
    {
    uint16_t size;
    printf_tiny("\n\r You have chosen to allocate more bytes. \n\r Enter a size between 30 and 300 bytes\n\r");
    size=(uint16_t)getnumber();

    printf_tiny("%d",size);
    if(size<=memsize && size>=30 && size <=300)             //bound checking on the size entered by the user.
        {
        buffer_arr[buffercount]=(char *)malloc(size);       //allocate the buffer, and store the pointer pointing to the first location of the buffer in the pointer array buffer_arr[]
        add[buffercount]=(uint16_t)buffer_arr[buffercount]; //Store the starting address of the buffer in the array add[]
        add_l[buffercount]=add[buffercount]+size;           //Store the ending address of the buffer in the array add_l[]
        tot_size[buffercount]=size;                         //Store the total size of the buffer in tot_size[]
        status[buffercount]=1;                              //To know whether the buffer exists or not. 1-exists, 0- freed.
        buffercount+=1;                                      //Increment the counter to the buffer_arr[]
        memsize=memsize-size;
        }
    else
        {
        printf_tiny("\n\rCannot allocate memory ");
        }
    return 0;
    }

int minus()
    {
    int buffer_number;
    printf("\n\rYou have chosen to delete a buffer.\n\r Enter the buffer number you wish to delete (1....n)\n\r");
    buffer_number=getnumber();
    printf_tiny("%d",buffer_number);
        if(buffer_number==0)                                //prevent user from deleting buffer 0
            {
            printf_tiny("\n\rCannot delete buffer 0\n\r");
            }
        else if(buffer_number<=buffercount)                 //if a valid bufffer number is entered by the user, free that buffer
            {
            memsize+=sizeof(buffer_arr[buffer_number]);
            free(buffer_arr[buffer_number]);                //free the buffer
            status[buffer_number]=0;                        //change the status of the buffer to 0.


            printf_tiny("\n\rFreeing buffer %d\n\r",buffer_number);
            }
        else
            {
            printf_tiny("\n\rBuffer does not exist\n\r");
            }
    return 0;
    }

int question()
    {
    int i;
    int count2;
    uint8_t count=0;
    int count1=0;
    question_status=1;

    printf_tiny("\n\rCharacters received after last ? %d",elements_question);
    printf_tiny("\n\rTotal number of characters entered %d",elements_total);

    for(i=0;i<buffercount;i++)              //Printing the details of each buffer.
        {
        if(status[i]!=0)
        {
            printf_tiny("\n\r********************Buffer %d Details********************",i);
            printf_tiny("\n\rBuffer starting address:%x\n\r",add[i]);
            printf_tiny("\n\rBuffer end address:%x\n\r",add_l[i]);
            printf_tiny("\n\rBuffer %d size is %d Bytes\n\r",i,tot_size[i]);

         if(i==0)
         {
            uint16_t check;
            check=(uint16_t)buffer0_NOE;
            printf_tiny("\n\rNumber of empty sizes available :%d\n\r",(tot_size[i]-check));
            printf_tiny("\n\rBuffer %d Number of Elements:%d\n\r",i,buffer0_NOE);
         }

         else
         {
            printf_tiny("\n\rNumber of empty sizes available :%d\n\r",tot_size[i]);
            printf_tiny("Buffer %d Number of Elements: 0\n\r",i);
         }

        }
        else
        {
            printf_tiny("\n\rBuffer %d is empty",i);
        }
        }

     printf_tiny("\n\rNow printing contents of buffer 0\n\r");

     for(count2=0;count2<64;count2++)
     {
     while(count1<=buffer0_NOE)
        {
        printf_tiny("%d ",*(buffer_arr[0]+count));
        *(buffer_arr[0]+count)=0;
        count1+=1;
        count+=1;
        }
     printf_tiny("\n\r");
     }

     printf_tiny("\n\rNow empty the buffer 0");
     buffer_arr[0]=buffer_arr[0];
     buffer0_NOE=0;
     return 0;
    }

int equal()                                         // Function to print the contents as specified in the question
    {
    int i,j;
    uint8_t count;                                  // counter used to point to the next data.
    uint16_t count1;                                // counter used to point to the next address.
    count=0;
    count1=0;
    printf_tiny("\n\r");
    for(i=0;i<=buffer0_NOE/16;i++)                  //Loop to manipulate the number of rows
     {

        printf_tiny("0x%x :",add[0]+count1);
        count1+=16;

        for(j=0;j<16,count<buffer0_NOE;j++)         // Loop to manipulate the columns.
            {
         printf("0x%x ",*(buffer_arr[0]+count));
         count+=1;
            }
         printf_tiny("\n\r");

     }
    return 0;
    }


void attherate()                // Function to free all the available buffers
    {
    int i;
    attherate_status=1;

    printf_tiny("\n\rFreeing all the buffers\n\r");
    for(i=0;i<buffercount;i++)
        {
        free(buffer_arr[i]);
        tot_size[buffercount]=0; // Making the size of the freed buffer to 0
        buffer0_NOE=0;           // Making the number of elements of the freed buffer to 0
        status[i]=0;             // Changing the status of the freed buffer to 0

        }
    }
