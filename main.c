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
#include "project.h"
#include <math.h>
#include <stdio.h>
#include <assert.h>
#include "u8x8.h"
#include "squircular.h"

//MACROS

// Not used anymore but still useful
#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
    
#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })
    
//FLAGS
#define FLAG_STATE_CHANGED  0b00000001
#define FLAG_STATE_QUIT     0b00000010
// insert other flags here
#define FLAG_STATE_TEST     0b10000000

//CONSTANTS
//-OLED
#define OLED_LINE_COMMAND 1
#define OLED_LINE_SPEED 3
// 5 not in use *yet*
#define OLED_LINE_EXTRA 7

//-UART
#define UART_COMMAND_DRIVE  'd'
#define UART_COMMAND_LIGHT  'l'
#define UART_COMMAND_TEST   't'
#define UART_COMMAND_QUIT   'q'

#define UART_RESPONSE_MSG_UNKNOWN   'u'

//-DRIVECOMMAND
#define DRIVECOMMAND_SPACE_CIRCLE 'c'
#define DRIVECOMMAND_SPACE_SQUARE 's'


//STRUCT DEFINITIONS AND STRUCT SPECIFIC FUNCTIONS
typedef struct driveCommand {
    char space; // 'c' denotes vector is on a circle 's' denotes it's on a square
    float forward;
    float direction; //Positive is right negative is left
} driveCommand_t;

typedef struct state {
    driveCommand_t driveCommand;
    uint8 flags;
} state_t;

typedef struct speeds {
    int16 left;
    int16 right;
} speeds_t;

//FUNCTION PREDECLERATIONS
void initialize(u8x8_t *u8x8);
void terminate(u8x8_t *u8x8);
driveCommand_t readDriveCommand();
speeds_t convertDriveCommand(driveCommand_t command);
void displaySpeed(u8x8_t* display);
void displayState(u8x8_t* display);

//-TESTING PURPOSES
void runTestRoutine(int delayMs);
int testCounter = 0;

//INTERRUPT PREDECLERATIONS
CY_ISR_PROTO(commandReceived);

//GLOBAL VARIABLES
volatile state_t state = {
    {DRIVECOMMAND_SPACE_SQUARE, 1.0, 0.0}, // driveCommand
    FLAG_STATE_CHANGED // hasChanged (Set to 1 because "The creation of state is a change of state" - <INSERT RANDOM PHILOSOPHER>)
};

//MAIN
int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
    u8x8_t display;
    initialize(&display);

    while(1)
    {
        if(state.flags & FLAG_STATE_TEST) {
            runTestRoutine(1000);
        }
        
        if(state.flags & FLAG_STATE_CHANGED) {
            // not working due to problems with linking newlib-nano (https://stackoverflow.com/a/28761856/6849045 could be the answer)
            // Changed the linker configuration. Changed the heap size. Did not work.
            //displayState(&display); 
            speeds_t speeds = convertDriveCommand(state.driveCommand);
            MotorControl_SetSpeeds(speeds.left, speeds.right);
            displaySpeed(&display);
            state.flags = state.flags & ~FLAG_STATE_CHANGED;
        }
        
        if(state.flags & FLAG_STATE_QUIT){
            terminate(&display);
        }
    }
}

//HELPER FUNCTION PREDECLERATIONS
uint8_t psoc_gpio_and_delay_callback(u8x8_t *u8x8, uint8_t msg,uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_byte_hw_i2c(u8x8_t *u8x8, uint8_t msg,uint8_t arg_int, void *arg_ptr);

//-INIT FUNCTIONS
void initUart();
void initDisplay(u8x8_t *u8x8);
void terminDisplay(u8x8_t *u8x8);
void terminUart();

//INTERRUPT DEFINITIONS
CY_ISR(commandReceived){
    char commandIdentifier = UARTCommands_GetChar();

    switch(commandIdentifier){
        case UART_COMMAND_DRIVE:
            state.driveCommand = readDriveCommand();
            state.flags = state.flags | FLAG_STATE_CHANGED;
            break;
        case UART_COMMAND_LIGHT: //Switch Light (for testing connection)
            OutTest_Write(~OutTest_Read());
            break;
        case UART_COMMAND_TEST:
            state.flags = state.flags | FLAG_STATE_TEST;
            break;
        case UART_COMMAND_QUIT:
            state.flags = state.flags | FLAG_STATE_QUIT;
            break;
        default: //Signal that an unknown message was received.
            UARTCommands_PutChar(UART_RESPONSE_MSG_UNKNOWN);
            
    }
}

//FUNCTION DEFINITIONS
//-TEST ROUTINE
void runTestRoutine(int delay){
    OutTest_Write(1);
    switch(testCounter){
        case 0:
            state.driveCommand.direction = 1.0;
            state.driveCommand.direction = 0.0;
            state.flags = state.flags | FLAG_STATE_CHANGED;
            testCounter++;
            break;
        case 1:
            CyDelay(delay);
            state.driveCommand.direction = -1.0;
            state.driveCommand.direction = 0.0;
            state.flags = state.flags | FLAG_STATE_CHANGED;
            testCounter++;
            break;
        case 2:
            CyDelay(delay);
            state.driveCommand.direction = 0.0;
            state.driveCommand.direction = 1.0;
            state.flags = state.flags | FLAG_STATE_CHANGED;
            testCounter++;
            break;
        case 3:
            CyDelay(delay);
            state.driveCommand.direction = 0.0;
            state.driveCommand.direction = -1.0;
            state.flags = state.flags | FLAG_STATE_CHANGED;
            testCounter++;
            break;
        case 4:
            CyDelay(delay);
            state.driveCommand.direction = 1.0;
            state.driveCommand.direction = 1.0;
            state.flags = state.flags | FLAG_STATE_CHANGED;
            testCounter++;
            break;
        case 5:
            CyDelay(delay);
            state.driveCommand.direction = -1.0;
            state.driveCommand.direction = -1.0;
            state.flags = state.flags | FLAG_STATE_CHANGED;
            testCounter++;
            break;
        case 6:
            CyDelay(delay);
            state.driveCommand.direction = 0.5;
            state.driveCommand.direction = 0.0;
            state.flags = state.flags | FLAG_STATE_CHANGED;
            testCounter++;
            break;
        default:
            CyDelay(delay);
            state.driveCommand.direction = 0.0;
            state.driveCommand.direction = 0.0;
            testCounter = 0;
            state.flags = (state.flags | FLAG_STATE_CHANGED) & ~FLAG_STATE_TEST ;
            OutTest_Write(0);
    }
    
}


//-INIT FUNCTIONS
void initialize(u8x8_t *u8x8){
    MotorControl_Start();
    initUart();
    initDisplay(u8x8);
}

void terminate(u8x8_t *u8x8){
    MotorControl_Stop();
    terminDisplay(u8x8);
    terminUart();
    CyPmHibernate();
}

void initUart(){
    InterruptCommandReceived_StartEx(commandReceived);
    UARTCommands_Start();
}

void terminUart(){
    InterruptCommandReceived_Stop();
    UARTCommands_Stop();
}

void initDisplay(u8x8_t *u8x8){
    I2C_DISPLAY_Start();
    u8x8_Setup(u8x8, u8x8_d_ssd1306_128x64_noname, u8x8_cad_ssd13xx_i2c,u8x8_byte_hw_i2c, psoc_gpio_and_delay_callback);
    u8x8_InitDisplay(u8x8);
    u8x8_SetPowerSave(u8x8, 0);
    u8x8_ClearDisplay(u8x8);
    u8x8_SetFont(u8x8, u8x8_font_amstrad_cpc_extended_f);
    u8x8_DrawString(u8x8, 1, OLED_LINE_EXTRA, "Ready to run." );
}

void terminDisplay(u8x8_t *u8x8){
    u8x8_SetPowerSave(u8x8, 1);
    I2C_DISPLAY_Stop();
}

//-UART FUNCTIONS
driveCommand_t readDriveCommand(){
    driveCommand_t command;
    
    char* pointer = (char*) &command;
    for(unsigned int i = 0; i<sizeof(driveCommand_t); i++){
        pointer[i] = UARTCommands_GetChar();
    }
    
    return command;
}


//-MOTOR FUNCTIONS

/**
 * This function transforms a vector (direction, forward), which is found in driveCommand, to the vector (left, right) which represents 
 * the speed at which the left and the right motors run.
 * This can be achieved by rotating the vector by 45 degrees and projecting it on a square. (if (f,d) == (1.0, 0.0) then (l,r) == (1.0, 1.0))
 * (direction, forward) is expected to be on or inside the unit circle when command.space == 'c'
 * (direction, forward) is expected to be on or inside the square with range ([-1.0, 1.0], [-1.0, 1.0]) command.space == 's'
 * This is not tested during runtime (except if ran in debug mode)
 * If command.space is anything else it will return (0.0, 0.0)
 *
 * @param command The drive command containing the vector (f,d) and the space on which they lie
 * @return The vector (l,r) In the range [-255,255]
**/
speeds_t convertDriveCommand(driveCommand_t command){
    float circledDirection;
    float circledForward;
    
    // project on circle if needed
    if (command.space == DRIVECOMMAND_SPACE_SQUARE){
        ellipticalSquareToDisc(command.direction, command.forward, &circledDirection, &circledForward);
    }
    else if(command.space == DRIVECOMMAND_SPACE_CIRCLE){
        circledDirection = command.direction;
        circledForward = command.forward;
    } else {
        //invalid
        speeds_t speeds = {0,0};
        return speeds;
    }
    assert((circledDirection*circledDirection + circledForward*circledForward) <= 1.0);
    //rotate by 45 degrees
    float right = (circledForward - circledDirection) * M_SQRT1_2;
    float left = (circledForward + circledDirection) * M_SQRT1_2;
    float mappedLeft;
    float mappedRight;
    
    //project back on square
    ellipticalDiscToSquare(left, right, &mappedLeft, &mappedRight);
    speeds_t speeds;
    speeds.left = (int16) (mappedLeft * 255.0);
    speeds.right = (int16) (mappedRight* 255.0);
    return speeds;
}

//-DISPLAY FUNCTIONS
void displayState(u8x8_t* display){
    char stringbuffer[16];
    sprintf(stringbuffer, "(%.1f, %.1f)",state.driveCommand.forward, state.driveCommand.direction);
    u8x8_ClearLine(display,OLED_LINE_COMMAND);
    u8x8_DrawString(display, 1, OLED_LINE_COMMAND, stringbuffer);
}

void displaySpeed(u8x8_t* display){
    char stringbuffer[16];
    sprintf(
        stringbuffer, 
        "(%d, %d, %d)", 
        MotorControl_Pwm_ReadCompare1(), 
        MotorControl_Pwm_ReadCompare2(), 
        MotorControl_DirectionReg_Read()
    );
    u8x8_ClearLine(display,OLED_LINE_SPEED);
    u8x8_DrawString(display, 1, OLED_LINE_SPEED, stringbuffer);
}

uint8_t psoc_gpio_and_delay_callback(u8x8_t *u8x8, uint8_t msg,uint8_t arg_int, void *arg_ptr) {
    (void) u8x8;
    (void) arg_ptr;
    switch(msg) {
        case U8X8_MSG_GPIO_AND_DELAY_INIT:
        break;
        case U8X8_MSG_DELAY_MILLI:
        CyDelay(arg_int);
        break;
        case U8X8_MSG_DELAY_10MICRO:
        CyDelayUs(10);
        break;
        case U8X8_MSG_DELAY_100NANO:
        CyDelayCycles(100);
        break;
    }
    return 1;
}

uint8_t u8x8_byte_hw_i2c(u8x8_t *u8x8, uint8_t msg,uint8_t arg_int, void *arg_ptr) {
    uint8_t *data;
    switch(msg) {
        case U8X8_MSG_BYTE_SEND:
        data = (uint8_t *)arg_ptr;
        while(arg_int > 0) {
            (void)I2C_DISPLAY_MasterWriteByte(*data);
            data++;
            arg_int--;
        }
        break;
        case U8X8_MSG_BYTE_INIT:
        break;
        case U8X8_MSG_BYTE_SET_DC:
        break;
        case U8X8_MSG_BYTE_START_TRANSFER:
        (void)I2C_DISPLAY_MasterSendStart(u8x8_GetI2CAddress(u8x8) >> 1,I2C_DISPLAY_WRITE_XFER_MODE);
        break;
        case U8X8_MSG_BYTE_END_TRANSFER:(void)I2C_DISPLAY_MasterSendStop();
        break;
        default:
        return 0;
    }
    return 1;
}

/* [] END OF FILE */
