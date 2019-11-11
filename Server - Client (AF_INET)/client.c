/* Client */

#include <stdio.h>		// Metodi come printf
#include <errno.h>		// Definisce le variabili di errore. Vengono settate automaticamente dalle chiamate di sistema
#include <string.h>		// Definisce funzioni di manipolazione delle stringhe: atoi
#include <stdlib.h> 	// Definisce il metodo exit()
#include <unistd.h> 	// Interfaccia per POSIX: Definisce il metodo close() (per i file descrittori)
#include <arpa/inet.h>	// Definisce i casting come htons
#include <sys/types.h>	// Definisce diversi tipi di dati
#include <sys/socket.h>	// Definisce le funzioni per la Socket come accept, listen ecc.

#define SIZE_BUFFER 1024

int main(int argc, char *argv[]){
	// Args
	if (argc < 3) {
		perror("Error: Needed two args (ip port)");
		exit(1);
	}

	// Return code
	int retcode;

	// Socket
	int client_socket;

	// Address
	struct sockaddr_in server_addr;

	// Buffer
	char buffer[SIZE_BUFFER];

	// Creazione della socket (Descrittore)
	client_socket = socket(AF_INET, SOCK_STREAM, 0);

	if (client_socket == -1){
		perror("Error: creating client socket"); 
		exit(1);
	}

	// Server address
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	server_addr.sin_addr.s_addr = inet_addr(argv[1]); // inet_addr: 

	// Tentativo di connessione al server
	int connection = connect(client_socket, (struct sockaddr*) &server_addr, sizeof(server_addr));

	if (connection == -1){
		perror("Error: Impossibile raggiungere il server");
		exit(1);
	}
	else {
		printf("Connessione al server riuscita \n");
	}

	// Scrittura
	while(1){
		printf("Inserisci il tuo messaggio: ");
		fgets(buffer, SIZE_BUFFER, stdin); // Input
		retcode = write(client_socket, buffer, SIZE_BUFFER - 1);

		if (retcode == -1){
			close(client_socket);
			perror("Error: cannot write");
		}
	}

	return 0;
}
