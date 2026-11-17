#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>

#define f_CLK 4194304

static volatile uint8_t overflow_counter = 0;
static volatile uint8_t dutycycle = 0;

void sawtooth_generation(void){

    TCCR1B |= (1 << CS11); // Already defined by the task description.

    DDRE |= (1 << DDE2); // Configure the OC1B pin PE2 as output
    OCR1B = 0; // Start the sawtooth signal at 0V.
    
    TCCR1A |= (1 << WGM10); // Fast PWM mode 8-bit
    TCCR1B |= (1 << WGM12);
    
    TCCR1A |= (1 << COM1B1); // Start HIGH and go LOW on Compare


    sei(); // Global interrupts enable
    TIMSK |= (1 << TOIE1); // Overflow interrupt enable
}


ISR(TIMER1_OVF_vect){
    overflow_counter++;

    if (overflow_counter >= 8){ // Every 8 overflow we update the dutycycle
        overflow_counter = 0;
        dutycycle++;
        OCR1B = dutycycle;
    }    
}
