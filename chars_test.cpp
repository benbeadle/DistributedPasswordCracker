#include <iostream>
using namespace std;

typedef struct {
	char* o;	//Very right letter
	char* t;
	char* h;
	char* th;	//Very left letter
} chars;

//SEG FAULT
chars* makeChars(char* input) {
	printf("One");
	int len = sizeof(input)/sizeof(char);
	chars* output;
	printf("Two");
	strcpy(output->o, '\0');
	strcpy(output->t, '\0');
	strcpy(output->h, '\0');
	strcpy(output->th, '\0');
	printf("Three");
	if(len == 4) {
		printf("Four");
		strcpy(output->th, input);
		strcpy(output->h, input + sizeof(char));
		strcpy(output->t, input + 2 * sizeof(char));
		strcpy(output->o, input + 3 * sizeof(char));
		printf("5");
	} else if(len == 3) {
		printf("6");
		strcpy(output->h, input);
		strcpy(output->t, input + sizeof(char));
		strcpy(output->o, input + 2 * sizeof(char));
		printf("7");
	} else if(len == 2) {
		printf("8");
		strcpy(output->t, input);
		strcpy(output->o, input + sizeof(char));
		printf("9");
	} else {
		printf("10");
		strcpy(output->o, input);
		printf("11");
	}
	printf("12");
	return output;
}
chars* plusPlus(chars* input) {
	*input->o++;
	if(*input->o == '{') {
		*input->o = 'a';
		if(*input->t == '\0') {
			*input->t = 'a';
			return input;
		} else {
			*input->t++;
			if(*input->t == '{') {
				*input->t = 'a';
				if(*input->h == '\0') {
					*input->h = 'a';
					return input;
				} else {
					*input->h++;
					if(*input->h == '{') {
						*input->h = 'a';
						if(*input->th == '\0') {
							*input->th = 'a';
							return input;
						} else {
							*input->th++;
							if(*input->th == '{')
								*input->th = 'a';
							return input;
						}
					}
				}
			}
		}
	}
}
char* returnCharArray(chars* input) {
	int length = 1;
	if(*input->th != '\0')
		length = 4;
	else if(*input->h != '\0')
		length = 3;
	else if(*input->t != '\0')
		length = 2;
	char* output;
	if(*input->th != '\0')
		strcpy(input->th, output);
	if(*input->h != '\0')
		strcpy(input->h, output);
	if(*input->t != '\0')
		strcpy(input->t, output);
	strcpy(input->o, output);
}

int main() {
	printf("Hey");
	char* out;
	strcpy(out, "a");
	printf("Howdy");
	chars* a = makeChars(out);
	printf("last");
	//cout << a << endl;
	//plusPlus(a);
	//cout << returnCharArray(a) << endl;
}
