/********************************************************
*    文件名称: Stepper28BYJ48.cpp
*    程序作者：Yobe Zhou  
*    创作时间：2019年2月1日
*    完成时间：2019年4月1日
********************************************************/

#include <EEPROM.h>
#include "Arduino.h"
#include "Stepper28BYJ48.h"

/*
 *  在构造函数中进行初始化
 */
MyStepper::MyStepper(int number_of_steps, int motor_pin_1, int motor_pin_2,
                                      int motor_pin_3, int motor_pin_4)
{
  this->step_number = 0;    // 电机当前步数
  this->speed = 0;          // 电机速度，以每分钟转数为单位
  this->direction = 0;      // 电机方向
  this->last_step_time = 0; // 最后一步的时间戳
  this->number_of_steps = number_of_steps; // 该电机的总步数

  // 用于电机控制连接的Arduino引脚：
  this->motor_pin_1 = motor_pin_1;
  this->motor_pin_2 = motor_pin_2;
  this->motor_pin_3 = motor_pin_3;
  this->motor_pin_4 = motor_pin_4;

  // 设置微控制器上的引脚：
  pinMode(this->motor_pin_1, OUTPUT);
  pinMode(this->motor_pin_2, OUTPUT);
  pinMode(this->motor_pin_3, OUTPUT);
  pinMode(this->motor_pin_4, OUTPUT);

  // pin_count由stepMotor（）方法使用：
  this->pin_count = 4;
}

/*
 * 设置以每分钟转速为单位的速度
 */
void MyStepper::setSpeed(long whatSpeed)
{
  this->step_delay = 60L * 1000L * 1000L / this->number_of_steps / whatSpeed;
}

/*
 * 移动电机steps_to_move步骤。如果数字是负数，电机则向反向运动。
 */
void MyStepper::step(int steps_to_move)
{
  /* EPRROM保存地址 */
  int address = 0;
  int valueL = 0;
  int valueH = 0;
  int tempSteps = steps_to_move;
  int steps_left = abs(steps_to_move);  // how many steps to take

  // 根据steps_to_mode是+还是 - 来确定方向：
  if (steps_to_move > 0) { this->direction = 1; EEPROM.write(address, 1);}
  if (steps_to_move < 0) { this->direction = 0; EEPROM.write(address, 0);}


  // 减少步数，每次移动一步：
  while (steps_left > 0)
  {
  	unsigned long now = micros();    
    // 只有在适当的延迟过去后移动：
    if (now - this->last_step_time >= this->step_delay)
    {
	  /* 保存操作，小端格式 */
      valueL = steps_left & 0x00FF;
      EEPROM.write(address + 1, valueL);
      valueH = steps_left >> 8;
      EEPROM.write(address + 2, valueH);
      
      // 得到要求的步时的时间标准：
      this->last_step_time = now;
      // 增加或减少步数，取决于方向：
      if (this->direction == 1)
      {
        this->step_number++;
        if (this->step_number == this->number_of_steps) {
          this->step_number = 0;
        }
      }
      else
      {
        if (this->step_number == 0) {
          this->step_number = this->number_of_steps;
        }
        this->step_number--;
      }

      steps_left--;
      
      // 将电机步进到步数0,1，...，{3或10}
      stepMotor(this->step_number % 8);
    }
  }
  digitalWrite(motor_pin_1, LOW);
  digitalWrite(motor_pin_2, LOW);
  digitalWrite(motor_pin_3, LOW);
  digitalWrite(motor_pin_4, LOW);
}

/*
 * 向前或向后移动电机。
 */
void MyStepper::stepMotor(int thisStep)
{
  
    switch (thisStep) {
      case 0:  // 0001
        digitalWrite(motor_pin_1, LOW);
        digitalWrite(motor_pin_2, LOW);
        digitalWrite(motor_pin_3, LOW);
        digitalWrite(motor_pin_4, HIGH);
      break;
      case 1:  // 0011
        digitalWrite(motor_pin_1, LOW);
        digitalWrite(motor_pin_2, LOW);
        digitalWrite(motor_pin_3, HIGH);
        digitalWrite(motor_pin_4, HIGH);
      break;
      case 2:  //0010
        digitalWrite(motor_pin_1, LOW);
        digitalWrite(motor_pin_2, LOW);
        digitalWrite(motor_pin_3, HIGH);
        digitalWrite(motor_pin_4, LOW);
      break;
      case 3:  //0110
        digitalWrite(motor_pin_1, LOW);
        digitalWrite(motor_pin_2, HIGH);
        digitalWrite(motor_pin_3, HIGH);
        digitalWrite(motor_pin_4, LOW);
      break;
    case 4:  //0100
        digitalWrite(motor_pin_1, LOW);
        digitalWrite(motor_pin_2, HIGH);
        digitalWrite(motor_pin_3, LOW);
        digitalWrite(motor_pin_4, LOW);
      break;
    case 5:  //1100
        digitalWrite(motor_pin_1, HIGH);
        digitalWrite(motor_pin_2, HIGH);
        digitalWrite(motor_pin_3, LOW);
        digitalWrite(motor_pin_4, LOW);
      break;
    case 6:  //1000
        digitalWrite(motor_pin_1, HIGH);
        digitalWrite(motor_pin_2, LOW);
        digitalWrite(motor_pin_3, LOW);
        digitalWrite(motor_pin_4, LOW);
      break;
    case 7:  //1001
        digitalWrite(motor_pin_1, HIGH);
        digitalWrite(motor_pin_2, LOW);
        digitalWrite(motor_pin_3, LOW);
        digitalWrite(motor_pin_4, HIGH);
      break;
    }
 
}



