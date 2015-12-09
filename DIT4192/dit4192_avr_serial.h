#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

#define DIT4192	PB4
#define SCK				PB2
#define DOUT			PB1

#define SELECT			PORTB &= ~_BV(DIT4192);
#define DESELECT		PORTB |= _BV(DIT4192);

// DIT4192 Register Definition

#define FACTORY_RSVD	0x00

#define	TRANSMI_CTRL	0x01	
	#define BLSM	0	
	#define VAL		1	
	#define MUTE	2	
	#define BYPAS	3	
	#define MONO	4	
	#define MDAT	5	
	#define	MCSD	6	
	#define	TXOFF	7	

#define	PWRDCLK_CTRL	0x02
	#define PDN		0	
	#define CLK0	1	
	#define	CLK1	2	
	#define	RST		3	

#define	AUDSERP_CTRL	0x03
	#define MS		0	
	#define SCLKR	1	
	#define WLEN0	2	
	#define WLEN1	3	
	#define JUS		4	
	#define DELAY	5	
	#define ISCLK	6	
	#define ISYNC	7	

#define	INTRUPT_STAT	0x04
	#define BTI		0	
	#define TSLIP	1	

#define	INTRUPT_MASK	0x05
	#define	MBTI	0	
	#define	MTSLIP	1	
	#define	BSSL 	2	

#define	INTRUPT_MODE	0x06
	#define	BTIM0	0	
	#define	BTIM1	1	
	#define	TSLIPM0	2	
	#define	TSLIPM1	3	

#define	CHSTATB_CTRL	0x07
	#define BTD 	0	




uint8_t SPISend (uint8_t b);
void DIT4192WriteReg (uint8_t reg, uint8_t value);
uint8_t DIT4192ReadReg (uint8_t reg);




int main () {

	DDRB = _BV(DIT4192) | _BV(SCK) /* SCK */ | _BV(DOUT) /* DO !!! */;

	DESELECT;

	_delay_ms (5);

	
	//DIT4192WriteReg (AUDSERP_CTRL, 0b00010100);		// 20-bit, right-justified audio data
	DIT4192WriteReg (AUDSERP_CTRL, _BV(JUS) | _BV(WLEN0));

	//DIT4192WriteReg (PWRDCLK_CTRL, 0b00000010);		// set MCLK rate to 256*fs, clear PDN bit
	DIT4192WriteReg (PWRDCLK_CTRL, _BV(CLK0));

	// DIT4192ReadReg (AUDSERP_CTRL);

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


void DIT4192WriteReg (uint8_t reg, uint8_t value) {
	SELECT;
	SPISend (reg & 0x3f);		// bit7 = 0 (~W), bit6 = 0 (autoinc.step 1)
	SPISend (0xff);				// dummy byte
	SPISend (value);
	DESELECT;
}

uint8_t DIT4192ReadReg (uint8_t reg) {
uint8_t value;
	SELECT;
	SPISend ((reg & 0x3f) | 0x80);		// bit7 = 1 (R), bit6 = 0 (autoinc.step 1)
	SPISend (0xff);						// dummy byte	
	value = SPISend (0xff);			// read data
	DESELECT;
	return (value);
}
