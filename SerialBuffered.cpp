
#include "mbed.h"
#include "SerialBuffered.h"

SerialBuffered::SerialBuffered( size_t bufferSize, PinName tx, PinName rx ) : Serial(  tx,  rx ) {
    m_buffSize = 0;
    m_contentStart = 0;
    m_contentEnd = 0;
    m_timeout = 1.0;


    attach( this, &SerialBuffered::handleInterrupt );

    m_buff = (uint8_t *) malloc( bufferSize );
    if ( m_buff == NULL ) {
        //loggerSerial.printf("SerialBuffered - failed to alloc buffer size %d\r\n", (int) bufferSize );
    } else {
        m_buffSize = bufferSize;
    }
}


SerialBuffered::~SerialBuffered() {
    if ( m_buff )
        free( m_buff );
}

void SerialBuffered::setTimeout( float seconds ) {
    m_timeout = seconds;
}

void SerialBuffered::flush( ) {
}

size_t SerialBuffered::readBytes( uint8_t *bytes, size_t requested ) {
    unsigned int i = 0;

    for ( ; i < requested; ) {
        int c = getc();
        if ( c < 0 )
            break;
        bytes[i] = c;
        i++;
    }
    return i;
}

int SerialBuffered::getc() {
    m_timer.reset();
    m_timer.start();
    while ( m_contentStart == m_contentEnd ) {


        wait_ms( 1 );
        if ( m_timeout > 0 &&  m_timer.read() > m_timeout )
            return EOF;
    }

    m_timer.stop();

    uint8_t result = m_buff[m_contentStart++];
    m_contentStart =  m_contentStart % m_buffSize;


    return result;
}

int SerialBuffered::readable() {
    return m_contentStart != m_contentEnd ;
}

void SerialBuffered::handleInterrupt() {

    while ( Serial::readable()) {
        if ( m_contentStart == (m_contentEnd +1) % m_buffSize) {
            //loggerSerial.printf("SerialBuffered - buffer overrun, data lost!\r\n" );
            Serial::getc();
        } else {
            m_buff[ m_contentEnd ++ ] = Serial::getc();
            m_contentEnd = m_contentEnd % m_buffSize;
        }
    }
}

/// imports from print.h

void SerialBuffered::print(char c, int base)
{
	print((long) c, base);
}

void SerialBuffered::print(unsigned char b, int base)
{
	print((unsigned long) b, base);
}

void SerialBuffered::print(int n, int base)
{
	print((long) n, base);
}

void SerialBuffered::print(unsigned int n, int base)
{
	print((unsigned long) n, base);
}

void SerialBuffered::print(long n, int base)
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

void SerialBuffered::print(unsigned long n, int base)
{
	if (base == 0) write(n);
	else printNumber(n, base);
}

void SerialBuffered::print(double n, int digits)
{
	printFloat(n, digits);
}

void SerialBuffered::println(void)
{
	print('\r');
	print('\n');
}

void SerialBuffered::println(const string &s)
{
	print(s);
	println();
}

void SerialBuffered::println(const char c[])
{
	print(c);
	println();
}

void SerialBuffered::println(char c, int base)
{
	print(c, base);
	println();
}

void SerialBuffered::println(unsigned char b, int base)
{
	print(b, base);
	println();
}

void SerialBuffered::println(int n, int base)
{
	print(n, base);
	println();
}

void SerialBuffered::println(unsigned int n, int base)
{
	print(n, base);
	println();
}

void SerialBuffered::println(long n, int base)
{
	print(n, base);
	println();
}

void SerialBuffered::println(unsigned long n, int base)
{
	print(n, base);
	println();
}

void SerialBuffered::println(double n, int digits)
{
	print(n, digits);
	println();
}

// Private Methods /////////////////////////////////////////////////////////////

void SerialBuffered::printNumber(unsigned long n, uint8_t base)
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

void SerialBuffered::printFloat(double number, uint8_t digits)
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
