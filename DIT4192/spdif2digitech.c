#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

#define DIT4192_CS	PB4
#define SCK			PB2
#define DOUT		PB1

#define SELECT		PORTB &= ~_BV(DIT4192_CS);
#define DESELECT	PORTB |= _BV(DIT4192_CS);

/******************************************************************************************************************
 *
 * DIT4192 Registers
 *
 ******************************************************************************************************************/

/******************************************************************************************************************
 *
 * Reserved for Factory Use (00H)
 *
 *		b7		b6		b5		b4		b3		b2		b1		b0
 *	-----------------------------------------------------------------
 *	|   0	|   0	|   0	|   0	|   0	|   0	|   0	|   0	|
 *  -----------------------------------------------------------------
 *
 ******************************************************************************************************************/
#define FACTORY_REG						0x00


/******************************************************************************************************************
 *
 * Transmitter Control Register (01H)
 *
 *		b7		b6		b5		b4		b3		b2		b1		b0
 *	-----------------------------------------------------------------
 *	| TXOFF	| MCSD	|  MDAT	|  MONO	| BYPAS	|  MUTE	|  VAL	|  BLSM	|
 *  -----------------------------------------------------------------
 *
 ******************************************************************************************************************/
#define	TRANSMITTER_CONTROL				0x01

#define BLSM	0	// Block Start Mode (Defaults to 0)
					// When set to 0, BLS (pin 25) is configured as an input pin.
					// When set to 1, BLS (pin 25) is configured as an output pin.

#define VAL		1	// Audio Data Valid (Defaults to 0)
					// When set to 0, valid Linear PCM audio data is indicated.
					// When set to 1, invalid audio data or non-PCM data is indicated.

#define MUTE	2	// Transmitter Mute (Defaults to 0)
					// When set to 0, the mute function is disabled.
					// When set to 1, the mute function is enabled, with Channel A and B audio data set to all 0’s.

#define BYPAS	3	// Transmitter Bypass—AES-3 Data Source for the Output Driver (Defaults to 0)
					// When set to 0, AES-3 encoded data is taken from the output of the on-chip encoder.
					// When set to 1, RXP (pin 9) is used as the source for AES-3 encoded data.

#define MONO	4	// Mono Mode Control (Defaults to 0). When set to 0, the transmitter is set to Stereo mode.
					// When set to 0, the transmitter is set to Stereo mode.
					// When set to 1, the transmitter is set to Mono mode.

#define MDAT	5	// Data Selection Bit (Defaults to 0)
					// (0 = Left Channel, 1 = Right Channel)
					// When MONO = 0 and MCSD = 0, the MDAT bit is ignored.
					// When MONO = 0 and MCSD = 1, the MDAT bit is used to select the source for Channel Status data.
					// When MONO = 1 and MCSD = 0, the MDAT bit is used to select the source for Audio data.
					// When MONO = 1 and MCSD = 1, the MDAT bit is used to select the source for both Audio and Channel Status data.

#define	MCSD	6	// Channel Status Data Selection (Defaults to 0)
					// When set to 0, Channel A data is used for the A sub-frame, while Channel B data is used for the B sub-frame.
					// When set to 1, use the same channel status data for both A and B sub-frames. Channel status data source is 
					// selected using the MDAT bit.

#define	TXOFF	7	// Transmitter Output Disable (Defaults to 0).
					// When set to 0, the line driver outputs, TX– (pin 17) and TX+ (pin 18) are enabled.
					// When set to 1, the line driver outputs are forced to ground.


/******************************************************************************************************************
 *
 * Power-Down and Clock Control Register (02H)
 *
 *		b7		b6		b5		b4		b3		b2		b1		b0
 *	-----------------------------------------------------------------
 *	|   0	|   0	|   0	|   0	|  RST	|  CLK1	|  CLK0	|  PDN	|
 *  -----------------------------------------------------------------
 *
 ******************************************************************************************************************/
#define	POWER_DOWN_AND_CLOCK_CONTROL	0x02

#define PDN		0	// Power-Down (Defaults to 1).
					// When set to 0, the DIT4192 operates normally. 
					// When set to 1, the DIT4192 is powered down, with the line driver outputs forced to ground.

#define CLK0	1	// CLK[1:0] - MCLK Rate Selection.
#define	CLK1	2	// 00 = 128fs, 01 = 256fs (default), 10 = 384fs, 11 = 512fs

#define	RST		3	// Software Reset (Defaults to 0). 
					// When set to 0, the DIT4192 operates normally. When set to 1, the DIT4192 is reset.


/******************************************************************************************************************
 *
 * Audio Serial Port Control Register (03H)
 *
 *		b7		b6		b5		b4		b3		b2		b1		b0
 *	-----------------------------------------------------------------
 *	| ISYNC	| ISCLK	| DELAY	|  JUS	| WLEN1	| WLEN0 | SCLKR	|   MS	|
 *  -----------------------------------------------------------------
 *
 ******************************************************************************************************************/
#define	AUDIO_SERIAL_PORT_CONTROL		0x03

#define MS		0	// Master/Slave Mode (Defaults to 0). When set to 0, the audio serial port is set for Slave operation.

#define SCLKR	1	// Master Mode SCLK Frequency (Defaults to 0).
					// When set to 0, the SCLK frequency is set to 64fs. When set to 1, the SCLK frequency is set to 128fs.

#define WLEN0	2	// WLEN[1:0] - Audio Data Word Length.
#define WLEN1	3	// 00 = 24 bits (default), 01 = 20 bits, 10 = 18 bits, 11 = 16 bits

#define JUS		4	// Audio Data Justification (Defaults to 0). 
					// When set to 0, the audio data is Left-Justified with respect to the SYNC edges. 
					// When set to 1, the audio data is Right-Justified with respect to the SYNC edges.

#define DELAY	5	// Audio Data Delay from the Start of Frame (Defaults to 0).
					// When set to 0, audio data starts with the SCLK period immediately following the SYNC edge which starts the frame.
					// This is referred to as a zero SCLK delay.
					// When set to 1, the audio data starts with the second SCLK period following the SYNC edge
					// which starts the frame. This is referred to as a one SCLK delay.

#define ISCLK	6	// SCLK Sampling Edge (Defaults to 0)
					// When set to 0, audio serial data at SDATA (pin 13) is sampled on rising edge of SCLK.
					// When set to 1, audio serial data at SDATA (pin 13) is sampled on falling edge of SCLK.

#define ISYNC	7	// SYNC Polarity (Defaults to 0)
					// When set to 0, Left channel data occurs when the SYNC clock is HIGH.
					// When set to 1, Left channel data occurs when the SYNC clock is LOW.


#define	INTERRUPT_STATUS				0x04

#define	INTERRUPT_MASK					0x05

#define	INTERRUPT_MODE					0x06

#define	CHANNEL_STATUS_BUFFER_CONTROL	0x07


uint8_t SPISend (uint8_t b);
void DIT4192WriteReg (uint8_t reg, uint8_t value);
uint8_t DIT4192ReadReg (uint8_t reg);


int main () {

	DDRB = _BV(DIT4192_CS) | _BV(SCK) /* SCK */ | _BV(DOUT) /* DO !!! */;

	DESELECT;

	_delay_ms (5);

	
	//DIT4192WriteReg (AUDIO_SERIAL_PORT_CONTROL, 0b00010100);		// 20-bit, right-justified audio data
	DIT4192WriteReg (AUDIO_SERIAL_PORT_CONTROL, _BV(JUS) | _BV(WLEN0));

	//DIT4192WriteReg (POWER_DOWN_AND_CLOCK_CONTROL, 0b00000010);		// set MCLK rate to 256*fs, clear PDN bit
	DIT4192WriteReg (POWER_DOWN_AND_CLOCK_CONTROL, _BV(CLK0));

	// DIT4192ReadReg (AUDIO_SERIAL_PORT_CONTROL);

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
