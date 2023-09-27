#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
// Header contenente la struttura dati utilizzata per immagazzinare le richieste client.
#include "queue.h"
// Header contenente la struttura dei messaggi inviati da server e client.
#include "message.h"
// Header contenente funzioni per la formattazione del file_record.
#include "format.h"
// Macro utilizzati nel programma.
#define MAX_CONNECTIONS 50
#define BUF_SIZE 6400
#define BUF_OVER "B"
// Variabili per nome file_record, nome biblioteca e numero di libri.
extern int numBooks;
char* records;
char* nome_bib;
// Definisco i Mutex.
pthread_mutex_t queueMutex;
pthread_mutex_t bufferMutex;
pthread_cond_t conditionVar = PTHREAD_COND_INITIALIZER;
// Variabile che termina il while loop infinito dei thread worker.
volatile int terminate=0;
// Variabili contenenti informazioni di connessione del serversocket.
int sockfd;
char address[30];
int active;
int port;
// Stream del file di log.
FILE* file_log;


// Funzione che aggiorna lo stato di attività del server.
void changeActiveStatus(char* bib, int change){
    int line_num=0;
    int current_line=1;
    FILE* configFile;
    configFile = fopen("bib.conf", "r+");
    if (configFile==NULL){
        fprintf(stderr, "[%s] Errore nell'apertura del file config.\n", bib);
        changeActiveStatus(nome_bib, 0);
        exit(1);
    }
    int fd = fileno(configFile);
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    if (fcntl(fd, F_SETLKW, &lock)==-1){
        fprintf(stderr, "[%s] Errore nell'acquisizione della flock.\n", bib);
        changeActiveStatus(nome_bib, 0);
        exit(1);
    }
    char host[50];
    char line[100];
    while(fgets(line, sizeof(line), configFile)!=NULL){
        if (sscanf(line, "Hostname: %40s\n", host)==1){
            if (strcmp(host, bib)==0){
                line_num=current_line+3;
            }
        }
        if (current_line == line_num) {
            if (change==1){
                fseek(configFile, -strlen(line), SEEK_CUR);
                fputs("Active: 1", configFile);
                break;
            } else {
                fseek(configFile, -strlen(line), SEEK_CUR);
                fputs("Active: 0", configFile);
                break;
            }
        }
        current_line++;
    }
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &lock);
    fclose(configFile);
}
// Preleva le informazioni di connessione dal file bib.conf.
// Chiama la funzione per aggiornare lo stato di attività del server.
void fetchConfig(char* bib_name){
    FILE* configFile = fopen("bib.conf", "r");
    if (configFile==NULL){
        fprintf(stderr, "[%s] Errore nell'apertura del file config.\n", bib_name);
        changeActiveStatus(nome_bib, 0);
        exit(1);
    }
    char host[50];
    char read[100];
    int scanned=0;
    while(fgets(read, sizeof(read), configFile)!=NULL){
        if (sscanf(read, "Hostname: %s\n", host)==1){
            if (strcmp(host, bib_name)==0){
                fscanf(configFile, "Address: %s\n", address);
                fscanf(configFile, "Port: %d\n", &port);
                fscanf(configFile, "Active: %d\n", &active);
                if(active==1){
                    fprintf(stderr, "[%s] Server già in uso.\n", bib_name);
                    changeActiveStatus(nome_bib, 0);
                    exit(1);
                }
                break;
            }
        }
    }
    if (address==NULL || port==0){
        fprintf(stderr, "Server non esistente.\n");
        exit(1);
    }
    fclose(configFile);
    changeActiveStatus(bib_name, 1);
}

// Funzione per convertire un valore time_t in stringa. Utilizzata per aggiornare la data del prestito.
void convertTimeToString(time_t timeValue, char *timeString) {
    struct tm *timeInfo = localtime(&timeValue);
    strftime(timeString, 20, "%d-%m-%Y %H:%M:%S", timeInfo);
}

// Funzione chiamata da searchBooks per verificare corrispondenze alla query dell'utente.
int matchesQuery(Book book, char* query) {
    int flag=1;
    char* token = strtok(query, ";");
    while (token != NULL && flag!=0) {
        char* equal = strchr(token, '=');
        if (equal != NULL) {
            *equal = '\0';
            char* value = equal + 1;
            if (strstr(book.autore, value) != NULL ||
                strstr(book.titolo, value) != NULL ||
                strstr(book.editore, value) != NULL ||
                strstr(book.anno, value)!=NULL ||
                strstr(book.nota, value)!=NULL ||
                strstr(book.prestito, value)!=NULL ||
                strstr(book.volume, value)!=NULL ||
                strstr(book.descrizione_fisica, value)!=NULL || 
                strstr(book.luogo_pubblicazione, value)!=NULL ||
                strstr(book.collocazione, value)!=NULL) {
                flag=1;
            } else {
                flag=0;
                break;
            }
        }
        token = strtok(NULL, ";");
    }
    return flag;
}

// Funzione per svuotare il buffer da dati significativi.
void emptyBuffer(char* buffer){
    char empty[] = {0};
    memcpy(buffer, empty, BUF_SIZE);
}
// Buffer di mezzo per impedire a strtok di modificare la stringa originale.
char comp[BUF_SIZE];
// Buffer per la scrittura dei risultati della query.
char tmp[2048];
char results[BUF_SIZE];
char* searchBooks(char* query, char type) {
    emptyBuffer(results);
	int pointer_position =0;
    int numQuery=0;
    pthread_mutex_lock(&bufferMutex);
    for (int k = 0; k < numBooks; k++) {
        strcpy(comp, query);
        // Verifico se la query ha delle corrispondenze nel database.
        if (matchesQuery(book_ARR[k], comp)) {
            numQuery++;
            // Ricavo il tempo.
            time_t currentTime = time(NULL);

            // Estraggo le componenti della data in rispetto al formato della data del prestito nel file_record
            struct tm targetTime;
            memset(&targetTime, 0, sizeof(targetTime));
            
            sscanf(book_ARR[k].prestito, "%d-%d-%d %d:%d:%d",
                    &(targetTime.tm_mday), &(targetTime.tm_mon), &(targetTime.tm_year),
                    &(targetTime.tm_hour), &(targetTime.tm_min), &(targetTime.tm_sec));

            // Aggiusto i parametri dello struct .
            targetTime.tm_hour -=1;
            targetTime.tm_mon -= 1;
            targetTime.tm_year -= 1900;

            // Una volta convertito il valore stringa del prestito in data,
            // possiamo procedere e utilizzare la funzione di libreria difftime()
            // per calcolarci il tempo passato dal prestito.
            time_t targetTimeValue = mktime(&targetTime);
            double difference = difftime(currentTime, targetTimeValue);
            char t_string[20];
            // Verifico il tipo di richiesta e la validità del prestito in questione.
            if (type==MSG_LOAN && (*book_ARR[k].prestito==(unsigned char)0 || difference>30)){
                convertTimeToString(currentTime, t_string);
                strcpy(book_ARR[k].prestito, t_string);
                printf("Prestito: %s\n", book_ARR[k].prestito);
                int written = snprintf(tmp, sizeof(tmp), 
                "Autore: %s; Titolo: %s; Editore: %s; Anno: %s; Note: %s; Prestito: %s; Volume: %s; Descrizione: %s; Luogo di pubblicazione: %s; Collocazione/Scaffale: %s;\n\n", 
                book_ARR[k].autore, book_ARR[k].titolo, book_ARR[k].editore, book_ARR[k].anno, book_ARR[k].nota, book_ARR[k].prestito, book_ARR[k].volume, book_ARR[k].descrizione_fisica, book_ARR[k].luogo_pubblicazione, book_ARR[k].collocazione);
                if(written>=sizeof(results)){
                    return BUF_OVER;
                }
			    strncpy(results+pointer_position, tmp, BUF_SIZE-pointer_position);
			    pointer_position+=written;
            }
            else if (type == MSG_QUERY){
                int written = snprintf(tmp, sizeof(tmp), 
                "Autore: %s; Titolo: %s; Editore: %s; Anno: %s; Note: %s; Prestito: %s; Volume: %s; Descrizione: %s; Luogo di pubblicazione: %s; Collocazione/Scaffale: %s;\n\n", 
                book_ARR[k].autore, book_ARR[k].titolo, book_ARR[k].editore, book_ARR[k].anno, book_ARR[k].nota, book_ARR[k].prestito, book_ARR[k].volume, book_ARR[k].descrizione_fisica, book_ARR[k].luogo_pubblicazione, book_ARR[k].collocazione);
                if(written>=sizeof(results)){
                    return BUF_OVER;
                }
			    strncpy(results+pointer_position, tmp, BUF_SIZE-pointer_position);
			    pointer_position+=written;
            }
            else {
                printf("Libro in prestito.\n");
                numQuery--;
            }
        } 
    }
    // Scrittura nel file di log.
    if (type==MSG_QUERY){
        fputs(results, file_log);
        fprintf(file_log, "query %d\n", numQuery);
        fputs("\n", file_log);
    } else if (type==MSG_LOAN){
        fputs(results, file_log);
        fprintf(file_log, "loan %d\n", numQuery);
        fputs("\n", file_log);
    }
    pthread_mutex_unlock(&bufferMutex);
    return results;
}
char res[BUF_SIZE]; 
// Funzione che processa le richieste dei client
void* task_worker (void* connection){
	int client = *((int*) connection);
	
    // Struct per lo storage della richiesta e risposta.
    Messaggio richiesta;
    Messaggio risposta;
    if (recv(client, &richiesta, sizeof(richiesta), 0)<0){
        fprintf(stderr, "[SERVER] Errore durante la lettura.\n");
        changeActiveStatus(nome_bib, 0);
        exit(1);
    }
    
    strcpy(res, searchBooks(richiesta.data, richiesta.type));

	if (strcmp(res, "B")==0){
		strcpy(risposta.data, "Errore Buffer Pieno!");
		risposta.type=MSG_ERROR;
		risposta.length = strlen(res) + sizeof(unsigned int) + sizeof(char);
	} else if (strlen(res)==0) {
        risposta.type = MSG_NO;
        risposta.length = 0;
    } else {
		strcpy(risposta.data, res);
		risposta.type=MSG_RECORD;
		risposta.length = strlen(res) + sizeof(unsigned int) + sizeof(char);
	}
	send(client, &risposta, sizeof(risposta), 0);
    free(connection);
    close(client);
}
// Funzione avviata dai worker per gestire le richieste in coda dei client.
void* task_handler(void* arg){
	while(terminate!=1){
		int* client_conn;
		pthread_mutex_lock(&queueMutex);
		
		if ((client_conn = dequeue())==NULL){
			pthread_cond_wait(&conditionVar, &queueMutex);
            if(terminate==1){
                pthread_mutex_unlock(&queueMutex);
                pthread_cond_signal(&conditionVar);
                pthread_exit(NULL);
                break;
            }
			client_conn = dequeue();
		}
		pthread_mutex_unlock(&queueMutex);
		if (client_conn!=NULL){
			task_worker(client_conn);
		}
	}
}
// Signal Handler.
void signalHandler(int signal){
    if (signal==SIGINT || signal == SIGTERM){
        // Attendo la terminazione dei thread.
        printf("\n[SERVER] Terminazione dei thread worker...\n");
        terminate=1;
        int pos=0;
        pthread_cond_signal(&conditionVar);
        // Riscrivo il file_record aggiornato.
        FILE* new_file_record;
        char buffer[BUF_SIZE];
        char bufTMP[BUF_SIZE];
        // Utilizzo i buffer per effettuare una scrittura record per record
        for (int k=0; k<numBooks; k++){
            int written=0;
            int numQuery = snprintf(bufTMP, sizeof(bufTMP), 
            "autore: %s; titolo: %s; editore: %s; anno: %s; nota: %s; prestito: %s; volume: %s; descrizione: %s; pubblicazione: %s; collocazione/scaffale: %s;\n", 
            book_ARR[k].autore, book_ARR[k].titolo, book_ARR[k].editore, book_ARR[k].anno, book_ARR[k].nota, book_ARR[k].prestito, book_ARR[k].volume, book_ARR[k].descrizione_fisica, book_ARR[k].luogo_pubblicazione, book_ARR[k].collocazione);
            written +=numQuery;
            if(written>=sizeof(buffer)){
                fprintf(stderr, "Buffer Overflow.\n");
                changeActiveStatus(nome_bib, 0);
                exit(1);
            }
            strncpy(buffer+pos, bufTMP, BUF_SIZE-pos);
            pos+=written;
        }
        printf("[SERVER] Wrote %ld characters.\n", strlen(buffer));
        // Apro il file in modalità scrittura.
        new_file_record= fopen(records, "w");
        if (new_file_record==NULL){
            fprintf(stderr, "[SERVER] Errore nell'apertura del file.\n");
            changeActiveStatus(nome_bib, 0);
            exit(1);
        }
        fwrite(buffer, sizeof(char), strlen(buffer), new_file_record);
        fclose(new_file_record);
        // Cambio lo stato di attività del server.
        changeActiveStatus(nome_bib, 0);
        // Eseguo azioni di pulizia.
        close(sockfd);
        pthread_mutex_destroy(&queueMutex);
        pthread_cond_destroy(&conditionVar);
        pthread_mutex_destroy(&bufferMutex);
        // Uscita dal programma.
        printf("\n[SERVER] Terminato.\n");
        exit(0);
    }
}

int main(int argc, char *argv[])
{
    // Segnali.
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // Mutex.
    if(pthread_mutex_init(&bufferMutex, NULL)!=0){
        perror("[SERVER] Errore creazione bufferMutex: ");
        exit(1);
        
    }
    if(pthread_mutex_init(&queueMutex, NULL)!=0){
        perror("[SERVER] Errore creazione bufferMutex: ");
        exit(1);
    }

    // Controlla i parametri su linea di comando.
    if (argc!=4) {
        perror("[SERVER] Argomenti Mancanti!: ");
        exit(1);
    }
    nome_bib = argv[1];
    records = argv[2];
    int W = atoi(argv[3]);

	// Inizializzo threadpool di grandezza W.
	pthread_t t_pool[W];
    // Definisco variabili per le socket.
    int client_sockfd;
    struct sockaddr_in serv_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Apro file_record in read mode.
    FILE* file_record;
    file_record= fopen(records, "r");
    if (file_record==NULL){
        fprintf(stderr, "[SERVER] Errore nell'apertura del file_record.\n");
        exit(1);
    }

    // Apertura del file di log in write mode.
    char copy[50];
    strcpy(copy, nome_bib);
    char* logName = strcat(copy, ".log");
    file_log = fopen(logName, "w");
    if (file_log==NULL){
        fprintf(stderr, "[SERVER] Errore nell'apertura del file log.\n");
        exit(1);
    }


	// Carico in memoria i volumi.
	int l=0;
	char buf[BUF_SIZE];
	while(fgets(buf, sizeof(buf), file_record) != NULL){
		// Tokenizzo le stringhe e carico le coppie chiave valore in memoria.
		Book book = parseBookRecord(buf);
		book_ARR[l]=book;
		l++;
	}
    fclose(file_record);
    emptyBuffer(buf);

    // Creo il socket del server.
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("[SERVER] Errore nella creazione del socket: ");
        exit(1);
    }

    // Definisco le informazioni di connessione al socket.
    memset(&serv_addr, '0', sizeof(serv_addr));
    fetchConfig(nome_bib);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY; 
    serv_addr.sin_port = htons(port);

    // Associo il socket alle informazioni di connessione 
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("[SERVER] Errore nell'associazione del socket: ");
        changeActiveStatus(nome_bib, 0);
        exit(1);
    }

    // Mi metto in ascolto sulla porta.
    if (listen(sockfd, MAX_CONNECTIONS) < 0) {
        perror("[SERVER] Errore durante l'ascolto per le connessioni: ");
        changeActiveStatus(nome_bib, 0);
        exit(1);
    }

	// Creo pool di thread che andranno a gestire le connessioni.
	for (int i=0; i<W; i++){
		pthread_create(&t_pool[i], NULL, task_handler, NULL);
	}
    // Ciclo while in cui accetto tutte le connessioni in arrivo.
	while (1) {
		printf("[SERVER]In ascolto sulla porta %d...\n", port);
		    client_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_len);
    	if (client_sockfd < 0) {
            perror("[SERVER] Errore nell'accettazione della connessione: ");
            changeActiveStatus(nome_bib, 0);
        	exit(1);
    	}
        printf("[SERVER] Connessione Accettata.\n");
		// Aggiungo in coda la connessione del client in mutua escusione.
		int *client_connection=malloc(sizeof(int));
		*client_connection = client_sockfd;
		pthread_mutex_lock(&queueMutex);
		enqueue(client_connection);
		pthread_cond_signal(&conditionVar);
		pthread_mutex_unlock(&queueMutex);
	}
	return 0;
}
