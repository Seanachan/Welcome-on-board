#ifndef MOTOR_H
#define MOTOR_H
static int speed;
void CCP_Seg7_Initialize(void);
void CCP_Initialize();
void INTERRUPT_Initialize(void);
void ADC_Initialize(void);
void forward();
void backward();
void GOGO();
void highSpeed();
void lowSpeed();
void park();
void turnLeft();
void turnRight();
#endif // MOTOR_H