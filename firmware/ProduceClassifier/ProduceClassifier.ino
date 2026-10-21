#include <Arduino.h>
#include "model.h"
constexpr int DRIVE=5,SENSE=4,RETURN_PIN=6,BUTTON=0,RGB=48;
constexpr uint32_t OFFSETS[]={1000,5000,20000};
enum State {READY,SETTLING,CAPTURING};
State state=READY;
uint32_t started=0,nextCycle=0,buttonChanged=0,buttonPressed=0;
uint32_t settleMs=5000; int targetCycles=6; bool slowMode=false;
bool buttonRaw=true,buttonStable=true,buttonArmed=false;
int cycles=0,railCount=0;
double sums[6]={};
struct Sample {uint32_t raw,mv;};
const char* LABELS[]={"apple","potato","cucumber"};
void led(int r,int g,int b){rgbLedWrite(RGB,r,g,b);}
void idle(){digitalWrite(DRIVE,LOW);digitalWrite(RETURN_PIN,LOW);}
int classify(const float* x){
  float best=1e30f;int winner=0;
  for(int c=0;c<3;c++){
    float distance=0;
    for(int j=0;j<6;j++){float d=(x[j]-MODEL_MU[j])/MODEL_SD[j]-CENTERS[c][j];distance+=d*d;}
    if(distance<best){best=distance;winner=c;}
  }
  return winner;
}
void selfTest(){
  int passed=0;
  for(int i=0;i<TEST_COUNT;i++)if(classify(TEST_VECTORS[i])==TEST_EXPECTED[i])passed++;
  Serial.printf("SELFTEST %d/%d %s\n",passed,TEST_COUNT,passed==TEST_COUNT?"PASS":"FAIL");
}
void start(bool slow=false){
  if(state!=READY){Serial.println("BUSY");return;}
  slowMode=slow;settleMs=slow?30000:5000;targetCycles=slow?30:6;
  idle();cycles=0;railCount=0;for(int j=0;j<6;j++)sums[j]=0;
  state=SETTLING;started=millis();led(0,0,25);
  Serial.printf("START mode=%s settle_ms=%lu cycles=%d; hands off\n",slowMode?"slow":"fast",(unsigned long)settleMs,targetCycles);
}
void pulse(bool reverse,Sample* samples){
  idle();delay(30);uint32_t begin=micros();
  digitalWrite(reverse?RETURN_PIN:DRIVE,HIGH);
  for(int i=0;i<3;i++){
    while((uint32_t)(micros()-begin)<OFFSETS[i])delayMicroseconds(10);
    samples[i].raw=analogRead(SENSE);samples[i].mv=analogReadMilliVolts(SENSE);
  }
  idle();
}
void finish(){
  idle();state=READY;
  Serial.printf("TIMING mode=%s elapsed_ms=%lu cycles=%d\n",slowMode?"slow":"fast",(unsigned long)(millis()-started),cycles);
  Serial.print("FEATURES_MV");float x[6];
  for(int j=0;j<6;j++){x[j]=sums[j]/cycles;Serial.printf(",%.3f",x[j]);}Serial.println();
  if(railCount){led(25,0,25);Serial.printf("INVALID rail_samples=%d; check electrodes\n",railCount);return;}
  int c=classify(x);
  if(c==0)led(35,0,0);else if(c==1)led(30,22,0);else led(0,35,0);
  Serial.printf("RESULT %s model=three-class-v1 cycles=%d\n",LABELS[c],cycles);
}
void setup(){
  digitalWrite(DRIVE,LOW);digitalWrite(RETURN_PIN,LOW);
  pinMode(DRIVE,OUTPUT);pinMode(RETURN_PIN,OUTPUT);pinMode(SENSE,INPUT);pinMode(BUTTON,INPUT_PULLUP);
  analogReadResolution(12);analogSetPinAttenuation(SENSE,ADC_11db);
  Serial.begin(115200);led(0,0,0);delay(500);
  analogRead(SENSE);analogReadMilliVolts(SENSE);
  buttonRaw=buttonStable=digitalRead(BUTTON);
  Serial.println("ProduceClassifier v1.1: BOOT short/r=fast; BOOT hold 2s/l=slow; s=stop; i=info; v=selftest");
}
void loop(){
  while(Serial.available()){
    char c=Serial.read();
    if(c=='r')start(false);
    if(c=='l')start(true);
    if(c=='s'){idle();state=READY;led(0,0,0);Serial.println("STOPPED");}
    if(c=='i')Serial.printf("INFO ProduceClassifier v1.1; model three-class-v1; state=%d; GPIO5--10k--A+GPIO4; B--GPIO6\n",state);
    if(c=='v'&&state==READY)selfTest();
  }
  bool reading=digitalRead(BUTTON);
  if(reading!=buttonRaw){buttonRaw=reading;buttonChanged=millis();}
  if(reading!=buttonStable&&(uint32_t)(millis()-buttonChanged)>=40){
    buttonStable=reading;
    if(!reading){buttonArmed=(state==READY);buttonPressed=millis();}
    else if(buttonArmed){buttonArmed=false;start((uint32_t)(millis()-buttonPressed)>=2000);}
  }
  if(state==SETTLING&&(uint32_t)(millis()-started)>=settleMs){state=CAPTURING;nextCycle=millis();Serial.println("CAPTURING");}
  if(state==CAPTURING&&(int32_t)(millis()-nextCycle)>=0){
    Sample a[3],b[3];pulse(false,a);pulse(true,b);
    for(int j=0;j<6;j++){Sample s=j<3?a[j]:b[j-3];sums[j]+=s.mv;if(s.raw<20||s.raw>4075)railCount++;}
    cycles++;
    if(cycles==targetCycles)finish();else nextCycle=millis()+400;
  }
  delay(1);
}

