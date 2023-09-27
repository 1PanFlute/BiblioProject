# BiblioProject
Examination project for the creation of BIBLIO a distributed library system, which allows users to search and borrow books. Made in C using standard POSIX libraries.

Il progetto consiste nella simulazione di 4 server biblioteca e 40 client utenti che vanno a effettuare le proprie interrogazioni ai database dei vari server biblioteca. 
La richiesta del client è composta dai campi dei libri di interesse (ad esempio: autore, editore, titolo ecc...) e nel caso da un flag -p, che determina la volontà di effettuare il prestito dei vari libri. 
(esempio: --autore: pippo, --editore: pluto, -p)
I server risponderanno diversamente a seconda del tipo di richiesta del client:
- Prestito effettuato.
- Libro/i non disponibile/i.
- Libro non esistente nel database corrente.
- Errore.

Alla fine della simulazione viene lanciato uno script (bibaccess.sh) che va a contare le occorrenze di prestito e di interrogazione di tutti i database della biblioteca durante il loro periodo di attività più recente.

-------------- ISTRUZIONI PER LA COMPILAZIONE/ESECUZIONE DEL CODICE ------------------
- Aprire il terminale e navigare dentro la directory contenente lo zip.
- Estrarre lo zip tramite comando "tar -czvf Gianni_Pan.tar.gz".
- Lo zip contiene una cartella chiamata Progetto.
- Eseguire il comando cd Progetto per spostarsi dentro la directory
- All'interno della cartella eseguire:
  - "make all" per creare tutti gli eseguibili.
  - "make test" per eseguire lo script per avviare 5 server e 40 client.
  - "make clean" per pulire la directory dai file generati.
--------------------------------------------------------------------------------------
