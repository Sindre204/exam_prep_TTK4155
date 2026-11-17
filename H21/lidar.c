#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>

#define f_CLK 8000000 // 8 MHz crystal
#define LIDAR_POWER_PIN PB0
#define f_TC3_CLK 1638400 // If one needs to change the timer clock, changing this variable wont change the actual prescaler. 


void LiDAR_on(void);
void LiDAR_off(void);
int16_t LiDAR_getrange_cm(void); // returns distance to object in cm

static volatile int16_t distance_in_cm = -1;
static volatile uint8_t start_of_pulse = 1;
static volatile uint16_t count;
static uint16_t start_count;
static uint16_t end_count;

/*
Use timer/Counter3 to measure the IR pulse output on mode control pin.
Power enable and triggering controlled på BPIO pins PB0 and PB1

*/

/*
CABLES
Black w resistor -> Trigger pin PB1
Yellow -> Modecontrol/monitoring ???
Yellow -> Mode Control connection ???
Black -> GND
Red -> Vdc
Orange -> Power enable PB0.

DESCRIPTION
We are using PWM interface with the LiDAR
*/

void LiDar_init(void) {
    // Set up the Power Pin
    PORTB &= ~(1 << LIDAR_POWER_PIN); // Turns the PBO/ LIDAR off
    DDRB |= (1 << DDB0); // Sets the PBO as an output

    // Set up the Control Pin
    PORTB &= ~(1 << PB1); // Drives the Control Pin Low, aka tells the LIDAR to measure.
    DDRB |= (1 << DDB1); // Configure PB1 as the Control Pin by making it an output pin

    // Set up Pulse Measuring ICP3 pin
    DDRD &= ~(1 << PD3); // Configure as input
    PORTD &= ~(1 << PD3); // Configure as pulldown

    // Set up Timer/Counter3 to capture the LiDAR Pulse
    TCCR3A = 0; // Normal mode
    TCCR3B |= (1 << CS31); // Prescaler = 8, we get 1MHz
    TCNT3 = 0;

    TCCR3B |= (1 << ICES3); // Trigger on rising edge
    ETIMSK |= (1 << TICIE3); // Enable external interrupts on the Measuring pin PD3
    sei();

}

void LiDAR_on(void){
    PORTB |= (1 << LIDAR_POWER_PIN); // Turns power on
    PORTB &= ~(1 << PB1); // Activate measuring
}

void LiDAR_off(void){
    PORTB &= ~(1 << LIDAR_POWER_PIN); // Turns power off
    PORTB |= (1 << PB1); // Deactivate measuring
}

int16_t LiDAR_getrange_cm(void){ // returns distance to object in cm
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        return distance_in_cm;
    }
}



ISR(TIMER3_CAPT_vector){

    count = ICR3; // Capture the counter

    if (start_of_pulse){
        start_count = count;
        start_of_pulse = 0;
        TCCR3B &= ~(1 << ICES3); // Trigger on falling edge
    }
    else{
        end_count = count;
        pulse_count = start_count - end_count;
        distance_in_cm = pulse_count/10;
        TCCR3B |= (1 << ICES3); // Trigger on rising edge

        start_of_pulse = 1;
    }

}

/*
CALCULATING FREQUENCY
We need a counter that does not overflow after 40ms. With 1,6MHz this works fine.
d[cm] = pulse_count/10 
*/