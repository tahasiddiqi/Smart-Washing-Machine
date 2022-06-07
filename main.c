#include "hal_LCD.h"
#include "driverlib.h"
#include "main.h"
#include "msp430.h"

volatile unsigned char * mode = &BAKMEM4_L;
volatile unsigned char * S1buttonDebounce = &BAKMEM2_L;
volatile unsigned char * S2buttonDebounce = &BAKMEM2_H;
volatile unsigned int holdCount = 0;
#define STARTUP_MODE       0
#define AUTO_MODE          1
#define MANUAL_MODE        2


uint16_t period = 6000;
uint16_t dutyCycle = 0;


Timer_A_initUpModeParam initUpParam_A0 =
{

    TIMER_A_CLOCKSOURCE_SMCLK,
    TIMER_A_CLOCKSOURCE_DIVIDER_1,
    10000,
    TIMER_A_TAIE_INTERRUPT_DISABLE,
    TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE ,
    TIMER_A_DO_CLEAR,
    true
};


void main(void)
{
    WDT_A_hold(WDT_A_BASE);

    Init_GPIO();
    Init_LCD();
    *S1buttonDebounce = *S2buttonDebounce = *mode = 0;

    GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN2);
    GPIO_clearInterrupt(GPIO_PORT_P2, GPIO_PIN6);

    //  __enable_interrupt();
    displayScrollText("WELCOME TO WASHING MACHINE");

    while(1)
    {
        switch(*mode)
        {
            case STARTUP_MODE:
                //displayScrollText("HOME");
                clearLCD();
                displayScrollText("PRESS S1 FOR AUTO MODE AND S2 FOR MANUAL MODE");
                dutyCycle=0;
                clearLCD();
                break;
            case AUTO_MODE:
                clearLCD();
                displayScrollText("AUTO MODE");
                dutyCycle =  dutyCycle+1000;
                initTimers( period, dutyCycle );
                __delay_cycles(1000);
                dutyCycle =  dutyCycle+1000;
                                initTimers( period, dutyCycle );
                                __delay_cycles(1000);
                dutyCycle =  dutyCycle+1000;
                initTimers( period, dutyCycle );
                __delay_cycles(1000);
                dutyCycle =  dutyCycle+1000;
                initTimers( period, dutyCycle );
                clearLCD();
                break;
            case MANUAL_MODE:
                clearLCD();
                displayScrollText("MANUAL MODE");
                initTimers( period, dutyCycle );
                clearLCD();
                break;
        }
        __bis_SR_register(LPM3_bits | GIE);         // enter LPM3

    }

}

void Init_GPIO()
{


    // Set pin P1.0 to output direction and turn LED off

    GPIO_setAsOutputPin( GPIO_PORT_P1, GPIO_PIN0 );                             // Red LED (LED1)
    GPIO_setOutputLowOnPin( GPIO_PORT_P1, GPIO_PIN0 );

    // Set pin P4.0 to output direction and turn LED off
    GPIO_setAsOutputPin( GPIO_PORT_P4, GPIO_PIN0 );                             // Green LED (LED2)
    GPIO_setOutputLowOnPin( GPIO_PORT_P4, GPIO_PIN0 );
    GPIO_setAsPeripheralModuleFunctionOutputPin( GPIO_PORT_P8, GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION );

    PMM_unlockLPM5();

    GPIO_setAsInputPinWithPullUpResistor( GPIO_PORT_P1, GPIO_PIN2 );
    GPIO_selectInterruptEdge ( GPIO_PORT_P1, GPIO_PIN2, GPIO_HIGH_TO_LOW_TRANSITION );
    GPIO_clearInterrupt ( GPIO_PORT_P1, GPIO_PIN2 );
    GPIO_enableInterrupt ( GPIO_PORT_P1, GPIO_PIN2 );

    GPIO_setAsInputPinWithPullUpResistor( GPIO_PORT_P2, GPIO_PIN6 );
    GPIO_selectInterruptEdge ( GPIO_PORT_P2, GPIO_PIN6, GPIO_HIGH_TO_LOW_TRANSITION );
    GPIO_clearInterrupt ( GPIO_PORT_P2, GPIO_PIN6 );
    GPIO_enableInterrupt ( GPIO_PORT_P2, GPIO_PIN6 );


}


#pragma vector=PORT1_VECTOR
__interrupt void pushbutton_ISR (void)
{
        // Toggle the LED on/off (initial code; moved into switch statement below)
        //GPIO_toggleOutputOnPin( GPIO_PORT_P1, GPIO_PIN0 );

    switch( __even_in_range( P1IV, P1IV_P1IFG7 ))
    {

        case P1IV_NONE:
             break;                               // None
        case P1IV_P1IFG0:                                      // Pin 0
             __no_operation();
             break;
        case P1IV_P1IFG1:                                       // Pin 1
            __no_operation();
             break;
        case P1IV_P1IFG2:                                       // Pin 2 (button 1)
            if ((*S1buttonDebounce) == 0)
            {
                *S1buttonDebounce = 1;
                holdCount = 0;
                GPIO_toggleOutputOnPin( GPIO_PORT_P1, GPIO_PIN0 );
                if (*mode == MANUAL_MODE)
                {
                    dutyCycle = dutyCycle+1000;
                }
                Timer_A_initUpMode(TIMER_A0_BASE, &initUpParam_A0);
            }

             break;
        case P1IV_P1IFG3:                                       // Pin 3
            __no_operation();
             break;
        case P1IV_P1IFG4:                                       // Pin 4
            __no_operation();
             break;
        case P1IV_P1IFG5:                                       // Pin 5
            __no_operation();
             break;
        case P1IV_P1IFG6:                                       // Pin 6
            __no_operation();
             break;
        case P1IV_P1IFG7:                                       // Pin 7
            __no_operation();
             break;
        default:
            _never_executed();
    }
}


#pragma vector=PORT2_VECTOR
__interrupt void pushbutton2_ISR (void)
{

    switch( __even_in_range( P2IV, P2IV_P2IFG7 ))
    {

        case P2IV_NONE:
             break;                               // None
        case P2IV_P2IFG0:                                      // Pin 0
             __no_operation();
             break;
        case P2IV_P2IFG1:                                       // Pin 1
           __no_operation();
             break;
        case P2IV_P2IFG2:                                       // Pin 2
            __no_operation();
             break;
        case P2IV_P2IFG3:                                       // Pin 3
            __no_operation();
             break;
        case P2IV_P2IFG4:                                       // Pin 4
            __no_operation();
             break;
        case P2IV_P2IFG5:                                       // Pin 5
            __no_operation();
             break;
        case P2IV_P2IFG6:                                       // Pin 6 (button 2)
           if ((*S2buttonDebounce) == 0)
           {
            *S2buttonDebounce = 1;                        // First high to low transition
            holdCount = 0;
            GPIO_toggleOutputOnPin( GPIO_PORT_P4, GPIO_PIN0 );

            if (*mode == MANUAL_MODE)
            {
                dutyCycle = dutyCycle-1000;
            }

            Timer_A_initUpMode(TIMER_A0_BASE, &initUpParam_A0);

           }
             break;
        case P2IV_P2IFG7:                                       // Pin 7
            __no_operation();
             break;
        default:
            _never_executed();
    }
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR (void)
{
    if (P1IN & BIT2)
        {
            *S1buttonDebounce = 0;                                   // Clear button debounce
            P1OUT &= ~BIT0;
        }

        // Button S2 released
        if (P2IN & BIT6)
        {
            *S2buttonDebounce = 0;                                   // Clear button debounce
            P4OUT &= ~BIT0;
        }


    if (!(P1IN & BIT2) && (P2IN & BIT6))
    {

        holdCount++;

        if (holdCount == 10)
        {
            if (*mode == STARTUP_MODE)
            (*mode) = AUTO_MODE;
            displayScrollText("");
            Timer_A_stop(TIMER_A0_BASE);

            __bic_SR_register_on_exit(LPM3_bits);
        }
    }


    if ((P1IN & BIT2) && !(P2IN & BIT6))
    { holdCount++;
    if (holdCount == 10)
    {

        if (*mode == STARTUP_MODE)
        (*mode) = MANUAL_MODE;
        displayScrollText("");
        Timer_A_stop(TIMER_A0_BASE);

        __bic_SR_register_on_exit(LPM3_bits);          // exit LPM3
    }
    }



}
