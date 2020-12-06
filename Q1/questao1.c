#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

// Número máximo de threads passageiro e capacidade do carro
#define MAX_THREADS 12
#define CAPACITY_CAR 4

/* Variaveis Globais */
pthread_mutex_t lock_embarque; // mutex para controle de acesso a variavel 'embarcou'
pthread_mutex_t lock_desembarque; // mutex para controle de acesso a variavel 'desembarcou'
sem_t embarque_fila; // semaphore para garantir o embarque das threads passageiro 
sem_t todos_embarcou; // semaphore binario para avisar as threads passageiro para esperar a proxima viagem
sem_t desembarque_fila; // emaphore para garantir o desembarque das threads passageiro
sem_t todos_desembarcou; //semaphore binario para avisar as threads passageiro para desembarcar
volatile int embarcou; // numero de threads passageiro que embarcou
volatile int desembarcou; // numero de threads passageiro que desembarcou
volatile int viagem_atual; // atual numero de viagens
volatile int qtd_passageiro; // quantidade atual de threads passageiro
volatile int capacidade; // capacidade atual da thread carro

void carregar() {
	printf("A viagem #%d vai começar, hora de carregar!\n", viagem_atual + 1);
	for (int i = 0; i < capacidade; i++) {
		sem_post(&embarque_fila); // Sinal para as threads passageiro paara embarcar no carro
	}
}

void correr() {
	sem_wait(&todos_embarcou); // Esperando todos os passageiros embarcar
	printf("O carro está cheio, hora de viajar!\n");
	sleep(1);
	printf("O carro está realizando a viagem!\n");
	sleep(2);
}

void descarregar() {
	printf("Fim da viagem, hora de descarregar!\n");

	for (int i = 0; i < capacidade; i++) {
		sem_post(&desembarque_fila); // Avisando os passageiros para desembarcar
	}
	sem_wait(&todos_desembarcou); // Avisando para começar a embarcar novamente
	printf("O carro está vazio!\n\n");
	sleep(1);
}

void embarcar() {

	sem_wait(&embarque_fila); // Esperando um sinal do carro para embarcar

	pthread_mutex_lock(&lock_embarque);
	embarcou++;

	printf("%d passageiro(s) embarcaram no carro...\n", embarcou);
	sleep(1);

	if (embarcou == capacidade) {
		sem_post(&todos_embarcou); // Caso seja o ultimo passageiro a embarcar, avisando o carro para correr
		embarcou = 0;
	}
	pthread_mutex_unlock(&lock_embarque);
}

void desembarcar() {
	sem_wait(&desembarque_fila); // Esperando a viagem terminar

	pthread_mutex_lock(&lock_desembarque);
	desembarcou++;

	printf("%d passageiro(s) desembarcaram do carro...\n", desembarcou);
	sleep(1);

	if (desembarcou == capacidade) {
		sem_post(&todos_desembarcou); // Caso seja o ultimo passageiro a desembarcar, avisando ao carro para carregar.
		desembarcou = 0;
	}
	pthread_mutex_unlock(&lock_desembarque);
}

void* carrro_thread() {
	while (viagem_atual < (qtd_passageiro / capacidade)) {
		carregar();
		correr();
		descarregar();
		viagem_atual++;
	}
}

void* passageiro_thread() {
	while (1) {
		embarcar();
		desembarcar();
	}
}

void initialize() {
	// Inicializando mutexes e semaphores
	pthread_mutex_init(&lock_embarque, NULL);
	pthread_mutex_init(&lock_desembarque, NULL);
	sem_init(&embarque_fila, 0, 0);
	sem_init(&todos_embarcou, 0, 0);
	sem_init(&desembarque_fila, 0, 0);
	sem_init(&todos_desembarcou, 0, 0);
	embarcou = 0;
	desembarcou = 0;
	viagem_atual = 0;
	qtd_passageiro = MAX_THREADS;
	capacidade = CAPACITY_CAR;
}

void finish() {
	// Destroindo mutexes e semaphores
	pthread_mutex_destroy(&lock_embarque);
	pthread_mutex_destroy(&lock_desembarque);
	sem_destroy(&embarque_fila);
	sem_destroy(&todos_embarcou);
	sem_destroy(&desembarque_fila);
	sem_destroy(&todos_desembarcou);
}

int main() {
	initialize();

	// Criando threads passageiro e thread carro
	pthread_t passageiros[qtd_passageiro];
	pthread_t carro;

	printf("O carro irá realizar %d viagens hoje!\n", (qtd_passageiro / capacidade));
	printf("A capacidade do carro é %d\n", capacidade);
	printf("Temos %d passageiros esperando para pegar um carona!\n\n", qtd_passageiro);

	pthread_create(&carro, NULL, carrro_thread, NULL);
	for (int i = 0; i < qtd_passageiro; i++) {
		pthread_create(&passageiros[i], NULL, passageiro_thread, NULL);
	}
	// Join a thread carro 
	pthread_join(carro, NULL);

	printf("Todas as viagens foram realizadas, desligando o carro e indo embora.\n");

	finish();
	return 0;
}