#include <stdlib.h>
#include <stdio.h>

void vul(char* argv) {
    char buf[32];

    strcpy(buf, argv);
}

int main(int argc, char** argv) {
    if (argc < 2) {
	printf("Please input the string\n");
	exit(-1);
    }

    vul(argv[1]);
}
