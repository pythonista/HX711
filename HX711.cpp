#include <Arduino.h>
#include "CyberLib.h"
#include "HX711.h"

HX711::HX711(int dout, int pd_sck, int gain)
{
	PD_SCK = pd_sck;
	DOUT = dout;

	D_Out(PD_SCK);
	D_In(DOUT);

	D_High(PD_SCK);
	delayMicroseconds(100);
	D_Low(PD_SCK);

	set_gain(gain);
}

void HX711::set_gain(int gain)
{
	switch (gain) {
		case 128:		// channel A, gain factor 128
			GAIN = 1;
			break;
		case 64:		// channel A, gain factor 64
			GAIN = 3;
			break;
		case 32:		// channel B, gain factor 32
			GAIN = 2;
			break;
	}

	D_Low(PD_SCK);
	read();
}

long HX711::read()
{
	// wait for the chip to become ready
	while (D_Read(DOUT) == HIGH);

    unsigned long value = 0;
    byte data[3] = { 0 };
    byte filler = 0x00;

	// pulse the clock pin 24 times to read the data
    data[2] = shiftIn(DOUT, PD_SCK, MSBFIRST);
    data[1] = shiftIn(DOUT, PD_SCK, MSBFIRST);
    data[0] = shiftIn(DOUT, PD_SCK, MSBFIRST);

	// set the channel and the gain factor for the next reading using the clock pin
	for (unsigned int i = 0; i < GAIN; i++)
    {
        D_High(PD_SCK);
        D_Low(PD_SCK);
    }

    // Datasheet indicates the value is returned as a two's complement value
    // Flip all the bits
    data[2] = ~data[2];
    data[1] = ~data[1];
    data[0] = ~data[0];

    // Replicate the most significant bit to pad out a 32-bit signed integer
    if ( data[2] & 0x80 )
    {
        filler = 0xFF;
    }
    else if ((0x7F == data[2]) && (0xFF == data[1]) && (0xFF == data[0]))
    {
        filler = 0xFF;
    }
    else
    {
        filler = 0x00;
	}

    // Construct a 32-bit signed integer
    value = ( static_cast<unsigned long>(filler) << 24
            | static_cast<unsigned long>(data[2]) << 16
            | static_cast<unsigned long>(data[1]) << 8
            | static_cast<unsigned long>(data[0]) );

    // ... and add 1
    Serial.println(value);
    return static_cast<long>(++value);
}

long HX711::read_average(int times)
{
	double sum = 0.;
	for (int i = 0; i < times; i++)
    {
		sum += read();
	}
	return sum / times;
}

long HX711::get_value(int times)
{
	return read_average(times) - OFFSET;
}

double HX711::get_units(int times)
{
	return get_value(times) / SCALE;
}

void HX711::tare(int times)
{
	set_offset(read_average(times));
}

void HX711::set_scale(double scale)
{
	SCALE = scale;
}

void HX711::set_offset(long offset)
{
	OFFSET = offset;
}

void HX711::power_down()
{
	D_Low(PD_SCK);
	D_High(PD_SCK);
}

void HX711::power_up() {
	D_Low(PD_SCK);
}
