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

#define HW_REGS_BASE ( ALT_STM_OFST )
#define HW_REGS_SPAN ( 0x04000000 )
/* Prototypes for functions used to access physical memory addresses */
int open_physical (int);
void * map_physical (int, unsigned int, unsigned int);
void close_physical (int);
int unmap_physical (void *, unsigned int);

int main(void)
{
   volatile signed int * LEDR_ptr;   // virtual address pointer to red LEDs
   volatile signed int * HEX_ptr;	//virutal address pointer to Hex Dispay
   volatile signed int * HEX_ptr2;
   volatile signed int * SW_ptr;	///address pointer to the switches
   int fd = -1;               // used to open /dev/mem for access to physical addresses
   void *LW_virtual;          // used to map physical addresses for the light-weight bridge
   LCD_CANVAS LcdCanvas;

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
      LEDR_ptr = (unsigned int *) (LW_virtual + LEDR_BASE);
      HEX_ptr = (unsigned int *)(LW_virtual + HEX3_HEX0_BASE);
      HEX_ptr2 = (unsigned int *)(LW_virtual + HEX5_HEX4_BASE);

      DataRegister dataRegister;
      dataRegister.value = 0;

      DataRegister2 dataRegister2;
      dataRegister2.value = 0;
///==============================

//======LCD Stuff========
  	void *virtual_base;
  	virtual_base = mmap( NULL, HW_REGS_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, HW_REGS_BASE );

//========================



   //===========================================
   //LCD Intro Message
   //================================
   //printf("Can you see LCD?(CTRL+C to terminate this program)\r\n");
   	printf("Graphic LCD Demo\r\n");

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
       DRAW_Rect(&LcdCanvas, 0,0, LcdCanvas.Width-1, LcdCanvas.Height-1, LCD_BLACK); // retangle
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



   //============================================================
  //===Look at buttons to see if they are being pressed and change 7 segment(save when switch 1 is flipped===
   //============================================================
/*
   SW_ptr = (unsigned int *)(LW_virtual + SW_BASE);	//set virtual pointer to I/O port
   DataRegister3 dataRegister3;
   dataRegister3.value = 0;
   dataRegister3.value = * (SW_ptr);//read the switch values to see which switches are flipped
*/


   //============================================================
  //===Calculate other time zones for different switch orientations===
   //============================================================












   // Add 1 to the I/O register
    *LEDR_ptr = 0;
    *HEX_ptr = 0;

   while(1){
	//test area
	int hour1 = getHour() /10;//first digit
	int hour2 = getHour() % 10;//second digit
	int min1 = getMinute() / 10;
	int min2 = getMinute() % 10;
	int sec1 = getSecond() / 10;
	int sec2 = getSecond() % 10;

	dataRegister.bits.hex1 = bcd2sevenSegmentDecoder(sec1);
	dataRegister.bits.hex0 = bcd2sevenSegmentDecoder(sec2);
	dataRegister.bits.hex3 = bcd2sevenSegmentDecoder(min1);
	dataRegister.bits.hex2 = bcd2sevenSegmentDecoder(min2);
	dataRegister2.bits.hex5 = bcd2sevenSegmentDecoder(hour1);
	dataRegister2.bits.hex4 = bcd2sevenSegmentDecoder(hour2);

	*HEX_ptr = dataRegister.value;
	*HEX_ptr2 = dataRegister2.value;
	sleep(1);
   }

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
