/*
 * MiniLedMatrixStrap.c
 *
 * Created: 2015/07/23 23:44:48
 *  Author: zgtk-guri
 */ 

#define F_CPU 1200000UL

#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>

#define EEPROM_DATA_LEN_ADDR 0x01

#define EEPROM_BUSY_WAIT_ENABLE

const uint8_t anode[5][4]={

	{2,0,1,0},
	{4,0,3,0},
	{3,1,2,1},
	{3,2,4,1},
	{4,3,4,2},
	
};

const uint8_t cathode[5][4]={
	
	{0,2,0,1},
	{0,4,0,3},
	{1,3,1,2},
	{2,3,1,4},
	{3,4,2,4},
	
};

uint8_t disp[5] = {0};

volatile uint8_t interruptFlag = 0;

volatile uint16_t clock = 0;
volatile uint16_t prevReadTime = 0;

volatile uint8_t nowPos = 0;
volatile uint8_t drawPos = 0;
volatile uint8_t dataLen = 0;

//Functions
//Matrix functions
void m_cls();
void m_point(int x, int y);
//Display Buffer functions
void d_readNextRow();
int d_getPoint(int x, int y);

ISR(TIM0_COMPA_vect){
	interruptFlag = 1;	
}



int main(void)
{
#ifdef EEPROM_BUSY_WAIT_ENABLE
	eeprom_busy_wait();
#endif
	dataLen = eeprom_read_byte((uint8_t *)EEPROM_DATA_LEN_ADDR);
			
	//Timer initialize
	TCCR0A = 0x02;	//CTC
	TCCR0B = 0x02;	//Ratio: 8 f=150,000
	TIMSK0 = 0x04;	//Match A interrupt enable
	
	OCR0A = 50;	// f = 150,000 / 50 = 3,000
	
	sei();
	
	d_readNextRow();
	
    while(1)
    {
		if(interruptFlag){
			interruptFlag = 0;
			if(clock == UINT16_MAX) {
				clock = 0;
			}else{
				clock++;
			}
				
			int x = drawPos % 4;
			int y = drawPos / 4;
				
			m_cls();
				
			if(d_getPoint(x,y)){
				m_point(x, y);
			}
			drawPos++;
			if(drawPos > 19) drawPos = 0;
		}
		
         if(clock - prevReadTime > 400){
			 prevReadTime = clock;
			 d_readNextRow();
		 }
    }
}

void m_cls(){
	DDRB = 0;
	PORTB = 0;
}

void m_point(int x, int y){
	DDRB = (1 << anode[y][x]) | (1 << cathode[y][x]);
	PORTB = (1 << anode[y][x]);
}

void d_readNextRow(){

	uint8_t buf[3] = {0};

#ifdef EEPROM_BUSY_WAIT_ENABLE
	eeprom_busy_wait();
#endif
	
	buf[0] = eeprom_read_byte((uint8_t *)(0x08 + ((nowPos / 2) % dataLen)));
#ifdef EEPROM_BUSY_WAIT_ENABLE
	eeprom_busy_wait();
#endif
	buf[1] = eeprom_read_byte((uint8_t *)(0x08 + ((nowPos / 2 + 1) % dataLen)));
#ifdef EEPROM_BUSY_WAIT_ENABLE
	eeprom_busy_wait();
#endif
	buf[2] = eeprom_read_byte((uint8_t *)(0x08 + ((nowPos / 2 + 2) % dataLen)));
	
	if(nowPos % 2){
		//odd
		disp[0] = buf[0] & 0x0F;
		disp[1] = buf[1] >> 4;
		disp[2] = buf[1] & 0x0F;
		disp[3] = buf[2] >> 4;
		disp[4] = buf[2] & 0x0F;
	}else{
		//even
		disp[0] = buf[0] >> 4;
		disp[1] = buf[0] & 0x0F;
		disp[2] = buf[1] >> 4;
		disp[3] = buf[1] & 0x0F;
		disp[4] = buf[2] >> 4;	
	}
	
	nowPos++;
	
}

int d_getPoint(int x, int y){
	return (disp[y] & (1 << x)) ? 1 : 0; 
}