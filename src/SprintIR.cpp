#include "sprintir.h"

#define SPRINT_BUFSIZE	16

bool SprintIR::begin()
{
	return true;
};

int SprintIR::getPPM()
{
	//flush UART
	while(_serial.available())
		_serial.read();
	
	_serial.print("Z\r\n");
	char buf[SPRINT_BUFSIZE];
	int n=0;
	while(_serial.available())
	{
		buf[n++] = _serial.read();
		if(n > SPRINT_BUFSIZE)
		{
			// too many characters
			Serial.printf("Too many chars");
			return -1;
		};

	};
	buf[n] = '\0';

	int ppm;
	if(1 != sscanf(buf, "Z %5d\r\n", &ppm))
	{
		Serial.printf("Failed interpret");
		return -1;
	};
	Serial.printf("ppm: %d\n", ppm);
	return ppm;
};

