// Macro che definiscono i tipi di messaggi.
#define MSG_QUERY 'Q'
#define MSG_LOAN 'L'
#define MSG_RECORD 'R'
#define MSG_NO 'N'
#define MSG_ERROR 'E'

// Struttura dei messaggi inviati e ricevuti da client e server.
typedef struct{
	char type;
    unsigned int length;
    char data[2048];
} Messaggio;