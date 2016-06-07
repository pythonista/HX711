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
	delay_us(100);
	D_Low(PD_SCK);

	set_gain(gain);
}

void HX711::set_gain(int gain)
{
	switch (gain)
	{
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

bool HX711::is_ready()
{
	return D_Read(DOUT) == LOW;
}

long HX711::read()
{
	// wait for the chip to become ready
	while (!is_ready());

	byte data[3];

	// pulse the clock pin 24 times to read the data
	for (byte j = 3; j--;)
    {
		for (char i = 8; i--;)
		{
			D_High(PD_SCK);
			bitWrite(data[j], i, D_Read(DOUT));
			D_Low(PD_SCK);
		}
	}

	// set the channel and the gain factor for the next reading using the clock pin
	for (int i = 0; i < GAIN; i++)
    {
		D_High(PD_SCK);
		D_Low(PD_SCK);
	}

	data[2] ^= 0x80;

	return ((uint32_t) data[2] << 16) | ((uint32_t) data[1] << 8) | (uint32_t) data[0];
}

long HX711::read_average(int times)
{
	long sum = 0;
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

float HX711::get_units(int times)
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
