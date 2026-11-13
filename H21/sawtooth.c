#include <avr/io.h>

#define f_CLK 4194304
#define TC1_CLK 524288
#define one_sec_period 524288
/*
I need to:
    1. Set up the T/C1 to be able to output a PWM signal.
    2. Set up the logic that makes the output the sawtooth pattern shown in the picture.
    3. Set up the interrupt logic so that this gets turned on and runs in the background 
        without being affected by the main instruction flow
*/

void DAC_init(void){
    /*
    TCNT1
    A match from TCNT1 and OCR1B will set the OCF1B Flag and if OCIE1B = 1 then an output 
    compare interrupt will be generated.

    What if i do period = 1000ms, and the vlue that is outputted is the counter TCNT1B normalized around 0 and 255?
    In order to get a period of 1s we need to reset the value after 524288 ticks. We therefore need to use a 16-bit timer,
    and a 16-bit compare value of 524288
    */

    // TCCR1B |= (00000010) // Already defined by the task description.
    // TODO: Activate clock signal on TCN1

    OCR1BH = (one_sec_period >> 8); // Sets the value for the compare register.
    OCR1BL = (one_sec_period);
    TCCR1B |= (1 << WGM12); // Turns on reset for TCNT1B on compare.
    

    DDRB |= (1 << OC1B); // Set the pin as output


}