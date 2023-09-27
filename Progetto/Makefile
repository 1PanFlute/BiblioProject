CC = gcc
ITERAZIONI = 1 2 3 4 5 6 7 8

BIB_NAMES = bib_san_miniato, bib_santa_croce, bib_fucecchio bib_pontedera bib_cerreto_guidi
WORKER_THREADS = 1 2 3 4 5

all: bibserver bibclient

bibserver: Server.o queue.o format.o
	$(CC) -pthread Server.o  queue.o format.o -o bibserver

bibclient: Client.o
	$(CC) -pthread Client.o -o bibclient

Server.o: Server.c queue.h format.h message.h
	$(CC) -c Server.c

Client.o: Client.c message.h
	$(CC) -c Client.c

queue.o: queue.c queue.h
	$(CC) -c queue.c

format.o: format.c format.h
	$(CC) -c format.c

test: bibserver bibclient
	./bibserver bib_san_miniato bib1.txt 1 & \
	./bibserver bib_santa_croce bib2.txt 2 & \
	./bibserver bib_fucecchio bib3.txt 3 & \
	./bibserver bib_cerreto_guidi bib4.txt 4 & \
	./bibserver bib_pontedera bib5.txt 5 & \
	sleep 1; \
	echo "Launching 40 clients..."; \
	./bibclient --autore="Kernighan, Brian W." -p
	./bibclient --autore="Melis, Antonio Pietro Angelo"
	./bibclient --titolo="Mathematical and Algorithmic Foundations of the Internet" -p
	./bibclient --autore="Melis, Antonio Pietro Angelo" -p
	./bibclient --autore="Kernighan, Brian W."
	sleep 1;
	./bibclient --autore="Di Ciccio, Antonio" -p
	./bibclient --autore: "Luccio, Fabrizio" --titolo="Mathematical and Algorithmic Foundations of the Internet" --editore="CRC Press, Taylor and Francis Group" --anno="2011"
	./bibclient --autore="Kernighan, Brian W." -p
	./bibclient --autore="Morgan Kaufmann" -p
	./bibclient --titolo="Compilers: Principles, Techniques, and Tools"
	sleep 1;
	./bibclient --autore="Kernighan, Brian W." -p
	./bibclient --autore="Melis, Antonio Pietro Angelo"
	./bibclient --titolo="Mathematical and Algorithmic Foundations of the Internet" -p
	./bibclient --autore="Melis, Antonio Pietro Angelo" --titolo="Manuale di architettura fiorentina" -p
	./bibclient --autore="Kernighan, Brian W."
	sleep 1;
	./bibclient --autore="Di Ciccio, Antonio" -p
	./bibclient --autore: "Luccio, Fabrizio" --titolo="Mathematical and Algorithmic Foundations of the Internet" --editore="CRC Press, Taylor and Francis Group" --anno="2011" -p
	./bibclient --autore="Kernighan, Brian W." -p
	./bibclient --autore="Morgan Kaufmann" -p
	./bibclient --titolo="Compilers: Principles, Techniques, and Tools"
	sleep 1;
	./bibclient --autore="Hennessy, John L., Patterson, David A." --titolo="Computer Architecture, Fifth Edition: A Quantitative Approach" -p
	./bibclient --autore="Kernighan, Brian W."
	./bibclient --nota="Chapman & Hall/CRC Applied Algorithms and Data Structures series"
	./bibclient --autore="Taibo II, Paco Ignacio" --titolo="Dias De Combate" --editore="Planeta Mexicana" -p
	./bibclient --autore="Hennessy, John L., Patterson, David A." --titolo="Computer Architecture, Fifth Edition: A Quantitative Approach"
	sleep 1;
	./bibclient --titolo="Compilers: Principles, Techniques, and Tools"
	./bibclient --descrizione_fisica="137p" --collocazione="WWhh.54"
	./bibclient --autore="Vasquez Montalban, Manuel" --titolo="La soledad del manager" --editore="Planeta Editorial" --anno="1977"
	./bibclient --autore="Kernighan, Brian W." -p
	./bibclient --autore="Kernighan, Brian W."
	sleep 1;
	./bibclient --autore="Hennessy, John L., Patterson, David A." --titolo="Computer Architecture, Fifth Edition: A Quantitative Approach"
	./bibclient --autore="Kernighan, Brian W." -p
	./bibclient --nota="Chapman & Hall/CRC Applied Algorithms and Data Structures series" -p
	./bibclient --autore="Taibo II, Paco Ignacio" --titolo="Dias De Combate" --editore="Planeta Mexicana"
	./bibclient --autore="Hennessy, John L., Patterson, David A." --titolo="Computer Architecture, Fifth Edition: A Quantitative Approach" -p
	sleep 1;
	./bibclient --autore="Kernighan, Brian W." -p
	--autore="Taibo II, Paco Ignacio" --titolo="Dias De Combate" --editore="Planeta Mexicana" -p
	./bibclient --autore="Kernighan, Brian W." -p
	./bibclient --autore: "Luccio, Fabrizio" --titolo="Mathematical and Algorithmic Foundations of the Internet" --editore="CRC Press, Taylor and Francis Group" --anno="2011" -p
	./bibclient ./bibclient --autore="Vasquez Montalban, Manuel" --titolo="La soledad del manager" --editore="Planeta Editorial" --anno="1977" -p
	sleep 1; \
	killall -SIGINT bibserver; \
	sleep 10; \
	echo "Avvio gli Script."; \
	./bibaccess.sh --query bib_san_miniato.log bib_santa_croce.log bib_fucecchio.log bib_pontedera.log bib_cerreto_guidi.log
	./bibaccess.sh --loan bib_san_miniato.log bib_santa_croce.log bib_fucecchio.log bib_pontedera.log bib_cerreto_guidi.log

clean:
	rm -f bibserver bibclient *.o

.PHONY: test clean