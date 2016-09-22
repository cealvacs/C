#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include <semaphore.h>
#include <unistd.h>

//Default Values
int ROOM_SIZE =  3;
int REPORTER_NUM = 5;

//Reporter Structure that store reporter id and number of questions.
struct reporterStruct {
	int id;
	int questNum; 
} reporter_struct;


pthread_cond_t cond;
pthread_cond_t cond2;
pthread_mutex_t exit_room;
pthread_mutex_t speaker_reporter;
pthread_mutex_t reporters_inside;
pthread_mutex_t skip_turn;
sem_t turn_to_ask;
sem_t room;
sem_t question;

int repNum;//Number of reporters that are left with questions to be asked
int last_asked;//ID of the last person that asked a question 
int question_asked;//Flag 1 = A question waiting to be answered
			//0 = No questions are waiting to be answered
int rep_inside = 0; //Reporters inside the room

void * speaker(void * in);
void * reporter(void * id);
void answerStart();
void answerDone();
void enterConferenceRoom();
void leaveConferenceRoom();
void questionStart();
void questionDone();

struct reporterStruct * currentReporter;//Utilized to create reporter threads
struct reporterStruct * curr;//Points to current reporter being serviced
struct reporterStruct * rep_enter;//Points to reporter entering room
struct reporterStruct * rep_leave;//Points to reporter exiting room

int main(int argc, char* argv[])
{
//Command line parsing
//----------------------------------------
  int rflag = 0;
  int sflag = 0;
  int c;

  opterr = 0;

  while ((c = getopt (argc, argv, "r:s:")) != -1)
    switch (c)
      {
      case 'r':
        
	if(!(repNum = atoi(optarg))){
		printf("Option -r requires an int argument.\n");
		rflag = 1;
	}
	REPORTER_NUM = repNum;
	
        break;
      case 's':
        
	if(!(ROOM_SIZE = atoi(optarg))){
		printf("Option -s requires an int argument.\n");
		sflag = 1;
	}

        break;
      case '?':
        if (optopt == 'r' || optopt == 's')
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr,
                   "Unknown option character `\\x%x'.\n",
                   optopt);
        return 1;
      default:
        abort ();
      }
	if(rflag || sflag){
		exit(0);
	}
//----------------------------------------

	//Initialization of threads
	pthread_t speaker_thread;
	pthread_t reporter_thread[REPORTER_NUM];
	int speaker_id, i ,reporter_id;
	repNum = REPORTER_NUM;
	if (ROOM_SIZE <= 0 || REPORTER_NUM <= 0)
	{
		printf("Values can only be greater than 0\n");
		return 1;
	}

	//Initialize semaphores and mutexes
	
	//Semaphore of size ROOM_SIZE, that controls the number of 
	//reporters that can be inside the room at one time.
	if(sem_init(&room, 0, ROOM_SIZE))
	{
		printf("Error initializing room semaphore\n");
		return -1;
	}

	//Semaphore that controls which reporter can ask a question.
	//It is of size 1 because only 1 reporter can ask at a given time.	
	if(sem_init(&turn_to_ask, 0, 1))
        {
                printf("Error initializing turn_to_ask semaphore\n");
                return -1;
        }
	
	//Mutex used with a conditional variable that synchronizes reporters
	//that ask questions inside the room. It makes sure that no 
	//reporter asks a 2 questions consecutively
	if(pthread_mutex_init(&skip_turn, NULL))
        {
                printf("Error initializing skip_turn semaphore\n");
                return -1;
        }

	//Semaphore that signals that a question has been asked. 
	//Initialized to 0 instead of 1 
	if(sem_init(&question, 0, 0))
        {
                printf("Error initializing question semaphore\n");
                return -1;
        }
	
	//Mutex that controls the critical section of changing the variable
	//which holds the number of reporters in total.
	if(pthread_mutex_init(&exit_room, NULL))
        {
                printf("Error initializing exit_room  mutex\n");
                return -1;
        }

	//Mutex responsible between the synchronization between reporter and
	//speaker. Used in conjunction with a conditional variable to make
	//the speaker fall sleep when no reporters is asking questions.
	if(pthread_mutex_init(&speaker_reporter, NULL))
        {
                printf("Error initializing speaker_reporter mutex\n");
                return -1;
        }
     
	//Mutex that controls the critical section that changes the number of
	//reporters inside the room
	if(pthread_mutex_init(&reporters_inside, NULL))
        {
                printf("Error initializing reporters_inside mutex\n");
                return -1;
        }

	//Conditional variable that controls the signaling between the speaker
	//and the reporter
	pthread_cond_init(&cond, 0);
	//Conditional variable that control synchronization between reporter
	//and reporter, so no reporter can ask 2 questions consecutively
	pthread_cond_init(&cond2, 0);	

	//Memory allocation of reporter structure
	speaker_id = pthread_create(&speaker_thread, NULL, speaker, NULL);
	currentReporter = (struct reporterStruct *)malloc(sizeof(struct reporterStruct ) * REPORTER_NUM);
	curr = (struct reporterStruct *)malloc(sizeof(struct reporterStruct ) * REPORTER_NUM);
	
	//Creation of reporter threads
	for(i = 0; i < REPORTER_NUM; i++)
	{

		currentReporter[i].id = i ;
		
		reporter_id = pthread_create(&reporter_thread[i], NULL, reporter, (void *)&currentReporter[i]);
	}


		
	//Waiting for reporter threads to finish
	for(i = 0; i < REPORTER_NUM; i++){
		pthread_join(reporter_thread[i], NULL);
	}

	//Waiting for reporter thread to finish
	pthread_join(speaker_thread, NULL);
//	printf("SPEAKER DONE\n");	
	
	//Destroying all conditional variables, mutexes, and semaphores
	sem_destroy(&question);
	sem_destroy(&room);
	sem_destroy(&turn_to_ask);
	pthread_mutex_destroy(&skip_turn);
	pthread_mutex_destroy(&speaker_reporter);
	pthread_mutex_destroy(&reporters_inside);
	pthread_mutex_destroy(&exit_room);
	pthread_cond_destroy(&cond);
	pthread_cond_destroy(&cond2);
	return 0;
}

void * speaker(void * in)
{
	//Loop while every reporter had his question answered
	//printf("%d\n",repNum);

	while(repNum > 0) {	
	//	printf("Num of Reporters left: %d\n",repNum);
		pthread_mutex_lock(&speaker_reporter);
		
	//	printf("%d\n", curr->id);
		
		//Checks if condition holds true when reporter wakes up after being signaled
		while (!question_asked || repNum == 0)
		{
			//If there are no reporters left then exit
			if (repNum == 0)
                        	pthread_exit(0);
			//Speaker goes to sleep until being signaled
			pthread_cond_wait(&cond, &speaker_reporter);
		}

		answerStart();
		answerDone();

		pthread_mutex_unlock(&speaker_reporter);

		//Signal the reporter that the speaker is done answering the question 
		//Also change value of flag telling a question has been asnwered
		question_asked = 0;
		sem_post(&question);
		pthread_cond_signal(&cond);
			
	}

	//Exit Speaker Thread
	pthread_exit(0);
}

void * reporter(void * id)
{
	//Creates reporterStruct thread assigning its id and the number of questions it has to ask
	struct reporterStruct  * reporterP = (struct reporterStruct *) id; 
	//printf("current id %d\n", reporterP->id);
	reporterP->questNum = (reporterP->id % 4) + 2; 


	//Calls EnterConferenceRoom() if there is an space available,
	//else it just waits until a spot is open
	sem_wait(&room);
	//Increases the number of reporters inside using a mutex lock to control the access 
	pthread_mutex_lock(&reporters_inside);
	rep_inside++;
	pthread_mutex_unlock(&reporters_inside);
	rep_enter = reporterP;
	enterConferenceRoom();

	//Once inside the room it loops questionStart() and questionDone() while it has questions to do 
	while(reporterP->questNum > 0)
	{	
		
		if(last_asked != reporterP->id || repNum == 1 || rep_inside == 1)
		{
		//wait for turn to ask a question
		sem_wait(&turn_to_ask);
		//Save id of the last reporter that asked a question
		last_asked = reporterP->id;

		//Condition to signal another reporter that someone else is asking a question
		//If there is a single reporter in the room then it does not need to wait for someone else
		//Used to make sure that the last reporter that asks holds on until someone else asks a question
		if(rep_inside >=2 && repNum != 1) 
		{
	
		pthread_cond_signal(&cond2);
		pthread_mutex_lock(&skip_turn);
         	//Waits for another reporter to signal it so current reporter can continue
		//It performs a full handshake with another reporter
		while(last_asked != reporterP->id)      
               		pthread_cond_wait(&cond2, &skip_turn);
		pthread_mutex_unlock(&skip_turn);
		}

		//Makes pointer to current reporter and starts asking a question
		curr = reporterP;
		questionStart();
		
		//Sets value of flag to say that there is a question being asked
		question_asked = 1;
		
		//Signals speaker so it can wake up an asnwer the question	
		pthread_cond_signal(&cond);
		
		//Reporter goes to sleep until speaker signals it saying it has finished answering its question	
		pthread_mutex_lock(&speaker_reporter);		
		//Condition to make sure that a question has been asnwered when reporter wakes up
		while (question_asked)
		{
			pthread_cond_wait(&cond, &speaker_reporter); 
		}
		//Waits for speaker to signal semaphore saying it has asnwered a question
		sem_wait(&question);
		curr = reporterP;	
		questionDone();
		//Decreases the number of questions left to ask
		reporterP->questNum--;
		
		pthread_mutex_unlock(&speaker_reporter);
		
		//Release the resource so someone else can ask a question
		sem_post(&turn_to_ask);
		
		//Last reporter to ask holds until someone else signals it that is has asked a question
		if(rep_inside >=2 && repNum != 1)
		{
		pthread_mutex_lock(&skip_turn);
		//Checks if a specific condition is true when is awaken
		while(last_asked == reporterP->id)
		{	
			pthread_cond_wait(&cond2,&skip_turn);
			if (repNum == 1)
				break;
		}
		//Last step of full handshake, it tells the other reporter he is proceeding to ask more questions
		pthread_cond_signal(&cond2);
		pthread_mutex_unlock(&skip_turn);	
		}

		}

	}
	
	//Gets out of the loop when there is no questions left and gets out of the room
	//Changes the amount of people inside the room and the reporters still left 
	pthread_mutex_lock(&exit_room);
	repNum--;
	rep_inside--;
	//If reporter is the last reporter to be serviced then signal the speaker so it can exit
	if (repNum == 0)	
		pthread_cond_signal(&cond);
	pthread_mutex_unlock(&exit_room);
	rep_leave = reporterP;
	leaveConferenceRoom();
	//Releases resources of the room so another reporter can enter
	sem_post(&room);
	//If there is only 1 reporter left, then signal it so it can continue asking questions without waiting for someone 
	if(repNum == 1)
		pthread_cond_signal(&cond2);	
	//Exits reporter thread
	pthread_exit(0);
}


void answerStart()
{
				
	printf("Speaker starts to answer question for reporter %d.\n", curr->id);
}

void answerDone()
{
	printf("Speaker is done with answer for reporter %d.\n", curr->id);
}

void enterConferenceRoom()
{
	printf("Reporter %d enters conference room.\n", rep_enter->id);
}

void leaveConferenceRoom()
{
	printf("Reporter %d leaves conference room.\n", rep_leave->id);
}

void questionStart()
{
	printf("Reporter %d asks question.\n", curr->id);
}

void questionDone()
{
	printf("Reporter %d is satisfied.\n", curr->id);
}

