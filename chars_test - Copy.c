#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	char in[] = "aaaaa";
	int i;
	for(i = 0; i < 10; i++) {
		increase(in, sizeof(in)-1);
	}
}
