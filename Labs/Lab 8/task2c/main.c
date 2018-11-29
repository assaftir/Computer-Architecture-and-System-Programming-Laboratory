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


int main(int argc, char *argv[]){

int i;

int iarray[3] = {1, 2, 3};
char* sarray[3] = {"A", "B", "C"};
char carray[3] = {'a', 'b', 'c'};

i = toy_printf("%X, %X\n", 1, -1);
toy_printf("%d\n", i);

i = toy_printf("%03d, %10s, %c\n", -1, "hello", 'A');
toy_printf("%d\n", i);

i = toy_printf("%Ad\n, %Ac\n%As\n", iarray, 3, carray, 3, sarray, 3);
toy_printf("%d\n", i);

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
	
