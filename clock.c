/*
============================================
Name		:	Nibardo Reyes Felix
Author		:	Me
Version		:	4.0
Description	:	Alarm Clock Application
Notes       :   You need to burn the DE10_Standard_Computer_time_limited.sof
				onto the board in order for the application to run properly.
				This file is located in the FPGA folder.
============================================
========Summary about application===========
The program is an alarm clock application designed for a hardware platform with an LCD
screen, buttons, switches, and LEDs. It allows the user to set the current time, view time
in different time zones (Pacific, Eastern, Atlantic, and Central), set alarms, and
activate/deactivate these alarms. User inputs are handled using buttons and switches, and
the current time and alarm status are displayed on the LCD and LEDs. The code handles hardware
interfacing, time management, and user interaction for a comprehensive alarm clock functionality.
*/
//=====Libraries===================
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
/*
Macros and global variables are defined for memory mapping and interfacing
with hardware components like HEX displays, switches, keys, and GPIOs.
*/
//===Macros====
#define HW_REGS_BASE ( ALT_STM_OFST )
#define HW_REGS_SPAN ( 0x04000000 )
//============
//===Variables/Functions====
int open_physical (int);
void * map_physical (int, unsigned int, unsigned int);
void close_physical (int);
int unmap_physical (void *, unsigned int);
void *virtual_base;
void displayMessageOnLCD(char* line1, char* line2, char* line3, char* line4);
void checkAlarm(int *alarmHour, int *alarmMinute, int *newHour, int *newMinute);
void updateDisplay(int hour, int minute, int second);
volatile signed int * HEX_ptr;	//virutal address pointer to Hex Dispay
volatile signed int *HEX_ptr2; // Global definition of HEX_ptr2
volatile signed int * SW_ptr;	///address pointer to the switches
volatile signed int *KEY_ptr;     // virtual address for the KEY port
volatile unsigned int *JP1_ptr;
LCD_CANVAS LcdCanvas;
GpioRegister gpioRegister1;
/*==============================
=======MAIN=====================
================================
This is the main execution block. It initializes hardware mappings, sets up the LCD display, and
enters a loop where it continuously updates the time and checks for user inputs using
buttons and switches. The switches are used to set time, change time zones, and set/activate alarms
================================
*/
int main(void)
{

   int fd = -1;               // used to open /dev/mem for access to physical addresses
   void *LW_virtual;          // used to map physical addresses for the light-weight bridge


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

   // Set virtual address pointer to I/O port
      LEDR_ptr = (signed int *) (LW_virtual + LEDR_BASE);
      HEX_ptr = (signed int *)(LW_virtual + HEX3_HEX0_BASE);
      HEX_ptr2 = (signed int *)(LW_virtual + HEX5_HEX4_BASE);
      SW_ptr = (signed int *)(LW_virtual + SW_BASE);//switch
      KEY_ptr =(signed int *)( LW_virtual + KEY_BASE);    // in it virtual address for KEY port
      JP1_ptr = (unsigned int *)( LW_virtual + JP1_BASE);


      //===================
      PushButton button;
      button.value = 0;
      Switches switch1;
      switch1.value = 0;
      *(JP1_ptr+ 1) = 0x00FFFFFF;
      gpioRegister1 = (GpioRegister)(*JP1_ptr);



///==============================
//Need this do not erase
virtual_base = mmap( NULL, HW_REGS_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, HW_REGS_BASE );
//========================
   //===========================================
   //LCD Intro Message
   //================================
   //printf("Can you see LCD?(CTRL+C to terminate this program)\r\n");
   	printf("===Clock Program===\n");
    displayMessageOnLCD("Use Buttons To", "Set PT Then", "Flip SW0 to", "save it");
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
   	//Flags for lcd
   	int flagP = 0;
   	int flagE = 0; // Set the flag to 0 initially
   	int flagA = 0;
   	int flagC = 0;
   	int flagAlarm = 0;
   	int flagNoSwitches = 0;
   	while(1){
   	//read first switch orientation
   	switch1.value = *SW_ptr;
   	   if(switch1.bits.sw0 == 0){
   		updateDisplay(00, 00, 00);
   		//====Setting up clock=====
   		//display 5
   		button.value = *KEY_ptr;
   		        if (button.bits.key3 == 1) {
   		            // Increment the counter if the button is pressed
   		            counter5++;
   		        }
   		     gpioRegister1.bits.gpio5 = counter5;
   		  *JP1_ptr = gpioRegister1.value;
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
			 gpioRegister1.bits.gpio4 = counter4;
			 *JP1_ptr = gpioRegister1.value;

			if (counter4 == 10) {
			counter4 = 0; // Reset the counter to 0 when it reaches 10
			}
			//display 3
						 if (button.bits.key1 == 1) {
						// Increment the counter if the button is pressed
						counter3++;
						}
						 gpioRegister1.bits.gpio3 = counter3;
						 *JP1_ptr = gpioRegister1.value;

						if (counter3 == 6) {
						counter3 = 0; // Reset the counter to 0 when it reaches 10
						}
						//display 2
									 if (button.bits.key0 == 1) {
									// Increment the counter if the button is pressed
									counter2++;
									}
									 gpioRegister1.bits.gpio2 = counter2;
									 *JP1_ptr = gpioRegister1.value;
									if (counter2 == 10) {
									counter2 = 0; // Reset the counter to 0 when it reaches 10
									}

   	   }

//The switch has been flipped and time can start
   	   if (switch1.bits.sw0 == 1){
	     displayMessageOnLCD("Sw1=PT, Sw2=ET", "Sw3=AT, Sw4=CT", "Sw9=setAlarm", "Sw8=activateAlarm");
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
   		//====Time Variables===
           //EASTER TIME
            int hour1e = ((newHour + 3) % 24) ;
            //ATLANTIC TIME
           int hour1a = ((newHour - 4 + 24) % 24);
        	//CENTRAL TIME
            int hour1c = ((newHour + 2) % 24);

//======Pacific Time set by user
  		   	     	   	     	   if (switch1.bits.sw1 == 1 && switch1.bits.sw9 != 1){
  		   	     	   	     		 if (flagP != 1){
  		   	     	   	     		displayMessageOnLCD("Displaying:", "Pacific Time", "Flip Sw1-4", "Only 1 Sw up");
  		   	     	   	     		 }
  		   	     	   	     		 updateDisplay(newHour, newMinute, newSecond);
  		   	     	   	     		 flagP = 1;
  		   	     		   	     	   }else if(switch1.bits.sw1 == 0){
  		   	     		   	     		   flagP = 0;
  		   	     		   	     	   }

//Eastern time
   		   	     	   if (switch1.bits.sw2 == 1 && switch1.bits.sw9 != 1){
   		   	     		 if (flagE != 1){
   		   	     		displayMessageOnLCD("Displaying:", "Eastern Time", "Flip Sw1-4", "Only 1 Sw up");
   		   	     		 }
   		   	     		updateDisplay(hour1e, newMinute, newSecond);
   		   	     		flagE = 1;
   		   	     	   }else if(switch1.bits.sw2 == 0){
   		   	     		   flagE = 0;
   		   	     	   }


//Atlantic time
   		   	     	   if (switch1.bits.sw3 == 1 && switch1.bits.sw9 != 1){
     		   	     		 if (flagA != 1){
     		   	     		displayMessageOnLCD("Displaying:", "Atlantic Time", "Flip Sw1-4", "Only 1 Sw up");
     		   	     		 }

   		   	     		updateDisplay(hour1a, newMinute, newSecond);
   		   	     		flagA = 1;
   	   		   	     	   }else if(switch1.bits.sw3 == 0){
   	   		   	     		   flagA = 0;
   		   	     	   }
//Central time
   		   	     	   if (switch1.bits.sw4 == 1 && switch1.bits.sw9 != 1){
   		   	     		 if (flagC != 1){
   		   	     		displayMessageOnLCD("Displaying:", "Central Time", "Flip Sw1-4", "Only 1 Sw up");
   		   	     		 }

   		   	     		updateDisplay(hour1c, newMinute, newSecond);
   		   	     		flagC = 1;
   	   		   	     	   }else if(switch1.bits.sw4 == 0){
   	   		   	     		   flagC = 0;

   		}
   		   			if(switch1.bits.sw1 == 0 && switch1.bits.sw2 != 1 && switch1.bits.sw3 != 1 && switch1.bits.sw4 != 1 && switch1.bits.sw9 != 1){
 		   	     		 if (flagNoSwitches != 1){
  		   	     		displayMessageOnLCD("Sw1=PT, Sw2=ET", "Sw3=AT, Sw4=CT", "Sw9=setAlarm", "Sw8=activateAlarm");
  		   	     		 }
   		   				updateDisplay(0, 0, 0);
   		   				flagNoSwitches = 1;
   		   			}else if(switch1.bits.sw1 == 1 ||  switch1.bits.sw2 == 1 || switch1.bits.sw3 == 1 || switch1.bits.sw4 == 1 || switch1.bits.sw9 == 1){
   		   	     		   flagNoSwitches = 0;
	}

   			   	 //=====ALARM==
   		   		//read first switch orientation
   		   		   	switch1.value = *SW_ptr;
   			   	        	if (switch1.bits.sw9 == 1){
   			   	        	//allow user to set an alarm for pacific time zone
   			   	         updateDisplay(00, 00, 00);
   			   	   if (flagAlarm != 1){
   			   	    		 displayMessageOnLCD("Make Alarm for", "Pacific Time", "Flip Sw8 to", "set it ");
   			   	    		   	     	   	     		 }
   			   	        	//allow buttons to be pressed
   			   	        	button.value = *KEY_ptr;
   			   	        	   		        if (button.bits.key3 == 1) {
   			   	        	   		            // Increment the counter if the button is pressed
   			   	        	   		            alarm5++;
   			   	        	   		        }
   			   	        	   		gpioRegister1.bits.gpio5 = alarm5;
   			   	        	   	    *JP1_ptr = gpioRegister1.value;
   			   	        	   		            if (alarm5 == 3) {
   			   	        	   		                alarm5 = 0; // Reset the counter to 0 when it reaches 10
   			   	        	   		            }
   			   	        	   		            //display 4
   			   	        				 if (button.bits.key2 == 1) {
   			   	        				// Increment the counter if the button is pressed
   			   	        				alarm4++;
   			   	      			if (alarm5 == 2){
   			   	      				if(alarm4 == 5){
   			   	      					alarm4 = 0;
   			   	      				}
   			   	      			}
   			   	        				}
   			   	        			gpioRegister1.bits.gpio4 = alarm4;
   			   	        			*JP1_ptr = gpioRegister1.value;
   			   	        				if (alarm4 == 10) {
   			   	        				alarm4 = 0; // Reset the counter to 0 when it reaches 10
   			   	        				}
   			   	        				//display 3
   			   	        							 if (button.bits.key1 == 1) {
   			   	        							// Increment the counter if the button is pressed
   			   	        							alarm3++;
   			   	        							}
   			   	        						gpioRegister1.bits.gpio3 = alarm3;
   			   	        						*JP1_ptr = gpioRegister1.value;
   			   	        							if (alarm3 == 6) {
   			   	        							alarm3 = 0; // Reset the counter to 0 when it reaches 10
   			   	        							}
   			   	        							//display 2
   			   	        										 if (button.bits.key0 == 1) {
   			   	        										// Increment the counter if the button is pressed
   			   	        										alarm2++;
   			   	        										}
   			   	        									gpioRegister1.bits.gpio2 = alarm2;
   			   	        									*JP1_ptr = gpioRegister1.value;
   			   	        										if (alarm2 == 10) {
   			   	        										alarm2 = 0; // Reset the counter to 0 when it reaches 10

   			   	        										}

flagAlarm = 1;
usleep(400000);

   			   	        	}else if(switch1.bits.sw9 == 0){
    	   		   	     		   flagAlarm = 0;
   			   	        	}

   			   	      // Check if the current time matches the alarm time
   			   	      if (switch1.bits.sw8 == 1){
   			   	      	alarmHour = combineNumbers(alarm5, alarm4);
   			   	        alarmMinute = combineNumbers(alarm3, alarm2);
   			   	        checkAlarm(&alarmHour, &alarmMinute, &newHour, &newMinute);
   			   	       }
   		}
   	   }
   	   usleep(400000);
   	   }//end of main while loop

   //=====Leave this at the end for now==========
   unmap_physical (LW_virtual, LW_BRIDGE_SPAN);   // release the physical-memory mapping
   close_physical (fd);   // close /dev/mem
   return 0;
}

//=========================================================================
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

//Function that initializes LCD and displays a message consisting of 4 lines
void displayMessageOnLCD(char* line1, char* line2, char* line3, char* line4) {
    // Create and initialize the LCD canvas
    LCD_CANVAS LcdCanvas;
    LcdCanvas.Width = LCD_WIDTH;
    LcdCanvas.Height = LCD_HEIGHT;
    LcdCanvas.BitPerPixel = 1;
    LcdCanvas.FrameSize = LcdCanvas.Width * LcdCanvas.Height / 8;
    LcdCanvas.pFrame = (void *)malloc(LcdCanvas.FrameSize);

    // Initialize LCD hardware
    LCDHW_Init(virtual_base);
    LCDHW_BackLight(true);

    LCD_Init();
    DRAW_Clear(&LcdCanvas, LCD_WHITE);
    DRAW_Rect(&LcdCanvas, 0, 0, LcdCanvas.Width - 1, LcdCanvas.Height - 1, LCD_BLACK);

        // Print the message on the LCD canvas
        DRAW_PrintString(&LcdCanvas, 5, 2, line1, LCD_BLACK, &font_16x16);
        DRAW_PrintString(&LcdCanvas, 5, 2+12, line2, LCD_BLACK, &font_16x16);
        DRAW_PrintString(&LcdCanvas, 5, 2+24, line3, LCD_BLACK, &font_16x16);
        DRAW_PrintString(&LcdCanvas, 5, 2+36, line4, LCD_BLACK, &font_16x16);
        DRAW_Refresh(&LcdCanvas);

        free(LcdCanvas.pFrame);
return;

}
//Function compares the current time with a set alarm time and triggers a sequence of LED flashes if they match.
void checkAlarm(int *alarmHour, int *alarmMinute, int *newHour, int *newMinute) {
    if (*newHour  == *alarmHour && *newMinute == *alarmMinute) {
        // Turn all 10 LEDs on
        *LEDR_ptr = 0x3FF; // Assuming 10 LEDs are controlled by LEDR_ptr
        usleep(250000); // Sleep for 0.5 seconds

        // Turn all 10 LEDs off
        *LEDR_ptr = 0x0;
        usleep(500000); // Sleep for another 0.5 seconds

        // Repeat the on-off cycle
        *LEDR_ptr = 0x3FF; // Turn all LEDs on again
        usleep(240000); // Sleep for 0.5 seconds

        *LEDR_ptr = 0x0; // Turn all LEDs off again
    }
}

//Function tupdates the time display on the hardware, the 7 segment display, with the current time.
void updateDisplay(int hour, int minute, int second) {
    // Convert time values to their respective BCD representations for the display
    int hour10 = hour / 10;
    int hour1 = hour % 10;
    int minute10 = minute / 10;
    int minute1 = minute % 10;
    int second10 = second / 10;
    int second1 = second % 10;

    // Set the BCD values to the display registers
    gpioRegister1.bits.gpio5 = hour10;
    gpioRegister1.bits.gpio4 = hour1;
    gpioRegister1.bits.gpio3 = minute10;
    gpioRegister1.bits.gpio2 = minute1;
    gpioRegister1.bits.gpio1 = second10;
    gpioRegister1.bits.gpio0 = second1;

    // Update the 7-segment displays
    *JP1_ptr = gpioRegister1.value;

}
