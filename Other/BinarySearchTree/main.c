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
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#define ARRAYSIZE 50
typedef struct NODE
{
	char *string;
	int counter;
	struct NODE * left;
	struct NODE * right;
	
}NODE;

NODE * insertNode(NODE * node, char * string, int caseFlag);
NODE * createNode(char * stringData);
int stringCompCaseSen (char *string, char *string2);
void inOrder(NODE * root);
int stringCompCaseIns (char *string, char *string2);
void deallocateMem(NODE * node);
char * copyString (char * string);
void printString (char * string);

NODE * readFromStdin(NODE * root, int caseFlag);
NODE * readFromFile(NODE * root, char ** fileName,  int caseFlag);
void printToStdout(NODE * node);
void printToFile(NODE * node, char * fileName);

int main (int argc, char **argv)
{
	
	int option;		//Holds getopt return value 
	int caseSenFlag = 0;	//1 if comparison is case sensitive, otherwise 0 if is case insensitive
	int outputFlag = 0;	//1 if there is an outputfile, otherwise 0
	int inputFlag = 0;	//1 if there is an inputfile, otherwise 0
	char * outPtr = NULL;	//Points to the output file name
	char ** inPtr = NULL;	//Points to the pointer holding the input file name
	
	struct NODE *root = NULL ;


	//Gets all the options specified by the optstring
	//Creates flags to handle the options specified by the user
	while ((option = getopt(argc, argv, "co:h")) != -1)
	{
		switch (option) {
			case 'c':
				caseSenFlag = 1;
				break;
			case 'o':
				outPtr = optarg;
				outputFlag = 1;
				break;
			case 'h' :
				printf("USAGE: %% bstsort [-c] [-o output_file_name] [input_file_name]");
				break;
			default:
				printf("USAGE: %% bstsort [-c] [-o output_file_name] [input_file_name]"); 
				break;

		}
	}
	//Checks to see if there is an inputfilename argument given, if there is then sets the appropriate flag
	if (*(inPtr = (argv + optind )))
		inputFlag = 1;	

	//Recieved inputs in the way the user specified
	if (inputFlag)
		root = readFromFile(root, inPtr,  caseSenFlag);
	else 	
		root = readFromStdin(root, caseSenFlag);

	//Outputs the correct format the user has specified
	if (outputFlag)
		printToFile(root, outPtr);
	else
		printToStdout(root);

	deallocateMem(root);

	
	return 0;
}

//Read String from standard input
NODE * readFromStdin (NODE * root, int caseFlag)
{
	char string[ARRAYSIZE];
	int exitFlag = 0;
	char e[] = "!e";
	while (!exitFlag)
	{
		//Uses "!e" to exit
		printf("Enter a word or sentence (!e to exit): ");
		scanf(" %[^\n]s",string);
		if ((stringCompCaseIns(string, e)) == 0)
			exitFlag = 1;
		else root = insertNode(root, string, caseFlag);
	}	
	
	return root;	
}

//Reads string from a file
NODE * readFromFile(NODE * root, char ** fileName, int caseFlag)
{
	char string[ARRAYSIZE];
	FILE *readPtr;
	
	if(!(readPtr = fopen(*fileName, "r")))
	{
		printf("ERROR at opening %s\n", *fileName);
		exit(0);
	}	
	
	while(fscanf(readPtr, " %[^\n]s", string) != EOF)
	{
		root = insertNode(root, string, caseFlag);		
	}
	
	if(fclose(readPtr) == EOF)
	{
		printf("ERROR at closing %s\n", fileName);
		exit(0);
	}

	return root;

}

//Prints node to the standard output
void printToStdout(NODE * node)
{
	if (!node)
                return;
        printToStdout(node->left);
        int i;
        for (i=0; i < node->counter;i++)
                printf("%s\n",node->string);
        printToStdout(node->right);	

	return;

}

//Prints a node to a file 
void printToFile(NODE * node, char * fileName)
{
	FILE *writePtr;			

	if (!node)
		return;

	printToFile(node->left, fileName);

	if (!(writePtr = fopen(fileName, "a")))
	{
		printf("ERROR at opening %s\n", fileName);
		exit(0);
	} 	
	int i;	
	//Prints as many times as the counter
	for (i = 0; i < node->counter; i++)
		fprintf(writePtr,"%s\n",node->string);
	

	if (fclose(writePtr) == EOF)
	{
		printf("ERROR at closing %s\n", fileName);
		exit(0);
	}

	printToFile(node->right, fileName);

	return;
	
}

//Inserts node on the right place on the tree, check if nodes are equal
NODE * insertNode(NODE *node, char * string, int caseFlag)
{
		
		
	if (node == NULL)
		return(createNode(string));
	if (caseFlag)
	{
		if (stringCompCaseSen(string, node->string) < 0)
			node->left = insertNode(node->left, string, caseFlag);
		else if (stringCompCaseSen(string, node->string) > 0)
			node->right = insertNode(node->right, string, caseFlag);
		//No need to deallocate node if is equal since it only increments a counter
		else if (stringCompCaseSen(string, node->string) ==  0)
			node->counter = node->counter + 1;
	}
	else
	{
		if (stringCompCaseIns(string, node->string) < 0)
                        node->left = insertNode(node->left, string, caseFlag);
                else if (stringCompCaseIns(string, node->string) > 0)
                        node->right = insertNode(node->right, string, caseFlag);
               
                else if (stringCompCaseIns(string, node->string) ==  0)
                  	node->counter = node->counter + 1;
	}
	
	return node;	
		
	
}

//Creates node with a specified string data
NODE * createNode(char * stringData)
{
	NODE * newNode = (NODE*) malloc(sizeof(NODE));
	if (!newNode)
		exit(0);
	//Stores only a copy of the string
	newNode->string = copyString(stringData);
	newNode->left = NULL;
	newNode->right = NULL;
	newNode->counter = 1;
	return newNode;

}

//In order traversal to print the value on the tree
void inOrder (NODE * node)
{
	if (node == NULL)
		return;
	inOrder(node->left);
	int i;
	for (i=0; i < node->counter;i++)	
		printf("inOrder: %50s \n",node->string);
	inOrder(node->right);

	return;
}

//Does a post order traversal while freeing allocated memory
void deallocateMem(NODE * node)
{
	if (node == NULL)
		return;

	deallocateMem(node->left);
	deallocateMem(node->right);
	free(node->string);
	free(node);	
	
	return;
}

//Creates a copy of a string by dynamically allocating it
char * copyString(char * string)
{	
	
	char * strPtr = (char*)calloc(ARRAYSIZE, sizeof(char));
	if (!strPtr)
		exit(0);

	int i = 0;
	while(string[i] != '\0')
	{
		strPtr[i] = string[i]; 
		i++;
	}
	strPtr[i] = '\0';
	
	return strPtr;
}

//Helper method to print String
void printString( char * string)
{
	int i = 0;
	while (string[i] != '\0')
	{
		printf("%c", string[i]);
		i++;
	}	
	printf("\n");
	return;
}

//Returns 0 if they are equal, 1 if string > string2, -1 if string < string2
int stringCompCaseSen(char *string, char *string2)
{

	int i = 0;
	
	//Uses compare string until it find the NULL character
	//Uses ASCCII numerical values to compare them
	while(string[i] != '\0' ||  string2[i] != '\0')
	{
		if(string[i] >  string2[i])
			return i+1;
		if(string[i] < string2[i])
			return -i-1;
	
		i++;
	}
	
	return 0;//If equal returns 0
}

//Case Insensitive String Compare
int stringCompCaseIns (char *string, char *string2)
{
	int i;

	for (i = 0;string[i] != '\0' || string2[i] != '\0'; i++)
	{
		
		//Covert all letter to lower case in order to compare
		if (tolower(string[i]) > tolower(string2[i]))
			return i + 1;
		if (tolower(string[i]) < tolower(string2[i]))
			return -i-1;

	}

	return 0;

}
		
