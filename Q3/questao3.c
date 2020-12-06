#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

// Número máximo de threads aluno
#define MAX_THREADS 20



/* Variaveis Globais */
pthread_mutex_t mutex;
pthread_cond_t full;
volatile int beberam;
volatile int bebendo;
volatile int saiu;
volatile int qtd_alunos;

void bebe() {
  pthread_mutex_lock(&mutex);

  beberam += 1;
  bebendo++;
  printf("Um aluno sentou pra beber! beberam=%d, saiu=%d, bebendo=%d\n", beberam, saiu, bebendo);

  if (beberam == qtd_alunos) {
    printf("Todos os alunos já sentaram pra beber! beberam=%d, saiu=%d, bebendo=%d\n", beberam, saiu, bebendo);
    pthread_cond_broadcast(&full);
  }
  pthread_mutex_unlock(&mutex);
}

void sai() {
  pthread_mutex_lock(&mutex);

  while (bebendo <= 2 && ((bebendo + saiu) != qtd_alunos)) {
    printf("Um aluno quer sair, mas se ele sair vai deixar alguém beberam sozinho. beberam=%d, saiu=%d, bebendo=%d\n", beberam, saiu, bebendo);
    pthread_cond_wait(&full, &mutex);
  }
  saiu += 1;
  bebendo--;
  printf("Um aluno saiu! beberam=%d, saiu=%d, bebendo=%d\n", beberam, saiu, bebendo);

  if ((bebendo + saiu) == qtd_alunos) {
    pthread_cond_broadcast(&full);
  }

  pthread_mutex_unlock(&mutex);
}

void* aluno_thread() {
  bebe();
  sai();
}

void initialize() {
  // Inicializando mutexes e cond
  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&full, NULL);
  qtd_alunos = MAX_THREADS;
  bebendo = 0;
}

void finish() {
  // Destroindo mutexes e cond
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&full);
}

int main() {
  initialize();

  // Criando threads alunos
  pthread_t alunos[qtd_alunos];

  printf("%d alunos vão beber hoje!\n", qtd_alunos);
  for (int i = 0; i < qtd_alunos; i++) {
    pthread_create(&alunos[i], NULL, aluno_thread, NULL);
  }
  // // Join a thread carro 
  // pthread_join(carro, NULL);
  for (int i = 0; i < qtd_alunos; i++) {
    pthread_join(alunos[i], NULL);
  }
  // printf("Todos os alunos saíram da mesa. Fechando o bar!\n");


  finish();
  return 0;
}