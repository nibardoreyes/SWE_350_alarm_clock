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
int initialHour = 0;
int initialMinute = 0;
int initialSecond = 0;

volatile signed int * LEDR_ptr;   // virtual address pointer to red LEDs
//First 2 unions are for the 7 segment displays
typedef union
{
	unsigned int value;
	struct
	{
		unsigned int hex0:8;	//lsb
		unsigned int hex1:8;
		unsigned int hex2:8;
		unsigned int hex3:8;


	}bits;
}DataRegister;

typedef union
{
	unsigned int value;
	struct
	{
		unsigned int hex4:8;	//lsb
		unsigned int hex5:8;
		unsigned int unused:16;


	}bits;
}DataRegister2;


typedef union{
	unsigned int value;
	struct{
		unsigned int key0:1;
		unsigned int key1:1;
		unsigned int key2:1;
		unsigned int key3:1;

	}bits;
}PushButton;

//the following union is for the switches
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

//GPIO Bit Structure, 6 nibbles (or 24 bits) are tied to the hex0 - Hex 5 BCD Decoder input
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


int getHour() {
    time_t now;
    struct tm *timeinfo;

    time(&now);
    timeinfo = localtime(&now);
    return timeinfo->tm_hour;
}
int getMinute() {
    time_t now;
    struct tm *timeinfo;

    time(&now);
    timeinfo = localtime(&now);
    return timeinfo->tm_min;
}
int getSecond() {
    time_t now;
    struct tm *timeinfo;

    time(&now);
    timeinfo = localtime(&now);
    return timeinfo->tm_sec;
}

int setHour(int newHour) {
    time_t now;
    struct tm *timeinfo;

    time(&now);
    timeinfo = localtime(&now);
    timeinfo->tm_hour = newHour;
    mktime(timeinfo); // Update the time using mktime function
    return timeinfo->tm_hour;
}

int setMinute(int newMinute) {
    time_t now;
    struct tm *timeinfo;

    time(&now);
    timeinfo = localtime(&now);
    timeinfo->tm_min = newMinute;
    mktime(timeinfo); // Update the time using mktime function
    return timeinfo->tm_min;
}

int setSecond(int newSecond) {
    time_t now;
    struct tm *timeinfo;

    time(&now);
    timeinfo = localtime(&now);
    timeinfo->tm_min = newSecond;
    mktime(timeinfo); // Update the time using mktime function
    return timeinfo->tm_sec;
}

int combineNumbers(int num1, int num2) {
    int combined;

    combined = num1 * 10 + num2;

    return combined;
}

void updateTime(int newHour, int newMinute, int newSecond) {
    setHour(newHour);
    setMinute(newMinute);
    setSecond(newSecond);
}

void fetchUpdatedTime(int *newHour, int *newMinute, int *newSecond) {
    *newHour = getHour();
    *newMinute = getMinute();
    *newSecond = getSecond();
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
