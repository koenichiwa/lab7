/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#define MOTOR_MASK          0b00000011
#define FLAG_LEFT_MOTOR     0b00000001
#define FLAG_RIGHT_MOTOR    0b00000010
#define FLAG_READY          0b00000100

#include <stdlib.h>
#include "`$INSTANCE_NAME`_MotorControl.h"
#include "`$INSTANCE_NAME`_DirectionReg.h"
#include "`$INSTANCE_NAME`_Pwm.h"

void `$INSTANCE_NAME`_Start(void){
    `$INSTANCE_NAME`_Pwm_Start();
    `$INSTANCE_NAME`_DirectionReg_Write(FLAG_READY);
}
void `$INSTANCE_NAME`_SetSpeeds(int16 left, int16 right){
    volatile uint8 leftIsReverse = left<0;
    volatile uint8 rightIsReverse = right<0;
    volatile uint8 leftFlag = leftIsReverse * FLAG_LEFT_MOTOR;
    volatile uint8 rightFlag = rightIsReverse * FLAG_RIGHT_MOTOR;
    volatile uint8 direction = leftFlag | rightFlag;
    `$INSTANCE_NAME`_DirectionReg_Write(`$INSTANCE_NAME`_DirectionReg_Read() | (direction & MOTOR_MASK));
    `$INSTANCE_NAME`_Pwm_WriteCompare1((uint8) abs(right));
    `$INSTANCE_NAME`_Pwm_WriteCompare2((uint8) abs(left));
}

void `$INSTANCE_NAME`_Stop(void){
    `$INSTANCE_NAME`_DirectionReg_Write(0);
    `$INSTANCE_NAME`_Pwm_WriteCompare1(`$INSTANCE_NAME`_Pwm_INIT_COMPARE_VALUE1);
    `$INSTANCE_NAME`_Pwm_WriteCompare2(`$INSTANCE_NAME`_Pwm_INIT_COMPARE_VALUE2);
    `$INSTANCE_NAME`_Pwm_Stop();
}

/* [] END OF FILE */
