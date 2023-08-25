#ifndef __SPRINTIR_H
#define __SPRINTIR_H

#include <Arduino.h>

class SprintIR
{
	public:
		SprintIR(Stream& stream) : _serial(stream) {};

		bool begin();
		// setFilter(filtermode_t filter);
		int getPPM();
        int getCompensatedPPM(uint P_mbar);
	private:
		Stream& _serial;
};

#endif // __SPRINTIR_H