#ifndef SPI_H_

#include <stdint.h>
#include <libopencm3/stm32/gpio.h>

void spi_transfer(void* in_buf, void* out_buf, int length);
void spi_cs_high(void);
void spi_cs_low(void);
void spi_begin(void);
void spi_end(void);

void spi_set(uint16_t spi_gpio);
void spi_out(uint16_t spi_gpio, uint8_t out);
uint8_t spi_in(uint16_t spi_gpio);
void spi_clear(uint16_t spi_gpio);

//PA9
#define CE	GPIO1
//PA10
#define CSN	GPIO0
//PA5
#define SCK	GPIO5
//PA7
#define MOSI GPIO7
//PA6
#define MISO GPIO6
//PORT A
#define SPI_PORT GPIOA
//Below is a hack to allow for PB1 CE due to scarcity of suitable pins
//on the less expensive TSSOP-20 package. Untested
#define CE_PORT GPIOB

#define SPI_H_
#endif