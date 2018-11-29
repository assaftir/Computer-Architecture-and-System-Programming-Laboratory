#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
	int c = 0, calcWords = 0, calcChars = 0, calcLongest = 0;
	int wordsCounter = 0, charsCounter = 0, currentWordLength = 0, maxLength = 0;
	calcWords = argc == 1 ? 1 : 0;
	for (int i = 1 ; i < argc ; i++) {		
		if (strcmp(argv[i], "-w") == 0) calcWords = 1;	
		if (strcmp(argv[i], "-c") == 0) calcChars = 1;
		if (strcmp(argv[i], "-l") == 0) calcLongest = 1;
	}
	while ((c = (char)fgetc(stdin)) == ' ') continue; //eat spaces at the beginning
	while(c != EOF){
		if((char)c == ' ' || (char)c == '\n'){ // end of a word
			wordsCounter++;
			if (currentWordLength > maxLength)
				maxLength = currentWordLength;
			currentWordLength = 0; //reset
			while ((c = fgetc(stdin)) == ' ') {} // eat many spaces
			if((char)c == '\n') wordsCounter--;
		}
		else{ //It's a character other than '\n', ' ', EOF
			charsCounter++;
			currentWordLength++;
			c = fgetc(stdin);			
		}
	}
	if(calcWords) printf("%d\n", wordsCounter);
	if(calcChars) printf("%d\n", charsCounter);
	if(calcLongest) printf("%d\n", maxLength);
	return 0;
}
