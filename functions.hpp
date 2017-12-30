#ifndef _FUNCTION_HPP_
#define _FUNCTION_HPP_

#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#include <Arduino.h>

#ifndef PWMDAC_OUTPUT_PIN
#define PWMDAC_OUTPUT_PIN 3
#endif
#ifndef PWMDAC_NOTE_A_FREQUENCY
#define PWMDAC_NOTE_A_FREQUENCY 440 // [Hz]
#endif
#ifndef PWMDAC_POLYPHONY
#define PWMDAC_POLYPHONY 3 //楽器の数
#endif

#if PWMDAC_OUTPUT_PIN == 6 || PWMDAC_OUTPUT_PIN == 5
// In Arduino, TIMER0 has been reserved by wiring.c in Arduino core,
//   so defining PWMDAC_OUTPUT_PIN = 5 or 6 causes compile error
//   (multiple definition of `__vector_16')
#define PWMDAC_USE_TIMER0
#define PWMDAC_OVF_vect TIMER0_OVF_vect
#if PWMDAC_OUTPUT_PIN == 6
#define PWMDAC_OCR OCR0A
#else
#define PWMDAC_OCR OCR0B
#endif
#elif PWMDAC_OUTPUT_PIN == 9 || PWMDAC_OUTPUT_PIN == 10
#define PWMDAC_USE_TIMER1
#define PWMDAC_OVF_vect TIMER1_OVF_vect
#if PWMDAC_OUTPUT_PIN == 9
#define PWMDAC_OCR OCR1A  // OC1A maybe used as MOSI for SPI
#else
#define PWMDAC_OCR OCR1B  // OC1B maybe used as SS for SPI
#endif
#elif PWMDAC_OUTPUT_PIN == 11 || PWMDAC_OUTPUT_PIN == 3
#define PWMDAC_USE_TIMER2
#define PWMDAC_OVF_vect TIMER2_OVF_vect
#if PWMDAC_OUTPUT_PIN == 11
#define PWMDAC_OCR OCR2A
#else
#define PWMDAC_OCR OCR2B  // OC2B maybe used as INT1
#endif
#endif


// Function-to-array generator
#define FX2(f,x)    f(x), f(x + 1)
#define FX4(f,x)    FX2(f,x), FX2(f,x + 2)
#define FX8(f,x)    FX4(f,x), FX4(f,x + 4)
#define FX16(f,x)   FX8(f,x), FX8(f,x + 8)
#define FX32(f,x)   FX16(f,x),FX16(f,x + 16)
#define FX64(f,x)   FX32(f,x),FX32(f,x + 32)
#define FX128(f,x)  FX64(f,x),FX64(f,x + 64)
#define ARRAY128(f) {FX128(f,0)}
#define ARRAY256(f) {FX128(f,0),FX128(f,128)}

// Built-in wavetable generator
#define PWMDAC_SQUARE_WAVE(x)   (((x) < 128 ? 0 : 255) / PWMDAC_POLYPHONY)
#define PWMDAC_SAWTOOTH_WAVE(x) ((x) / PWMDAC_POLYPHONY)
#define PWMDAC_TRIANGLE_WAVE(x) (((x) < 128 ? (x) : 255 - (x)) * 2 / PWMDAC_POLYPHONY)
#define PWMDAC_TRIANGLE_WAVE2(x) (((x) < 128 ? (x) >> 1 : 255 - (x) >> 1) * 4 / PWMDAC_POLYPHONY)

#define SINPI(x, t) sin(PI * (x) / (t))
#define PWMDAC_MAX_VOLUME_SINE_WAVE(x)  ((SINPI(x,128) + 1) * 128)
#define PWMDAC_SINE_WAVE(x)     (PWMDAC_MAX_VOLUME_SINE_WAVE(x) / PWMDAC_POLYPHONY)
//#define PWMDAC_SHEPARD_TONE(x)  ((SINPI(x,128) + SINPI(x,64) + SINPI(x,32) + SINPI(x,16) + SINPI(x,8) + SINPI(x,4)  + SINPI(x,2) + 7 ) * 18.22 / PWMDAC_POLYPHONY)

#define PWMDAC_CREATE_WAVETABLE(table, function) PROGMEM const byte table[] = ARRAY256(function)

//周波数から位相の変化速度を求める
// (512 / F_CPU) -> PWM周期
#define FREQUENCY_TO_PHASE_SPEED 512 / F_CPU

//MIDIのノート番号から位相の変化速度を求める
#define NOTE_TO_PHASE_SPEED_OF(note_number) ( \
    pow( 2, (double)(note_number - 69)/12 + (sizeof(unsigned long) * 8)) \
        * PWMDAC_NOTE_A_FREQUENCY * FREQUENCY_TO_PHASE_SPEED)

#define EACH_INST(p) for(Instrument *(p) = Instruments; (p)<= Instruments + (PWMDAC_POLYPHONY - 1); (p)++)
#define IF_IS_THERE(ch) if(ch < PWMDAC_POLYPHONY) Instruments[ch].

#endif //_FUNCTION_H_