#include "systick.h"
#include "uart.h"

#define NUMEVENTS 3

fifo_t SerTXFifo[1];
int32_t SerTXIRQ;
int32_t SerRXIRQ;
//uint32_t SerTXBuffer[TXBUFFERSIZE];
int32_t SerTXDMA;
int32_t SerTXSize;
int32_t SerTXIRQ;
fifo_t SerRXFifo[1];
int32_t SerRXSize;
int32_t SerRXDMA;
int32_t SerRXIRQ;
fifo_t TestingFifo[1];
int32_t TestingDMA;
int32_t TestingSize;

volatile uint32_t TheTime = 0;

eventType events[NUMEVENTS];

int32_t tmp = 1;
volatile uint8_t BufRcv[4];

// ******* qtx_manager *******
// Periodic event that manages transmit queue.
// Inputs: fifo_t pointer
// Outputs: none
void static qtx_manager(fifo_t *fifo)
{
	uint32_t buf;
	uint32_t len;
	
	if ((len = FifoGet(&buf, fifo, 1)))
	//if (1)
	{
		
		Sys_Wait(fifo->dma_flag);
		//gpio_toggle(GPIOA, GPIO10); /* LED2 on/off */
		//uart_qtx_dma(&buf, len); // fold this into fifo block
		//gpio_set(GPIOA, GPIO10);
		fifo->handler_function(&buf, len);
		//FifoDelete(fifo, 1);

	}
}
// ******* qrx_manager *******
// Periodic event that manages receive queue.
// Inputs: fifo_t pointer
// Outputs: none
void static qrx_manager(fifo_t *fifo)
{
	Sys_Wait(fifo->dma_flag);
	//fifo->handler_function((uint32_t*)fifo->bufRcv, 1);
	//gpio_set(GPIOA, GPIO10);
	fifo->handler_function(&fifo->bufRcv, sizeof(fifo->bufRcv));
	//gpio_toggle(GPIOA, GPIO10);
}

void static test_event(void *foo)
{
	gpio_toggle(GPIOA, GPIO10); /* LED2 on/off heartbeast */
}

void static dummy_event(void *foo)
{
	int j;
	for (j=0; j<1000; j++);
}

// ******* Systick_Init *******
// Initializes system management functionality.
void Sys_Init(void) {
	// 48MHz 
	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);

	// 48000000/1000 = 47999 overflows per second - every 1ms one interrupt
	// SysTick interrupt every N clock pulses: set reload to N-1 
	systick_set_reload(RELOADTIME);

	systick_interrupt_enable();
	Sys_InitSema(&SerTXDMA, 1);
	Sys_InitSema(&SerTXSize, 0);
	Sys_InitSema(&SerTXIRQ, 0);
	FifoInit(SerTXFifo, &SerTXDMA, &SerTXSize, &SerTXIRQ, &uart_qtx_dma, FIFO_TX);
	Sys_InitSema(&SerRXDMA, 0);
	Sys_InitSema(&SerRXSize, 0);
	Sys_InitSema(&SerRXIRQ, 0);
	FifoInit(SerRXFifo, &SerRXDMA, &SerRXSize, &SerRXIRQ, &uart_qrx_dma, FIFO_RX);
	Sys_InitSema(&TestingSize, 1);
	Sys_InitSema(&TestingDMA, 1);
	FifoInit(TestingFifo, &TestingDMA, &TestingSize, &SerTXIRQ, &uart_qtx_dma, FIFO_TX);
	Sys_AddPeriodicEvent(&qtx_manager, 1, SerTXFifo);
	Sys_AddPeriodicEvent(&qrx_manager, 1, SerRXFifo);
	Sys_AddPeriodicEvent(&test_event, 1000, TestingFifo);
	//Sys_AddPeriodicEvent(&dummy_event, 100000, &tmp, &tmp);

	// Start counting
	systick_counter_enable();
	
}

void sys_tick_handler(void)
{
	run_periodic_events();
	TheTime++;
}

// ******* SysDelay *******
// Causes a delay to program execution.
// Inputs: number of timer cycles (as set in Systick_Init) to block execution.
//         Interrupts continue to fire during this time.
// Outputs: none
void SysDelay(uint32_t delay)
{
	uint32_t start;
	start = TheTime;
	while ((TheTime - start) < delay);
}

// ******* SysInitSema *******
// Initialize a counting semaphore
// Inputs: pointer to a semaphore
//         initial value of semaphore
// Outputs: none
void Sys_InitSema(int32_t *semaPt, int32_t value) 
{
	(*semaPt) = value;

}

// ******* SysWait *******
// Decrement semaphore, blocking task if less than zero
// Inputs: pointer to a counting semaphore
void Sys_Wait(int32_t *semaPt) 
{
	(*semaPt)--;

}

// ******* SysSignal *******
// Increment semaphore
// Inputs: pointer to a counting semaphore
void Sys_Signal(int32_t *semaPt)
{
	(*semaPt)++;
}

// ******* Sys_AddPeriodicEvent *******
// Adds event to period event table
// Inputs: pointer to a manager function
//         period in milliseconds
//         pointer to a fifo type
// Outputs: nothing
void Sys_AddPeriodicEvent(void(*function)(fifo_t*), uint32_t period_ms, fifo_t *fifo)
{
	int j;
	for (j=0; j<NUMEVENTS; j++)
	{
		if (!events[j].managerFunction)
		{
			events[j].managerFunction = function;
			events[j].interval = period_ms;
			events[j].last = 0;
			events[j].fifo = fifo;
			break;
		}
	}
}

// ******* runPeriodicEvents *******
// Runs periodic event scheduler - temporary stand-in
// Inputs/Outputs: none
void static run_periodic_events(void)
{
	cm_disable_interrupts();
	int j;
	for (j=0; j<NUMEVENTS; j++)
	{

		if ((events[j].fifo->dir) == FIFO_TX) 
		{
			
			//if: reception conditions are true (at or past interval delta, fifo size > 0, dma_flag not busy), run transmit manager
			if ( ((TheTime - events[j].last) >= events[j].interval) && ((*events[j].fifo->size_flag) && (*events[j].fifo->dma_flag)) )  
			{
				//gpio_set(GPIOA, GPIO10);
				//gpio_toggle(GPIOA, GPIO10);
				events[j].managerFunction(events[j].fifo);
				events[j].last = TheTime;
			}			
		}
		// else: check receive manager conditions (at or past interval delta, rx irq has fired, dma_flag not busy) and run if needed
		else
		{
			
			if ((*events[j].fifo->dma_flag > 0))
				// here a positive dma flag indicates a fresh BufRcv to be added
			{
				
				FifoPut(&events[j].fifo->bufRcv, events[j].fifo, sizeof(events[j].fifo->bufRcv));
				//FifoPut(&BufRcv, events[j].fifo, WORDBYTES);
				Sys_Wait(events[j].fifo->dma_flag);
				gpio_set(GPIOA, GPIO10);
			}
			
			if ( ((TheTime - events[j].last) >= events[j].interval) && ((*events[j].fifo->irq_flag > 0) && (*events[j].fifo->dma_flag == 0)) )
			{
				// RXNE interrupt has fired and there are no dma actions happening, so run the manager
				gpio_set(GPIOA, GPIO10);
				Sys_Wait(events[j].fifo->irq_flag);
				events[j].managerFunction(events[j].fifo);
				events[j].last = TheTime;
			}


		}
	}
	TheTime++;
	cm_enable_interrupts();
}

// ******* FifoInit *******
// Initializes a FIFO structure, preparing it for usage.
// Inputs: pointer to fifo_t
//         number of elements in the buffer <- follow-up: may not be necessary
//		   pointer to a DMA flag
//		   pointer to a buffer size flag
//		   pointer to an irq flag
//		   pointer to a handler function (handler is called by qxx_manager)
//		   direction enumerator FIFO_TX or FIFO_RX
// Ouputs: None
void FifoInit(fifo_t *fifo, int32_t *dma_flag, int32_t *size_flag, int32_t *irq_flag, void(*handler)(volatile void *data, int length), fifo_direction_e dir)
{
	//fifo->data = &data;
	fifo->getPt = (uint32_t*)&fifo->data[0];
	fifo->putPt = (uint32_t*)&fifo->data[0];
	
	fifo->dma_flag = dma_flag;
	fifo->size_flag = size_flag;
	fifo->irq_flag = irq_flag;
	fifo->handler_function = handler;
	fifo->dir = dir;
	//fifo->bufRcv = 0x30;

}

// ******* FifoPut *******
// Appends a length of data to a supplied buffer.
// Inputs: pointer to data, pointer to a fifo_t, data length
// Outputs: number of data inserted successfully
uint32_t FifoPut(volatile void *data, fifo_t *fifo, uint32_t length)
{
	cm_disable_interrupts();
	int j;
	uint32_t *nextPutPt;
	nextPutPt = &fifo->putPt[1];
	char *p;
	p = data;
	for (j=0; j < length; j++)
	{
		// testing for end of buffer
		// Check to see if there's space in the buffer
		// full if advancing the putPt to the next element in the buffer is met with the getPt
		// also check for this condition when we reach the buffer length.
		//if (nextPutPt == &fifo->data[fifo->size]) 
		if (nextPutPt == &fifo->data[BUFFERSIZE]) 
		{
			nextPutPt = &fifo->data[0];
		}
		if (nextPutPt == &fifo->getPt)
		{
			cm_enable_interrupts();
			return j; // no room left, return number of chars fetched
		}

		(*fifo->putPt) = *p++;
		Sys_Signal(fifo->size_flag);
		//(*p)++;
		fifo->putPt = &fifo->putPt[1]; // advance to next buffer byte

		// if we reach the buffer size, then wrap the putPt
		if (fifo->putPt == &fifo->data[BUFFERSIZE])
			{
				//gpio_set(GPIOA, GPIO10);
				fifo->putPt = &fifo->data[0];
			}
		
	}
	cm_enable_interrupts();
	return j; // return number of chars added to the buffer
}

// ******* FifoPeek *******
// Retrieves data from a specified buffer non-destructively.
// Inputs: data pointer, fifo_t pointer, and data length.
// Outputs: number of data retrieved
uint32_t FifoPeek(void *data, fifo_t *fifo, uint32_t length)
{
	cm_disable_interrupts();
	uint32_t j;
	char *p;
	uint32_t *tmpGet;
	tmpGet = (uint32_t*) fifo->getPt[1];
	p = data;

	for (j=0; j < length; j++) {
		
		if (tmpGet != (uint32_t*)&fifo->putPt)
		//if (fifo->tail != fifo->head)
		{
			*p++ = (*tmpGet);
			//*p++ = fifo->data[fifo->tail];
			//uart_qtx_dma(fifo->data[fifo->tail], sizeof(fifo->data[fifo->tail]));
			
			tmpGet++;
			//fifo->tail++;
			if (tmpGet == &fifo->data[-1])
			//if (fifo->tail == fifo->size)
			{
				tmpGet = (uint32_t*) &fifo->data[0];
			}

		}
		else
		{
			cm_enable_interrupts();
			
			return j;
		}
	}
	cm_enable_interrupts();
	return j;
}

// ******* FifoDelete *******
// Removes a specified number of elements from the fifo_t provided
// Inputs: fifo_t pointer, length to delete
// Outputs: none
void FifoDelete(fifo_t *fifo, uint32_t length) 
{
	cm_disable_interrupts();
	//return 0;
	cm_enable_interrupts();
}

// ******* FifoGet *******
// Retrieves and removes data from a specified buffer.
// Inputs: data pointer, fifo_t pointer, and data length.
// Outputs: none
uint32_t FifoGet(volatile void *data, fifo_t *fifo, uint32_t length)
{
	cm_disable_interrupts();
	uint32_t j;
	char *p;
	//uint32_t *tmpGet;
	//tmpGet = (uint32_t*) fifo->getPt;
	p = data;

	for (j=0; j < length; j++) {
		
		//if (tmpTail != fifo->head)
		if (fifo->getPt != fifo->putPt)
		{
			//*p++ = fifo->data[tmpTail];
			*p++ = (*fifo->getPt);
			//uart_qtx_dma(fifo->data[fifo->tail], sizeof(fifo->data[fifo->tail]));
			
			//tmpTail++;
			fifo->getPt = &fifo->getPt[1];
			Sys_Wait(fifo->size_flag);
			//if (tmpTail == fifo->size)
			if (fifo->getPt == &fifo->data[BUFFERSIZE])
			{
				fifo->getPt = &fifo->data[0];
			}

		}
		else
		{
			cm_enable_interrupts();
			
			return j;
		}
	}
	cm_enable_interrupts();
	return j;
}

