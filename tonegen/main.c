#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define BUTTON_PIN PB2
#define LEFT_CHANNEL_PIN PB0
#define RIGHT_CHANNEL_PIN PB1
#define DEBOUNCE_DELAY 50 // milliseconds

volatile uint8_t currentFrequency = 0;
volatile uint8_t currentChannel = 0; // 0: Left, 1: Right, 2: Both, 3: Stopped

void initTimer0PWM() {
    DDRB |= (1 << LEFT_CHANNEL_PIN) | (1 << RIGHT_CHANNEL_PIN); // Set as output
    TCCR0A |= (1 << WGM01) | (1 << WGM00); // Fast PWM mode
    TCCR0B |= (1 << CS00); // No prescaling
}

void setFrequency(uint8_t freq) {
    switch (freq) {
        case 0: // ~1 kHz
            OCR0A = 124; // Placeholder value, adjust for actual frequency
            break;
        case 1: // ~2 kHz
            OCR0A = 62; // Placeholder value, adjust for actual frequency
            break;
        case 2: // ~4 kHz
            OCR0A = 31; // Placeholder value, adjust for actual frequency
            break;
        case 3: // ~8 kHz
            OCR0A = 15; // Placeholder value, adjust for actual frequency
            break;
    }
    TCCR0B = (TCCR0B & 0xF8) | (1 << CS00); // Apply no prescaling
}

void cycleChannel() {
    currentChannel = (currentChannel + 1) % 4; // Increment and wrap around, with stop state
    TCCR0A &= ~((1 << COM0A0) | (1 << COM0B0)); // Disable both channels initially
    
    if (currentChannel == 3) { // Stop state
        // PWM outputs are already disabled, nothing more to do
    } else {
        // Enable the appropriate channels
        if (currentChannel == 0 || currentChannel == 2) TCCR0A |= (1 << COM0A0);
        if (currentChannel == 1 || currentChannel == 2) TCCR0A |= (1 << COM0B0);
    }
}

void initButton() {
    DDRB &= ~(1 << BUTTON_PIN); // Button pin as input
    PORTB |= (1 << BUTTON_PIN); // Enable pull-up
    GIMSK |= (1 << PCIE); // Pin Change Interrupt Enable
    PCMSK |= (1 << PCINT2); // Mask for button pin
    sei(); // Enable global interrupts
}

ISR(PCINT0_vect) {
    _delay_ms(DEBOUNCE_DELAY); // Debounce
    if (!(PINB & (1 << BUTTON_PIN))) { // Check if button is still pressed
        if (currentChannel == 3) { // If in stop state, restart cycle
            currentFrequency = 0; // Optionally reset frequency
        }
        cycleChannel(); // Cycle through channels and stop state
        setFrequency(currentFrequency); // Re-apply frequency setting if not stopped
    }
}

int main(void) {
    initTimer0PWM();
    setFrequency(currentFrequency); // Set initial frequency
    initButton();

    while (1) {
        // Main loop does nothing, ISR handles the functionality
    }
}

