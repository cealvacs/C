/*
NAME:       Carlos Alva
Section:    COP4338-U01

I affirm that this program is entirely my own work and none of it is the work
of any other person.

CARLOS ALVA
*/
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#define MAX_ARGS 20
#define BUFSIZ2 1024
#define MAX 100

//Global Variables to hold arguments from cmdline
char* args[MAX_ARGS];
char* argsTemp[MAX_ARGS];
char*argsAft[MAX_ARGS]; //Another Temp variable

//Flags for Input and Output Redirection
int outFlag = 0;
int outAppFlag = 0;
int inFlag = 0;
char *inFile = NULL;
char *outFile = NULL;

void execute(char* cmdline);
int get_args(char* cmdline, char* args[]);
int argucopy(char **args, char **argsTemp, char* symb, int mode);
void resetVar();
void swap(char ** args2, char **argsTemp2);
int process(int in, int out, char **inData, char **outData);
int pipeNumb();
int fileIOnames(char **args);

//Give method that iterates infinitely getting the cmdline and calling the
//appropriate methods
int main (int argc, char* argv [])
{
  char cmdline[BUFSIZ2];
  
  for(;;) {
    printf("COP4338$ ");
    if (fgets(cmdline, BUFSIZ2, stdin) == NULL)
	{ perror("fgets failed");
	exit (1);
	}
	execute(cmdline);	
  }
  return 0;
}

//Given method that stores cmdline into a pointer array of strings
int get_args(char* cmdline, char* args[]) 
{
  int i = 0;

  /* if no args */
  if((args[0] = strtok(cmdline, "\n\t ")) == NULL) 
    return 0; 

  while((args[++i] = strtok(NULL, "\n\t ")) != NULL) {
    if(i >= MAX_ARGS) {
      printf("Too many arguments!\n");
      exit(1);
    }
  }
  /* the last one is always NULL */
  return i;
}

//Separates args, into two parts. Leaves everything before the symbol (symb) in args,
//copies anything after the symbol (symb) to argsTemp.
int argucopy(char **args, char ** argsTemp, char *symb, int mode)
{

	int i = 0;
	int j;
	int k = 0;

	//Mode 0 compares only to a symbol
	//Mode 1 compares to all of "< > >>", stops at first found
	while(args[i] != NULL)
	{
		if (mode == 0)
		{
			if(!strcmp(args[i], symb))
			{	
				args[i] = NULL;
				i++;
				while(args[i] != NULL)
		        	{
                			argsTemp[k] = args[i];
		                	args[i] = NULL;
		                	i++;
                			k++;
		        	}		
				i--;

			}
			i++;
		}
		if (mode == 1)
                {
                        if(!strcmp(args[i], ">") || !strcmp(args[i], ">>") || !strcmp(args[i], "<"))
                        {
                                args[i] = NULL;
                                i++;
                                while(args[i] != NULL)
                                {
                                        argsTemp[k] = args[i];
                                        args[i] = NULL;
                                        i++;
                                        k++;
                                }
                                i--;

                        }
			i++;
		}
	}

	//Fill remaining argsTemp with NULLs	
	while (k < MAX_ARGS)
	{
		argsTemp[k] = NULL;
		k++;
	}

		
	return 0;
}

//Reset Global variables to their default values in preparation for the next iteration
void resetVar()
{
	*args = NULL;
	*argsTemp = NULL;
	*argsAft =  NULL;
	outFlag = 0;
	outAppFlag = 0;
	inFlag = 0;
	inFile = NULL;
	outFile = NULL;
	
}

//Method that handles the forking and piping
void execute(char* cmdline)
{
	int nargs = get_args(cmdline, args);
	if(nargs <= 0) return;

	if(!strcmp(args[0], "quit") || !strcmp(args[0], "exit")) {
		exit(0);
	}

	int pipesNum = pipeNumb();//Gets amount of pipes on cmdline
	  
	int inSave;
	int outSave; 
	inSave = dup(1); //Saves stdin
	outSave = dup(0);//Saves stdout
	//If there are no pipes
	if(pipesNum == 0)
	{
		int pid1;
		pid1 = process(inSave, outSave, args, argsAft);
		waitpid(pid1, NULL,0);

		resetVar();	
	}  	
	 
	 //If there is 1 or more pipes
	if(pipesNum > 0)
	{
		int procNum;
		int pipefd[2];
		int nextIn = 0; //First Iteration with stdin
		int pid2;
				
		for(procNum = 0; procNum < pipesNum; procNum++)
		{
			argucopy(args, argsTemp, "|", 0); //Separates args by the "|" symbol
			pipe(pipefd);
	
			pid2 = process(nextIn, pipefd[1], args, argsAft);
				
			waitpid(pid2, NULL, 0);
			close(pipefd[1]); //Close the Write Part

			swap(args, argsTemp);
			nextIn = pipefd[0]; 
		}
		
		if(nextIn != 0)
			dup2(nextIn, 0);
		
		//Input from previous iteration, output to stdout
		pid2 = process(nextIn, outSave, args, argsAft);//Last iteration with stdout
		waitpid(pid2, NULL, 0);
	
	}
	   
	resetVar(); //Reset variables in preparation for next iteration
	dup2(inSave, 0); //Reset inputs to stdin
	dup2(outSave, 1);//Reset output to stdout
}

//Swaps two pointer arrays
void swap (char **args2, char **argsTemp2)
{
	char *copyTemp[MAX_ARGS];
	int i = 0;
	while (i < MAX_ARGS)
	{
		copyTemp[i] = args2[i];
		i++;
	}	
	
	i = 0;
	while (i < MAX_ARGS)
	{
		args2[i] = argsTemp2[i];
		i++;		
	}

	i = 0;
        while (i < MAX_ARGS)
        {
                argsTemp2[i] = copyTemp[i];
                i++;
        }
		
}

//Method in charge of Input and Output Redirection
int  process(int inPipe, int outPipe, char **inData, char **outData)
{
	int pid;
	
	if ((pid = fork ()) == 0)
    {
		//If input isnt stdin then set to the read part of pipe
		if (inPipe != 0)
			{
			  dup2 (inPipe, 0);
			  close (inPipe);
			}
		
		//If output isnt stdout then set to the write part of pipe
		if (outPipe != 1)
			{
			  dup2 (outPipe, 1);
			  close (outPipe);
			}
		
		int err = 0;
		int outRed;
		int inRed;

		err = fileIOnames(inData);	
			if (err !=0)
				exit(1);

		//Separates args only
		if (inFlag == 1 && (outFlag == 1 ||  outAppFlag == 1))
			argucopy(inData, outData , "", 1); //IN OUT Redirection
		else if (inFlag == 1 && outFlag == 0 && outAppFlag == 0 )
			argucopy(inData, outData,"<", 0);// IN Redirection ONLY
		else if ((outFlag == 1 || outAppFlag == 1) && inFlag == 0)
		{	//OUT Redirection ONLY
			if(outFlag == 1)
				argucopy(inData, outData, ">", 0); //OUT MODE
			if( outAppFlag == 1)
			argucopy(inData, outData,">>", 0);// APPEND MODE
		}
		
		//Set file for output redirection
		if (outFlag == 1)//Truncate output
		{
			outRed = open(outFile, O_CREAT | O_RDWR | O_TRUNC, 0644);
			if (outRed < 0)
			{
				perror("Opening File");
				exit(1);
			}
		}
		else if (outAppFlag == 1)//Append output
		{	
			outRed = open(outFile, O_CREAT | O_RDWR | O_APPEND, 0644);
			if (outRed < 0)
			{
				perror("Opening File");
				exit(1);
			}
		}	
		
		//Set file for input redirection
		if (inFlag == 1)
		{
			inRed = open(inFile, O_RDONLY, 0644);
			if (inRed < 0)
			{
			perror("Opening File");
			exit(1);
			}	
		}
		
		//Redirect Output
		if (outFlag == 1 || outAppFlag == 1)
		{			
			if (dup2(outRed, fileno(stdout)) < 0)
			{
				perror("Dup2");
				exit(1);
			}
			close (outRed);
		}
		
		//Redirect Input
		if (inFlag == 1)
		{
			if (dup2(inRed, fileno(stdin)) < 0)
			{
				perror("In Dup2");
				exit(1);
			}
			close (inRed);
		}

		//Run command
		execvp(inData[0], inData);
		// return only when exec fails
		perror("exec failed");
		exit(-1);
		
	}
	if (pid < 0)
	{
		perror("cant not fork");
		exit(1);
	}
	
	close(inPipe); //Close in pipe
	close(outPipe);//Close out pipe
	return pid;
}

//Counts the number of "|" pipes character available from the cmdline
int pipeNumb()
{
	int i = 0;
	int pipesNum = 0;

	while(args[i] != NULL)
	{	
		if(!strcmp(args[i], "|"))
		pipesNum++;

		i++;
	}

	return pipesNum;
}


//Sets flags for the Input or Output Redirection as well as saving the filenames needed for such processes
int  fileIOnames(char ** args)
{
	int i = 0;
	while (args[i] != NULL)		
	{
		if(!strcmp(args[i], ">"))
		{	
			outFile = args[i+1];				
			outFlag = 1;	
		}
		if(!strcmp(args[i], ">>"))
                {
			outFile = args[i+1];
                 	outAppFlag = 1;
                }
		if(!strcmp(args[i], "<"))
                {
                        inFile = args[i+1];
                        inFlag = 1;
                }
		i++;
	}

	return 0;
}
