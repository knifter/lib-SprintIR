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
		float getPercent();
        // int getCompensatedPPM(const uint mbar);
		// float getCompensatedPercent(const uint32_t mbar);
		void zeroFreshAir();
		void zeroNitrogen();
		void setFilter(const uint32_t filter);

	private:
		void flush();
		void setMode(const uint32_t mode);
		void setDataTypes(const uint32_t mask_in);

		uint32_t command(const char cmd, const uint32_t* param = nullptr);

		Stream& _serial;
};

#endif // __SPRINTIR_H