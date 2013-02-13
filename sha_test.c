#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>
#include <time.h>
#include <math.h>

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
}

int sha(char* input, int len, char* pass) {
  int i = 0;
	unsigned char temp[SHA_DIGEST_LENGTH];
	char buf[SHA_DIGEST_LENGTH*2];

	memset(buf, 0x0, SHA_DIGEST_LENGTH*2);
	memset(temp, 0x0, SHA_DIGEST_LENGTH);

	SHA1((unsigned char *)input, len, temp);

	for (i=0; i < SHA_DIGEST_LENGTH; i++) {
		sprintf((char*)&(buf[i*2]), "%02x", temp[i]);
    //printf("%02x", temp[i]);
	}
  //printf("%s", buf);
	return strcmp(buf,pass)==0;
}

int main() {
  //clock_t begin, end;
  //double time_spent;
  char ibuf[] = "aaaa";
  char pass_hash[] = "cb990257247b592eaaed54b84b32d96b7904fd95";
  int res;
  int len = sizeof(ibuf)-1;
  int count = 1;
  int i;
  for(i = 0; i < len; i++) {
    count *= 26;
  }
  
  clock_t tic = clock();
  
  printf("Running algorithm %d times.\n", count);
  int j;
  for(j = 0; j < count; j++) {
    //printf("%s = ", ibuf);
    res = sha(ibuf, len, pass_hash);
    //printf(" = %d\n", res);
    increase(ibuf, len);
  }

  clock_t toc = clock();
  printf("Elapsed: %f seconds\n", (double)(toc - tic) / CLOCKS_PER_SEC);
  return 0;
  

	return 0;
}