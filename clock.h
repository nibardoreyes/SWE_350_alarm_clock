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
}DataRegister3;



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




#endif
