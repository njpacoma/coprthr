
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "stdcl.h"

int main( int argc, char** argv )
{
	int i,j;

	CONTEXT* cp = 0;

	i=1;
	char* arg;
	while(i<argc) {
		arg=argv[i++];
		if (!strncmp(arg,"--dev",6)) { cp = stddev; }
		else if (!strncmp(arg,"--cpu",6)) { cp = stdcpu; }
		else if (!strncmp(arg,"--gpu",6)) { cp = stdgpu; }
	}

	if (!cp) exit(-1);

	int ndev = clgetndev(cp);

	printf("%d\n",ndev); 
	exit(ndev);

}
