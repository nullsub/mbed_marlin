
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
