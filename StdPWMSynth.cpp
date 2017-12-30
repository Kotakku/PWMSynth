#include "StdPWMSynth.hpp"
#include <Arduino.h>

Instrument::Instrument()
{
    reset();
}

void Instrument::reset()
{
    byte defaultADSR[] = {9, 255, 8, 3};
    setADSR(defaultADSR);
    setVoiseTable(defaultSinWave);
    posPhase.v32 = dposPhase.real = 0;
    isTone = false;
}
void Instrument::setADSR(byte *adsr)
{
    for(int i = 0; i < 4; i++)
        ADSR[i] = *(adsr + i);
}
void Instrument::setVoiseTable(byte *table) { wavetable = table; }
void Instrument::noteOn(byte note, byte volume)
{
    if(volume == 0)
        noteOff();
    else
    {
        dposPhase.real = pgm_read_dword(phase_speed_table + note);
        isTone = true;        
    }

}
void Instrument::noteOff()
{
    dposPhase.real = 0;
    posPhase.v32 = 0;
    isTone = false;
}

///////////////////////////////////////////////////////////////

PWMSynth::PWMSynth()
{
    PWMDAC_CREATE_WAVETABLE(defalutSineWave, PWMDAC_SINE_WAVE);
}

void PWMSynth::setup(bool startPWM)
{
    pinMode(PWMDAC_OUTPUT_PIN, OUTPUT);
    
    // No prescaling 分周なし
    sbi (TCCR2B, CS20);
    cbi (TCCR2B, CS21);
    cbi (TCCR2B, CS22);  
        
    // Phase-correct PWM　位相基準PWM
    sbi (TCCR2A, WGM20);
    cbi (TCCR2A, WGM21);
    cbi (TCCR2B, WGM22);
    
    //コンペアマッチB(3pin)でLOW
    cbi (TCCR2A, COM2B0);
    sbi (TCCR2A, COM2B1);
      
    //PWM value
    PWMDAC_OCR = 0;
        
    if(startPWM)
        sbi(TIMSK2,TOIE2); // Enable interrupt　タイマー2オーバーフロー割り込み許可
    
    systemReset();
}

void PWMSynth::systemReset()
{
    EACH_INST(i) i->reset();
}