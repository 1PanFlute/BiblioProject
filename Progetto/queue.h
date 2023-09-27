// Struttura per la logica della coda
struct node {
    struct node* next;
    int* client_socket;
};

typedef struct node node_t;

/**  Mette in coda le connessioni client
 */
void enqueue(int *client_socket);

/**  Ritorna un puntatore a una connessione client.
 *  
 *   \retval Puntatore alla connessione client.
 */
int *dequeue();
