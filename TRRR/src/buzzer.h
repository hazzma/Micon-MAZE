#ifndef BUZZER_H
#define BUZZER_H

// Setup Timer 2 untuk buzzer di PD3
void buzzer_init(void);

// Mainkan efek suara rusuh (Blocking selama beberapa detik)
void buzzer_rusuh(void);

// Matikan suara
void buzzer_stop(void);

#endif