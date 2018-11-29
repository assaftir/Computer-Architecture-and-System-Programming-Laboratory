/* caspl-lab-1.c
 * Limited versions of printf
 *
 * Programmer: Mayer Goldberg, 2018
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>

#define UINT_MAX 4294967295 

 /* the states in the printf state-machine */
enum printf_state {
	st_printf_init,
	st_printf_meta_char,
	st_printf_percent,
	st_printf_octal2,
	st_printf_octal3
};

#define MAX_NUMBER_LENGTH 64
#define is_octal_char(ch) ('0' <= (ch) && (ch) <= '7')

int toy_printf(char *fs, ...);
int print_array_helper(char* format, void* array, int array_size);
int numOfDigits(int n);

const char *digit = "0123456789abcdef";
const char *DIGIT = "0123456789ABCDEF";


int print_int_helper(unsigned int n, int radix, const char *digit) {
	int result;

	if (n < radix) {
		putchar(digit[n]);
		return 1;
	}
	else {
		result = print_int_helper(n / radix, radix, digit);
		putchar(digit[n % radix]);
		return 1 + result;
	}
}

int print_int(int n, int radix, const char * digit) {
	if (radix < 2 || radix > 16) {
		toy_printf("Radix must be in [2..16]: Not %d\n", radix);
		exit(-1);
	}

	if (n > 0) {
		return print_int_helper(n, radix, digit);
	}
	if (n == 0) {
		putchar('0');
		return 1;
	}
	else if (radix != 10) {
		return print_int_helper(n + (UINT_MAX + 1), radix, digit);
	}
	else {
		putchar('-');    
		return print_int_helper(-n, radix, digit);
	
	}
}

/* SUPPORTED:
 *   \f, \t, \n, \r -- formfeed, tab, newline, return
 *   \F, \T, \N, \R -- extensions: visible versions of the above
 *   \c -- extension: CASPL'2018
 *   \C -- extension: visible version of the above
 *   %b, %d, %o, %x, %X --
 *     integers in binary, decimal, octal, hex, and HEX
 *   %s -- strings
 */

int toy_printf(char *fs, ...) {
	int chars_printed = 0;
	int int_value = 0;
	char *string_value;
	char char_value;
	char octal_char;
	va_list args;
	enum printf_state state;
	int array_size = 0;
	void* arr;
	int pads = 0;
	int placeholders = 0;
	char format[3]; // Will hold the format to be printed, e.g '%d'

	va_start(args, fs);

	state = st_printf_init;

	for (; *fs ; ++fs) {
		switch (state) {
		case st_printf_init:
			switch (*fs) {
			case '\\':
				state = st_printf_meta_char;
				break;

			case '%':
				state = st_printf_percent;
				break;

			default:
				putchar(*fs);
				++chars_printed;
			}
			break;

		case st_printf_meta_char:
			switch (*fs) {
			case '\\':
				putchar('\\');
				++chars_printed;
				state = st_printf_init;
				break;

			case '\"':
				putchar('\"');
				++chars_printed;
				state = st_printf_init;
				break;

			case 't':
				putchar('\t');
				++chars_printed;
				state = st_printf_init;
				break;

			case 'T':
				chars_printed += toy_printf("<tab>\t");
				state = st_printf_init;
				break;

			case 'f':
				putchar('\f');
				++chars_printed;
				state = st_printf_init;
				break;

			case 'F':
				chars_printed += toy_printf("<formfeed>\f");
				state = st_printf_init;
				break;

			case 'n':
				putchar('\n');
				++chars_printed;
				state = st_printf_init;
				break;

			case 'N':
				chars_printed += toy_printf("<newline>\n");
				state = st_printf_init;
				break;

			case 'r':
				putchar('\r');
				++chars_printed;
				state = st_printf_init;
				break;

			case 'R':
				chars_printed += toy_printf("<return>\r");
				state = st_printf_init;
				break;

			case 'c':
				chars_printed += toy_printf("CASPL'2018");
				state = st_printf_init;
				break;

			case 'C':
				chars_printed += toy_printf("<caspl magic>");
				chars_printed += toy_printf("\\c");
				state = st_printf_init;
				break;

			default:
				if (is_octal_char(*fs)) {
					octal_char = *fs - '0';
					state = st_printf_octal2;
				}
				else {
					toy_printf("Unknown meta-character: \\%c", *fs);
					exit(-1);
				}
			}
			break;

		case st_printf_octal2:
			if (is_octal_char(*fs)) {
				octal_char = (octal_char << 3) + (*fs - '0');
				state = st_printf_octal3;
				break;
			}
			else {
				toy_printf("Missing second octal char. Found: \\%c", *fs);
				exit(-1);
			}

		case st_printf_octal3:
			if (is_octal_char(*fs)) {
				octal_char = (octal_char << 3) + (*fs - '0');
				putchar(octal_char);
				++chars_printed;
				state = st_printf_init;
				break;
			}
			else {
				toy_printf("Missing third octal char. Found: \\%c", *fs);
				exit(-1);
			}

		case st_printf_percent:
			switch (*fs) {
			case '%':
				putchar('%');
				++chars_printed;
				state = st_printf_init;
				break;

			case 'd':
				int_value = va_arg(args, int);
				chars_printed += print_int(int_value, 10, digit);
				state = st_printf_init;
				break;

			case 'b':
				int_value = va_arg(args, int);
				chars_printed += print_int(int_value, 2, digit);
				state = st_printf_init;
				break;

			case 'o':
				int_value = va_arg(args, int);
				chars_printed += print_int(int_value, 8, digit);
				state = st_printf_init;
				break;

			case 'x':
				int_value = va_arg(args, int);
				chars_printed += print_int(int_value, 16, digit);
				state = st_printf_init;
				break;

			case 'X':
				int_value = va_arg(args, int);
				chars_printed += print_int(int_value, 16, DIGIT);
				state = st_printf_init;
				break;

			case 's':
				string_value = va_arg(args, char *);
				chars_printed += toy_printf(string_value);
				state = st_printf_init;
				break;

			case 'c':
				char_value = (char)va_arg(args, int);
				putchar(char_value);
				++chars_printed;
				state = st_printf_init;
				break;

			case 'u':
				int_value = va_arg(args, int);
				chars_printed += print_int_helper(int_value < 0 ? int_value + (UINT_MAX + 1) : int_value, 10, digit);
				state = st_printf_init;
				break;

			case 'A':
				format[0] = '%';
				format[1] = *(++fs);
				switch (format[1]) {
					case 'd':
					case 'b':
					case 'o':
					case 'x':
					case 'X':
					case 'u':
						arr = va_arg(args, int*);
						array_size = va_arg(args, int);
						chars_printed += print_array_helper(format, arr, array_size);
						state = st_printf_init;
						break;

					case 's':
						arr = va_arg(args, char**);
						array_size = va_arg(args, int);
						chars_printed += print_array_helper("%s", arr, array_size);
						state = st_printf_init;
						break;

					case 'c':
						arr = va_arg(args, char*);
						array_size = va_arg(args, int);
						chars_printed += print_array_helper("%c", arr, array_size);
						state = st_printf_init;
						break;

					default:
						toy_printf("Unhandled format %%%c...\n", *fs);
				}
				break;

			case '-': // Left-Padded
				++fs; // Move pointer to the beginning of the integer
				if ((pads = atoi(fs)) > 0) { // width
					fs += numOfDigits(pads); // Move pointer to (<char>) format
					switch (*fs) {
					case 's':
						string_value = va_arg(args, char *);
						int str_len = strlen(string_value);
						if (str_len < pads) {
							for (int i = 0; i < pads - str_len; i++)
								putchar(' ');
						}
						chars_printed += toy_printf(string_value);
						state = st_printf_init;
						break;

					case 'd':
						int_value = va_arg(args, int);
						int numDigits;
						if ((numDigits = numOfDigits(int_value)) < pads) {
							for (int i = 0; i < pads - numDigits; i++)
								putchar(' ');
						}
						chars_printed += print_int(int_value, 10, digit);
						state = st_printf_init;
						break;

					default:
						toy_printf("Unhandled format %%%c...\n", *fs);
					}
				}
				else {
					toy_printf("Dont cheat\n", *fs);
					exit(-1);
				}
				break;

			case '0': //Numeric placeholders
				if ((placeholders = atoi(++fs)) > 0) { // Width
					fs += numOfDigits(placeholders);
					switch (*fs) { //Switch in order to support format extensions to numeric placeholders
					case 'd':
						int_value = va_arg(args, int);
						int isNegative = 0;
						if (int_value < 0) {
							putchar('-');
							int_value = -int_value;
							isNegative = 1;
						}
						int numDigits = numOfDigits(int_value);
						for (int i = 0; i < placeholders - numDigits - isNegative; i++)
							putchar('0');
						toy_printf("%d", int_value);
						state = st_printf_init;
						break;

					default:
						toy_printf("Unhandled format yet %%%c...\n", *fs);
					}
				}
				else {
					toy_printf("Dont cheat\n", *fs);
					exit(-1);
				}
				break;

			default: // Rigt-Padded?				
				if ((pads = atoi(fs)) > 0) { // Width
					fs += numOfDigits(pads);
					switch (*fs) {
					case 's':
						string_value = va_arg(args, char *);
						chars_printed += toy_printf(string_value);
						int str_len = strlen(string_value);
						if (str_len < pads) {
							for (int i = 0; i < pads - str_len; i++)
								toy_printf(" ");
							toy_printf("#");
						}
						state = st_printf_init;
						break;

					case 'd':
						int_value = va_arg(args, int);
						chars_printed += print_int(int_value, 10, digit);
						int numDigits = numOfDigits(int_value);
						if (numOfDigits(int_value) < pads) {
							for (int i = 0; i < pads - numDigits; i++)
								toy_printf(" ");
							toy_printf("#");
						}
						state = st_printf_init;
						break;

					default:
						toy_printf("Unhandled forrmat %%%c...\n", *fs);				
					}
				}
				else {
					toy_printf("Unhandled forrrmat %%%c...\n", *fs);
					exit(-1);
				}
			}
			break;

		default:
			toy_printf("toy_printf: Unknown state -- %d\n", (int)state);
			exit(-1);
		}
	}

	va_end(args);

	return chars_printed;
}

int print_array_helper(char* format, void* array, int array_size) {
	int chars_printed = 0;
	switch (format[1]) { //format = "%<char>"
	case 's':
		toy_printf("{");
		++chars_printed;
		for (int i = 0; i < array_size; i++) {
			chars_printed += toy_printf("\"%s\"", ((char**)array)[i]);
			if (i != array_size - 1)
				toy_printf(", ");
		}
		toy_printf("}");
		++chars_printed;
		break;

	case 'c':
		toy_printf("{");
		++chars_printed;
		for (int i = 0; i < array_size; i++) {
			toy_printf("\'%c\'", ((char*)array)[i]);
			if (i != array_size - 1)
				toy_printf(", ");
		}
		toy_printf("}");
		++chars_printed;
		break;

	default: // '%d' , '%b' , '%o' , '%x' , '%X', '%u' 
		toy_printf("{");
		++chars_printed;
		for (int i = 0; i < array_size; i++) {
			chars_printed += toy_printf(format, ((int*)array)[i]);
			if (i != array_size - 1)
				toy_printf(", ");
		}
		toy_printf("}");
		++chars_printed;
	}
	return chars_printed;
}

int numOfDigits(int n) {
	if (n < 10)
		return 1;
	return 1 + numOfDigits(n / 10);
}