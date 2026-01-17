#include "buzzer.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h> 

void buzzer_init(void) {
    // Set PD3 sebagai Output
    DDRD |= (1 << PD3);
    
    // Pastikan logic awal 0 (LOW) agar diam
    PORTD &= ~(1 << PD3);
    
    // Timer tidak kita set disini dulu, biar anteng.
    // Nanti diaktifkan pas fungsi rusuh dipanggil.
    TCCR2A = 0;
    TCCR2B = 0; 
}

void buzzer_stop(void) {
    // 1. MATIKAN TIMER CLOCK
    TCCR2B = 0; 
    OCR2B = 0;

    // 2. PUTUSKAN TIMER DARI PIN (Disconnect OC2B)
    // Ini langkah kuncinya. Kita hapus bit COM2B1.
    // Jadi Pin PD3 kembali jadi pin I/O biasa, bukan pin PWM.
    TCCR2A &= ~(1 << COM2B1);

    // 3. PAKSA PIN KE GROUND (LOW)
    // Buat memastikan buzzer dapet tegangan 0V mutlak.
    PORTD &= ~(1 << PD3); 
}

void buzzer_rusuh(void) {
    // 1. SAMBUNGKAN TIMER KE PIN (Connect OC2B)
    // Kita set COM2B1 lagi biar PWM nyambung ke kaki buzzer
    TCCR2A = (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);

    // Loop efek suara
    for (int i = 0; i < 20; i++) { 
        // Nada Tinggi
        TCCR2B = (1 << CS22); 
        OCR2B = 60 + (rand() % 100); 
        _delay_ms(40);
        
        // Nada Rendah
        TCCR2B = (1 << CS22) | (1 << CS20);
        OCR2B = 20 + (rand() % 60);
        _delay_ms(40);
        
        // Efek Slide
        TCCR2B = (1 << CS21) | (1 << CS20);
        for(int j=10; j<150; j+=30){
             OCR2B = j;
             _delay_ms(5);
        }
    }
    
    // Matikan total setelah selesai
    buzzer_stop(); 
}