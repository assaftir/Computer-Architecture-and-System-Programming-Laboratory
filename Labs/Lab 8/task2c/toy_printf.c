#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>

#define UINT_MAX 4294967295
#define ASCII 127
#define MAX_NUMBER_LENGTH 64
#define is_octal_char(ch) ('0' <= (ch) && (ch) <= '7')

/* the states in the printf state-machine */
enum printf_state {
   st_printf_init,
    st_printf_percent,
    st_printf_octal2,
    st_printf_octal3
};

typedef struct state_args {
    char* fs;
    char* octal_char;
    int pads;
    int placeholders;
    char * format;
    /* Any extra required args */
} state_args;

typedef struct state_result {
  int printed_chars;
  enum printf_state new_state;
} state_result;

/*arrays with 128 function pointers, each pointer points to a specific char handler*/
state_result (*init_state_handlers[ASCII])(va_list args, state_args* sa);
state_result (*percent_state_handlers[ASCII])(va_list args, state_args* sa);
state_result (*percent_array_state_handlers[ASCII])(va_list args, state_args* sa);
state_result (*minus_char_handlers[ASCII])(va_list args, state_args* sa);
state_result (*zero_char_handlers[ASCII])(va_list args, state_args* sa);
state_result (*default_char_handlers[ASCII])(va_list args, state_args* sa);
int (*array_helper_handlers[ASCII])(char* format, void* array, int array_size);
state_result (*state_handlers[5])(va_list args, state_args* sa); //4 states, 1 deafult

/*state handlers*/
state_result init_state_handler(va_list, state_args*);
state_result percent_state_handler(va_list, state_args*);
state_result octal2_state_handler(va_list, state_args*);
state_result octal3_state_handler(va_list, state_args*);

/*state chars handlers*/
state_result init_state_percent_char_handler(va_list args, state_args* sa);
state_result init_state_percent_default_handler(va_list args, state_args* sa);
state_result percent_state_percent_char_handler(va_list args, state_args* sa);
state_result percent_state_d_char_handler(va_list args, state_args* sa);
state_result percent_state_b_char_handler(va_list args, state_args* sa);
state_result percent_state_o_char_handler(va_list args, state_args* sa);
state_result percent_state_x_char_handler(va_list args, state_args* sa);
state_result percent_state_X_char_handler(va_list args, state_args* sa);
state_result percent_state_s_char_handler(va_list args, state_args* sa);
state_result percent_state_c_char_handler(va_list args, state_args* sa);
state_result percent_state_u_char_handler(va_list args, state_args* sa);
state_result percent_state_A_char_handler(va_list args, state_args* sa);
state_result percent_state_Ad_char_handler(va_list args, state_args* sa);
state_result percent_state_As_char_handler(va_list args, state_args* sa);
state_result percent_state_Ac_char_handler(va_list args, state_args* sa);
state_result percent_state_minus_char_handler(va_list args, state_args* sa);
state_result percent_state_minus_s_char_handler(va_list args, state_args* sa);
state_result percent_state_minus_d_char_handler(va_list args, state_args* sa);
state_result percent_state_0_char_handler(va_list args, state_args* sa);
state_result percent_state_0d_char_handler(va_list args, state_args* sa);
state_result default_char_handler(va_list args, state_args* sa);
state_result default_s_char_handler(va_list args, state_args* sa);
state_result default_d_char_handler(va_list args, state_args* sa);

/*forward decls*/
int toy_printf(char *fs, ...);
int print_array_helper(char* format, void* array, int array_size);
int num_of_digits(int n);
int array_s_handler(char* format, void* array, int array_size);
int array_c_handler(char* format, void* array, int array_size);
int array_def_handler(char* format, void* array, int array_size);

const char *digit = "0123456789abcdef";
const char *DIGIT = "0123456789ABCDEF";

/*code*/
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


void init_handlers(){
    /*init state handlers*/
    state_handlers[0] = init_state_handler;
    state_handlers[1] = percent_state_handler;
    state_handlers[2] = octal2_state_handler;
    state_handlers[3] = octal3_state_handler;
    
    /*init init state handlers*/
    for(int i = 0 ; i < ASCII ; i++)
        init_state_handlers[i] = init_state_percent_default_handler;
    init_state_handlers['%'] = init_state_percent_char_handler;       
    
    /*init percent state handlers*/
    for(int i = 0 ; i < ASCII ; i++)
        percent_state_handlers[i] = default_char_handler;
    percent_state_handlers['%'] = percent_state_percent_char_handler;
    percent_state_handlers['d'] = percent_state_d_char_handler;
    percent_state_handlers['b'] = percent_state_b_char_handler;
    percent_state_handlers['o'] = percent_state_o_char_handler;
    percent_state_handlers['x'] = percent_state_x_char_handler;
    percent_state_handlers['X'] = percent_state_X_char_handler;
    percent_state_handlers['s'] = percent_state_s_char_handler;
    percent_state_handlers['c'] = percent_state_c_char_handler;
    percent_state_handlers['u'] = percent_state_u_char_handler;
    percent_state_handlers['A'] = percent_state_A_char_handler;
    percent_state_handlers['-'] = percent_state_minus_char_handler;
    percent_state_handlers['0'] = percent_state_0_char_handler;
    
    /*init percent array state handlers*/
    percent_array_state_handlers['d'] = percent_state_Ad_char_handler;
    percent_array_state_handlers['b'] = percent_state_Ad_char_handler;
    percent_array_state_handlers['o'] = percent_state_Ad_char_handler;
    percent_array_state_handlers['x'] = percent_state_Ad_char_handler;
    percent_array_state_handlers['X'] = percent_state_Ad_char_handler;
    percent_array_state_handlers['u'] = percent_state_Ad_char_handler;
    percent_array_state_handlers['s'] = percent_state_As_char_handler;
    percent_array_state_handlers['c'] = percent_state_Ac_char_handler;
    percent_array_state_handlers['-'] = percent_state_minus_char_handler;
    percent_array_state_handlers['0'] = percent_state_0_char_handler;
    
    /*init left padding*/
    minus_char_handlers['s'] = percent_state_minus_s_char_handler;
    minus_char_handlers['d'] = percent_state_minus_d_char_handler;
    
    /*init right padding handlers*/
    default_char_handlers['s'] = default_s_char_handler;
    default_char_handlers['d'] = default_d_char_handler;
            
    /*init numeric placeholders*/
    zero_char_handlers['d'] = percent_state_0d_char_handler;

    /*init array helper handlers*/    
    for(int i = 0 ; i < ASCII ; i++){
	array_helper_handlers[i] = array_def_handler;
    }     
    array_helper_handlers['s'] = array_s_handler;
    array_helper_handlers['c'] = array_c_handler;
}


/* SUPPORTED:
 *   %b, %d, %o, %x, %X --
 *     integers in binary, decimal, octal, hex, and HEX
 *   %s -- strings
 */

int toy_printf(char *fs, ...) {
    
    init_handlers();
    
    int chars_printed = 0;
    char octal_char;
    enum printf_state state;
    
    state_args sa;
    sa.fs = fs;
    sa.octal_char = &octal_char;
    va_list args;
    
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

int num_of_digits(int n) {
    if (n < 10)
        return 1;
    return 1 + num_of_digits(n / 10);
}


state_result init_state_percent_char_handler(va_list args, state_args* sa){
    state_result sr;
    sr.new_state = st_printf_percent;
    sr.printed_chars = 0;
    return sr;
}

state_result init_state_percent_default_handler(va_list args, state_args* sa){
    state_result sr;
    sr.new_state = st_printf_init;
    putchar(*(sa->fs));
    sr.printed_chars = 1;
    return sr;
}

state_result percent_state_percent_char_handler(va_list args, state_args* sa){
    state_result sr;
    putchar('%');
    sr.printed_chars = 1;
    sr.new_state = st_printf_init;
    return sr;
}

state_result percent_state_d_char_handler(va_list args, state_args* sa){
   state_result sr;
   sr.printed_chars = 0;
   int int_value = va_arg(args, int);
   sr.printed_chars += print_int(int_value, 10, digit);
   sr.new_state = st_printf_init;
   return sr;
}
state_result percent_state_b_char_handler(va_list args, state_args* sa){
   state_result sr;
   sr.printed_chars = 0;
   int int_value = va_arg(args, int);
   sr.printed_chars += print_int(int_value, 2, digit);
   sr.new_state = st_printf_init;
   return sr;
}
state_result percent_state_o_char_handler(va_list args, state_args* sa){
   state_result sr;
   sr.printed_chars = 0;
   int int_value = va_arg(args, int);
   sr.printed_chars += print_int(int_value, 8, digit);
   sr.new_state = st_printf_init;
   return sr;
}
state_result percent_state_x_char_handler(va_list args, state_args* sa){
   state_result sr;
   sr.printed_chars = 0;
   int int_value = va_arg(args, int);
   sr.printed_chars += print_int(int_value, 16, digit);
   sr.new_state = st_printf_init;
   return sr;
}
state_result percent_state_X_char_handler(va_list args, state_args* sa){
   state_result sr;
   sr.printed_chars = 0;
   int int_value = va_arg(args, int);
   sr.printed_chars += print_int(int_value, 16, DIGIT);
   sr.new_state = st_printf_init;
   return sr;
}
state_result percent_state_s_char_handler(va_list args, state_args* sa){
    state_result sr;
    sr.printed_chars = 0;
    char * string_value = va_arg(args, char *);
    sr.printed_chars += toy_printf(string_value);
    sr.new_state = st_printf_init;
    return sr;    
}
state_result percent_state_c_char_handler(va_list args, state_args* sa){
    state_result sr;
    sr.printed_chars = 0;
    char char_value = (char)va_arg(args, int);
    putchar(char_value);
    sr.printed_chars = 1;
    sr.new_state = st_printf_init;
    return sr;
}
state_result percent_state_u_char_handler(va_list args, state_args* sa){
    state_result sr;
    sr.printed_chars = 0;
    int int_value = va_arg(args, int);
    sr.printed_chars += print_int_helper(int_value < 0 ? int_value + (UINT_MAX + 1) : int_value, 10, digit);
    sr.new_state = st_printf_init;
    return sr;
}

state_result percent_state_A_char_handler(va_list args, state_args* sa){
    char format[3];
    format[0] = '%';
    format[1] = *(++sa->fs);
    format[2] = '\0';
    sa->format = format;
    return percent_array_state_handlers[(int)format[1]](args, sa);   
}
state_result percent_state_Ad_char_handler(va_list args, state_args* sa){
    state_result sr;
    sr.printed_chars = 0;
    void * arr;
    int array_size;
    arr = va_arg(args, int*);
    array_size = va_arg(args, int);
    sr.printed_chars += print_array_helper(sa->format, arr, array_size);
    sr.new_state = st_printf_init;
    return sr;
}
state_result percent_state_As_char_handler(va_list args, state_args* sa){
    state_result sr;
    sr.printed_chars = 0;
    void * arr;
    int array_size;
    arr = va_arg(args, char**);
    array_size = va_arg(args, int);
    sr.printed_chars += print_array_helper("%s", arr, array_size);
    sr.new_state = st_printf_init;
    return sr;
}
state_result percent_state_Ac_char_handler(va_list args, state_args* sa){
    state_result sr;
    sr.printed_chars = 0;
    void * arr;
    int array_size;
    arr = va_arg(args, char*);
    array_size = va_arg(args, int);
    sr.printed_chars += print_array_helper("%c", arr, array_size);
    sr.new_state = st_printf_init;
    return sr;
}

state_result percent_state_minus_char_handler(va_list args, state_args* sa){
    char * fs = sa->fs;
    int pads;
    ++fs; // Move pointer to the beginning of the integer
    if ((pads = atoi(fs)) > 0) { // width
        fs += num_of_digits(pads); // Move pointer to (<char>) format
        sa->pads = pads;
        sa->fs = fs;
        return minus_char_handlers[(int)*fs](args, sa);
    }
    else {
        toy_printf("Dont cheat\n", *fs);
        exit(-1);
    }
    sa->fs = fs;
}

state_result percent_state_minus_s_char_handler(va_list args, state_args* sa){
    state_result sr;
    sr.printed_chars = 0;
    char* string_value;
    int pads = sa->pads;
    string_value = va_arg(args, char *);
    int str_len = strlen(string_value);
    if (str_len < pads) {
        for (int i = 0; i < pads - str_len; i++)
            putchar(' ');
    }
    sr.printed_chars += toy_printf(string_value);
    sr.printed_chars += pads - str_len;
    sr.new_state = st_printf_init;
    return sr; 
}
state_result percent_state_minus_d_char_handler(va_list args, state_args* sa){
    state_result sr;
    sr.printed_chars = 0;
    int pads = sa->pads;
    int int_value;
    int_value = va_arg(args, int);
    int num_digits;
    if ((num_digits = num_of_digits(int_value)) < pads) {
        for (int i = 0; i < pads - num_digits; i++)
            putchar(' ');
    }
    sr.printed_chars += print_int(int_value, 10, digit);
    sr.printed_chars += pads - num_digits;
    sr.new_state = st_printf_init;
    return sr;
}
state_result percent_state_0_char_handler(va_list args, state_args* sa){
    char * fs = sa->fs;
    int placeholders = 0;  
    if ((placeholders = atoi(++fs)) > 0) { // Width
        fs += num_of_digits(placeholders);
        sa->placeholders = placeholders;
        sa->fs = fs;
        return zero_char_handlers[(int)*fs](args, sa);
    }
    else {
        toy_printf("Dont cheat\n", *fs);
        exit(-1);
    }            
}
state_result percent_state_0d_char_handler(va_list args, state_args* sa){
    state_result sr;
    sr.printed_chars = 0;
    int int_value;
    int placeholders = sa->placeholders;
    int_value = va_arg(args, int);
    int is_negative = 0;
    if (int_value < 0) {
        putchar('-');
	sr.printed_chars++;
        int_value = -int_value;
        is_negative = 1;
    }
    int num_digits = num_of_digits(int_value);
    for (int i = 0; i < placeholders - num_digits - is_negative; i++)
        putchar('0');
    sr.printed_chars += placeholders - num_digits - is_negative;
    toy_printf("%d", int_value);
    sr.new_state = st_printf_init;
    return sr;
    
}
state_result default_char_handler(va_list args, state_args* sa){
    int pads;
    char * fs = sa->fs;
    if ((pads = atoi(fs)) > 0) { // Width
        fs += num_of_digits(pads);
        sa->pads = pads;
        sa->fs = fs;
        return default_char_handlers[(int)*fs](args, sa);
    }
    else {
        toy_printf("Unhandled forrrmat %%%c...\n", *fs);
        exit(-1);
    }
}

state_result default_s_char_handler(va_list args, state_args* sa){
    state_result sr;
    sr.printed_chars = 0;
    char * string_value;
    int pads = sa->pads;
    string_value = va_arg(args, char *);
    sr.printed_chars += toy_printf(string_value);
    int str_len = strlen(string_value);
    if (str_len < pads) {
        for (int i = 0; i < pads - str_len; i++)
            sr.printed_chars += toy_printf(" ");
        toy_printf("#");
	sr.printed_chars++;
    }
    sr.new_state = st_printf_init;
    return sr;

}
state_result default_d_char_handler(va_list args, state_args* sa){
    state_result sr;
    sr.printed_chars = 0;
    int int_value;
    int pads = sa->pads;
    int_value = va_arg(args, int);
    sr.printed_chars += print_int(int_value, 10, digit);
    int num_digits = num_of_digits(int_value);
    if (num_of_digits(int_value) < pads) {
        for (int i = 0; i < pads - num_digits; i++)
            sr.printed_chars += toy_printf(" ");
        toy_printf("#");
	sr.printed_chars++;
    }
    sr.new_state = st_printf_init;
    return sr;
}

/*state handlers impl*/

state_result init_state_handler(va_list args, state_args* sa){
    char * fs = sa->fs;
    state_result sr = init_state_handlers[(int)*fs](args, sa);
    return sr;
}

state_result percent_state_handler(va_list args, state_args* sa){
    char * fs = sa->fs;
    return percent_state_handlers[(int)*fs](args, sa);
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

int print_array_helper(char* format, void* array, int array_size) {
    int chars_printed;
    if(array_size == 0){
        toy_printf("{}");
        return 2; //2 chars printed
    }
    chars_printed = array_helper_handlers[(int)format[1]](format, array, array_size);
    return chars_printed;
}

int array_s_handler(char* format, void* array, int array_size){
	int chars_printed = 0;
	toy_printf("{");
        ++chars_printed;
        for (int i = 0; i < array_size - 1; i++) 
            chars_printed += toy_printf("\"%s\", ", ((char**)array)[i]);
        chars_printed += toy_printf("\"%s\"", ((char**)array)[array_size - 1]); // print last element
        toy_printf("}");
        ++chars_printed;
return chars_printed;
}

int array_c_handler(char* format, void* array, int array_size){
	int chars_printed = 0;
	toy_printf("{");
        ++chars_printed;
        for (int i = 0; i < array_size - 1; i++)
            chars_printed += toy_printf("\'%c\', ", ((char*)array)[i]);
        chars_printed += toy_printf("\'%c\'", ((char*)array)[array_size - 1]);
        toy_printf("}");
        ++chars_printed;
return chars_printed;
}

int array_def_handler(char* format, void* array, int array_size){
	int chars_printed = 0;
	toy_printf("{");
        ++chars_printed;
        for (int i = 0; i < array_size - 1; i++) {
            chars_printed += toy_printf(format, ((int*)array)[i]);
            chars_printed += toy_printf(", ");
        }
        chars_printed += toy_printf(format, ((int*)array)[array_size - 1]);
        chars_printed += toy_printf("}");
        ++chars_printed;
return chars_printed;
}
