#ifndef MAIN
#define MAIN
#include <msp430.h>

#include "interpreter.h"
#include "Bytes.h"
extern rDex  * p;

/**
 * main.c
 */
int main(void)
{
	 WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	 p = readBytes();
	 RunMain();

	return 0;
}
#endif
