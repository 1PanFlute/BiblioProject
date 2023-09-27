// C program for the Client Side
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

// Per formattare il messaggio di richiesta/risposta
#include "message.h"

void* clienthread(void* args);
int checkDuplicate(int numArgs, char** key);
char address[50];
int port, active;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[])
{
	if (argc<2){
		fprintf(stderr, "Argomenti mancanti!\n");
		exit(1);
	}
	// Struttura che immagazzina la struttura del messaggio inviato al server.
	Messaggio richiesta;
	// Buffer per immagazzinare le varie opzioni di richiesta del client.
	char data[128];
	// Inizializzo le strutture.
	memset(&richiesta, 0, sizeof(richiesta));
	memset(&data, 0, sizeof(data));
	int i=1;
	// Condizione che rileva se esiste almeno una doppia opzione --
	if(strstr(argv[i], "--")==NULL && strstr(argv[i+1], "--")==NULL){
		fprintf(stderr, "Richiesta non valida. Esempio valido: ./bibclient --author=\"Ciccio\" --title=\"Biancaneve\"\n");
		exit(1);
	}
	richiesta.type = MSG_QUERY;
	pthread_mutex_lock(&mutex);
	// Controllo se esistono opzioni duplicate.
	int cod=checkDuplicate(argc, argv);
	if (cod==0){
		fprintf(stderr, "Errore: Per ogni opzione è ammessa solo un'occorrenza \n");
		exit(1);
	}
	pthread_mutex_unlock(&mutex);
	// Scrivo le opzioni nel buffer.
	while(argv[i]!=NULL){
		if (strcmp(argv[i], "-p")==0){
			richiesta.type=MSG_LOAN;
		}
			strcat(data, (argv[i])+2);
			strcat(data, ";");
		i++;
	}
	strcpy(richiesta.data, data);
	richiesta.length = strlen(data) + sizeof(unsigned int) + sizeof(char);
	char line[100];
    char host[50];
	// Apro il file bib.conf per prendere le informazioni relative ai server attivi.
	FILE* configFile = fopen("bib.conf", "r");
	if (configFile==NULL){
		fprintf(stderr, "Errore nell'apertura del file.\n");
		exit(1);
	}
	while(fgets(line, sizeof(line), configFile)!=NULL){
		fscanf(configFile, "Hostname: %s\n", host);
		fscanf(configFile, "Address: %s\n", address);
		fscanf(configFile, "Port: %d\n", &port);
		fscanf(configFile, "Active: %d\n", &active);
		// Se il server è attivo, avvio un thread con le corrispondenti informazioni di comunicazione.
		if(active==1){
			pthread_t tid;
			pthread_create(&tid, NULL, clienthread, &richiesta);
			pthread_join(tid, NULL);
		}
	}
	pthread_exit(0);
	return 0;
}

// Funzione che controlla se ci sono opzioni duplicate.
int checkDuplicate(int numArgs, char** key){
	int flagArray[] = {0,0,0,0,0,0,0,0,0,0};
	for (int r=0; r<numArgs; r++){
		if (strstr(key[r], "--autore") != NULL) {
			flagArray[0]+=1;
		} else if (strstr(key[r], "--titolo")!= NULL) {
			flagArray[1]+=1;
		} else if (strstr(key[r], "--editore")!= NULL) {
			flagArray[2]+=1;
		} else if (strstr(key[r], "--anno")!= NULL) {
			flagArray[3]+=1;
		} else if (strstr(key[r], "--nota")!= NULL) {
			flagArray[4]+=1;
		} else if (strstr(key[r], "--prestito")!= NULL) {
			flagArray[5]+=1;
		} else if (strstr(key[r], "--volume")!= NULL) {
			flagArray[6]+=1;
		} else if (strstr(key[r], "--descrizione_fisica")!= NULL) {
			flagArray[7]+=1;
		} else if (strstr(key[r], "--luogo_pubblicazione")!= NULL) {
			flagArray[8]+=1;
		} else if (strstr(key[r], "--collocazione")!= NULL || strstr(key[r], "--scaffale")!= NULL){
			flagArray[9]+=1;
		}
	}
	for (int g=0; g<10; g++){
		if(flagArray[g]>1) return 0;
	}
	return 1;
}

// Funzione che gestisce il protocollo di comunicazione con i server.
void* clienthread(void* args)
{
	// Struttura contenente la richiesta.
	Messaggio client_request = *((Messaggio*)args);
	int network_socket;

	// Creo un socket
	network_socket = socket(AF_INET,
							SOCK_STREAM, 0);

	// Definisco variabili per indirizzo e porta.
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr(address);
	server_address.sin_port = htons(port);

	// Inizio un tentativo di connessione al server.
	int connection_status = connect(network_socket,
									(struct sockaddr*)&server_address,
									sizeof(server_address));

	// Check for connection error
	if (connection_status < 0) {
		fprintf(stderr, "[CLIENT] Errore di connessione.\n");
		exit(1);
	}

	printf("[CLIENT] Connessione Aperta.\n");

	// Invio la query al client
	send(network_socket, &client_request,
		client_request.length, 0);

	Messaggio risposta;
    if (recv(network_socket, &risposta, sizeof(risposta), 0)<0){
        fprintf(stderr, "[CLIENT] Errore durante la lettura.\n");
    } else {
		if (risposta.type==MSG_NO) {
			printf("[CLIENT] La ricerca non ha prodotto nessun risultato.\n");
		} else if (risposta.type==MSG_ERROR) {
			fprintf(stderr, "[CLIENT] La ricerca non è andata a buon fine.\n");
		} else {
			printf("%s\n", risposta.data);
		}
	}
	// Chiudo la connessione.
	printf("[CLIENT] Chiudo la connessione...\n");
	close(network_socket);
	pthread_exit(NULL);

	return 0;
}