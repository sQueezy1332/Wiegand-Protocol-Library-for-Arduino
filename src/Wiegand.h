#pragma once
#include "Arduino.h"
#if defined(ESP8266)
#define INTERRUPT_ATTR ICACHE_RAM_ATTR
#elif defined(ESP32)
#define INTERRUPT_ATTR IRAM_ATTR
#else
#define INTERRUPT_ATTR
#endif

class WIEGAND {

public:
	WIEGAND() { WIEGAND(2, 3);}
	WIEGAND(byte pinD0, byte pinD1);
	uint32_t getCode() { return _card; };
	byte getWiegandType() { return _wiegandType; };
	bool DoWiegandConversion();
private:
	static void ReadD0() INTERRUPT_ATTR;
	static void ReadD1() INTERRUPT_ATTR;
	uint32_t GetCardId();
	static volatile uint64_t	_bitstream;
	static volatile uint32_t 	_lastWiegand;
	static volatile byte		_bitCount;
	byte						_wiegandType;
	uint32_t 					_card;
};
