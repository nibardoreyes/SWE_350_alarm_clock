//the integration of the increment leds with 7-segment works well

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "./address_map_arm.h"
#include "bcd2seven.h"

/* Prototypes for functions used to access physical memory addresses */
int open_physical (int);
void * map_physical (int, unsigned int, unsigned int);
void close_physical (int);
int unmap_physical (void *, unsigned int);

//--Methods--
typedef union
{
	unsigned int value;
	struct
	{
		unsigned int hex0:8;	//lsb
		unsigned int hex1:8;
	}bits;
}DataRegister;



/* This program increments the contents of the red LED parallel port */
int main(void)
{
   volatile signed int * LEDR_ptr;   // virtual address pointer to red LEDs
   volatile signed int * HEX_ptr;	//virutal address pointer to Hex Dispay
   int fd = -1;               // used to open /dev/mem for access to physical addresses
   void *LW_virtual;          // used to map physical addresses for the light-weight bridge


   // Create virtual memory access to the FPGA light-weight bridge
   if ((fd = open_physical (fd)) == -1)
      return (-1);
   if ((LW_virtual = map_physical (fd, LW_BRIDGE_BASE, LW_BRIDGE_SPAN)) == NULL)
      return (-1);

   // Set virtual address pointer to I/O port
   LEDR_ptr = (unsigned int *) (LW_virtual + LEDR_BASE);
   HEX_ptr = (unsigned int *)(LW_virtual + HEX3_HEX0_BASE);

   DataRegister dataRegister;
   dataRegister.value = 0;

   // Add 1 to the I/O register
   *LEDR_ptr = 0;
   *HEX_ptr = 0;
   for(int x = 0; x < 20; ++x){
	*LEDR_ptr = *LEDR_ptr + 1;
	dataRegister.bits.hex0 = bcd2sevenSegmentDecoder(x);
	*HEX_ptr = dataRegister.value;
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

//Functions
//Poor mans decimal to bcd converter
int decimal_bcd(int decimal){
	switch(decimal){
	case 0:
			return 0x3f;
	case 1:
			return 0x06;
	case 2:
			return 0x5b;
	case 3:
			return 0x4f;
	case 4:
			return 0x66;
	case 5:
			return 0x6d;
	case 6:
			return 0x7d;
	case 7:
			return 0x07;
	case 8:
			return 0x7f;
	case 9:
			return 0x67;
	default:
			return 0xff;
	}
};

