/***
  PCINT0���g����PB1�̕ω��Ɋ��荞��ŁAPB0��LED��UART�Ŕ�������
  
  CPU: ATMEGA168 8MHz(����)
  Compiler: WinAVR20060421(AVR-GCC3.4.6)
  Date: 2006/12/23
  Author: Sho Hashimoto
  WebSite: http://shokai.org
  ***/

#include <avr/io.h>
#include <avr/interrupt.h>

#define TRUE 1
#define FALSE 0
#define NULL '\0'
#define sbi(BYTE,BIT) BYTE|=_BV(BIT) // BYTE�̎w��BIT��1���Z�b�g
#define cbi(BYTE,BIT) BYTE&=~_BV(BIT) // BYTE�̎w��BIT���N���A

/** ����ݒ� **/
#define FOSC 8000000 // 8MHz

/** UART�ݒ� **/
#define BAUD 9600 // 9600bps
#define MYUBRR FOSC/16/BAUD-1 // UART������
// #define UCSR0A_U2X0 1 // �{���t���O �����Ȃ�R�����g�A�E�g
#ifdef UCSR0A_U2X0 // �{������`����Ă���Ȃ��
 #define MYUBRR FOSC/16/(BAUD/2)-1 // UART������(�{��)
#endif
volatile char usart_recvData; // USART�Ŏ�M�����f�[�^

#define LED_SET() sbi(PORTB, PB0) // ��Տ�̓���m�FLED
#define LED_CLR() cbi(PORTB, PB0)


/* PORT�ݒ� */
void port_init(void){
  sbi(DDRB, PB0);
}


/* USART�ݒ� */
void usart_init(unsigned int ubrr){
  UBRR0H = (unsigned char)(ubrr>>8); // �{�[���[�g���8bit
  UBRR0L = (unsigned char)ubrr; // �{�[���[�g����8bit
  UCSR0A = (0<<U2X0); // ����
  UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(0<<RXCIE0); // ����M���A��M�������荞�ݕs��
  UCSR0C = (0<<UMSEL00)|(3<<UCSZ00)|(1<<USBS0)|(0<<UPM00);
  // �t���[���ݒ� �񓯊��ʐM 8�r�b�g 1�X�g�b�v�r�b�g �p���e�B����
}


/* UART�ŕ����񑗐M */
void usart_sendStr(char *str){
  while(*str != NULL){
    loop_until_bit_is_set(UCSR0A,UDRE0); // ���M�f�[�^���W�X�^�󂫂܂őҋ@
    UDR0 = *str++; // 1�������M�A1�����i��
  }
}

/* �s���ω����荞�ݐݒ� */
void pinchange_init(void){
  sbi(PCICR, PCIE0); // �s���ω����荞��0����
  PCMSK0 = (1<<PCINT1); // PCINT1�̂݋���
  
  // sbi(PCICR, PCIE1); // �s���ω����荞��1����
  // PCMSK1 = (1<<PCINT8)|(1<<PCINT14);
  
  // sbi(PCICR, PCIE2); // �s���ω����荞��2����
  // PCMSK2 = (1<<PCINT16)|(PCINT23);
}


int main(void){
  port_init(); // PORT�ݒ�
  usart_init(MYUBRR); // USART�ݒ�
  pinchange_init(); // �s���ω����荞�ݐݒ�
  sei(); // �S���荞�݋���
  
  LED_SET(); // �N���m�FLED
    
  for(;;){
  }
}


/** �O�����荞��0 **/
SIGNAL(SIG_PIN_CHANGE0){
  if(bit_is_set(PINB, PB1)){ // PB1�����オ��̎�
    LED_SET(); // LED�_��
    usart_sendStr("PB1_HIGH\r\n");
  }
  else{
    LED_CLR(); // LED����
    usart_sendStr("PB1_LOW\r\n");
  }
}


