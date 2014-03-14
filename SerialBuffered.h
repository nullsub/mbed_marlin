#ifndef SERIAL_BUFFERED_H_
#define SERIAL_BUFFERED_H_

#include "mbed.h"
#include <string>

#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2
#define BYTE 0


// This is a buffered serial reading class, using the serial interrupt introduced in mbed library version 18 on 17/11/09

// In the simplest case, construct it with a buffer size at least equal to the largest message you
// expect your program to receive in one go.

class SerialBuffered : public Serial {
public:
    SerialBuffered( size_t bufferSize, PinName tx, PinName rx );
    virtual ~SerialBuffered();

    int getc();     // will block till the next character turns up, or return -1 if there is a timeout

    void flush();     //

    int readable(); // returns 1 if there is a character available to read, 0 otherwise

    size_t readBytes( uint8_t *bytes, size_t requested );    // read requested bytes into a buffer,
    // return number actually read,
    // which may be less than requested if there has been a timeout


    private:
    void printNumber(unsigned long, uint8_t);
    void printFloat(double, uint8_t);

	SerialBuffered *pc;

  public:

     void write(const char c)
    {
	putc(c);
    }
     void write(const char *str)
    {
      while (*str)
        write(*str++);
    }


     void write(const uint8_t *buffer, size_t size)
    {
      while (size--)
        write(*buffer++);
    }

     void print(const string &s)
    {
      for (int i = 0; i < (int)s.length(); i++) {
        write(s[i]);
      }
    }

     void print(const char *str)
    {
      write(str);
    }
    void print(char, int = BYTE);
    void print(unsigned char, int = BYTE);
    void print(int, int = DEC);
    void print(unsigned int, int = DEC);
    void print(long, int = DEC);
    void print(unsigned long, int = DEC);
    void print(double, int = 2);

    void println(const string &s);
    void println(const char[]);
    void println(char, int = BYTE);
    void println(unsigned char, int = BYTE);
    void println(int, int = DEC);
    void println(unsigned int, int = DEC);
    void println(long, int = DEC);
    void println(unsigned long, int = DEC);
    void println(double, int = 2);
    void println(void);
private:

    void handleInterrupt();


    uint8_t *m_buff;            // points at a circular buffer, containing data from m_contentStart, for m_contentSize bytes, wrapping when you get to the end
    volatile uint16_t  m_contentStart;   // index of first bytes of content
    volatile uint16_t  m_contentEnd;     // index of bytes after last byte of content
    volatile uint16_t m_buffSize;
};

#endif
