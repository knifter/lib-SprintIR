#include "sprintir.h"

// #include <tools-log.h>

#define SPRINT_BUFSIZE	    32
#define SPRINT_TIMEOUT_MS   100

bool SprintIR::begin()
{
    // Filtered co2 only
    setDataTypes(4);
    setFilter(32);

	return true;
};

void SprintIR::flush()
{
    char buf[SPRINT_BUFSIZE];

    // Flush UART
    while(int a = _serial.available())
    {   
        // DBG("flush %d", a);
        _serial.readBytes(buf, min(a, SPRINT_BUFSIZE-1));
        // DBG("[%s]", buf);
    };
};

void SprintIR::zeroFreshAir()
{
    command('G');
};

void SprintIR::zeroNitrogen()
{
    command('G');
};

// Modes:
// 0 = Command Mode, no auto output
// 1 = Streaming Mode, auto output configured values twice per second
// 2 = Polling Mode
void SprintIR::setMode(uint32_t mode)
{
    uint32_t param = mode | 0x03;
    command('K', &param);
};


// Flow Rate Recommended Digital Filter Setting ‘a’
// 0.1 litre/minute     64
// 0.5 litre/minute     32
// 1 litre/minute       16
// 5 litre/minute       8
void SprintIR::setFilter(uint32_t filter)
{
    command('A', &filter);
};

int SprintIR::getPPM()
{
    char buf[SPRINT_BUFSIZE];

    flush();

    // Wait for 'Z': filtered PPM (z is unfiltered)
    _serial.setTimeout(SPRINT_TIMEOUT_MS);
    size_t len = _serial.readBytesUntil('Z', buf, SPRINT_BUFSIZE-1);
    // buf[len] = 0x00;
    // DBG("%d bytes discarded, left=[%s]", len, buf);

    // ' #####\r\n' incoming
    len = _serial.readBytes(buf, 8);
    buf[len] = 0x00;
    // DBG("%d read: %s", len, buf);
    
	int ppm ;
	if(1 != sscanf(buf, " %05d\r\n", &ppm))
	{
		// ERROR("Failed to interpret SprintIR result.");
		return -1;
	};
    // For 0..60% sensors, scaling factor is 10 (Datasheet page 25)
	return ppm * 10;
};

// Polynomial blows up also doesn't make sense:
// plotted externally and the >1500 and <1500 don't only not match at 1500 but also have completely different sign and derivative
// int SprintIR::getCompensatedPPM(const uint32_t mbar)
// {
//     double C1 = getPPM();
//     if(C1 < 0)
//         return -1;

//     double Y;
//     if(C1 < 1500)
//         Y = 2.6661E-16*pow(C1, 4) - 1.1146E-12*pow(C1, 3) + 1.7397E-09*pow(C1, 2) - 1.2556E-06*C1 - 9.8754E-04;
//     else
//         Y = 2.37472E-30*pow(C1, 6) - 2.70695E-25*pow(C1, 5) + 1.24012E-20*pow(C1, 4) - 2.91716E-16*pow(C1, 3) + 3.62939E-12*pow(C1, 2) - 1.82753E-08*C1 - 1.35129E-03;
//     int res = C1 / (1.0+Y*(1013.0 - mbar));
//     // DBG("C1 = %d, Y = %f, mbar = %d, cppm = %d", C1, Y, mbar, res);
//     return res;
// };

// 0.000 000
//         X = ppm
// 0.0P      = percent
//    00 400 = 400ppm = 0.04%
//     1 000 = 1000pm = 0.10%
//    10 000 = 10.000ppm = 1%
// 0.050 000 = 5% = 50.000ppm
// 0.200 000 = 20% = 200.000ppm
float SprintIR::getPercent()
{
    int ppm = getPPM();
   	if(ppm < 0)
	{
        return NAN;
	};

    return ppm / 10000.0;
};

// float SprintIR::getCompensatedPercent(const uint32_t mbar)
// {
//     int ppm = getCompensatedPPM(mbar);
//    	if(ppm < 0)
// 	{
//         return NAN;
// 	};
    
//     return ppm / 10000.0;
// };

void SprintIR::setDataTypes(const uint32_t mask_in)
{
    uint32_t mask = mask_in & (2 | 4 | 64 | 4096);
    command('M', &mask);
};

// Commands, Response, description
// T            T #####        get tempereature (optional, 01000 otherwise)
// H            H #####        get humidity (optional, 00000 otherwise)
// K #          K #            switch control modes, default=1
// A #          A #####        set value of digital filter 0..65535, default=16
// a            A #####        get value of digital filter
// F #          F #####        Fine tune zero point
// G            G #####        Zero point on fresh air
// U            U #####        Zero point on nitrogen
// u #          u #####        Manual setting zero-point
// X #          X #####        Zero point using known concentraion
// P 8          P 8 #          Set co2 background for auto-zero, msb
// P 9          P 9 ###        Set co2 background for auto-zero, lsb
// P 10 #       P 8 #          Set co2 background for zero-point, msb
// P 11 #       P 9 ###        Set co2 background for zero-point, lsb
// F # #        F #####        Calibrates zero-point using known reading and concentration
// @ #.# #.#    @ #.# #.#      Set Auto-zeroing interval, in days
// S #          S #####        Set pressure compensation value
// s            S #####        Read pressure compensation value
// M #          M #####        Set number of measurement data types
// Q            # ## # ##      Report data fields
// Y            Y str\nstr\n   Read firmware version and serial number
uint32_t SprintIR::command(const char cmd, const uint32_t* param)
{
	//flush UART
	flush();

    // Assemble command
    char buf[SPRINT_BUFSIZE];
    if(param != nullptr)
    {
        snprintf(buf, SPRINT_BUFSIZE-1, "%c %u\r\n", cmd, *param);
    }else{
        snprintf(buf, SPRINT_BUFSIZE-1, "%c\r\n", cmd);
    };
    
    // send command
    // DBG("CMD send [%s]", buf);
	_serial.print(buf);

    // Wait for [cmd] character
    _serial.setTimeout(SPRINT_TIMEOUT_MS);
    size_t len = _serial.readBytesUntil(cmd, buf, SPRINT_BUFSIZE-1);
    // DBG("%d bytes discarded, left=[%s]", len, buf);

    // ' #####\r\n' incoming
    len = _serial.readBytesUntil('\r', buf, SPRINT_BUFSIZE-1);
    buf[len] = 0x00;
    // DBG("CMD result(%d bytes): %c%s", len+1, cmd, buf);

	int result = 0xFFFFFFFF;
	if(1 != sscanf(buf, " %05d\r\n", &result))
	{
		// ERROR("Failed to interpret result.");
		return 0xFFFFFFFF;
	};

    return result;
};
