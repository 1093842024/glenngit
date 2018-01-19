#include "svm-predict.h"
#include "svm-scale.h"
#include "svm-train.h"
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#define NUM 4
#define FINDSCALERRANGE
#define USESCALERANGE

int main(int argc, char **argv)
{


	 int i;
	argc=NUM;
	argv=(char **)malloc(NUM*sizeof(char*));
	for(i=0;i<NUM;i++) 
	{argv[i]=(char*)malloc(100*sizeof(char));}
#ifdef FINDSCALERRANGE

	strcpy(argv[0],"scale");
	strcpy(argv[1],"-s");
	strcpy(argv[2],"range");
	strcpy(argv[3],"allsound");
	
	SVM_SCALE(argc,argv);
	
#endif

#ifdef USESCALERANGE

	strcpy(argv[0],"scale");
	strcpy(argv[1],"-r");
	strcpy(argv[2],"range");
	strcpy(argv[3],"baby");

	SVM_SCALE(argc,argv);

#endif

    for(i=0;i<NUM;i++)
	{free((void*)argv[i]);}
	free((void*)argv);
	system("pause");
	return 0;
}