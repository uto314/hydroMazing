/*
* @file commons.h
* Copyright (C) 2014 Cory J. Potter - All Rights Reserved
* You may use, distribute and modify this code under the
* terms of the LICENSE.txt
* NOT INTENDED FOR COMMERCIAL USE!
* You should have received a copy of the LICENSE.txt with
* this file. If not, please write to: <bitsandbots@gmail.com>
*/

#ifndef __COMMONS_H__
#define __COMMONS_H__

#ifdef ARDUINO

const unsigned long DEFAULT_TIME = 1420092000; // 06:00:00, 1/1/2015

#define READY_BUFFER 300

// clocks
unsigned long time_now;
unsigned long time_diff;
unsigned long time_millis;
boolean clockFlag = false;
//

/************************************************************************/
/*  ANALOG PIN DEFINITIONS                                              */
/************************************************************************/
#define PIN_A0 A0
#define PIN_A1 A1
#define PIN_A2 A2
#define PIN_A3 A3
#define PIN_A4_I2C_SDA A4
#define PIN_A5_I2C_SCL A5
#define PIN_A6 A6
#define PIN_A7 A7
/************************************************************************/
/*  DIGITAL PIN DEFINITIONS                                             */
/************************************************************************/

#define PIN0_RX 0
#define PIN1_TX 1
#define PIN2_INT0 2  //interrupt 0
#define PIN3_INT1 3  //interrupt 1
#define PIN4 4
#define PIN5 5
#define PIN6 6
#define PIN7 7
#define PIN8 8
#define PIN9_RF_CE 9
/** SPI CONNECTIONS **/
#define PIN10_CS 10  // Reserved for ChipSelect
/*
* Note that even if you don't use the hardware CS/SS pin,
* it must be left as an output or the SD library won't work.
*/
#define PIN11_MISO 11
#define PIN12_MOSI 12
#define PIN13_SCK 13
//LED_BUILTIN = 13

#define OFF false
#define ON  true

enum SENSOR_TYPE {
	SENSOR_UNKNOWN,
	SENSOR_TEMPF,
	SENSOR_TEMPC,
	SENSOR_WATER_TEMPF,
	SENSOR_HUMIDITY,
	SENSOR_MOISTURE,
	SENSOR_PHOTO,
	SENSOR_SOUND,
	SENSOR_MOTION,
	SENSOR_FLOAT,
	SENSOR_FLOW,
	SENSOR_PRESSURE,
	SENSOR_CURRENT,
	SENSOR_VOLTAGE,
SENSOR_ULTRASONIC };

enum APPLIANCE_TYPE {
	APPLIANCE_UNKNOWN,
	APPLIANCE_INTAKE_FAN,
	APPLIANCE_EXHAUST_FAN,
	APPLIANCE_CIRCULATION_FAN,
	APPLIANCE_LIGHT,
	APPLIANCE_PUMP,
	APPLIANCE_HUMIDIFIER,
APPLIANCE_HEATER};

enum NETWORK_TYPE {
	NETWORK_TX_PIPE,
	NETWORK_RX_PIPE,
	NETWORK_RCS_ON,
NETWORK_RCS_OFF };


typedef struct DataObject
{
	char object;
	uint8_t node_address;
	unsigned long timestamp;
	byte type;
	uint8_t freq;
	float value;
	bool ready;
	bool triggered;
	bool state;
} DataObject;

typedef struct Sensor
{
	uint8_t pin;		// PIN_A2
	uint8_t node_address;
	SENSOR_TYPE type;		// The type of the sensor (ex: SENSOR_TEMPF)
	uint8_t freq;		// How frequently should the sensor take a reading?
	uint8_t minVal;		// 65.0
	uint8_t maxVal;		// 75.5
	unsigned long timestamp;
	float value;
	struct Sensor *next;
} Sensor;

typedef struct Appliance
{
	uint8_t pin;			// i.e.,PIN_A2
	uint8_t node_address;
	APPLIANCE_TYPE type;	// The type of the sensor (ex: SENSOR_TEMPF)
	unsigned long timestamp;
	bool ready;
	bool triggered;
	bool state;
	struct Appliance *next;
} Appliance;


bool DEBUG = false;
void isDebugMode ( bool forceDebug )
{
	Serial.begin(56000);
//	printf_begin();
	Serial.println(F("DEBUG?"));
	//Wait for 5 seconds or until data is available on serial,
	//whichever occurs first.
	int test = 0;
	while( millis() < 5000 ) {
		test = Serial.read(); //We then clear the input buffer
		if (forceDebug) { test = forceDebug; }
		if ( test > 0 )
		{
			//If data is available, we enter here.
			//Serial.print(F("received:  ") );
			Serial.println( test );
			//On timeout or availability of data, we come here.
			DEBUG = true; //Enable debug
			break;
		}
	} // end while
	
	//	if ( DEBUG == false ) { Serial.println("Goodbye!"); Serial.end(); }
}

/***********************************************************************
*
* Interrupt Service Routine (ISR)
*
* When writing an Interrupt Service Routine (ISR):
*** Keep it short
*** Don't use delay ()
*** Don't do serial prints
*** Make variables shared with the main code: volatile
*** Variables shared with main code may need to be protected by "critical sections"
*** Don't try to turn interrupts off or on
************************************************************************/

//void interrupted() {
// digitalWrite(PIN8, HIGH);
//}

/*
What free_ram() is actually reporting
is the space between the heap and the stack.
it does not report any de-allocated memory
that is buried in the heap.
Buried heap space is not usable by the stack,
and may be fragmented enough that it is not
usable for many heap allocations either.
The space between the heap and the stack
is what you really need to monitor if you are
trying to avoid stack crashes.
*/

int freeRam () {
	extern int __heap_start, *__brkval;
	int v;
	return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void errorHandler ( uint8_t result )
{
	if ( DEBUG ) {
		switch ( result )
		{
			case 0: //success
			break;
			case 1: Serial.println(F("1 - Waiting for Start bit")); break;
			case 2: Serial.println(F("2 - slave in transmit mode")); break;
			case 3: Serial.println(F("3 - Sending to slave")); break;
			case 4: Serial.println(F("4 - Repeated Start")); break;
			case 5: Serial.println(F("5 - Slave in receiver mode")); break;
			case 6: Serial.println(F("6 - Receiving from slave")); break;
			case 7: Serial.println(F("7 - Stop bit")); break;
		}
	}
}

void conservePower( void )
{
	for ( uint8_t _p = 0; _p < 8; _p++ )
	{
		digitalWrite( _p, LOW );
	}
}

/**
Temperature is returned in milli-°C. So 25000 is 25°C.

How it works
The chip has an internal switch that selects which pin the analogue to digital converter reads. That switch has a few leftover connections, so the chip designers wired them up to useful signals. One of those signals is that simple temperature sensor.

If you measure the sensor voltage against the internal precision 1.1V reference, you can calculate approximate temperature. It requires a bit of messing around with the registers, but it can be done. That is how this works.

Additional notes
The sensor isn't very accurate - the data sheet says ±10°C. But once you've worked out the offset and correct for it, accuracy improves.

Note the following:

This works on Arduinos using CPU's with '8P' in the part number. For standard Arduinos, that means 328 only.
If you have an Arduino clone with an ATmega168P or ATmega168PA, it will work there too. It will not work with an ATmega168. (Thanks @blalor)
This sensor is pretty useless unless you calibrate it against a known temperature.
The sensor outputs in approximately 1°C steps.
But hey - it's free!

It turns out the Arduino 328 has a built in thermometer. Not the old Mega8 or 168.
Not the Arduino Mega.  It only works on an Arduino with a 328 chip.

**/

long readTemp() {
	long result = 0;
	// Read temperature sensor against 1.1V reference
	ADMUX = _BV(REFS1) | _BV(REFS0) | _BV(MUX3);
	delay(2); // Wait for Vref to settle
	ADCSRA |= _BV(ADSC); // Convert
	while (bit_is_set(ADCSRA,ADSC));
	result = ADCL;
	result |= ADCH<<8;
	result = (result - 125) * 1075;
	result = ( result / 1000 );
	result = (result * 9.0)/ 5.0 + 32.0; // Convert Celcius to Fahrenheit
	return result;
}


/**
If you want to make ACCURATE readings you have to know exactly what your supply voltage is.

Measuring the 5V connection on my Arduino while plugged in to the USB is actually reading 5.12V.
That makes a big difference to the results of the conversion from ADC to voltage value.
And it fluctuates. Sometimes it's 5.12V, sometimes it's 5.14V.
so, you really need to know the supply voltage at the time you are doing your ADC reading.

Sounds tricky, yes?

Yes.

However, if you have a known precise voltage you can measure using the ADC,
then it is possible to calculate what your supply voltage is. Fortunately,
some of the AVR chips used on Arduinos have just such a voltage available,
and can be measured with the ADC. Any Arduino based on the 328 or 168 chips has this facility.

I came across this nice piece of code on the TinkerIt site. It measures this 1.1V reference voltage,
and uses the resultant ADV value to work out what the supply voltage must be.
*/

float readVcc() {
	long result;
	// Read 1.1V reference against AVcc
	ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
	delay(2); // Wait for Vref to settle
	ADCSRA |= _BV(ADSC); // Convert
	while (bit_is_set(ADCSRA,ADSC));
	result = ADCL;
	result |= ADCH<<8;
	result = 1125300L / result; // Back-calculate AVcc in mV
	return (float) result / 1000;
}

// "sleep Data, sleep.;-D "
// void sleepMode ( int selector )
// {
// 
// 	switch ( selector )
// 	{
// 		case 1:
// 		// ATmega328P, ATmega168
// 		LowPower.idle(SLEEP_8S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF,
// 		SPI_OFF, USART0_OFF, TWI_OFF);
// 		break;
// 		// ATmega32U4
// 		//LowPower.idle(SLEEP_8S, ADC_OFF, TIMER4_OFF, TIMER3_OFF, TIMER1_OFF,
// 		//		  TIMER0_OFF, SPI_OFF, USART1_OFF, TWI_OFF, USB_OFF);
// 
// 		// ATmega2560
// 		//LowPower.idle(SLEEP_8S, ADC_OFF, TIMER5_OFF, TIMER4_OFF, TIMER3_OFF,
// 		//		  TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART3_OFF,
// 		//		  USART2_OFF, USART1_OFF, USART0_OFF, TWI_OFF);
// 		case 2:
// 		// Configure wake up pin as input.
// 		// This will consumes few uA of current.
// 		pinMode(PIN2_INT0, INPUT);
// 		// Allow wake up pin to trigger interrupt on low.
// 		attachInterrupt(PIN2_INT0, 0, LOW);
// 
// 		// Enter power down state with ADC and BOD module disabled.
// 		// Wake up when wake up pin is low.
// 		LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
// 		// Disable external pin interrupt on wake up pin.
// 		detachInterrupt(0);
// 		// Do something here
// 		break;
// 		default:
// 		for ( uint8_t s = selector ; s > 0 ; s-- )
// 		{
// 			// Enter power down state for 1 s with ADC and BOD module disabled
// 			LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
// 		}
// 		//LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
// 		break;
// 	}
// }

#else
#error This example is only for use on Arduino.
#endif // ARDUINO

#endif // __COMMONS_H__
