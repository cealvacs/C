/*
 * NAME: Carlos Alva
 * Section: COP4338-U01
 *
 * I affirm that this program is entirely my own work
 * and none of it is the work of any other person.
 *
 * CARLOS ALVA
 * */
#include <stdio.h>
#include "bmplib2.h"
#include "bmplib2.c"
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>


int flip (PIXEL *orig, PIXEL **new, int rows, int cols);
int rotate (PIXEL *orig, PIXEL **new, int *rows, int *cols, int degree);
int enlarge (PIXEL *orig, PIXEL **new, int *rows, int *cols, int scale);

int main(int argc, char **argv)
{	

	char *outPtr = NULL;
	char *inPtr = NULL;
	
	int rowInt ;
	int colInt ;
	int degree = 0;
	int scale = 1;
	int option;
	int scaleFlag = 0;
	int rotateFlag = 0;
	int flipFlag = 0;
	int outFlag = 0;
	int inFlag = 0;
	char stdName[] = "default.bmp\0";
	
	while((option = getopt(argc, argv, "s:r:fo:h")) != -1)
	{
		switch (option)
		{

			case 's':
				
				scale = atoi(optarg);
				scaleFlag = 1;
				if(scale <= 0){
                                        printf("\nScale requires only + integers greater than 0\n");
                                        return -1;}
				break;
			case 'r':
				degree = atoi(optarg);
				rotateFlag = 1;
				if(degree % 90 != 0){
					printf("\nDegree %d needs to be -/+ multiples of 90\n", degree);
					return -1;}
				break;
			case 'f':
				flipFlag = 1;
				break;
			case 'o':
				outPtr = optarg;
				outFlag = 1;
				break;
			case '?':
				if(optopt == 's')
					printf("\nOption -%c requires an scale (only postive integers)\n", optopt);
				else if (optopt == 'r')
					printf("\nOption -%c requires a degree (-/+ multiples of 90)\n", optopt);	
				else if (optopt == 'o')
                                        printf("\nOption -%c requires an output file\n", optopt);
				else if (isprint(optopt))
					printf("\nUnknown option -%c\n", optopt);
				else printf("\nIllegal character\n");
				return -1;
			case 'h':
				printf("\nUSAGE: %%bmptool [-s scale | -r degree |-f] [-o output_file.bmp] [input_file.bmp]\n");
				 return -1;
			default:
				return -1;
	
		}

	}
	


	char outFileName[50];
	char inFileName[50];

	if (optind < argc)
	{
		inPtr = argv[optind];
		inFlag = 1;
	}
	else
	{
		printf("\nPlease type an input file name: ");
		scanf("	%s", inFileName);
		inPtr = inFileName;
		inFlag = 1;
	}


	if (!outFlag)
        {
                printf("\nPlease type an output file name: ");
                scanf("	%s", outFileName);
                outPtr = outFileName;
		outFlag = 1;
        }



//	printf("\nScale: %d Degree: %d Flip: %d OutputFile: %s InputFile: %s\n",scale , degree, flipFlag, outPtr,inPtr);

	int multi = scaleFlag + rotateFlag + flipFlag;
	
	PIXEL  * imageOrig;
        PIXEL  * imageNew;


	int err = 0;
	if(scaleFlag == 1)
	{
		err += readFile (inPtr, &rowInt, &colInt, &imageOrig);
		err += enlarge(imageOrig,&imageNew, &rowInt, &colInt, scale);
		err += writeFile(outPtr, rowInt, colInt, imageNew);
		if( err != 0 )//Checks to see if there is any error returned from methods
	        {
                	printf("\nERROR: Scale method Failed\n");
                	return -1;
       		 }		

		if(multi > 1)
			inPtr = outPtr;

	}
	if(rotateFlag == 1)
	{
		err += readFile (inPtr, &rowInt, &colInt, &imageOrig);
		err += rotate(imageOrig, &imageNew, &rowInt, &colInt, degree);
		err += writeFile(outPtr, rowInt, colInt, imageNew);
		if( err != 0 )//Checks to see if there is any error returned from methods
                {
                        printf("\nERROR: Rotate method Failed\n");
                        return -1;
                 }

		if(multi > 1)
			inPtr = outPtr;
	
	}
	if(flipFlag == 1)
	{
		err += readFile (inPtr, &rowInt, &colInt, &imageOrig);
		err += flip (imageOrig, &imageNew, rowInt, colInt);
                err += writeFile(outPtr, rowInt, colInt, imageNew);
		if( err != 0 )//Checks to see if there is any error returned from methods
                {
                        printf("\nERROR: Flip method Failed\n");
                        return -1;
                 }

	}

	
	if (multi != 0) //Dealloactes memory only if a .bmp file has been read
	{
		free(imageOrig);
		free(imageNew);
	}

	return 0;
}

//Flip Image Horizontally
int flip (PIXEL *orig, PIXEL **new, int rows, int cols)
{
	int rowIdx;
	int colIdx;

  	if (rows <= 0 || cols <= 0)//Checks for negative values of rows and columns
    		return -1;
  	*new = (PIXEL*) malloc(rows*cols*sizeof(PIXEL));
	if(!*new)
		exit(1);	

	//FlipHorizontal
	for (rowIdx=0; rowIdx < rows*cols; rowIdx += cols)
		for (colIdx=0; colIdx < cols; colIdx++)
			(*new)[colIdx+rowIdx] = orig[cols - colIdx - 1 + rowIdx];

  	return 0;
}

//Rotates Image buy a certain degree (+/-)
int rotate (PIXEL *orig, PIXEL **new, int *rows, int *cols, int degree)
{
	int rotation;
	int rowIdx;
        int colIdx;
        int newpos = 0;
        if (rows <= 0 || cols <= 0)//Checks to see if rows and columns are negative
                return -1;
 	
	//Checks a second time it is a multiple of 90
	if (degree % 90 != 0)
		return -1;

	degree = degree % 360;//Checks for multiples of 360
	
	//Checks for equal values of -/+ degrees
	if (degree == 90 || degree == -270)
		rotation = 1;
	if (degree == 180 || degree == -180)
		rotation = 2;
	if (degree == 270 || degree == -90)
		rotation = 3;
	if (degree == 0)//-/+ 360
		rotation = 4;
	
	int i;
	int tempcol;
	for (i = 0; i < rotation; i++)
	{
		*new = (PIXEL*) malloc((*rows)*(*cols)*sizeof(PIXEL));
		if(!*new)
			exit (1);	
		for (colIdx=1; colIdx <= *cols; colIdx++)
                	for (rowIdx=0; rowIdx < (*cols)*(*rows); rowIdx += *cols)
                        	(*new)[newpos++] = orig[*cols -colIdx + rowIdx];
	
		orig = *new;
		newpos = 0;
		tempcol = *cols;
		*cols = *rows;//Flips rows and columns value when image is rotated
		*rows = tempcol;		
		colIdx = 1;//Resets values for next iteration
		rowIdx = 0;
	}


	return 0;
}

//Scales Image by a certain scale
int enlarge (PIXEL *orig, PIXEL **new, int *rows, int *cols, int scale)
{

        int rowIdx;
        int colIdx;
	int copyCol;
	int copyRow = 0;
        int newpos = 0;
        if (rows <= 0 || cols <= 0)//Checks if rows and columns are negative
                return -1;
	*new = (PIXEL*) malloc((*rows)*(*cols)*scale*scale*sizeof(PIXEL));
                if(!*new)
                        exit (1);

	int oldpos = 0;
	int oldpostemp;
	for(rowIdx=0; rowIdx < (*rows); rowIdx++)
	{	for(copyRow=0; copyRow < scale; copyRow++)
		{	
			for(colIdx=0; colIdx < (*cols); colIdx++)
			{	for(copyCol=0; copyCol < scale; copyCol++)
					(*new)[newpos++] = orig[oldpos];
			
				oldpos++;	
			}
			
			oldpostemp = oldpos;
			oldpos = oldpostemp - *cols;

		}

		oldpos = oldpostemp;
	}	


	*rows= *rows * scale;//Calculates new row values after enlarging image
	*cols = *cols * scale;//Calculates new column values after enlarging image


	return 0;
}
