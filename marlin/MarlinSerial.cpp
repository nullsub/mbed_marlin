/*
   HardwareSerial.cpp - Hardware serial library for Wiring
   Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

   Modified 23 November 2006 by David A. Mellis
   Modified 28 September 2010 by Mark Sproul
 */

#include "Marlin.h"
#include "MarlinSerial.h"
#include "SerialBuffered.h"
// Constructors ////////////////////////////////////////////////////////////////

MarlinSerial::MarlinSerial()
{
	pc = new SerialBuffered( 4096, USBTX, USBRX);
}

// Public Methods //////////////////////////////////////////////////////////////

void MarlinSerial::begin(long baud)
{

}

int MarlinSerial::available()
{
	return pc->readable();
}

void MarlinSerial::end()
{
}



int MarlinSerial::peek(void)
{
	return 0;
}

int MarlinSerial::read(void)
{
	return pc->getc();
}

void MarlinSerial::flush()
{
}




/// imports from print.h

void MarlinSerial::print(char c, int base)
{
	print((long) c, base);
}

void MarlinSerial::print(unsigned char b, int base)
{
	print((unsigned long) b, base);
}

void MarlinSerial::print(int n, int base)
{
	print((long) n, base);
}

void MarlinSerial::print(unsigned int n, int base)
{
	print((unsigned long) n, base);
}

void MarlinSerial::print(long n, int base)
{
	if (base == 0) {
		write(n);
	} else if (base == 10) {
		if (n < 0) {
			print('-');
			n = -n;
		}
		printNumber(n, 10);
	} else {
		printNumber(n, base);
	}
}

void MarlinSerial::print(unsigned long n, int base)
{
	if (base == 0) write(n);
	else printNumber(n, base);
}

void MarlinSerial::print(double n, int digits)
{
	printFloat(n, digits);
}

void MarlinSerial::println(void)
{
	print('\r');
	print('\n');
}

void MarlinSerial::println(const string &s)
{
	print(s);
	println();
}

void MarlinSerial::println(const char c[])
{
	print(c);
	println();
}

void MarlinSerial::println(char c, int base)
{
	print(c, base);
	println();
}

void MarlinSerial::println(unsigned char b, int base)
{
	print(b, base);
	println();
}

void MarlinSerial::println(int n, int base)
{
	print(n, base);
	println();
}

void MarlinSerial::println(unsigned int n, int base)
{
	print(n, base);
	println();
}

void MarlinSerial::println(long n, int base)
{
	print(n, base);
	println();
}

void MarlinSerial::println(unsigned long n, int base)
{
	print(n, base);
	println();
}

void MarlinSerial::println(double n, int digits)
{
	print(n, digits);
	println();
}

// Private Methods /////////////////////////////////////////////////////////////

void MarlinSerial::printNumber(unsigned long n, uint8_t base)
{
	unsigned char buf[8 * sizeof(long)]; // Assumes 8-bit chars.
	unsigned long i = 0;

	if (n == 0) {
		print('0');
		return;
	}

	while (n > 0) {
		buf[i++] = n % base;
		n /= base;
	}

	for (; i > 0; i--)
		print((char) (buf[i - 1] < 10 ?
					'0' + buf[i - 1] :
					'A' + buf[i - 1] - 10));
}

void MarlinSerial::printFloat(double number, uint8_t digits)
{
	// Handle negative numbers
	if (number < 0.0)
	{
		print('-');
		number = -number;
	}

	// Round correctly so that print(1.999, 2) prints as "2.00"
	double rounding = 0.5;
	for (uint8_t i=0; i<digits; ++i)
		rounding /= 10.0;

	number += rounding;

	// Extract the integer part of the number and print it
	unsigned long int_part = (unsigned long)number;
	double remainder = number - (double)int_part;
	print(int_part);

	// Print the decimal point, but only if there are digits beyond
	if (digits > 0)
		print(".");

	// Extract digits from the remainder one at a time
	while (digits-- > 0)
	{
		remainder *= 10.0;
		int toPrint = int(remainder);
		print(toPrint);
		remainder -= toPrint;
	}
}
// Preinstantiate Objects //////////////////////////////////////////////////////

MarlinSerial MSerial;
