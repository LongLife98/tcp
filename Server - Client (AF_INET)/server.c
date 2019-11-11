
/* Server */

#include <stdio.h>		// Metodi come printf
#include <errno.h>		// Definisce le variabili di errore. Vengono settate automaticamente dalle chiamate di sistema
#include <stdlib.h> 	// Definisce il metodo exit()
#include <unistd.h> 	// Interfaccia per POSIX: Definisce il metodo close() (per i file descrittori)
#include <arpa/inet.h>	// Definisce i casting come htons
#include <sys/types.h>	// Definisce diversi tipi di dati
#include <sys/socket.h>	// Definisce le funzioni per la Socket come accept, listen ecc.

#define SERVER_ADDR "localhost"
#define SERVER_PORT 9000 
#define SIZE_BUFFER 1024

int main(){
	// Return code
	int retcode;

	// Socket (File descriptors)
	int server_sock, client_sock;

	// Address (Structure)
	struct sockaddr_in server_addr, client_addr;
	socklen_t client_addr_len = sizeof(client_addr);

	// Buffer 
	char buffer[SIZE_BUFFER];

	// Apertura della socket (Creazione del file descrittore)
	if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror ("Error: opening server socket");
		exit(1); // 0 indica una "fine normale", qualsiasi altro valore indica un evento insolito.
	}

	// Configurazione del server address
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY; // INADR_ANY: Binding di tutte le interfaccie (le interfaccie si possono visualizzare con ifconfig)
	server_addr.sin_port = htons(SERVER_PORT); // htons: convert unsigned short to network short

	// Binding: collega il descrittore del file a un indirizzo (in questo caso al server_addr)
	retcode = bind(server_sock, (struct sockaddr*) &server_addr, sizeof(server_addr));

	if (retcode == -1){
		perror("Error: binding address");
		exit(1);
	}

	// Listening: mette la server sock ascolto, il secondo parametro e' la lunghezza massima della coda per le richieste pervenute
	retcode = listen(server_sock, 1); 

	if (retcode == -1){
		perror("Error: listening");
		exit(1);
	}

	// Accept: accetta la connessione del client
	while (1){
		client_sock = accept(server_sock, (struct sockaddr*) &client_addr, &client_addr_len);

		if (client_sock == -1){
			perror("Error: accepting client connection");
			close(server_sock);
			exit(1);
		}

		printf("Connessione riuscita: %d\n", client_sock);

		// Read data from client sock
		while (1){ 
			retcode = read(client_sock, buffer, SIZE_BUFFER - 1); // Riserviamo un byte per settarlo a NULL

			if (retcode > 0) {
				buffer[retcode] = '\0'; // = NUL: The length of a C string (an array containing the characters and terminated with a '\0' character) is found by searching for the (first) NUL byte.
				printf("Client: %s", buffer);
			}

			else if (retcode == 0 || errno == EPIPE) {
				printf("Client connection closed: %d\n", client_sock);
				close(client_sock);
				break; // Se il client si disconette ritorna alla accept e attende per un nuovo client
			}

			else if (retcode == -1){
				perror("Error: reading");
			}
		}
	}

	return 0;
}