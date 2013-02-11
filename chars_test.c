#include <stdio.h>

//TODO: How to get string length?
//Increase the string. Ex: aaaa -> aaab
void increase(char* input, int len) {
	
	input[len - 1]++;
	int i;
	for(i = len-1; i >= 0; i--) {
		if(input[i] == '{') {
			input[i] = 'a';
			if(i - 1 >= 0) {
				input[i - 1]++;
			}
		}
	}
	printf("%s\n", input);
}

int main() {
	char in[] = "zz";
	int i;
	for(i = 0; i < 1; i++) {
		increase(in, 2);
	}
}
