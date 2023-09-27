#define LONG_DESCR_LENGTH 100
#define SHORT_DESCR_LENGTH 20

// Struttura che descrive un volume.
typedef struct {
    char autore[LONG_DESCR_LENGTH];
    char titolo[LONG_DESCR_LENGTH];
    char editore[LONG_DESCR_LENGTH];
    char anno[SHORT_DESCR_LENGTH];
    char nota[LONG_DESCR_LENGTH];
    char prestito[SHORT_DESCR_LENGTH];
    char volume[SHORT_DESCR_LENGTH];
    char descrizione_fisica[LONG_DESCR_LENGTH];
    char luogo_pubblicazione[LONG_DESCR_LENGTH];
    char collocazione[SHORT_DESCR_LENGTH];
} Book;

// Array di volumi.
static Book book_ARR[20];

/** Formatta le stringhe passate in modo da eliminare 
 *  spazi bianchi superflui agli estremi di una stringa.
 *
 *   \retval Puntatore alla stringa modificata.
 */
extern char* trimWhitespace(char* str);


/**  Esegue il tokenise delle stringhe relative
 *   ai volumi nel file_record aperto.
 *
 *   \retval struttura contenente un volume.
 */
extern Book parseBookRecord(char* line);