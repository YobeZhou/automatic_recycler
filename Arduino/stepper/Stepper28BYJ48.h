/********************************************************
*    文件名称: Stepper28BYJ48.h
*    程序作者：Yobe Zhou  
*    创作时间：2019年2月1日
*    完成时间：2019年4月1日
********************************************************/

// 避免重复包含
#ifndef Stepper28BYJ48_h
#define Stepper28BYJ48_h

// 库接口描述
class MyStepper {
  public:
    // 构造函数：
    MyStepper(int number_of_steps, int motor_pin_1, int motor_pin_2,
                                 int motor_pin_3, int motor_pin_4);

    // 速度设定器方法：
    void setSpeed(long whatSpeed);

    // 移动方法：
    void step(int number_of_steps);

  private:
    void stepMotor(int this_step);

    int direction;            // 旋转方向
    int speed;                // 以RPM为单位的速度
    unsigned long step_delay; // 基于速度的步骤之间的延迟，以ms为单位
    int number_of_steps;      // 这台电机可以采取的总步数
    int pin_count;            // 有多少针正在使用中。
    int step_number;          // 电机在哪一步

    // 电机引脚：
    int motor_pin_1;
    int motor_pin_2;
    int motor_pin_3;
    int motor_pin_4;

    unsigned long last_step_time; // 最后一步的时间戳
};

#endif



