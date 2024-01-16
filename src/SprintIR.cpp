#include "sprintir.h"

#include <tools-log.h>

#define SPRINT_BUFSIZE	    32
#define SPRINT_TIMEOUT_MS   100

bool SprintIR::begin()
{
	return true;
};

int SprintIR::getPPM()
{
    char buf[SPRINT_BUFSIZE];

    // Flush UART
    while(int a = _serial.available())
    {   
        // DBG("flush %d", a);
        _serial.readBytes(buf, min(a, SPRINT_BUFSIZE-1));
    };

    // Wait for 'Z': filtered PPM (z is unfiltered)
    _serial.setTimeout(SPRINT_TIMEOUT_MS);
    size_t len = _serial.readBytesUntil('Z', buf, SPRINT_BUFSIZE-1);
    buf[len] = 0x00;
    // DBG("%d discarded [%s]", len, buf);

    // ' #####\r\n' incoming
    len = _serial.readBytes(buf, 8);
    buf[len] = 0x00;
    // DBG("%d read: %s", len, buf);
    
	int ppm ;
	if(1 != sscanf(buf, " %05d\r\n", &ppm))
	{
		// ERROR("Failed interpret");
		return -1;
	};
    // For 0..60% sensors, scaling factor is 10 (Datasheet page 25)
	return ppm * 10;
};

int SprintIR::getCompensatedPPM(uint P)
{
    int C1 = getPPM();
    double Y;
    if(C1 < 1500)
        Y = 2.6661E-16*pow(C1, 4) - 1.1146E-12*pow(C1, 3) + 1.7397E-09*pow(C1, 2) - 1.2556E-06*C1 - 9.8754E-04;
    else
        Y = 2.37472E-30*pow(C1, 6) - 2.70695E-25*pow(C1, 5) + 1.24012E-20*pow(C1, 4) - 2.91716E-16*pow(C1, 3) + 3.62939E-12*pow(C1, 2) - 1.82753E-08*C1 - 1.35129E-03;
    return getPPM() / (1+Y*(1013 - P));
};

// void SprintIR::command(const char* cmd)
// {
	// //flush UART
	// _serial.flush();
	
    // // send command
	// _serial.print(cmd);
    // // wait for first char
    // time_t start = millis();
    // while(_serial.available() < 1)
    // {
    //     DBG("w");
    //     if(millis() - start > SPRINT_TIMEOUT_MS)
    //     {
    //         ERROR("Timeout waiting for reply.");
    //         return -1;
    //     };
    // };

    // DBG("%d bytes waiting.", _serial.available());
    // delay(100);
    // DBG("%d bytes waiting.", _serial.available());

	// char buf[SPRINT_BUFSIZE];
	// int n=0;
	// while(_serial.available())
	// {
	// 	buf[n++] = _serial.read();
	// 	if(n == SPRINT_BUFSIZE-1)
	// 	{
	// 		// too many characters
    //         buf[n] = '\0';
	// 		ERROR("Too many chars [%s]", buf);
	// 		return -1;
	// 	};
	// };
    // if(n == 0)
    // {
    //     ERROR("No reply from SprintIR");
    //     return -1;
    // };
	// buf[n] = '\0';
    
    // DBG("read [%s]", buf);
// };
