/*
 * printf.h
 *
 *  Created on: Feb 21, 2023
 *      Author: marco
 */

#ifndef PRINTF_H_
#define PRINTF_H_

#define DEBUG_MODE //comment out to disable printf support

#ifdef DEBUG_MODE

	#ifdef __GNUC__
	#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
	#else
	#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
	#endif

#endif // DEBUG_MODE

#endif /* PRINTF_H_ */
