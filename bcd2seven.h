/*
 ============================================================================
 Name        : Topic3.c
 Author      :
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ********** Number Conversion and Bitshift Operators **********

void decimal2Binary(int input, char output[])
{
	int result = -1;
	int bit = strlen(output) - 1;

	// While last Quotient is not 0
	while(result != 0)
	{
		// Get Quotient and Remainder
		result = input/2;
		int remainder = input % 2;
		// Set a 1 or 0 bit
		output[bit] = remainder ? '1' : '0';
		// Next bit (note that MSB is the bottom of the array) and set input for next iteration
		--bit;
		input = result;
	}
}

int binary2Decimal(char input[])
{
	int output = 0;
	int power = 1;

	// Loops thru bit array and convert to decimal value
	for(int x=strlen(input)-1;x >=0;--x)
	{
		output = output + ((input[x] - 48) * power);
		power = power * 2;
	}

	// Return decimal value
	return output;
}

void binary2Hex(int input, char output[])
{
	char digit[2] = "0\0";
	int nibbles = strlen(output);

	while(nibbles > 0)
	{
		// Convert nibble to hex
		sprintf(&digit[0], "%1x", input & 0x0F);
		output[nibbles - 1] = digit[0];
		// Next nibble
		--nibbles;
		input = input >> 4;
	}
}

// ********** Logic Gates **********

/*
 * AND Gate
 */
int and(int b1, int b2)
{
	return (b1 & 1) & (b2 & 1);
}

/*
 * OR Gate
 */
int or(int b1, int b2)
{
	return (b1 & 1) | (b2 & 1);
}

/*
 * NOT Gate
 */
int not(int b1)
{
	return (b1 & 1) ? 0 : 1;
}

/*
 * NAND Gate
 */
int nand(int b1, int b2)
{
	return not(and(b1, b2));
}

/*
 * NOR Gate
 */
int nor(int b1, int b2)
{
	return not(or(b1, b2));
}

/*
 * XOR Gate
 */
int xor(int b1, int b2)
{
	return (b1 & 1) == (b2 & 1) ? 0 : 1;
}

// ********** BCD to 7-Segment Logic Design **********

int bcd2sevenSegmentDecoder(int ABCD)
{
	// ABCD is the input BCD number
	char output[5] = {"0000\0"};
	decimal2Binary(ABCD, output);
	int A = output[0] - 48;
	int B = output[1] - 48;
	int C = output[2] - 48;
	int D = output[3] - 48;

	// K-Map for the logic design (a is the lsb of the output)
	// a = (~B * ~D) + C + (B * D) + A
	// int a = or(or(or(and(not(B), not(D)), C), and(B, D)), A);
	int a = (~B & 0x01 & ~D & 0x01) | C | (B & D) | A;

	// b = ~B  + (~C * ~D) + (C * D)
	// int b = or(or(not(B), and(not(C), not(D))), and(C, D));
	int b = (~B & 0x01) | (~C & 0x01 & ~D & 0x01) | (C & D);

	// c = ~C + D + B
	// int c = or(or(not(C), D), B);
	int c = (~C & 0x01) | D | B;

	// d = (~B * ~D) + (~B * C) + (B * ~C * D) + (C * ~D) + A
	// int d = or(or(or(or(and(not(B), not(D)), and(not(B), C)), and(and(B, not(C)), D)), and(C, not(D))), A);
	int d = (~B & 0x01 & ~D & 0x01) | (~B & 0x01 & C) | (B & (~C & 0x01) & D) | (C & ~D & 0x01) | A;

	// e = (~B * ~D) + (C * ~D)
	// int e = or(and(not(B), not(d)), and(C, not(D)));
	int e = (~B & 0x01 & ~D & 0x01) | (C & ~D & 0x01);

	// f = (~C * ~D) + (B * ~C) + (B * ~D) + A
	// int f = or(or(or(and(not(C), not(D)), and(B, not(C))), and(B, not(D))), A);
	int f = (~C & 0x01 & ~D & 0x01) | (B & ~C & 0x01) | (B & ~D & 0x01) | A;

	// g = (~B * C) + (B * ~C) + A + (B * ~D)
	// int g = or(or(or(and(not(B), C), and(B, not(C))), A), and(B, not(D)));
	int g = (~B & 0x01 & C) | (B & ~C & 0x01) | A | (B & ~D & 0x01);

	// Calculate return value by converting each bit to a Binary Array and then convert that to a decimal number
	char values[9];
	values[0] = 48;
	values[1] = g + 48;
	values[2] = f + 48;
	values[3] = e + 48;
	values[4] = d + 48;
	values[5] = c + 48;
	values[6] = b + 48;
	values[7] = a + 48;
	values[8] = 0;
	return binary2Decimal(values);
}
