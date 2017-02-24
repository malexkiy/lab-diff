#include <stdio.h>
#include <string.h>
#include "diff.h"


int main(int argc, char* argv[])
{
	if (argc == 3)
	{
		diff(argv[1], argv[2], "diff.out");
	}
	else if (argc == 4)
	{
		diff(argv[1], argv[2], argv[3]);
	}
	else
	{
		char f1[1025], f2[1025], of[1025];

		printf("File 1: ");
		fgets(f1, 1024, stdin);
		f1[strlen(f1) - 1] = '\0';

		printf("File 2: ");
		fgets(f2, 1024, stdin);
		f2[strlen(f2) - 1] = '\0';

		printf("Out file: ");
		fgets(of, 1024, stdin);
		of[strlen(of) - 1] = '\0';

		diff(f1, f2, of);
	}

	return 0;
}
