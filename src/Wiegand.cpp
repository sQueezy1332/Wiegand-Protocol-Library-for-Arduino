#include "Wiegand.h"

WIEGAND::WIEGAND(byte pinD0, byte pinD1) {
	_lastWiegand = 0, _bitstream = 0, _card = 0, _wiegandType = 0, _bitCount = 0;
	pinMode(pinD0, INPUT);					// Set D0 pin as input
	pinMode(pinD1, INPUT);					// Set D1 pin as input
	attachInterrupt(digitalPinToInterrupt(pinD0), ReadD0, FALLING);  // Hardware interrupt - high to low pulse
	attachInterrupt(digitalPinToInterrupt(pinD1), ReadD1, FALLING);  // Hardware interrupt - high to low pulse
}

void WIEGAND::ReadD0() {
	_lastWiegand = millis();	// Keep track of last wiegand bit received
	_bitCount++;				// Increament bit count for Interrupt connected to D0
	_bitstream <<= 1;				// D0 represent binary 0, so just left shift card data
}

void WIEGAND::ReadD1() {
	ReadD0();
	_bitstream |= 1;			// D1 represent binary 1, so OR card data with 1 then

}

uint32_t WIEGAND::GetCardId() {
	if (_bitCount == 26)								// EM tag
		return _bitstream & 0x1FFFFFE;

	if (_bitCount == 24)
		return _bitstream & 0x7FFFFE;

	if (_bitCount == 34)								// Mifare 
		return _bitstream >> 1;
	if (_bitCount == 32) return _bitstream & 0x7FFFFFFE;
	return _bitstream;										// EM tag or Mifare without parity bits
}

byte translateEnterEscapeKeyPress(byte originalKeyPress) {
	switch (originalKeyPress) {
	case 0xB:   return 0x0D;   // 11 or * key// 13 or ASCII ENTER
	case 0xA:   return 0x1B;   // 10 or # key // 27 or ASCII ESCAPE
	default: return originalKeyPress;
	}
}

bool WIEGAND::DoWiegandConversion() {
	if (_bitCount) {							// if no more signal coming through after 25ms
		uint32_t sysTick = millis();
		if (sysTick - _lastWiegand > 25) {
			bool ret;
			noInterrupts();
			if ((_bitCount == 24) || (_bitCount == 26) || (_bitCount == 32) || (_bitCount == 34)) {  	// bitCount for keypress=4 or 8, Wiegand 26=24 or 26, Wiegand 34=32 or 34
				_card = GetCardId();
				_wiegandType = _bitCount;
				ret = true;
			}
			else if (_bitCount == 8) {		// keypress wiegand with integrity
					// 8-bit Wiegand keyboard data, high nibble is the "NOT" of low nibble
					// eg if key 1 pressed, data=E1 in binary 11100001 , high nibble=1110 , low nibble = 0001 
				byte highNibble = (_bitstream & 0xF0) >> 4;
				byte lowNibble = (_bitstream & 0x0F);
				_wiegandType = _bitCount;
				if (lowNibble == (~highNibble & 0x0F)) {	// check if low nibble matches the "NOT" of high nibble.
					_card = translateEnterEscapeKeyPress(lowNibble);
					ret = true;
				}
				else ret = false;
				// TODO: Handle validation failure case!
			}
			else if (_bitCount == 4) {
				// 4-bit Wiegand codes have no data integrity check so we just
				// read the LOW nibble.
				_card = translateEnterEscapeKeyPress(_bitstream & 0xF);
				_wiegandType = _bitCount;
				ret = true;
			}
			else ret = false; // well time over 25 ms and bitCount !=8 , !=26, !=34 , must be noise or nothing then.
			_lastWiegand = sysTick;
			_bitCount = 0;
			_bitstream = 0;
			interrupts();
			return ret;
		}
	} return false;

}
