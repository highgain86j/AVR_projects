/***
  PCINT0を使ってPB1の変化に割り込んで、PB0のLEDとUARTで反応する
  
  CPU: ATMEGA168 8MHz(内蔵)
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
#define sbi(BYTE,BIT) BYTE|=_BV(BIT) // BYTEの指定BITに1をセット
#define cbi(BYTE,BIT) BYTE&=~_BV(BIT) // BYTEの指定BITをクリア

/** 動作設定 **/
#define FOSC 8000000 // 8MHz

/** UART設定 **/
#define BAUD 9600 // 9600bps
#define MYUBRR FOSC/16/BAUD-1 // UART分周率
// #define UCSR0A_U2X0 1 // 倍速フラグ 等速ならコメントアウト
#ifdef UCSR0A_U2X0 // 倍速が定義されているならば
 #define MYUBRR FOSC/16/(BAUD/2)-1 // UART分周率(倍速)
#endif
volatile char usart_recvData; // USARTで受信したデータ

#define LED_SET() sbi(PORTB, PB0) // 基盤上の動作確認LED
#define LED_CLR() cbi(PORTB, PB0)


/* PORT設定 */
void port_init(void){
  sbi(DDRB, PB0);
}


/* USART設定 */
void usart_init(unsigned int ubrr){
  UBRR0H = (unsigned char)(ubrr>>8); // ボーレート上位8bit
  UBRR0L = (unsigned char)ubrr; // ボーレート下位8bit
  UCSR0A = (0<<U2X0); // 等速
  UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(0<<RXCIE0); // 送受信許可、受信完了割り込み不可
  UCSR0C = (0<<UMSEL00)|(3<<UCSZ00)|(1<<USBS0)|(0<<UPM00);
  // フレーム設定 非同期通信 8ビット 1ストップビット パリティ無し
}


/* UARTで文字列送信 */
void usart_sendStr(char *str){
  while(*str != NULL){
    loop_until_bit_is_set(UCSR0A,UDRE0); // 送信データレジスタ空きまで待機
    UDR0 = *str++; // 1文字送信、1文字進む
  }
}

/* ピン変化割り込み設定 */
void pinchange_init(void){
  sbi(PCICR, PCIE0); // ピン変化割り込み0許可
  PCMSK0 = (1<<PCINT1); // PCINT1のみ許可
  
  // sbi(PCICR, PCIE1); // ピン変化割り込み1許可
  // PCMSK1 = (1<<PCINT8)|(1<<PCINT14);
  
  // sbi(PCICR, PCIE2); // ピン変化割り込み2許可
  // PCMSK2 = (1<<PCINT16)|(PCINT23);
}


int main(void){
  port_init(); // PORT設定
  usart_init(MYUBRR); // USART設定
  pinchange_init(); // ピン変化割り込み設定
  sei(); // 全割り込み許可
  
  LED_SET(); // 起動確認LED
    
  for(;;){
  }
}


/** 外部割り込み0 **/
SIGNAL(SIG_PIN_CHANGE0){
  if(bit_is_set(PINB, PB1)){ // PB1立ち上がりの時
    LED_SET(); // LED点灯
    usart_sendStr("PB1_HIGH\r\n");
  }
  else{
    LED_CLR(); // LED消灯
    usart_sendStr("PB1_LOW\r\n");
  }
}


