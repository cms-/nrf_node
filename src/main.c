#include "uart.h"
#include "systick.h"
#include "rf.h"
#include "spi.h"
#define PORT_LED GPIOA
#define PIN_LED GPIO10
#define DIRECTION NRF_DIR_TX

int main(void)
{

	rcc_init();
	gpio_init();
	nvic_init();
	dma_init();
	uart_init();
	Sys_Init();
	
	//rf_init();
	UartSend("\nHello\n",7);
	SysDelay(1000);
	rf_init(DIRECTION);
	SysDelay(1000);
	rf_flush_all();
	rf_clear_irq();
	UartSend("\n",1);

	volatile int s;
	volatile int f;
	volatile int i;
	volatile int16_t j;
	volatile char rcv[10];
	volatile char seq[10] = "0123456789";

	s = rf_read_reg_byte(NRF_REG_RF_CH);
	UartSend("\n", 1);

	while (1) {
		

		if (DIRECTION == NRF_DIR_TX)
		{
			UartSend("\n------------", 13);
			for (i=0; i<sizeof(seq); i++)
			{
				SysDelay(1000);

				//gpio_toggle(PORT_LED, PIN_LED);
				UartSend("\nWRITE: ",8);
				rf_write_tx_payload(&seq[i], 1);
				spi_set(CE);
				SysDelay(1000);
				UartSend("\nSTATUS: ",9);
				s = rf_read_reg_byte(NRF_REG_STATUS);
				SysDelay(1000);
				UartSend("\nRF_FIFO: ",10);
				f = rf_fifo_status();
				SysDelay(1000);
				//rf_flush_all();
				//rf_clear_irq();
			}
			UartSend("\nFLUSH: ",8);
			rf_flush_all();
			UartSend("\n",1);
			SysDelay(500);
			rf_clear_irq();
			spi_clear(CE);
			SysDelay(3000);
		}

		if (DIRECTION == NRF_DIR_RX)
		{
			s = rf_status();

			if (s & NRF_STATUS_RX_DR) {
				do {
					rf_read_rx_payload(&rcv, 1);
					uart_send(&rcv,1);
					uart_send("\n",1);
					SysDelay(500);
				
				} while (!(rf_fifo_status() & NRF_FIFO_RX_EMPTY));
				rf_clear_rx_irq();
				rf_flush_all();
			}
		}

	}

	return 0;
}