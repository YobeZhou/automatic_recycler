/********************************************************
*    文件名称: stpper.ino 
*    程序作者：Yobe Zhou  
*    创作时间：2019年2月1日
*    完成时间：2019年4月4日
********************************************************/

/* 
    42相步进电机接线：
            A+ --- A+
            A- --- A-
            B+ --- B+
            B- --- B-
    Arduino与TB6600接线：
            PUL+ --- vcc
            PUL- --- D5
            DIR+ --- VCC
            DIR- --- D4
            ENA+ --- VCC
            ENA- --- D6 （高电平使能）
    28BYJ48电机：
       28BYJ48电机的步进角是5.625度，内部存在减速装置减速比是1:64，
       也就是说给28BYJ48发送一个脉冲信号电机旋转(5.625/64)度，电机旋转一圈360度需要360/(5.635/64)=4096个脉冲信号。
            IN1 -- PIN8    |    OUT1 -- 橙色线
            IN2 -- PIN9    |    OUT2 -- 黄色线
            IN3 -- PIN10   |    OUT3 -- 粉色线
            IN4 -- PIN11   |    OUT4 -- 蓝色线
                           |    5V   -- 红色线
*/

#include <EEPROM.h>
#include "Stepper28BYJ48.h"

#define DIR_PIN 4     // 控制方向
#define STEP_PIN 5    // 控制速度
#define ENABLE_PIN 6  // 使能控制
#define INTERRUPT 3   // 中断1引脚
#define BLINK_LED 13  // LED提示灯

// start reading from the first byte (address 0) of the EEPROM
int address = 0;
int readValueL = 0;
int readValueH = 0;
int value = 0;
int dir = 0;
int resteStep = 0;

int c = 0;
volatile int findTheEND = 0; // 寻找原点标志位

MyStepper myStepper(4096, 8, 9, 10, 11);

void setup() {
  // put your setup code here, to run once:
  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);
  pinMode(INTERRUPT, INPUT);
  pinMode(BLINK_LED, OUTPUT);

  digitalWrite(BLINK_LED, LOW);

  Serial.begin(9600);
  
  dir = EEPROM.read(address++);
  readValueL = EEPROM.read(address++);
  readValueH = EEPROM.read(address);
  value = (readValueH << 8) + readValueL; 
 

  if (value > 1) {
    if (dir == 1) {
      myStepper.step(value);
    }
    else {
      myStepper.step(-value);
    }
  }
  myStepper.setSpeed(10);

  /* 在初始化函数中注册中断 */
  attachInterrupt(1, interrupt, RISING);

}

void step(boolean dir, int steps) {
  if (dir) {
    digitalWrite(DIR_PIN, LOW);
    delay(1);
  }
  else {
    digitalWrite(DIR_PIN, HIGH);
    delay(1);
  }

  for (int i = 0; i < steps; i++) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(500);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(500);
  }
}


void classify(boolean dir, int steps) {
  boolean d = dir;
  int s = steps;
  int i = 0;

  digitalWrite(ENABLE_PIN, HIGH);
  for (i = 0; i < s; i++) {
    step(d, 200);
  }
  digitalWrite(ENABLE_PIN, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (findTheEND == 0) {
    digitalWrite(ENABLE_PIN, HIGH);
    while (1) {
      /* 已找到原点，停止寻找操作 */
      if (findTheEND != 0) {
        //detachInterrupt(digitalPinToInterrupt(3));
        break;
      }
      step(true, 200);
    }
    digitalWrite(ENABLE_PIN, LOW);
  }
  else if (findTheEND == 1) {
    //digitalWrite(BLINK_LED, HIGH);
    delay(500);

    /* 返回到原点 */
    digitalWrite(ENABLE_PIN, HIGH);

    for (c = 0; c < 26; c++) {
      step(false, 200);
    }

    findTheEND = 2;
    digitalWrite(ENABLE_PIN, LOW);
    /* 将串口的数据清空 */
    while(Serial.read() >= 0){};
  }
  else {
    if (Serial.available()) {
      digitalWrite(BLINK_LED, HIGH);
      char flag = Serial.read();
      Serial.write(flag);
      /* 将串口的数据清空，用以获取最新的数据 */
      while(Serial.read() >= 0){};
	  
      /* 金属 */
      if ('1' == flag) {
        classify(true, 26);
        //myStepper.step(2048);
        myStepper.step(-2048);
        classify(false, 26);
        Serial.write("1");
      }
	  /* 塑料 */
      else if ('2' == flag) {
        classify(true, 9);
        //myStepper.step(2048);
        myStepper.step(-2048);
        classify(false, 9);
        Serial.write("1");
      }
	  /* 废纸 */
      else if ('3' == flag) {
        classify(false, 8);
        //myStepper.step(2048);
        myStepper.step(-2048);
        classify(true, 8);
        Serial.write("1");
      }
	  /* 果皮 */
      else if ('4' == flag) {
        classify(false, 26);      
        //myStepper.step(2048);
        myStepper.step(-2048);
        classify(true, 26);
        Serial.write("1");
      }
      else{
		/* 回复"1"，告诉Raspberry目前电机空闲 */
        Serial.write("1");
      }  
    }
    else{
	  /* 回复"1"，告诉Raspberry目前电机空闲 */
      Serial.write("1");
      }
    //delay(500);
    digitalWrite(BLINK_LED, LOW);
  }
}

/* 外部中断配置函数 */
void interrupt() {
  findTheEND = 1;
  /* 删除自己 */
  detachInterrupt(digitalPinToInterrupt(3));
}

