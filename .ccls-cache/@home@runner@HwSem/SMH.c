/* After the #includes, the function prototypes and the global variable, we come
to the main function. There the semaphore is created with a call to semget,
which returns the semaphore ID. If the program is the rst to be called (i.e.
it's called with a parameter and argc > 1), a call is made to set_semvalue to
initialize the semaphore and op_char is set to X. [./sem & ./sem 1]*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
static int set_semvalue(int);
static void del_semvalue(void);
static int semaphore_p(int);
static int semaphore_v(int);
#define SEM_S1 0  // Semaphore for critical function f1
#define SEM_S2 1  // Semaphore for critical function f2

int sem_id;


void parentproc(pid_t);
void childproc(pid_t);

union semun {
  int val;
  struct semid_ds *buf;
  unsigned short *array;
};

int main(int argc, char *argv[]) {
  pid_t pid;

  sem_id = semget((key_t)1234, 2, 0666 | IPC_CREAT);
  if (!set_semvalue(0) || !set_semvalue(1)) {
    fprintf(stderr, "Failed to initialize semaphore\n");
    exit(EXIT_FAILURE);
  }

  if((pid = fork()) == -1){
    perror("fork");
    exit(1);
  }

  if(pid == 0){//child
    childproc(pid);
  }else{//parent
    parentproc(pid);
  }

  if(pid){
    wait(NULL);
    del_semvalue();
  }
  printf("\nDone");
  exit(EXIT_SUCCESS);
}
/* The function set_semvalue initializes the
semaphore using the SETVAL command in a
semctl call. We need to do this before we can
use the semaphore. */
static int set_semvalue(int sem_num) {
  union semun sem_union;
  sem_union.val = 1;
  if (semctl(sem_id, sem_num, SETVAL, sem_union) == -1)
    return (0);
  return (1);
}
/* The del_semvalue function has almost the same
form, except the call to semctl uses the command
IPC_RMID to remove the semaphore's ID. */

static void del_semvalue() {
  union semun sem_union;
  if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1)
    fprintf(stderr, "Failed to delete semaphore\n");
}
/* semaphore_p changes the semaphore
by -1 (waiting). */

static int semaphore_p(int sem_num) {
  struct sembuf sem_b;

  sem_b.sem_num = sem_num;
  sem_b.sem_op = -1; /* P() */
  sem_b.sem_flg = SEM_UNDO;
  if (semop(sem_id, &sem_b, 1) == -1) {
    fprintf(stderr, "semaphore_p failed\n");
    return (0);
  }
  return (1);
}
/* semaphore_v is similar except for setting
the sem_op part of the sembuf structure to 1,
so that the semaphore becomes available. */

static int semaphore_v(int sem_num) {
  struct sembuf sem_b;

  sem_b.sem_num = sem_num;
  sem_b.sem_op = 1; /* V() */
  sem_b.sem_flg = SEM_UNDO;
  if (semop(sem_id, &sem_b, 1) == -1) {
    fprintf(stderr, "semaphore_v failed\n");
    return (0);
  }
  return (1);
}

void parentproc(pid_t pid){
  if(!semaphore_p(0)) exit(EXIT_FAILURE);
  printf("Parent: Acquired Semaphore S1 & waiting for Child proc\n");
  if(pid) childproc(pid);
  if(!semaphore_v(0)) exit(EXIT_FAILURE);

}

void childproc(pid_t pid){
  if(!semaphore_p(1)) exit(EXIT_FAILURE);
  printf("Child: Acquired Semaphore S2 & waiting for Child proc\n");
  if(!pid) parentproc(pid);
  if(!semaphore_v(1)) exit(EXIT_FAILURE);

}