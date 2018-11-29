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

typedef struct state_result {
  int printed_chars;
  enum printf_state new_state;
} state_result;

int toy_printf(char *fs, ...);
int print_array_helper(char* format, void* array, int array_size);
int num_of_digits(int n);


const char *digit = "0123456789abcdef";
const char *DIGIT = "0123456789ABCDEF";

typedef struct state_args {
    char* fs;
    char* octal_char;
    /* Any extra required args */
} state_args;

/*state handlers*/
state_result init_state_handler(va_list, state_args*);
state_result meta_char_state_handler(va_list, state_args*);
state_result percent_state_handler(va_list, state_args*);
state_result octal2_state_handler(va_list, state_args*);
state_result octal3_state_handler(va_list, state_args*);

state_result (*state_handlers[6])(va_list args, state_args* sa); //5 states, 1 deafult

void init_handlers(){
    /*init state handlers*/
    state_handlers[0] = init_state_handler;
    state_handlers[1] = meta_char_state_handler;
    state_handlers[2] = percent_state_handler;
    state_handlers[3] = octal2_state_handler;
    state_handlers[4] = octal3_state_handler;
}

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
    
    init_handlers();
    
    int chars_printed = 0;
    char octal_char;
    va_list args;
    enum printf_state state;
    
    state_args sa;
    sa.fs = fs;
    sa.octal_char = &octal_char;
    
    va_start(args, fs);
    state = st_printf_init;
    state_result sr;
        
    for (; *sa.fs ; ++sa.fs) {
        sr = state_handlers[state](args, &sa);
        state = sr.new_state;
        chars_printed += sr.printed_chars;
    }
    
    va_end(args);   
    return chars_printed;
}

int print_array_helper(char* format, void* array, int array_size) {
        if(array_size == 0){
        toy_printf("{}");
        return 2; //2 chars printed
    }
        
    int chars_printed = 0;
    switch (format[1]) { //format = "%<char>"
	case 's':
            toy_printf("{");
            ++chars_printed;
            for (int i = 0; i < array_size - 1; i++) 
                chars_printed += toy_printf("\"%s\", ", ((char**)array)[i]);
            chars_printed += toy_printf("\"%s\"", ((char**)array)[array_size - 1]); // print last element
            toy_printf("}");
            ++chars_printed;
            break;
            
	case 'c':
            toy_printf("{");
            ++chars_printed;
            for (int i = 0; i < array_size - 1; i++)
                chars_printed += toy_printf("\'%c\', ", ((char*)array)[i]);
            chars_printed += toy_printf("\'%c\'", ((char*)array)[array_size - 1]);
            toy_printf("}");
            ++chars_printed;
            break;
            
	default: // '%d' , '%b' , '%o' , '%x' , '%X', '%u' 
            toy_printf("{");
            ++chars_printed;
            for (int i = 0; i < array_size - 1; i++) {
                chars_printed += toy_printf(format, ((int*)array)[i]);
                chars_printed += toy_printf(", ");
            }
            chars_printed += toy_printf(format, ((int*)array)[array_size - 1]);
            chars_printed += toy_printf("}");
            ++chars_printed;
    }
    return chars_printed;
}

int num_of_digits(int n) {
    if (n < 10)
        return 1;
    return 1 + num_of_digits(n / 10);
}



/*state handlers impl*/

state_result init_state_handler(va_list args, state_args* sa){
    state_result sr;
    sr.printed_chars = 0;
    sr.new_state = st_printf_init;
    char * fs = sa->fs;
    switch (*fs) {
        case '\\':
            sr.new_state = st_printf_meta_char;
            break;
            
        case '%':
            sr.new_state = st_printf_percent;
            break;
            
        default:
            putchar(*fs);
            sr.printed_chars++;
    }
    return sr;
}

state_result meta_char_state_handler(va_list args, state_args* sa){
    state_result sr;
    sr.printed_chars = 0;
    char * fs = sa->fs;
    switch (*fs) {
        case '\\':
            putchar('\\');
            sr.printed_chars++;
            sr.new_state = st_printf_init;
            break;
            
        case '\"':
            putchar('\"');
            sr.printed_chars++;
            sr.new_state = st_printf_init;
            break;
            
        case 't':
            putchar('\t');
            sr.printed_chars++;
            sr.new_state = st_printf_init;
            break;
            
        case 'T':
            sr.printed_chars += toy_printf("<tab>\t");
            sr.new_state = st_printf_init;
            break;
            
        case 'f':
            putchar('\f');
            sr.printed_chars++;
            sr.new_state = st_printf_init;
            break;
            
        case 'F':
            sr.printed_chars += toy_printf("<formfeed>\f");
            sr.new_state = st_printf_init;
            break;
            
        case 'n':
            putchar('\n');
            sr.printed_chars++;
            sr.new_state = st_printf_init;
            break;
            
        case 'N':
            sr.printed_chars += toy_printf("<newline>\n");
            sr.new_state = st_printf_init;
            break;
            
        case 'r':
            putchar('\r');
            sr.printed_chars++;
            sr.new_state = st_printf_init;
            break;
            
        case 'R':
            sr.printed_chars += toy_printf("<return>\r");
            sr.new_state = st_printf_init;
            break;
            
        case 'c':
            sr.printed_chars += toy_printf("CASPL'2018");
            sr.new_state = st_printf_init;
            break;
            
        case 'C':
            sr.printed_chars += toy_printf("<caspl magic>");
            sr.printed_chars += toy_printf("\\c");
            sr.new_state = st_printf_init;
            break;
            
        default:
            if (is_octal_char(*fs)) {
                *(sa->octal_char) = *fs - '0';
                sr.new_state = st_printf_octal2;
            }
            else {
                toy_printf("Unknown meta-character: \\%c", *fs);
                exit(-1);
            }
    }
    return sr;
    
}

state_result percent_state_handler(va_list args, state_args* sa){
    char * fs = sa->fs;
    enum printf_state state = st_printf_percent;
    int int_value = 0;
    char *string_value;
    char char_value;
    char format[3]; // Will hold the format to be printed, e.g '%d'
    int array_size = 0;
    void* arr;
    int pads = 0;
    int placeholders = 0;
    
    state_result sr;
    sr.printed_chars = 0;
    
    switch (*fs) {
        case '%':
            putchar('%');
            ++sr.printed_chars;
            state = st_printf_init;
            break;
            
        case 'd':
            int_value = va_arg(args, int);
            sr.printed_chars += print_int(int_value, 10, digit);
            state = st_printf_init;
            break;
            
        case 'b':
            int_value = va_arg(args, int);
            sr.printed_chars += print_int(int_value, 2, digit);
            state = st_printf_init;
            break;
            
        case 'o':
            int_value = va_arg(args, int);
            sr.printed_chars += print_int(int_value, 8, digit);
            state = st_printf_init;
            break;
            
        case 'x':
            int_value = va_arg(args, int);
            sr.printed_chars += print_int(int_value, 16, digit);
            state = st_printf_init;
            break;
            
        case 'X':
            int_value = va_arg(args, int);
            sr.printed_chars += print_int(int_value, 16, DIGIT);
            state = st_printf_init;
            break;
            
        case 's':
            string_value = va_arg(args, char *);
            sr.printed_chars += toy_printf(string_value);
            state = st_printf_init;
            break;
            
        case 'c':
            char_value = (char)va_arg(args, int);
            putchar(char_value);
            ++sr.printed_chars;
            state = st_printf_init;
            break;
            
        case 'u':
            int_value = va_arg(args, int);
            sr.printed_chars += print_int_helper(int_value < 0 ? int_value + (UINT_MAX + 1) : int_value, 10, digit);
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
                    sr.printed_chars += print_array_helper(format, arr, array_size);
                    state = st_printf_init;
                    break;
                    
                case 's':
                    arr = va_arg(args, char**);
                    array_size = va_arg(args, int);
                    sr.printed_chars += print_array_helper("%s", arr, array_size);
                    state = st_printf_init;
                    break;
                    
                case 'c':
                    arr = va_arg(args, char*);
                    array_size = va_arg(args, int);
                    sr.printed_chars += print_array_helper("%c", arr, array_size);
                    state = st_printf_init;
                    break;
                    
                default:
                    toy_printf("Unhandled format %%%c...\n", *fs);
            }
            break;
            
        case '-': // Left-Padded
            ++fs; // Move pointer to the beginning of the integer
            if ((pads = atoi(fs)) > 0) { // width
                fs += num_of_digits(pads); // Move pointer to (<char>) format
                switch (*fs) {
                    case 's':
                        string_value = va_arg(args, char *);
                        int str_len = strlen(string_value);
                        if (str_len < pads) {
                            for (int i = 0; i < pads - str_len; i++)
                                putchar(' ');
                        }
                        sr.printed_chars += toy_printf(string_value);
                        state = st_printf_init;
                        break;
                        
                    case 'd':
                        int_value = va_arg(args, int);
                        int num_digits;
                        if ((num_digits = num_of_digits(int_value)) < pads) {
                            for (int i = 0; i < pads - num_digits; i++)
                                putchar(' ');
                        }
                        sr.printed_chars += print_int(int_value, 10, digit);
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
                fs += num_of_digits(placeholders);
                switch (*fs) { //Switch in order to support format extensions to numeric placeholders
                    case 'd':
                        int_value = va_arg(args, int);
                        int is_negative = 0;
                        if (int_value < 0) {
                            putchar('-');
                            int_value = -int_value;
                            is_negative = 1;
                        }
                        int num_digits = num_of_digits(int_value);
                        for (int i = 0; i < placeholders - num_digits - is_negative; i++)
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
            
        default: // Right-Padded?				
            if ((pads = atoi(fs)) > 0) { // Width
                fs += num_of_digits(pads);
                switch (*fs) {
                    case 's':
                        string_value = va_arg(args, char *);
                        sr.printed_chars += toy_printf(string_value);
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
                        sr.printed_chars += print_int(int_value, 10, digit);
                        int num_digits = num_of_digits(int_value);
                        if (num_of_digits(int_value) < pads) {
                            for (int i = 0; i < pads - num_digits; i++)
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
    sr.new_state = state;
    sa->fs = fs;
    return sr;
}

state_result octal2_state_handler(va_list args, state_args* sa){    
    state_result sr;
    sr.printed_chars = 0;
    char * fs = sa->fs;
    sr.new_state = st_printf_octal2;
    if (is_octal_char(*fs)) {
        *(sa->octal_char) = (*(sa->octal_char) << 3) + (*fs - '0');
        sr.new_state = st_printf_octal3;
    }
    else {
        toy_printf("Missing second octal char. Found: \\%c", *fs);
        exit(-1);
    }
    return sr;  
}

state_result octal3_state_handler(va_list args, state_args* sa){
    state_result sr;
    sr.printed_chars = 0;
    char * fs = sa->fs;
    sr.new_state = st_printf_octal3;
    if (is_octal_char(*fs)) {
        *(sa->octal_char) = (*(sa->octal_char) << 3) + (*fs - '0');
        putchar(*(sa->octal_char));
        sr.printed_chars = 1;
        sr.new_state = st_printf_init;
    }
    else {
        toy_printf("Missing third octal char. Found: \\%c", *fs);
        exit(-1);
    }
    return sr;
}
