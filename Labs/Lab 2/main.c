#include "toy_stdio.h"

int main(int argc, char *argv[]) {
	//Welcome
	int chars_printed = toy_printf("%x, %X\\n", 496351, 496351);
	chars_printed += toy_printf("Welcome to \\c\\n");
	chars_printed += toy_printf("Support for explicit\\N");
	//Tasks 1a, 1b
	toy_printf("Printed %d chars\n", chars_printed);
	toy_printf("Hex unsigned: %x\n", -1);
	toy_printf("Octal unsigned: %o\n", -1);
	toy_printf("Dec unsigned: %d\n", -10);
	toy_printf("Unsigned value: %u\n", 15);
	toy_printf("Unsigned value: %u\n", -15);
	toy_printf("binary value: %b\n", 32);
	toy_printf("binary value: %b\n", -16);
	//Task 1c
	int integers_array[] = { 53,850,29,31,1 };
	char * strings_array[] = { "This", "is", "array", "of", "strings" };
	char * str = "hello man";
	int array_size = 5;
	toy_printf("Print array of integers-d: %Ad\n", integers_array, array_size);
	toy_printf("Print array of integers-X: %AX\n", integers_array, array_size);
	toy_printf("Print array of integers-b: %Ab\n", integers_array, array_size);
	toy_printf("Print array of integers-x: %Ax\n", integers_array, array_size);
	toy_printf("Print array of integers-u: %Au\n", integers_array, array_size);
	toy_printf("Print array of integers-o: %Ao\n", integers_array, array_size);
	toy_printf("Print array of chars-   c: %Ac\n", str, 9);
	toy_printf("Print array of strings: %As\n", strings_array, array_size);
	//Task 1d
	toy_printf("Right-padded string: %6s\n", "str");
	toy_printf("Right-padded string: %10d\n", 12345678);
	toy_printf("Left-padded string: %-6s\n", "str");
	toy_printf("Right-padded string: %-10d\n", 12345678);
	toy_printf("With numeric placeholders: %05d\n", -1);
	//Additional
	toy_printf("Unsigned value: %u\n", -0xABC);
	toy_printf("Unsigned value: %u\n", 0xABC);
	//Lab
	toy_printf("%x, %x\n", -1, 1);
	toy_printf("%d, %d\n", -1, 1);
	toy_printf("%o, %o\n", -1, 1);
	toy_printf("%x, %x\n", -1, 1);

}
