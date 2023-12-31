#include "lcd/terasic_os_includes.h"
#include "lcd/LCD_Lib.h"
#include "lcd/lcd_graphic.h"
#include "lcd/font.h"
#include "hwlib.h"
#include "socal/socal.h"
#include "socal/hps.h"
#include "socal/alt_gpio.h"

#define HW_REGS_BASE ( ALT_STM_OFST )
#define HW_REGS_SPAN ( 0x04000000 )
#define HW_REGS_MASK ( HW_REGS_SPAN - 1 )

#define USER_IO_DIR     (0x01000000)
#define BUTTON_MASK     (0x02000000)


void introMessege() {

	void *virtual_base;
	int fd;
	
	LCD_CANVAS LcdCanvas;

		
	// map the address space for the LED registers into user space so we can interact with them.
	// we'll actually map in the entire CSR span of the HPS since we want to access various registers within that span
	if( ( fd = open( "/dev/mem", ( O_RDWR | O_SYNC ) ) ) == -1 ) {
		printf( "ERROR: could not open \"/dev/mem\"...\n" );
		return( 1 );
	}

	virtual_base = mmap( NULL, HW_REGS_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, HW_REGS_BASE );



	if( virtual_base == MAP_FAILED ) {
		printf( "ERROR: mmap() failed...\n" );
		close( fd );
		return( 1 );
	}


		
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
    DRAW_Circle(&LcdCanvas, 10, 10, 6, LCD_BLACK);
    DRAW_Circle(&LcdCanvas, LcdCanvas.Width-10, 10, 6, LCD_BLACK);
    DRAW_Circle(&LcdCanvas, LcdCanvas.Width-10, LcdCanvas.Height-10, 6, LCD_BLACK);
    DRAW_Circle(&LcdCanvas, 10, LcdCanvas.Height-10, 6, LCD_BLACK);
    
    // demo font
    DRAW_PrintString(&LcdCanvas, 40, 5, "Hello", LCD_BLACK, &font_16x16);
    DRAW_PrintString(&LcdCanvas, 40, 5+16, "SoCFPGA", LCD_BLACK, &font_16x16);
	DRAW_PrintString(&LcdCanvas, 40, 5+32, "Terasic ", LCD_BLACK, &font_16x16);
    DRAW_Refresh(&LcdCanvas);
    
    //update for activity
    usleep(5* 1000000);
    // clear screen
       DRAW_Clear(&LcdCanvas, LCD_WHITE);
       DRAW_PrintString(&LcdCanvas, 40, 5, "GCU", LCD_BLACK, &font_16x16);
       DRAW_PrintString(&LcdCanvas, 40, 5+16, "CSET", LCD_BLACK, &font_16x16);
       DRAW_PrintString(&LcdCanvas, 40, 5+32, "Rocks!", LCD_BLACK, &font_16x16);
       DRAW_Refresh(&LcdCanvas);


       free(LcdCanvas.pFrame);
	}



	// clean up our memory mapping and exit
	
	if( munmap( virtual_base, HW_REGS_SPAN ) != 0 ) {
		printf( "ERROR: munmap() failed...\n" );
		close( fd );
	}

	close( fd );

}
