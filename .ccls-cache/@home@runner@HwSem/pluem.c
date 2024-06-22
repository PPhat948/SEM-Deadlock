#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
static int set_semvalue(int);
static void del_semvalue(void);
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

int main(int argc, char *argv[]){
  pid_t pid;
  pid = fork();
  sem_id = semget((key_t)1234, 2, 0666 | IPC_CREAT);

  if((pid = fork()) == -1){
    perror("fork");
    exit(1);
  }
  if(pid == 0){//child
    f1(pid);
  }else{//parent
    f2(pid);
  }
}

void f1(pid_t pid){
  if (!semaphore_p(s1)) exit(EXIT_FAILURE);

  printf("kuy");
  if(pid) f2(pid);

  if (!semaphore_v(s1)) exit(EXIT_FAILURE);
}

void f2(pid_t pid){
  if (!semaphore_p(s2)) exit(EXIT_FAILURE);
  
  printf("hee");
  if(!pid) f1(pid);

  if (!semaphore_v(s2)) exit(EXIT_FAILURE);
}

static int set_semvalue(int num){
  union semun sem_union;
  sem_union.val = 1;
  if (semctl(sem_id, num, SETVAL, sem_union) == -1) return(0);
  return(1);
}

static void del_semvalue(void){
  union semun sem_union;
  if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1)
  fprintf(stderr, "Failed to delete semaphore\n");
}

static int semaphore_p(int num){
  struct sembuf sem_b;
  sem_b.sem_num = num;
  sem_b.sem_op = -1; /* P() */
  sem_b.sem_flg = SEM_UNDO;
  if (semop(sem_id, &sem_b, 1) == -1){
  fprintf(stderr, "semaphore_p failed\n");
  return(0);
  }
return(1);
}

static int semaphore_v(int num) {
  struct sembuf sem_b;

  sem_b.sem_num = num;
  sem_b.sem_op = 1; /* V() */
  sem_b.sem_flg = SEM_UNDO;
  if (semop(sem_id, &sem_b, 1) == -1) {
    fprintf(stderr, "semaphore_v failed\n");
    return(0);
  }
  return(1);
}