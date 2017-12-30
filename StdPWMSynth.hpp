#ifndef _PWMSYNTH_H_
#define _PWMSYNTH_H_

#include "functions.hpp"

PROGMEM const unsigned long phase_speed_table[] = ARRAY128(NOTE_TO_PHASE_SPEED_OF);
PWMDAC_CREATE_WAVETABLE(defaultSinWave, PWMDAC_SINE_WAVE);

class Instrument
{
public:
	Instrument();
	void reset();
	void setADSR(byte *adsr);
	void setVoiseTable(byte *table);
	byte conf_table(byte pos) { return pgm_read_byte(wavetable + pos); }
	void noteOn(byte note, byte volume);
	void noteOff();
	inline unsigned int Instrument::nextPulseWidth() { return getVolume() * getNextWavePosValue(); }	

protected:
	inline byte Instrument::getVolume() { return (isTone ? ADSR[2] : 0); }
	inline byte Instrument::getNextWavePosValue()
	{
		posPhase.v32 += dposPhase.real;
		return pgm_read_byte( wavetable + posPhase.v8[3]); //32bit精度の上位8bitを使う
	}

	byte *wavetable;
	byte ADSR[4];
	byte volumePhase;
	bool isTone;
	union { unsigned long v32; byte v8[4];} posPhase; //テーブルから読み出す位相位置データ
	struct { unsigned long real; } dposPhase; //割り込みで位相の位置を進める数

};

class PWMSynth
{
public:
	PWMSynth();
	void setup(bool startPWM = true);
	void systemReset();
	inline static void PWMSynth::updatePulseWidth()
	{
		unsigned int pw = 0;
		EACH_INST(i) pw += i->nextPulseWidth();
		//Serial.println(pw >> 8);
		PWMDAC_OCR = (pw >> 8);
	}

	void setADSR(byte ch, byte *adsr) { IF_IS_THERE(ch)setADSR(adsr); }
	void setVoiseTable(byte ch, byte *table) { IF_IS_THERE(ch)setVoiseTable(table); }
	void noteOn(byte ch, byte note, byte volume) { IF_IS_THERE(ch)noteOn(note, volume); }
	void noteOff(byte ch) { IF_IS_THERE(ch)noteOff(); }

protected:
	static Instrument Instruments[PWMDAC_POLYPHONY];
};

#define CREATE_INST() \
ISR(PWMDAC_OVF_vect) { PWMSynth::updatePulseWidth(); } \
Instrument PWMSynth::Instruments[PWMDAC_POLYPHONY];

//割り込みの関数をそのまま置くと重複定義される(なんで?)
//Instrument PWMSynth::Instruments[PWMDAC_POLYPHONY];はよくわからん

#endif //_PWMSYNTH_H_
