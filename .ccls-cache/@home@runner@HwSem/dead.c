//Praphasiri Wannawong 6513116
//Puttipong Yomabut 6513134
//Patiharn Kamenkit 6513170
//Phattaradanai Sornsawang 6513172
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <unistd.h>
static int set_semvalue(int);
static void del_semvalue(int);
static int semaphore_p(int);
static int semaphore_v(int);
void f1(pid_t, int);
void f2(pid_t, int);

union semun {
  int val;
  struct semid_ds *buf;
  unsigned short *array;
};

static int set_semvalue(int sem_id) {
  union semun sem_union;
  sem_union.val = 1;
  if (semctl(sem_id, 0, SETVAL, sem_union) == -1)
    return (0);
  return (1);
}

static void del_semvalue(int sem_id) {
  union semun sem_union;

  if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1)
    fprintf(stderr, "Failed to delete semaphore\n");
}

static int semaphore_p(int sem_id) {
  struct sembuf sem_b;

  sem_b.sem_num = 0;
  sem_b.sem_op = -1; /* P() */
  sem_b.sem_flg = SEM_UNDO;
  if (semop(sem_id, &sem_b, 1) == -1) {
    fprintf(stderr, "semaphore_p failed\n");
    return (0);
  }
  return (1);
}

static int semaphore_v(int sem_id) {
  struct sembuf sem_b;

  sem_b.sem_num = 0;
  sem_b.sem_op = 1; /* V() */
  sem_b.sem_flg = SEM_UNDO;
  if (semop(sem_id, &sem_b, 1) == -1) {
    fprintf(stderr, "semaphore_v failed\n");
    return (0);
  }
  return (1);
}

void f1(pid_t pid, int sem_id){
  semaphore_p(sem_id);
  if(pid) f2(pid, sem_id);
  semaphore_v(sem_id);
  printf("Done!\n");
}

void f2(pid_t pid, int sem_id){
  semaphore_p(sem_id);
  if(!pid) f1(pid, sem_id);
  semaphore_v(sem_id);
  printf("Done!\n");
}

int main(){

  int child = fork();
  //create semaphore 1
  int s1 = semget((key_t)1111, 1, 0666 | IPC_CREAT);
  //create semaphore 1
  int s2 = semget((key_t)2222, 1, 0666 | IPC_CREAT);

  switch(child){
    case -1:
      perror("Forking failed");
      exit(EXIT_FAILURE);
    case 0:

      if (!set_semvalue(s2)) {
        fprintf(stderr, "Failed to initialize semaphore\n");
        exit(EXIT_FAILURE);
      }
      //child call f2
      f2(child, s2);
      break;
    default:
      if (!set_semvalue(s1)) {
        fprintf(stderr, "Failed to initialize semaphore\n");
        exit(EXIT_FAILURE);
      }
      //child call f1
      f1(child, s1);
      break;
  }



}

