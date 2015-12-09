#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

#define CONTROL_1	0x00
	#define ACKS	7
	#define EXDF	6
	#define ECS		5
	#define DIF2	3
	#define DIF1	2
	#define DIF0	1
	#define RSTN	0

#define CONTROL_2	0x01
	#define DZFE	7
	#define DZFM	6
	#define SD		5
	#define DFS1	4
	#define DFS0	3
	#define DEM1	2
	#define DEM0	1
	#define SMUTE	0

#define CONTROL_3	0x02
	#define DP		7
	#define DCKS	5
	#define DCKB	4
	#define MONO	3
	#define DZFB	2
	#define SELLR	1
	#define SLOW	0

#define Lch_ATT		0x03
	#define ATT7	7
	#define ATT6	6
	#define ATT5	5
	#define ATT4	4
	#define ATT3	3
	#define ATT2	2
	#define ATT1	1
	#define ATT0	0

#define Rch_ATT		0x04
	#define ATT7	7
	#define ATT6	6
	#define ATT5	5
	#define ATT4	4
	#define ATT3	3
	#define ATT2	2
	#define ATT1	1
	#define ATT0	0

#define CONTROL_4	0x05
	#define INVL	7
	#define INVR	6
	#define DFS2	1
	#define SSLOW	0

#define CONTROL_5	0x06
	#define DDM		7
	#define DML		6
	#define DMR		5
	#define DMC		4
	#define DMRE	3
	#define DSDD	1
	#define DSDSEL0	0

#define CONTROL_6	0x07
	#define SYNCE	0

#define CONTROL_7	0x08
	#define SC1		1
	#define SC0		0

#define CONTROL_8	0x09
	#define DSDF	1
	#define DSDSEL1	0



uint8_t SPISend (uint8_t b);
void AK4490WriteReg (uint8_t reg, uint8_t value);
uint8_t AK4490ReadReg (uint8_t reg);


int main () {

	DDRB = _BV(AK4490_CS) | _BV(SCK) /* SCK */ | _BV(DOUT) /* DO !!! */;

	DESELECT;

	_delay_ms (5);

	
	//AK4490WriteReg (AUDIO_SERIAL_PORT_CONTROL, 0b00010100);		// 20-bit, right-justified audio data
	AK4490WriteReg (AUDIO_SERIAL_PORT_CONTROL, _BV(JUS) | _BV(WLEN0));

	//AK4490WriteReg (POWER_DOWN_AND_CLOCK_CONTROL, 0b00000010);		// set MCLK rate to 256*fs, clear PDN bit
	AK4490WriteReg (POWER_DOWN_AND_CLOCK_CONTROL, _BV(CLK0));

	// AK4490ReadReg (AUDIO_SERIAL_PORT_CONTROL);

	cli ();
	set_sleep_mode (SLEEP_MODE_PWR_DOWN);	
	sleep_cpu ();

	// just in case
	while (1);
}


uint8_t SPISend (uint8_t b) {
	
	USIDR = b;
	
	USISR = _BV(USIOIF);

	while (!(USISR & _BV(USIOIF))) {
		USICR = _BV(USIWM0) | _BV(USICS1) | _BV(USICLK) | _BV(USITC);
	}

	return USIDR;

}


void AK4490WriteReg (uint8_t reg, uint8_t value) {
	SELECT;
	SPISend (reg & 0x3f);		// bit7 = 0 (~W), bit6 = 0 (autoinc.step 1)
	SPISend (0xff);				// dummy byte
	SPISend (value);
	DESELECT;
}

uint8_t AK4490ReadReg (uint8_t reg) {
uint8_t value;
	SELECT;
	SPISend ((reg & 0x3f) | 0x80);		// bit7 = 1 (R), bit6 = 0 (autoinc.step 1)
	SPISend (0xff);						// dummy byte	
	value = SPISend (0xff);			// read data
	DESELECT;
	return (value);
