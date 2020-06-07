
#include <stdio.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <sys/types.h> 
#include <unistd.h> 
#include <time.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

void action ()
{
  //  printf("Action called!!\n");
};			

void referee (char *, int, int, int *);
void player (char *, int, int *);

int main (int argc, char *argv[])
{
  pid_t pid1, pid2, pid3;
  
  //*** get a shared segment ***//
  int SharedSegment = shmget(IPC_PRIVATE, 3*sizeof(int), IPC_CREAT | 0600);
  if (SharedSegment < 0){
      printf("*** shmget error *** \n");   
      exit(1);
  }
  
   //*** get a shared array ***//
  int *sharedArray = (int *) shmat(SharedSegment, NULL, 0);
  if ( (int ) sharedArray == -1) {
     printf("*** shmat error ***\n");
     exit(1);
 }
  
  printf ("This is a 3-players game with a referee\n");
  if ((pid1 = fork ()) == 0)
    player ("TATA", 0, sharedArray);
  if ((pid2 = fork ()) == 0)
    player ("TITI", 1, sharedArray);
  if ((pid3 = fork ()) == 0)
    player ("TOTO", 2, sharedArray);        
  sleep (1);
  signal (SIGUSR1, action);
  
 // printf("pid1 = %d, pid2=%d, pid3=%d" ,pid1, pid2, pid3);
  
  while (1)
    { 
      // referee will print players score and who won the game
      referee("TATA", 0, pid1, sharedArray);
      referee("TITI", 1, pid2, sharedArray);
      referee("TOTO", 2, pid3, sharedArray);
    }
    
    //*** detach a shared memory ***//
    if(shmdt(sharedArray) == -1){
        perror("*** shmdt error ***");
        exit(1);
    }

}

void player (char *name, int playerID, int *sharedArray)
{
  int points = 0, dice;
  long int ss = 0;
  while (1)
    {
      signal (SIGUSR1, action);	// block myself
      pause ();
      printf ("%s: playing my dice\n", name);
      dice = (int) time (&ss) % 10 + 1;
      printf ("%s: got %d points\n", name, dice);
      points += dice;
      sharedArray[playerID]  = points; // adds individual score to its total points in shared memory
      sleep (3);
      kill (getppid (), SIGUSR1);
    }
}

void referee(char *name, int playerID, pid_t child_pid, int *sharedArray){

      printf ("\nReferee: %s plays\n\n", name);
      kill (child_pid, SIGUSR1);
      pause ();
      printf ("By referee :: %s : Total so far %d\n\n", name, sharedArray[playerID]);


       if (sharedArray[playerID] >= 50)
      {
        printf("*** GAME OVER ***\n");
        printf("By referee :: %s won with its total score: %d \n",name, sharedArray[playerID]);
        kill (0, SIGTERM);
    
      }
}
