/*
============================================
Name		:	Nibardo Reyes Felix
Author		:	Me
Version		:	2.0
Description	:	Alarm Clock Application
============================================
*/

#include "lcd/terasic_os_includes.h"
#include "lcd/LCD_Lib.h"
#include "lcd/lcd_graphic.h"
#include "lcd/font.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/mman.h>
#include "hwlib.h"
#include "socal/socal.h"
#include "socal/hps.h"
#include "socal/alt_gpio.h"
#include "./address_map_arm.h"
#include "bcd2seven.h"
#include "clock.h"

volatile signed int *HEX_ptr2; // Global definition of HEX_ptr2
//===Defines====
#define HW_REGS_BASE ( ALT_STM_OFST )
#define HW_REGS_SPAN ( 0x04000000 )
//============
//===Variables====
int open_physical (int);
void * map_physical (int, unsigned int, unsigned int);
void close_physical (int);
int unmap_physical (void *, unsigned int);
volatile signed int * LEDR_ptr;   // virtual address pointer to red LEDs
volatile signed int * HEX_ptr;	//virutal address pointer to Hex Dispay
volatile signed int * SW_ptr;	///address pointer to the switches
volatile signed int *KEY_ptr;     // virtual address for the KEY port
//==================
// Base address of the GPIO connected to LEDs (adjust according to your board's specifications)
#define GPIO_LED_BASE 0xFF200000  // Example base address

// Offset to control the LEDs
#define LED_OFFSET 0x00000000     // Example offset for LED control

// Structure to represent a 32-bit register
typedef struct {
    unsigned int data;
} GPIO_Reg;


void checkAlarm(int *alarmHour, int *alarmMinute, int *newHour, int *newMinute) {
    static int ledState = 0; // State variable to toggle LEDs
    int i;

    if (*newHour  == *alarmHour && *newMinute == *alarmMinute) {
        printf("In the alarm!\n");

        // Toggle LEDs multiple times as an indicator
        for (i = 0; i < 5; i++) {
            // Turn all 10 LEDs on
            *LEDR_ptr = 0x3FF; // Assuming 10 LEDs are controlled by LEDR_ptr

            usleep(500000); // Sleep for 0.5 seconds

            // Turn all 10 LEDs off
            *LEDR_ptr = 0x0;

            usleep(500000); // Sleep for 0.5 seconds
        }
        // Reset the LED state after the alarm
        ledState = 0;
    }
}


int main(void)
{

   int fd = -1;               // used to open /dev/mem for access to physical addresses
   void *LW_virtual;          // used to map physical addresses for the light-weight bridge
   LCD_CANVAS LcdCanvas;
   GPIO_Reg *gpio_led;


   // Open /dev/mem to give access to physical addresses
   fd = open_physical(fd);
   if (fd == -1)
       return (-1);

   // Create virtual memory access to the FPGA light-weight bridge
   LW_virtual = map_physical(fd, LW_BRIDGE_BASE, LW_BRIDGE_SPAN);
   if (LW_virtual == NULL) {
       close_physical(fd);
       return (-1);
   }

   //====Stuff for 7-Segment
   // Set virtual address pointer to I/O port
      LEDR_ptr = (signed int *) (LW_virtual + LEDR_BASE);
      HEX_ptr = (signed int *)(LW_virtual + HEX3_HEX0_BASE);
      HEX_ptr2 = (signed int *)(LW_virtual + HEX5_HEX4_BASE);
      SW_ptr = (signed int *)(LW_virtual + SW_BASE);//switch
      KEY_ptr =(signed int *)( LW_virtual + KEY_BASE);    // in it virtual address for KEY port


      //Registers=============
      DataRegister dataRegister;
      dataRegister.value = 0;
      DataRegister2 dataRegister2;
      dataRegister2.value = 0;

      PushButton button;
      button.value = 0;
      Switches switch1;
      switch1.value = 0;


///==============================

//======LCD Stuff========
  	void *virtual_base;
  	virtual_base = mmap( NULL, HW_REGS_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, HW_REGS_BASE );

//========================

   //===========================================
   //LCD Intro Message
   //================================
   //printf("Can you see LCD?(CTRL+C to terminate this program)\r\n");
   	printf("Graphic LCD Demo\n");

   		LcdCanvas.Width = LCD_WIDTH;
   		LcdCanvas.Height = LCD_HEIGHT;
   		LcdCanvas.BitPerPixel = 1;
   		LcdCanvas.FrameSize = LcdCanvas.Width * LcdCanvas.Height / 8;
   		LcdCanvas.pFrame = (void *)malloc(LcdCanvas.FrameSize);

   	if (LcdCanvas.pFrame == NULL){
   			printf("failed to allocate lcd frame buffer\r\n");
   	}else{


   		LCDHW_Init(virtual_base);
   		LCDHW_BackLight(true); // turn on LCD backlight

       LCD_Init();
       // initialize the pio controller
      // clear screen
       DRAW_Clear(&LcdCanvas, LCD_WHITE);

   		// demo grphic api
       DRAW_Rect(&LcdCanvas, 0,0, LcdCanvas.Width-1, LcdCanvas.Height-1, LCD_BLACK); // rectangle
       //usleep(5* 1000000);//this code waits 5 seconds
       // clear screen
         // DRAW_Clear(&LcdCanvas, LCD_WHITE);//clears the screen of old messege
          DRAW_PrintString(&LcdCanvas, 30, 5, "Use Buttons", LCD_BLACK, &font_16x16);
          DRAW_PrintString(&LcdCanvas, 30, 5+16, "To Set", LCD_BLACK, &font_16x16);
          DRAW_PrintString(&LcdCanvas, 30, 5+32, "The Clock", LCD_BLACK, &font_16x16);
          DRAW_Refresh(&LcdCanvas);


          free(LcdCanvas.pFrame);
   	}
   	//============================================================
   	//===Start up LCD(set 7 segments to 0) that says to set time in hours and minutes===
   	//============================================================
   	*LEDR_ptr = 0;
   	*HEX_ptr = 0;
   	int counter5 = 0;
   	int counter4 = 0;
   	int counter3 = 0;
   	int counter2 = 0;
   	int alarm5 = 0;
   	int alarm4 = 0;
   	int alarm3 = 0;
   	int alarm2 = 0;
   	// Define variables outside the loop
   	int newHour = 0;
   	int newMinute = 0;
   	int newSecond = 0;
   	int alarmHour = 0;
   	int alarmMinute = 0;
   	while(1){
   	//read first switch orientation
   	switch1.value = *SW_ptr;
   	   if(switch1.bits.sw0 == 0){
   		dataRegister.bits.hex1 = bcd2sevenSegmentDecoder(0);
   		dataRegister.bits.hex0 = bcd2sevenSegmentDecoder(0);
   		dataRegister.bits.hex3 = bcd2sevenSegmentDecoder(0);
   		dataRegister.bits.hex2 = bcd2sevenSegmentDecoder(0);
   		dataRegister2.bits.hex5 = bcd2sevenSegmentDecoder(0);
   		dataRegister2.bits.hex4 = bcd2sevenSegmentDecoder(0);
   		*HEX_ptr = dataRegister.value;
   		*HEX_ptr2 = dataRegister2.value;
   		//====Setting up clock=====
   		//display 5
   		button.value = *KEY_ptr;
   		        if (button.bits.key3 == 1) {
   		            // Increment the counter if the button is pressed
   		            counter5++;
   		        }
   		         dataRegister2.bits.hex5 = bcd2sevenSegmentDecoder(counter5);
   		      *HEX_ptr2 = dataRegister2.value;
   		            if (counter5 == 3) {
   		                counter5 = 0; // Reset the counter to 0 when it reaches 10
   		            }
   		            //display 4
			 if (button.bits.key2 == 1) {
			// Increment the counter if the button is pressed
			counter4++;
			if (counter5 == 2){
				if(counter4 == 5){
					counter4 = 0;
				}
			}
			}
			dataRegister2.bits.hex4 = bcd2sevenSegmentDecoder(counter4);
			*HEX_ptr2 = dataRegister2.value;
			if (counter4 == 10) {
			counter4 = 0; // Reset the counter to 0 when it reaches 10
			}
			//display 3
						 if (button.bits.key1 == 1) {
						// Increment the counter if the button is pressed
						counter3++;
						}
						dataRegister.bits.hex3 = bcd2sevenSegmentDecoder(counter3);
						*HEX_ptr = dataRegister.value;
						if (counter3 == 6) {
						counter3 = 0; // Reset the counter to 0 when it reaches 10
						}
						//display 2
									 if (button.bits.key0 == 1) {
									// Increment the counter if the button is pressed
									counter2++;
									}
									dataRegister.bits.hex2 = bcd2sevenSegmentDecoder(counter2);
									*HEX_ptr = dataRegister.value;
									if (counter2 == 10) {
									counter2 = 0; // Reset the counter to 0 when it reaches 10
									}

   	   }

//The switch has been flipped and time can start
   	   if (switch1.bits.sw0 == 1){
   		  int newHour0 = combineNumbers(counter5, counter4);
   		  int newMinute0 = combineNumbers(counter3, counter2);
   		  int newSecond0 = combineNumbers(0, 0);

   		  startClock(newHour0, newMinute0, newSecond0);
   		while (1) {
   		switch1.value = *SW_ptr;
           newHour = getHour2();
           newMinute = getMinute2();
           newSecond = getSecond2();
   		//==============================================
   		   //clock functionality
   		   // Check if seconds reach 60, then increment minutes
           // Update time (for demonstration, incrementing seconds)
           newSecond++;
           if (newSecond >= 60) {
               newSecond = 0;
               newMinute++;
               if (newMinute >= 60) {
                   newMinute = 0;
                   newHour++;
                   if (newHour >= 24) {
                       newHour = 0;
                   }

               }

           }
           setHour(newHour); // Update hours
           setMinute(newMinute); // Update minutes
           setSecond(newSecond); // Update seconds

           // Update the 7-segment displays with the new time values
           int hour11 = newHour / 10;
           int hour22 = newHour % 10;
           int min11 = newMinute / 10;
           int min22 = newMinute % 10;
           int sec11 = newSecond / 10;
           int sec22 = newSecond % 10;

           // Assuming dataRegister and dataRegister2 are used for the 7-segment displays
           dataRegister.bits.hex1 = bcd2sevenSegmentDecoder(sec11);
           dataRegister.bits.hex0 = bcd2sevenSegmentDecoder(sec22);
           dataRegister.bits.hex3 = bcd2sevenSegmentDecoder(min11);
           dataRegister.bits.hex2 = bcd2sevenSegmentDecoder(min22);
           dataRegister2.bits.hex5 = bcd2sevenSegmentDecoder(hour11);
           dataRegister2.bits.hex4 = bcd2sevenSegmentDecoder(hour22);

           *HEX_ptr = dataRegister.value; // Update the 7-segment display 1
           *HEX_ptr2 = dataRegister2.value; // Update the 7-segment display 2

   		//====Time Variables===
   			   	//PACIFIC TIME
   			   	int hour1 = newHour /10;//first digit
   			   	int hour2 = newHour % 10;//second digit

   			   	//EASTER TIME
   			   	int hour1e = ((newHour + 3) % 24) /10;//first digit
   			   	int hour2e = ((newHour + 3) % 24) % 10;//second digit
   			   	//ATLANTIC TIME
   			   	int hour1a = ((newHour - 4 + 24) % 24) / 10;
   			   	int hour2a= ((newHour - 4 + 24) % 24) % 10 ;
   			   	//CENTRAL TIME
   			   	int hour1c = ((newHour + 2) % 24) / 10;
   			   	int hour2c =((newHour + 2) % 24) % 10;
   			   	//minute and seconds are the same
   		   	   	int min1 = newMinute / 10;
   		   	   	int min2 = newMinute % 10;
   		   	   	int sec1 = newSecond / 10;
   		   	   	int sec2 = newSecond % 10;

   		 //======Pacific Time set by user
   		        	dataRegister.bits.hex1 = bcd2sevenSegmentDecoder(sec1);
   		   	   		dataRegister.bits.hex0 = bcd2sevenSegmentDecoder(sec2);
   		   	   		dataRegister.bits.hex3 = bcd2sevenSegmentDecoder(min1);
   		   	   		dataRegister.bits.hex2 = bcd2sevenSegmentDecoder(min2);
   		   	   		dataRegister2.bits.hex5 = bcd2sevenSegmentDecoder(hour1);
   		   	   		dataRegister2.bits.hex4 = bcd2sevenSegmentDecoder(hour2);
   		   	   		*HEX_ptr = dataRegister.value;
   		   	   		*HEX_ptr2 = dataRegister2.value;
   		   	  //Eastern time
   		   	     	   if (switch1.bits.sw0 == 1 && switch1.bits.sw1 == 1){
   		   	     		dataRegister.bits.hex1 = bcd2sevenSegmentDecoder(sec1);
   		   	     		dataRegister.bits.hex0 = bcd2sevenSegmentDecoder(sec2);
   		   	     		dataRegister.bits.hex3 = bcd2sevenSegmentDecoder(min1);
   		   	     		dataRegister.bits.hex2 = bcd2sevenSegmentDecoder(min2);
   		   	     		dataRegister2.bits.hex5 = bcd2sevenSegmentDecoder(hour1e);
   		   	     		dataRegister2.bits.hex4 = bcd2sevenSegmentDecoder(hour2e);
   		   	     		*HEX_ptr = dataRegister.value;
   		   	     		*HEX_ptr2 = dataRegister2.value;
   		   	     	   }
   		   	     	   //Atlantic time
   		   	     	   if (switch1.bits.sw0 == 1 && switch1.bits.sw2 == 1){
   		   	     		dataRegister.bits.hex1 = bcd2sevenSegmentDecoder(sec1);
   		   	     		dataRegister.bits.hex0 = bcd2sevenSegmentDecoder(sec2);
   		   	     		dataRegister.bits.hex3 = bcd2sevenSegmentDecoder(min1);
   		   	     		dataRegister.bits.hex2 = bcd2sevenSegmentDecoder(min2);
   		   	     		dataRegister2.bits.hex5 = bcd2sevenSegmentDecoder(hour1a);
   		   	     		dataRegister2.bits.hex4 = bcd2sevenSegmentDecoder(hour2a);
   		   	     		*HEX_ptr = dataRegister.value;
   		   	     		*HEX_ptr2 = dataRegister2.value;
   		   	     	   }
   		   	     	   //Central time
   		   	     	   if (switch1.bits.sw0 == 1 && switch1.bits.sw3 == 1){
   		   	     		dataRegister.bits.hex1 = bcd2sevenSegmentDecoder(sec1);
   		   	     		dataRegister.bits.hex0 = bcd2sevenSegmentDecoder(sec2);
   		   	     		dataRegister.bits.hex3 = bcd2sevenSegmentDecoder(min1);
   		   	     		dataRegister.bits.hex2 = bcd2sevenSegmentDecoder(min2);
   		   	     		dataRegister2.bits.hex5 = bcd2sevenSegmentDecoder(hour1c);
   		   	     		dataRegister2.bits.hex4 = bcd2sevenSegmentDecoder(hour2c);
   		   	     		*HEX_ptr = dataRegister.value;
   		   	     		*HEX_ptr2 = dataRegister2.value;

   		}

   			   	     //=====ALARM==
   		   		//read first switch orientation
   		   		   	switch1.value = *SW_ptr;
   			   	        	if (switch1.bits.sw9 == 1){
   			   	        	dataRegister2.value = *HEX_ptr2;
   			   	        	dataRegister.value = *HEX_ptr;
   			   	        	//allow user to set an alarm for pacific time zone
   			   	         dataRegister.bits.hex1 = bcd2sevenSegmentDecoder(0);
   			   	         dataRegister.bits.hex0 = bcd2sevenSegmentDecoder(0);
   			   	         dataRegister.bits.hex3 = bcd2sevenSegmentDecoder(0);
   			   	         dataRegister.bits.hex2 = bcd2sevenSegmentDecoder(0);
   			   	         dataRegister2.bits.hex5 = bcd2sevenSegmentDecoder(0);
   			   	         dataRegister2.bits.hex4 = bcd2sevenSegmentDecoder(0);
   			   	        	*HEX_ptr = dataRegister.value;
   			   	        	*HEX_ptr2 = dataRegister2.value;
   			   	        	//allow buttons to be pressed
   			   	        	button.value = *KEY_ptr;
   			   	        	   		        if (button.bits.key3 == 1) {
   			   	        	   		            // Increment the counter if the button is pressed
   			   	        	   		            alarm5++;
   			   	        	   		        }
   			   	        	   		         dataRegister2.bits.hex5 = bcd2sevenSegmentDecoder(alarm5);
   			   	        	   		      *HEX_ptr2 = dataRegister2.value;
   			   	        	   		            if (alarm5 == 3) {
   			   	        	   		                alarm5 = 0; // Reset the counter to 0 when it reaches 10
   			   	        	   		            }
   			   	        	   		            //display 4
   			   	        				 if (button.bits.key2 == 1) {
   			   	        				// Increment the counter if the button is pressed
   			   	        				alarm4++;
   			   	        				}
   			   	        				dataRegister2.bits.hex4 = bcd2sevenSegmentDecoder(alarm4);
   			   	        				*HEX_ptr2 = dataRegister2.value;
   			   	        				if (alarm4 == 10) {
   			   	        				alarm4 = 0; // Reset the counter to 0 when it reaches 10
   			   	        				}
   			   	        				//display 3
   			   	        							 if (button.bits.key1 == 1) {
   			   	        							// Increment the counter if the button is pressed
   			   	        							alarm3++;
   			   	        							}
   			   	        							dataRegister.bits.hex3 = bcd2sevenSegmentDecoder(alarm3);
   			   	        							*HEX_ptr = dataRegister.value;
   			   	        							if (alarm3 == 6) {
   			   	        							alarm3 = 0; // Reset the counter to 0 when it reaches 10
   			   	        							}
   			   	        							//display 2
   			   	        										 if (button.bits.key0 == 1) {
   			   	        										// Increment the counter if the button is pressed
   			   	        										alarm2++;
   			   	        										}
   			   	        										dataRegister.bits.hex2 = bcd2sevenSegmentDecoder(alarm2);
   			   	        										*HEX_ptr = dataRegister.value;
   			   	        										if (alarm2 == 10) {
   			   	        										alarm2 = 0; // Reset the counter to 0 when it reaches 10

   			   	        										}

   		usleep(1000000); // Sleep for 1 second (1000000 microseconds)
   		}//inner while loop
   			   	      // Check if the current time matches the alarm time
   			   	      if (switch1.bits.sw8 == 1){
   			   	      	alarmHour = combineNumbers(alarm5, alarm4);
   			   	        alarmMinute = combineNumbers(alarm3, alarm2);
   			   	        checkAlarm(&alarmHour, &alarmMinute, &newHour, &newMinute);
   			   	      printf("%d, %d, %d, %d\n", newHour, newMinute, alarmHour, alarmMinute);
   			   	      }
   		}

   	}

   	   sleep(1);
   	   }//end of main while loop


   //=====Leave this at the end for now==========
   unmap_physical (LW_virtual, LW_BRIDGE_SPAN);   // release the physical-memory mapping
   close_physical (fd);   // close /dev/mem
   return 0;
}

// Open /dev/mem, if not already done, to give access to physical addresses
int open_physical (int fd)
{
   if (fd == -1)
      if ((fd = open( "/dev/mem", (O_RDWR | O_SYNC))) == -1)
      {
         printf ("ERROR: could not open \"/dev/mem\"...\n");
         return (-1);
      }
   return fd;
}

// Close /dev/mem to give access to physical addresses
void close_physical (int fd)
{
   close (fd);
}

/*
 * Establish a virtual address mapping for the physical addresses starting at base, and
 * extending by span bytes.
 */
void* map_physical(int fd, unsigned int base, unsigned int span)
{
   void *virtual_base;

   // Get a mapping from physical addresses to virtual addresses
   virtual_base = mmap (NULL, span, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, base);
   if (virtual_base == MAP_FAILED)
   {
      printf ("ERROR: mmap() failed...\n");
      close (fd);
      return (NULL);
   }
   return virtual_base;
}

/*
 * Close the previously-opened virtual address mapping
 */
int unmap_physical(void * virtual_base, unsigned int span)
{
   if (munmap (virtual_base, span) != 0)
   {
      printf ("ERROR: munmap() failed...\n");
      return (-1);
   }
   return 0;
}


