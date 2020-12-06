#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

// Número máximo de threads passageiro e capacidade do carro
#define MAX_THREADS 12
#define CAPACITY_BOAT 4

/* Variaveis Globais */
pthread_mutex_t mutex;
pthread_cond_t available;

sem_t embarque_fila; // semaphore para garantir o embarque das threads passageiro 
sem_t todos_embarcou; // semaphore binario para avisar as threads passageiro para esperar a proxima viagem

volatile int embarcou; // numero de threads passageiro que embarcou
volatile int qtd_alunos; // quantidade atual de threads passageiro
volatile int capacidade; // capacidade atual da thread carro

volatile int qtd_ufcg; // capacidade atual da thread carro
volatile int qtd_uepb; // capacidade atual da thread carro


int can_embarcar(char* id) {
	if (((qtd_ufcg + qtd_uepb) < 3)) return 1;

	if (id == "ufcg") {
		if(qtd_uepb == 1 && qtd_ufcg == 2) return 0;

		if(qtd_uepb < 3) return 1;
		else return 0;
	}
	else {
		if(qtd_ufcg == 1 && qtd_uepb == 2) return 0;

		if (qtd_ufcg < 3) return 1;
		else return 0;
	}
}

void rema() {
	sem_wait(&todos_embarcou); // Esperando todos os passageiros embarcar

	printf("O barco está atravessando açude\n");
	// sleep(2);
	printf("Chegou! Magicamente voltando para o começo!\n");
	qtd_ufcg = 0;
	qtd_uepb = 0;
	embarcou = 0;

	for (int i = 0; i < capacidade; i++) {
		sem_post(&embarque_fila); // Sinal para as threads passageiro paara embarcar no barco
	}
	pthread_cond_broadcast(&available);
}

void embarcar(char* id) {
	pthread_mutex_lock(&mutex);

	while (!can_embarcar(id)) {
		printf("Sou aluno da %s e vou ficar esperando a proxima viagem!\n", id);
		pthread_cond_wait(&available, &mutex);
	}
	sem_wait(&embarque_fila); // Esperando um sinal do carro para embarcar

	embarcou++;

	if (id == "ufcg") {
		printf("Aluno(s) %d da (ufcg) embarcou no barco...\n", embarcou);
		qtd_ufcg++;
	}
	else {
		printf("Aluno(s) %d da (uepb) embarcou no barco...\n", embarcou);
		qtd_uepb++;
	}


	if (embarcou == capacidade) {
		sem_post(&todos_embarcou); 
		//ultimo que entrar, rema!
		rema();
	}
	pthread_mutex_unlock(&mutex);
}

void* aluno_ufcg_thread() {
	embarcar("ufcg");
}

void* aluno_uepb_thread() {
	embarcar("uepb");
}

void initialize() {
	// Inicializando mutexes e semaphores
	pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&available, NULL);
	sem_init(&embarque_fila, 0, CAPACITY_BOAT);
	sem_init(&todos_embarcou, 0, 0);
	embarcou = 0;
	qtd_alunos = MAX_THREADS;
	capacidade = CAPACITY_BOAT;
}

void finish() {
	// Destroindo mutexes e semaphores
	pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&available);
	sem_destroy(&embarque_fila);
	sem_destroy(&todos_embarcou);
}

int main() {
	initialize();

	// Criando threads alunos
	pthread_t alunos[qtd_alunos];

	printf("O barco irá realizar %d viagens hoje!\n", (qtd_alunos / capacidade));
	printf("A capacidade do barco é %d\n", capacidade);
	printf("Temos %d alunos esperando para atravessar!\n\n", qtd_alunos);

	for (int i = 0; i < qtd_alunos; i++) {
		if (rand() % 2) {
			pthread_create(&alunos[i], NULL, aluno_ufcg_thread, NULL);
		}		
else {
			pthread_create(&alunos[i], NULL, aluno_uepb_thread, NULL);
		}
	}

	for (int j = 0; j < qtd_alunos; j++) {
		pthread_join(alunos[j], NULL);
	}

	printf("Todas as viagens foram realizadas, abandonando o barco e indo embora.\n");

	finish();
	return 0;
}