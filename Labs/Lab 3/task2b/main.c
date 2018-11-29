#include "toy_stdio.h"
#include <stdlib.h>
#include <stdio.h>
int multiple_num_tests(char,int*, int);
int num_test(char,int);
int arr_num_test(char,int*,int);
int placeholder_test(int,int);

int int_arr[]= {-2000000000,-1000,-1,0,1,1000,2000000000};
char flags[] = {'d','u','b','o','x','X'};
char* strings[] = {"word1","word2","word3"};
int padd_arr[] = {1,-1,10,-10};
int widths[] = {6,16};

int main(int argc, char *argv[]) {
	int chars_printed =0;
	int flags_size = sizeof(flags)/sizeof(flags[0]); 
	int int_arr_size = sizeof(int_arr) / sizeof(int_arr[0]);
	int str_arr_size = sizeof(strings) / sizeof(strings[0]);
	int padd_size = sizeof(padd_arr) / sizeof(padd_arr[0]);
	int widths_size = sizeof(widths) / sizeof(widths[0]);
	
	for(int i=0 ;i<flags_size;++i)
		chars_printed+=multiple_num_tests(flags[i],int_arr,int_arr_size);
	
	toy_printf("%%c flag check with 'a':\n%c\n\n",'a');
	toy_printf("%%s flag check with 'string_variable':\n%s\n\n","string_variable");
	toy_printf("Starting num arrays tests with \"{-2000000000,-1000,-1,0,1,1000,2000000000}\"\n\n");
	
	for(int i=0 ;i<flags_size;++i)
		chars_printed+=arr_num_test(flags[i],int_arr,int_arr_size);
	
	toy_printf("%%Ac flag check with \"{'d','u','b','o','x','X'}\":\n%Ac\n\n",flags,flags_size);

	toy_printf("%%As flag check with '{\"word1\",\"word2\",\"word3\"}':\n%As\n\n",strings,str_arr_size);

        toy_printf("Starting arrays tests with \"{}\"\n\n");
        for(int i=0 ;i<flags_size;++i)
                chars_printed+=arr_num_test(flags[i],0,0);
        toy_printf("%%Ac flag check:\n%Ac\n\n",0,0);
        toy_printf("%%As flag check:\n%As\n\n",0,0);
  
	toy_printf("rp 6:\n%6s\n\n", "str");
	toy_printf("lp -6:\n%-6s\n\n", "str");
	toy_printf("Right-padded string with width 16:\n%16s\n\n", "str");
	toy_printf("Left-added string with width -16:\n%-16s\n\n", "str");
	
	for(int i=0;i<padd_size;++i)
		for(int j=0;j<widths_size;++j)
			placeholder_test(padd_arr[i],widths[j]);
		
	toy_printf("Mixed flags:\n %d%u\n%Ad%b\n%o%Ac\n%x%Ax%Ao\n\n" ,-10,-10,int_arr,int_arr_size,-10,-10,flags,flags_size,-10,int_arr,int_arr_size,int_arr,int_arr_size);
  
	toy_printf("Printed %d chars\n", chars_printed); 
}

int multiple_num_tests(char flag,int *arr , int size){
	int chars_printed = 0;
	for(int i=0;i<size;++i)
		chars_printed+=num_test(flag,arr[i]);
	return chars_printed;
	
}

int num_test(char flag,int num){
	char *format_string = (char*)malloc(80 * sizeof(char));
	sprintf(format_string, 
			   "%%%%%c flag check with %d:\n%%%c\n\n", flag,num, flag);
	int char_count= toy_printf(format_string,num);
	free(format_string);
	return char_count;
}

int arr_num_test(char flag,int* arr,int size){
	char *format_string = (char*)malloc(80 * sizeof(char));
	sprintf(format_string, 
			   "%%%%A%c flag check:\n%%A%c\n\n", flag, flag);
	int char_count= toy_printf(format_string,arr,size);
	free(format_string);
	return char_count;
}	

int placeholder_test(int num,int width){
	char *format_string = (char*)malloc(80 * sizeof(char));
	sprintf(format_string, 
			   "numeric placeholders %d with width %d:\n%%0%dd\n\n", num, width,width);
	int char_count= toy_printf(format_string,num);
	free(format_string);
	return char_count;
}
	