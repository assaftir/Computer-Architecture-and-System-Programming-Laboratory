#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
	FILE* input = stdin;
	int c = 0, prevChar = 0, calcWords = 0, calcChars = 0, calcLongest = 0, calcLines = 0; //program modes
	int wordsCounter = 0, charsCounter = 0, currentWordLength = 0, maxLength = 0, numOfLines = 0; //counters
	calcWords = argc == 1 ? 1 : 0;
	for (int i = 1 ; i < argc ; i++) {		
		if (strcmp(argv[i], "-w") == 0) calcWords = 1;	
		if (strcmp(argv[i], "-c") == 0) calcChars = 1;
		if (strcmp(argv[i], "-l") == 0) calcLongest = 1;
		if (strcmp(argv[i], "-n") == 0) calcLines = 1;
		if (strcmp(argv[i], "-i") == 0){	
			input = fopen(argv[++i], "r");
			if(!input){
				fprintf(stderr, "%s", "Error while trying to open input file \n");
				exit(-1);
			}
		}		
	}
	while ((c = (char)fgetc(input)) == ' ') { prevChar = ' ' ; continue ; } //eat spaces at the beginning of the first line
	while(c != EOF){
		if((char)c == ' '){ // end of word
			wordsCounter++;
			if (currentWordLength > maxLength)
				maxLength = currentWordLength;
			currentWordLength = 0; //reset
			prevChar = c;
			while ((c = fgetc(input)) == ' ') {} //eat multiple spaces and prepare next char
		}
		else if((char)c == '\n'){ //end of line / word
			if(prevChar != ' ' && prevChar != '\n'){ //for the case when there are multiple spaces at the end of a line or line is empty
				wordsCounter++;
				if (currentWordLength > maxLength)
					maxLength = currentWordLength;
				currentWordLength = 0; //reset			
			}
			numOfLines++;
			prevChar = c;
			if((c = fgetc(input)) == ' '){ //prepare next char
				while ((c = fgetc(input)) == ' ') {} //eat multiple spaces
				prevChar = ' ';
			}
		}
		else{ //it's a character other than '\n', ' ', EOF
			charsCounter++;
			currentWordLength++;
			prevChar = c;
			c = fgetc(input);			
		}
	}
	if(calcWords) printf("%d\n", wordsCounter);
	if(calcChars) printf("%d\n", charsCounter);
	if(calcLongest) printf("%d\n", maxLength);
	if(calcLines) printf("%d\n", numOfLines);
	
	return 0;
	
}
