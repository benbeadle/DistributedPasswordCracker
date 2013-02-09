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
	int len = sizeof(input)/sizeof(char);
	chars* output;
    memset(&output, 0, sizeof(chars));
	output->o = '\0';
	output->t = '\0';
	output->h = '\0';
	output->th = '\0';
	
	if(len == 4) {
		memcpy(output->th, input, sizeof(char));
		memcpy(output->h, input + sizeof(char), sizeof(char));
		memcpy(output->t, input + 2 * sizeof(char), sizeof(char));
		memcpy(output->o, input + 3 * sizeof(char), sizeof(char));
	} else if(len == 3) {
		memcpy(output->h, input, sizeof(char));
		memcpy(output->t, input + sizeof(char), sizeof(char));
		memcpy(output->o, input + 2 * sizeof(char), sizeof(char));
	} else if(len == 2) {
		memcpy(output->t, input, sizeof(char));
		memcpy(output->o, input + sizeof(char), sizeof(char));
	} else {
		memcpy(output->o, input, sizeof(char));
	}
			
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
    memset(&output, 0, sizeof(char)*length);
	if(*input->th != '\0')
		strncpy(input->th, output, sizeof(char));
	if(*input->h != '\0')
		strncpy(input->h, output, sizeof(char));
	if(*input->t != '\0')
		strncpy(input->t, output, sizeof(char));
	strncpy(input->o, output, sizeof(char));
}

int main() {
	char* out = "a";
	chars* a = makeChars(out);
	//cout << a << endl;
	//plusPlus(a);
	//cout << returnCharArray(a) << endl;
}
