#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h> // Include for rand() and abs()

#define DATA_PIN    PB0
#define CLOCK_PIN   PB1
#define LATCH_PIN   PB2
#define BUTTON_PIN  PB3

#define VOLUME_HISTORY_SIZE 16

#define TOTAL_MODES 13

uint8_t volumeHistory[VOLUME_HISTORY_SIZE];
uint8_t volumeHistoryIndex = 0;
uint16_t volumeSum = 0;

volatile uint8_t mode = 0; // Modes: 0-Off, 1-VU, 2-Knight Rider, 3-Inverse, 4-Center-Out, 5-Ping-Pong

void initSystem() {
    ADMUX = (1 << REFS0); // AVcc as the reference
    ADCSRA = (1 << ADEN) | (1 << ADPS1) | (1 << ADPS0); // Enable ADC, prescaler = 8
    
    DDRB |= (1 << DATA_PIN) | (1 << CLOCK_PIN) | (1 << LATCH_PIN); // Set as output
    DDRB &= ~(1 << BUTTON_PIN); // Button pin as input
    PORTB |= (1 << BUTTON_PIN); // Enable pull-up
    GIMSK |= (1 << PCIE); // Pin change interrupt enable
    PCMSK |= (1 << PCINT3); // Pin change interrupt for BUTTON_PIN
    sei(); // Enable global interrupts
}

void shiftOut(uint8_t data) {
    PORTB &= ~(1 << LATCH_PIN);
    for (int8_t i = 7; i >= 0; i--) {
        PORTB &= ~(1 << CLOCK_PIN);
        if (data & (1 << i)) PORTB |= (1 << DATA_PIN);
        else PORTB &= ~(1 << DATA_PIN);
        PORTB |= (1 << CLOCK_PIN);
    }
    PORTB |= (1 << LATCH_PIN);
}

// Adjust the main loop and ISR to include the new TOTAL_MODES
ISR(PCINT0_vect) {
    if (!(PINB & (1 << BUTTON_PIN))) { // Button press detected
        _delay_ms(50); // Debounce delay
        if (!(PINB & (1 << BUTTON_PIN))) {
            mode = (mode + 1) % TOTAL_MODES; // Now includes Raindrop Effect
        }
    }
}


uint8_t readVolume() {
    ADCSRA |= (1 << ADSC); // Start conversion
    while (ADCSRA & (1 << ADSC)); // Wait for completion
    return ADC >> 2; // Simplify to 8-bit
}

void updateVolumeHistory(uint8_t volume) {
    volumeSum -= volumeHistory[volumeHistoryIndex];
    volumeHistory[volumeHistoryIndex] = volume;
    volumeSum += volume;
    volumeHistoryIndex = (volumeHistoryIndex + 1) % VOLUME_HISTORY_SIZE;
}

void processVolume() {
    uint8_t volume = readVolume();
    updateVolumeHistory(volume);
    uint8_t averageVolume = volumeSum / VOLUME_HISTORY_SIZE;
    uint8_t ledPattern = 0xFF >> (8 - (averageVolume >> 3)); // For VU and Inverse modes

    switch (mode) {
        case 1: // VU Mode
            ledPattern = 0xFF << (averageVolume >> 3);
            break;
        case 3: // Inverse Mode
            ledPattern = ~ledPattern;
            break;
        case 4: // Center-Out Mode
            ledPattern = 0;
            uint8_t centerPattern = (averageVolume >> 4) + 1;
            for(uint8_t i = 0; i < centerPattern; ++i) {
                ledPattern |= (1 << (4 - i)) | (1 << (3 + i));
            }
            break;
    }
    shiftOut(ledPattern);
}


void displayKnightRider() {
    static uint8_t position = 0;
    static int8_t direction = 1;
    uint8_t ledPattern = 1 << position;
    shiftOut(ledPattern);
    position += direction;
    if (position == 0 || position == 7) direction = -direction;
    _delay_ms(100);
}

void displayPingPongBounce() {
    static uint8_t width = 1; // Width of the lit section
    static int8_t direction = 1; // Direction of growth or shrinkage: 1 for grow, -1 for shrink
    uint8_t ledPattern = 0;

    // Calculate the LED pattern based on the current width
    for (uint8_t i = 0; i < width; ++i) {
        ledPattern |= (1 << i) | (1 << (7 - i));
    }

    // Display the pattern
    shiftOut(ledPattern);

    // Update width and direction
    if (width == 4 || width == 1) direction *= -1; // Change direction at extremes
    width += direction;

    _delay_ms(100); // Adjust timing as needed
}

void displayRaindropEffect() {
    static uint8_t lastDrop = 0; // Track the last "raindrop" position

    // Clear the last drop (optional, depends on desired effect)
    shiftOut(0x00);
    _delay_ms(100); // Pause before the next drop

    // Generate a new drop at a random position
    uint8_t dropPosition = rand() % 8; // Assuming rand() is seeded/initiated elsewhere
    lastDrop = (1 << dropPosition);
    shiftOut(lastDrop);

    // Optionally, add a fading effect here if PWM control is available
    // This would involve gradually decreasing the brightness of the lastDrop LED
}

void displaySpectrumAnalyzer() {
    uint8_t volume = readVolume(); // Read the current volume level
    uint8_t ledPattern = 0;

    // Divide the volume range into 8 segments and set LEDs accordingly
    if (volume < 32) ledPattern = 0x01; // Lowest volume segment
    else if (volume < 64) ledPattern = 0x03;
    else if (volume < 96) ledPattern = 0x07;
    else if (volume < 128) ledPattern = 0x0F;
    else if (volume < 160) ledPattern = 0x1F;
    else if (volume < 192) ledPattern = 0x3F;
    else if (volume < 224) ledPattern = 0x7F;
    else ledPattern = 0xFF; // Highest volume segment

    shiftOut(ledPattern); // Display the LED pattern
}

void displayWaveformSimulation() {
    static uint8_t lastVolume = 0;
    uint8_t volume = readVolume(); // Read current volume level
    uint8_t ledPattern = 0;

    // Map the volume to LED pattern more dynamically
    uint8_t volumeDifference = abs(volume - lastVolume); // Get volume change magnitude
    uint8_t ledsToLight = volumeDifference / 32; // Determine LEDs to light based on change

    if (ledsToLight > 8) ledsToLight = 8; // Cap at 8 LEDs
    ledPattern = (1 << ledsToLight) - 1; // Create LED pattern

    shiftOut(ledPattern); // Display the LED pattern
    lastVolume = volume; // Update last volume for the next cycle
    _delay_ms(50); // Adjust for desired simulation speed
}

void displayHeartbeatPulse() {
    // Define the heartbeat pattern: a quick double pulse
    uint8_t heartbeatPattern[] = {0x18, 0x3C, 0x7E, 0xFF, 0x7E, 0x3C, 0x18, 0x00, // First pulse
                                  0x00, // Short pause between pulses
                                  0x18, 0x3C, 0x7E, 0xFF, 0x7E, 0x3C, 0x18, 0x00, // Second pulse
                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Longer pause after the pulses
                                  };

    // Display the heartbeat pattern
    for (uint8_t i = 0; i < sizeof(heartbeatPattern); ++i) {
        shiftOut(heartbeatPattern[i]);
        _delay_ms(75); // Adjust timing as needed for visual effect
    }
}

void displayFireworks() {
    // Launch phase - a single LED lights up moving upwards
    for (uint8_t i = 0; i < 4; ++i) {
        shiftOut(1 << i);
        _delay_ms(150); // Adjust for visual effect
    }

    // Explosion phase - multiple LEDs light up
    shiftOut(0xFF); // All LEDs on to simulate the explosion
    _delay_ms(300); // Explosion pause

    // Fade phase - LEDs turn off sequentially to simulate fading
    for (uint8_t i = 0; i < 8; ++i) {
        shiftOut(0xFF >> i); // Sequentially turn off LEDs
        _delay_ms(150); // Adjust for fading effect
    }

    // Ensure a pause between fireworks
    shiftOut(0x00); // All LEDs off
    _delay_ms(500); // Pause before the next firework
}

void displayStrobeEffect() {
    // Define the strobe on and off durations
    const uint8_t strobeOnTime = 50; // Time in milliseconds LEDs are on
    const uint8_t strobeOffTime = 50; // Time in milliseconds LEDs are off
    
    // Turn all LEDs on
    shiftOut(0xFF);
    _delay_ms(strobeOnTime);
    
    // Turn all LEDs off
    shiftOut(0x00);
    _delay_ms(strobeOffTime);
}

void displayRandomSparkle() {
    uint8_t ledPattern = 0;
    for (uint8_t i = 0; i < 8; ++i) {
        if (rand() % 2) {  // Randomly decide to light up each LED
            ledPattern |= (1 << (rand() % 8));  // Choose a random LED to toggle
        }
    }
    shiftOut(ledPattern);
    _delay_ms(100);  // Adjust delay for desired sparkle effect speed
}

void displayRandomFlash() {
    uint8_t ledPattern = 0;
    for (uint8_t i = 0; i < 8; ++i) {
        // Generate a random pattern
        if (rand() % 2) {  // Decide randomly to turn on each LED
            ledPattern |= (1 << (rand() % 8));  // Select a random LED
        }
    }
    shiftOut(ledPattern); // Flash the random pattern
    _delay_ms(5); // Adjust for desired flashing speed
}



int main(void) {
    initSystem();

    while (1) {
        switch (mode) {
            case 0: shiftOut(0x00); break; // Off Mode
            case 2: displayKnightRider(); break; // Knight Rider Mode
			case 5: displayPingPongBounce(); break; // New Ping-Pong Bounce Mode
			case 6: displayRaindropEffect(); break;
			case 7: displaySpectrumAnalyzer(); break;
			case 8: displayWaveformSimulation(); break;
			case 9: displayHeartbeatPulse(); break;
			case 10: displayFireworks(); break;
			case 11: displayStrobeEffect(); break;
			case 12: displayRandomSparkle(); break;
			case 13: displayRandomFlash(); break;
            default: processVolume(); break; // Handles VU, Inverse, and Center-Out Modes
        }
        _delay_ms(50); // Main loop delay
    }
}
