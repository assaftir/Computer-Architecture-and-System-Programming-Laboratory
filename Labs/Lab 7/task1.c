#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <math.h>

#define MAX_FILENAME 100

char filename[MAX_FILENAME+1];
int u_size = 1;
int verbose = 0;


char* data_pointer = NULL;
char data_buffer[1024];
#define MENU_SIZE 9

typedef struct menu_item {
  const char* title;
  void (*fun)();
} menu_item;


//For some reason failed to link with math.h
unsigned int mypow(unsigned int x, unsigned int y) {
	
	unsigned int res = 1;
	while(y--) res *= x;
	return res;
	
}

FILE *open_a_file(char *filename, char *mod, unsigned int *file_size, char *no_file_msg) {
  if (!filename[0]) { 
    printf("%s\n", no_file_msg); 
    return NULL; 
  }

  FILE* file = fopen(filename, mod);
  if (file == NULL) {
    if (strcmp(mod, "r+") == 0)  //Maybe the file doesnt exists, need to be created
		return open_a_file(filename, "w+", file_size, no_file_msg);
    else {		
        printf("Open %s with \"%s\" failed\n", filename, mod); 
        return NULL; 
    }
  }
    
  //Get the size of the file
  fseek(file, 0, SEEK_END); //To get the size
  *file_size = ftell(file);
  fseek(file, 0, SEEK_SET); //back to the begining
  
  return file;
}
  

void set_file_name() {
  filename[0]=0;
  printf("Enter a filename: ");
  while(filename[0] == 0) {
    scanf("%s[^\n\t\r ]", filename);  //Read a single word
  }

  if (verbose) printf("set_file_name: file name set to '%s' \n", filename);
}

void set_unit_size() {
  int size;

  printf("Enter a size: ");
  scanf("%d", &size);
  fflush(stdin); // Clear the input stream from left over

  if (size == 1 || size == 2 || size == 4) {
    u_size = size;
    if (verbose) printf("set_unit_size: set size to %d \n", u_size); 
  }
  else {
    printf("Invalid size...\n");
  }
}



void print_data(char *title, char type, char* buffer, int length) {
	
  printf("%s\n", title);	
  	
  int i = 0;
  char modifier[5];  
  //Build the modifier to be "%02x ", "%04x ", "%08x ", "%d " according to type and u_size
  if (type == 'x') {
	strcpy(modifier, "%0_x ");
	modifier[2] = u_size*2 + '0';  //"2", "4" or "8"
  }
  else {
	strcpy(modifier, "%d ");
  }
  
  //Build the mask to get 1 byte, 2 bytes or 4 bytes
  unsigned int mask = mypow(16, u_size*2)-1;
  
  
  //Modolu value to cut the line for nicer print out
  int mod = 32/u_size;   //32, 16 or 8

  if (verbose) printf("modifier=%s, mask=%x, mod=%d", modifier, mask, mod);
  
  char *eob = buffer + length*u_size;
  for (i = 0; buffer < eob; buffer = buffer+u_size, i++) {
	  if (i % mod == 0)
		  printf("\n%p:  ", buffer);
	  if (i % 4 == 0)
		  printf(" ");
      printf(modifier, (mask & *((unsigned int *)buffer)));    
  }

  printf("\n");
} 

void file_display() {

  unsigned int file_size;
  FILE *file = open_a_file(filename, "r", &file_size, "No file mame is selected");
  if (!file) return;

  unsigned int location, length;  
  int c;
  printf("Please enter location(hex) and length(dec): ");
  if ((c = scanf("%x %d", &location, &length)) != 2) {
    printf("Bad input: scanf returned %d\n", c); 
	return; 
  }	
  fflush(stdin); // Clear the input stream from left over

  if ((length == 0) || (location+length > file_size)) { 
    printf("Bad input, file_size=%d, location=%d, length=%d\n", file_size, location, length); 
	return; 
  }
  if (verbose) printf("location=%x, length=%d\n", location, length);
  
  char *buff = (char*) malloc(u_size * length);
  fseek(file, location, SEEK_SET);
  size_t len_read = fread(buff, u_size, length, file);
  fclose(file);
  
  if (len_read != length) {
    printf("Warning: fread failed. actual read %d, expected read %d\n", (int)len_read, length); 
  }
   
  print_data("Hexadecimal:", 'x', buff, len_read);
  print_data("Decimal:", 'd', buff, length);
  free(buff);
}

void copy_from_file() {
	
  unsigned int src_size, trg_size;
  unsigned int src_off, trg_off;
  unsigned int length;
  char *buff = NULL;
  char src_file_name[MAX_FILENAME+1];
  
  FILE *trg_file = open_a_file(filename, "r+", &trg_size, "Failed to open trg file");
  if (!trg_file) return;

  src_off=999; trg_off=999; length=999;
  printf("Please enter src_file, src_offset(hex), trg_offset(hex) length(dec>: ");
  scanf("%s[^\n\t\r ]", src_file_name);
  fflush(stdin);  //Clear stdin
  scanf("%x %x %d", &src_off, &trg_off, &length);
  fflush(stdin);  //Clear stdin
  if (verbose) printf("srcName=%s srcOff=%x trgOff=%x length=%d", src_file_name, src_off, trg_off, length);

  FILE *src_file = open_a_file(src_file_name, "r", &src_size, "No src file is selected");
  if (!src_file) {
  }
  else if (src_off + length > src_size) {
    printf("Can't read %d bytes starting at %x location from src file %s\n",
           length, src_off, src_file_name);
  }
  else if (trg_off > trg_size) {
    printf("Invalid location in trg file %s\n", filename);
  }
  else {

    fseek(src_file, src_off, SEEK_SET);
	buff = (char*) malloc(length);
	size_t len_read = fread(buff, 1, length, src_file);
  
    if (len_read != length) {
      printf("Warning: fread failed. actual read %d, expected read %d\n", (int)len_read, length); 
    }
    fseek(trg_file, trg_off, SEEK_SET);
    fwrite(buff, 1, length, trg_file);
	free(buff);
  }
  
  if (src_file) fclose(src_file);
  if (trg_file) fclose(trg_file);
} 

void file_modify() {
	
  unsigned int location, val;

  unsigned int file_size;
  FILE *file = open_a_file(filename, "r+", &file_size, "Faild to open the file for write");
  if (!file) return;

  int c;
  printf("Please enter location(hex) val(hex): ");
  if ((c = scanf("%x %x", &location, &val)) != 2) {
    printf("File modify: Bad input - scanf returned %d\n", c); 
	return; 
  }	
  fflush(stdin); // Clear the input stream from left over
  
  if (fseek(file, location, SEEK_SET) < 0) {
    printf("Invalid location\n");
  }
  else {  
    fwrite(&val, u_size, 1, file);
  }
  fclose(file);
}

void quit() {
   exit(0);
}


int print_menu(struct menu_item menu[]) {

  int i;
  printf("\nChoose an action::\n");
  for (i=0; menu[i].title != NULL; i++) {
    printf("%d- %s\n", i, menu[i].title);
  }
  return i;
}

int main() {
  menu_item menu[] = {
    {"Set File Name", set_file_name},
    {"Set Unit Size", set_unit_size},
    {"File Display", file_display},
    {"File Modify", file_modify},
    {"Copy From File", copy_from_file},
    {"Quit", quit},
    {NULL, NULL}
  };

  int opt;
  int n_items;
  for (n_items=0; menu[n_items].title != NULL; n_items++);  //Count number of items
  
  while (1) {
	  fflush(stdin); // Clear the input stream from left over
	  print_menu(menu);
      if (scanf("%d", &opt) < 1) {   //Bad input - not a number
		scanf("%*s");
        printf("Bad input, please try again\n");		   
	  }
	  else if (opt >= 0 && opt < n_items)
		  menu[opt].fun();     //Perform the corresponding function
	  else 
		  printf("Wrong option, please try again\n");		   
  }

  return 0;
}
