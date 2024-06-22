#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
static int set_semvalue(int);
static void del_semvalue(int);
static int semaphore_p(int);
static int semaphore_v(int);
static int sem_id;
static int s1,s2;

void f1(pid_t pid);
void f2(pid_t pid);

union semun{
  int val;
  struct semid_ds *buf;
  unsigned short *array;
};
void f1(pid_t pid){
  if (!semaphore_p(s1)) exit(EXIT_FAILURE);
  if(pid) f2(pid);
  printf("calling f2");
 if (!semaphore_v(s1)) exit(EXIT_FAILURE);
}

void f2(pid_t pid){
 if (!semaphore_p(s2)) exit(EXIT_FAILURE);
  
  if(!pid) f1(pid);
  printf("calling f1");
 if (!semaphore_v(s2)) exit(EXIT_FAILURE);
}
int main(int argc, char *argv[]){
  pid_t pid;
  pid = fork();
  s1 = semget((key_t)1234, 1, 0666 | IPC_CREAT);
  s2 = semget((key_t)1234, 1, 0666 | IPC_CREAT);
  switch (pid) {
  case -1:
    perror("Forking failed");
    exit(EXIT_FAILURE);
  case 0: //child
    f2(pid);
    printf("done");
    break;
  default: //parent 
    f1(pid);
    printf("done");
    break;
  }
}



static int set_semvalue(int sem_id){
  union semun sem_union;
  sem_union.val = 1;
  if (semctl(sem_id, 0, SETVAL, sem_union) == -1) return(0);
  return(1);
}

static void del_semvalue(int sem_id){
  union semun sem_union;
  if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1)
  fprintf(stderr, "Failed to delete semaphore\n");
}

static int semaphore_p(int sem_num){
  struct sembuf sem_b;
  sem_b.sem_num = 0;
  sem_b.sem_op = -1; /* P() */
  sem_b.sem_flg = SEM_UNDO;
  if (semop(sem_id, &sem_b, 1) == -1){
  fprintf(stderr, "semaphore_p failed\n");
  return(0);
  }
return(1);
}

static int semaphore_v(int sem_num) {
  struct sembuf sem_b;

  sem_b.sem_num = 0;
  sem_b.sem_op = 1; /* V() */
  sem_b.sem_flg = SEM_UNDO;
  if (semop(sem_id, &sem_b, 1) == -1) {
    fprintf(stderr, "semaphore_v failed\n");
    return(0);
  }
  return(1);
}