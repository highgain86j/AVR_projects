/*目標
 * uno本体のButtonで増加していく電子ボリューム
 * mute->-95.5db->0dB->+31.5dB
 * 
 */

// define | const ?
#define cs 10     //pin in PGA2311->2
#define sdi 11    //pin in PGA2311->3
#define sclk 12   //pin in PGA2311->6
#define mute 13   //pin in PGA2311->8

#define button 2  //

//encVolume : 0 to 255(0 == mute)
int encVolume = 0;

void setup() {
  //test
  Serial.begin(9800);
  
  // PGA2311
  pinMode(cs, OUTPUT);
  pinMode(sdi, OUTPUT);
  pinMode(sclk, OUTPUT);

  //Button
  pinMode(button, INPUT);

  //Mute Test
  //mute on
  digitalWrite(mute, LOW);
  //delay 5sec
  delay(5000);
  //mute off
  digitalWrite(mute, HIGH);

}

void loop() {
  if(digitalRead(button) == HIGH) {
    if(encVolume <= 255){
      encVolume++;
      digitalWrite(13,HIGH);
      Serial.println(encVolume);
      volWritePGA2311(encVolume);
    }
  }else{
      digitalWrite(13,LOW);
      Serial.println(encVolume);
  }
}

  
/*
 * volWritePGA2311
 *  
 * #define cs 11
 * #define sdi 12
 * #define sclk 13
 * #define mute 14
 * volNum : encVolume : 0 to 255
 */
void volWritePGA2311(int volNum){
  int voldata[15];

  for (int i=0;i<8;i++){
    voldata[i] = bitRead(volNum,7-i);
    voldata[i+8] = bitRead(volNum,7-i);
  }

  digitalWrite(cs,LOW);
  delayMicroseconds(80);
  for(int i=0;i<16;i++){
    digitalWrite(sdi,voldata[i]);
    delayMicroseconds(80);
    digitalWrite(sclk,HIGH);
    delayMicroseconds(80);
    digitalWrite(sdi,LOW);
    delayMicroseconds(80);
    digitalWrite(sclk,LOW);
    delayMicroseconds(80);
  }
  delayMicroseconds(80);
  digitalWrite(cs,HIGH);
}


