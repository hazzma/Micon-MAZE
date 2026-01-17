#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdbool.h>
#include "lcd.h"
#include "buzzer.h"

// --- 1. TUNING SERVO (Limit Max Turun) ---
#define SERVO_MIN 1150  
// Tadi 1850, sekarang 1820 biar gak mentok (turun ~2-3 derajat)
#define SERVO_MAX 1820  

#define SERVO_SPEED_STEP 12 

// --- Global Variables ---
bool game_running = false;
uint16_t game_ticks = 0;   // Penghitung detak loop
uint16_t game_seconds = 0; // Penghitung detik total
uint8_t last_ir_state = 1;

// Posisi Servo
uint16_t current_pos_1 = 1500; 
uint16_t current_pos_2 = 1500; 

// Setup Hardware Low Level
void servo_write_hardware(uint8_t channel, uint16_t us) {
    if (channel == 1) OCR1A = us * 2;
    if (channel == 2) OCR1B = us * 2;
}

void servos_init(void) {
    DDRB |= (1 << PB1) | (1 << PB2);
    // Timer 1 Config
    TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM11);
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);
    ICR1 = 40000;
    
    // Posisi Awal Tengah
    OCR1A = 3000; 
    OCR1B = 3000; 
}

uint16_t update_servo_smooth(uint16_t current_pos, uint16_t target_pos) {
    if (current_pos < target_pos) {
        current_pos += SERVO_SPEED_STEP; 
        if (current_pos > target_pos) current_pos = target_pos; 
    } 
    else if (current_pos > target_pos) {
        current_pos -= SERVO_SPEED_STEP; 
        if (current_pos < target_pos) current_pos = target_pos;
    }
    return current_pos;
}

void adc_init(void) {
    DDRC &= ~((1 << PC0) | (1 << PC1) | (1 << PC2) | (1 << PC3));
    PORTC |= (1 << PC2) | (1 << PC3); 
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    ADMUX = (1 << REFS0);
}

uint16_t read_adc(uint8_t channel) {
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADC;
}

// --- FUNGSI TAMPILAN WAKTU ---
void update_timer_display(void) {
    char buffer[16];
    uint8_t minutes = game_seconds / 60;
    uint8_t seconds = game_seconds % 60;
    
    lcd_gotoxy(0, 1);
    // Format: "Time: 01:25   "
    sprintf(buffer, "Time: %02d:%02d   ", minutes, seconds);
    lcd_puts(buffer);
}

int main(void) {
    initlcd();
    servos_init();
    buzzer_init();
    adc_init();

    // Intro
    lcd_clear();
    lcd_puts("MAZE RUNNER");
    lcd_gotoxy(0, 1);
    lcd_puts("Siap-siap...");
    _delay_ms(1000);
    
    // Tampilan Standby
    lcd_clear();
    lcd_puts("TEKAN TOMBOL");
    lcd_gotoxy(0, 1);
    lcd_puts("UNTUK START!");

    while (1) {
        // --- 1. SERVO CONTROL (Selalu Aktif) ---
        uint16_t val_x = read_adc(0); 
        uint16_t val_y = read_adc(1);

        // Mapping (Arah disesuaikan request sebelumnya: 1 Normal, 1 Balik)
        uint32_t target_x = SERVO_MIN + ((uint32_t)val_x * (SERVO_MAX - SERVO_MIN)) / 1023;
        uint32_t target_y = SERVO_MAX - ((uint32_t)val_y * (SERVO_MAX - SERVO_MIN)) / 1023;

        current_pos_1 = update_servo_smooth(current_pos_1, (uint16_t)target_x);
        current_pos_2 = update_servo_smooth(current_pos_2, (uint16_t)target_y);

        servo_write_hardware(1, current_pos_1);
        servo_write_hardware(2, current_pos_2);

        // --- 2. GAME LOGIC ---
        
        // A. CEK TOMBOL START/RESET (Active LOW)
        if (!(PINC & (1 << PC2))) {
            _delay_ms(50); // Debounce
            if (!(PINC & (1 << PC2))) {
                // Reset Game
                game_running = true;
                game_seconds = 0;
                game_ticks = 0;
                
                buzzer_stop();
                lcd_clear();
                lcd_puts("GO! CARI JALAN");
                update_timer_display();
                
                // Tunggu tombol dilepas biar gak restart trs
                while(!(PINC & (1 << PC2))); 
            }
        }

        // B. CEK SENSOR GOAL (Active LOW)
        uint8_t current_ir = (PINC & (1 << PC3));
        
        // Jika Goal terjadi DAN game sedang berjalan
        if (game_running && last_ir_state && !current_ir) {
            game_running = false; // Matikan Timer
            
            // Efek Visual & Suara
            lcd_clear();
            lcd_gotoxy(4, 0); 
            lcd_puts("GOAALLL!!");
            
            // Tampilkan Waktu Akhir di baris bawah
            update_timer_display(); 
            
            // Bunyi Rusuh
            buzzer_rusuh(); 
            
            // Balikin text biar user tau waktu finalnya
            lcd_gotoxy(4, 0); 
            lcd_puts("SELESAI !!  ");
        }
        last_ir_state = current_ir;

        // C. LOGIC PENGHITUNG WAKTU (TIMER)
        if (game_running) {
            game_ticks++;
            // Delay loop kita 15ms. 
            // 1 detik = 1000ms. 
            // 1000 / 15 = 66.6 ticks per detik.
            if (game_ticks >= 66) { 
                game_seconds++;
                game_ticks = 0;
                update_timer_display();
            }
        }

        _delay_ms(15); // Main Loop Delay
    }
}