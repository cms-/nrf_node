#include "spi.h"

void spi_cs_high() {
	spi_set(CSN);
}

void spi_cs_low() {
	spi_clear(CSN);	
}

void spi_begin() {
	spi_cs_low();
}

void spi_end() {
	spi_cs_high();
	//UartSend("\r\n", 2);
}

void spi_transfer(void* in_buf, void* out_buf, int length) {
	volatile int i;
	volatile int j;
	for (i=0; i<length; i++) {
		int send = out_buf ? ((char*)out_buf)[i] : 0;
		int receive=0;
		
		if (send) {
			
			UartSend(" O ", 3);
			
			UartSend(&("0123456789ABCDEF"[send >> 4]),1);
			
			UartSend(&("0123456789ABCDEF"[send & 0xF]),1);
			
		}
		volatile int bit;
		for (bit=0; bit<8; bit++) {
			spi_out(MOSI, send & (1 << (7-bit)));

			receive <<= 1;
			
			if (spi_in(MISO)) {
				receive |= 0x1;
			}					
			
			spi_set(SCK);
			spi_clear(SCK);

		}
		
		if (in_buf) {
			((char*)in_buf)[i] = receive;
			
			UartSend(" I ", 3);
			UartSend(&("0123456789ABCDEF"[receive >> 4]),1);
			UartSend(&("0123456789ABCDEF"[receive & 0xF]),1);			
		}
	}
}

void spi_set(uint16_t spi_gpio) {
	// this is a hack because proto has CE on another port
	if (spi_gpio == CE)
	{
		gpio_set(CE_PORT, spi_gpio);
	}
	else
	{
		gpio_set(SPI_PORT, spi_gpio);
	}
}

void spi_out(uint16_t spi_gpio, uint8_t out) {
	if (out) {
		gpio_set(SPI_PORT, spi_gpio);
	} else {
		gpio_clear(SPI_PORT, spi_gpio);
	}
}

uint8_t spi_in(uint16_t spi_gpio) {
	return gpio_get(SPI_PORT, spi_gpio);
}

void spi_clear(uint16_t spi_gpio) {
	gpio_clear(SPI_PORT, spi_gpio);
}