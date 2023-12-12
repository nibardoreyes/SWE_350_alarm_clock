//This header will hold all the necessary includes in order for the clock to run
#ifndef _CLOCK_H_
#define _CLOCK_H_

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/mman.h>
time_t programStartTime;
volatile signed int * LEDR_ptr;   // virtual address pointer to red LEDs
int initialHour = 0;
int initialMinute = 0;
int initialSecond = 0;

//Union for representing the pushbuttons
typedef union{
	unsigned int value;
	struct{
		unsigned int key0:1;
		unsigned int key1:1;
		unsigned int key2:1;
		unsigned int key3:1;

	}bits;
}PushButton;

//Union representing the switches
typedef union{
	unsigned int value;
	struct{
		unsigned int sw0:1;	//lsb-switch on the left
		unsigned int sw1:1;
		unsigned int sw2:1;
		unsigned int sw3:1;
		unsigned int sw4:1;
		unsigned int sw5:1;
		unsigned int sw6:1;
		unsigned int sw7:1;
		unsigned int sw8:1;
		unsigned int sw9:1;
	}bits;
}Switches;

//Union representing the GPIO Bit Structure, 6 nibbles (or 24 bits) are tied to the hex0 - Hex 5 BCD Decoder input
typedef union{
	unsigned int value;
	struct{
	unsigned int gpio0 : 4;
	unsigned int gpio1 : 4;
	unsigned int gpio2 : 4;
	unsigned int gpio3 : 4;
	unsigned int gpio4 : 4;
	unsigned int gpio5 : 4;
	unsigned int gpiou : 11;
	}bits;
}GpioRegister;


int combineNumbers(int num1, int num2) {
    int combined;

    combined = num1 * 10 + num2;

    return combined;
}


void startClock(int hour, int minute, int second) {
    programStartTime = time(NULL); // Record the program start time
    initialHour = hour;
    initialMinute = minute;
    initialSecond = second;
}

int getHour2() {
    time_t now = time(NULL);
    double secondsElapsed = difftime(now, programStartTime);
    return ((int)secondsElapsed / 3600 + initialHour) % 24; // Calculate hours
}

int getMinute2(){
    time_t now = time(NULL);
    double secondsElapsed = difftime(now, programStartTime);
    return ((int)secondsElapsed / 60 + initialMinute) % 60; // Calculate minutes
}

int getSecond2() {
    time_t now = time(NULL);
    double secondsElapsed = difftime(now, programStartTime);
    return ((int)secondsElapsed + initialSecond) % 60; // Calculate seconds
}



#endif
