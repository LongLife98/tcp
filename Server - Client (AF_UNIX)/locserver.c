//server nel dominio locale AF_UNIX

#include <stdio.h>              // libreria standard per Input e Output 
#include <unistd.h>             // consente l'accesso alle funzioni di sistema POSIX (read,write,close,fork,pipe)
#include <stdlib.h>             // libreria standard per molteplici utilizzi su controllo processi,allocazione memoria,conversione tra tipi e numeri random
#include <sys/socket.h>         // libreria per la definizione delle socket (bind(),socket()...)
#include <sys/un.h>             // libreria per la definizione delle socket nel dominio UNIX
#include <signal.h>             // libreria per la gestione dei segnali fra processi

#define MAXBUF 16               // specifica la dimensione del buffer di ricezione
#define NOMESOCK "servsock"   // definisce il nome della socket

//funzioni e variabili globali 

void terminazione(int);         // prototipo funzione per terminare il server da tastiera,necessario un signal handler.
void cleanup();                      // prototipo funzione per chiudere connect_socket e server_socket
int terminated = 0;             // variabile di controllo per il ciclo di accettazione delle connessioni
int server_socket;              // variabile contenente il risultato di socket()
int connect_socket;             // variabile contenente il risultato dell'accept, seconda variabile di controllo per il ciclo di accettazione delle connessioni
char buffer[MAXBUF];            // casting del buffer

int main (int argc, char * argv[]) {

    socklen_t client_addr_len;                         // variabile di tipo socklen_t necessario alla funzione accept()
    int retcode;                                       // variabile per il controllo delle funzioni bind() e listen(), e per contenere il risultato della read()
    struct sockaddr_un client_addr , server_addr;       // strutture di tipo sockaddr_un , quindi di dominio UNIX. Rappresentano il client e il server

    signal(SIGINT,terminazione);                        //handler per terminare il server da tastiera, legato alla funzione terminazione()

    // apertura socket del server con socket()

    if ( (server_socket = socket(AF_UNIX,SOCK_STREAM,0)) == -1 ) // inizializzazione dentro l'if
        {   
            perror("openin server socket: ");
            exit(-1);
        }
    
    // assegnazione indirizzo della socket necessario alla pubblicazione 

    if ( unlink(NOMESOCK) == -1) {                              //  unlink deve essere eseguito prima della bind(), questo elimina il pathname dal filesystem.
            perror("cancellando");                              //  se ritorna -1 la cancellazione non è stata eseguita
            fprintf(stderr,"no old socket \"%s\" to delete\n", NOMESOCK) ;
    } else
        fprintf(stderr, "old socket %s deleted\n", NOMESOCK);   // se ritorna 0 la cancellazione è stata eseguita
	server_addr.sun_family = AF_UNIX;                           // assegnazione dei parametri di server_addr 
	strcpy(server_addr.sun_path, NOMESOCK);   


    // pubblicazione socket con bind() e listen()

    retcode = bind(server_socket, (struct sockaddr *) &server_addr,sizeof(server_addr) ); // inizializzazione retcode con bind(), ritorna 0 in caso di successo, -1 assieme ad un errno viceversa.
	if(retcode == -1)                                                                     // la bind() associa un indirizzo alla socket creata con socket().L'indirizzo è specificato in server_addr.
		{perror("error binding socket"); exit(-1);}                                       // Dev'essere eseguito prima dell'accept().Negli argomenti passati viene eseguito un casting a server_addr 
                                                                                          // per evitare warning del compilatore causati da diversi tipi di struct sockaddr ( _un , _in)
	
    retcode = listen(server_socket, 1);                                                   // riassegnazione retcode con listen() (risultato uguale a bind()), 
	if(retcode == -1)                                                                     // listen() marca la socket dell'argomento come socket passiva, pronta ad accettare connessioni in arrivo con l'accept().
		{perror("error listening"); exit(-1);}	
	printf("Server ready (CTRL-C quits)\n");


    // ciclo che accetta le connessioni con accept()

	client_addr_len = sizeof(client_addr);                                              // inizializzazione client_addr_len 
	while (!terminated &&                                                               // condizioni per la continuazione del ciclo, variabile terminated impostata a false e 
	       (connect_socket = accept(server_socket,(struct sockaddr *) & client_addr, &client_addr_len)) != -1 ) // e risultato della funzione accept() diversa da -1.
        {                                                                                                       // accept() estrae la prima connessione in coda per il server in ascolto                                            
                                                                                                                // crea una nuova socket connessa e ritorna un file descrittore di questa socket
		// ciclo che processa i dati della connessione accettata

		printf("Server: new connection from client\n");
		while ((retcode = read(connect_socket, buffer, MAXBUF)) > 0)                    // assegnazione variabile retcode con il risultato della read() sulla connect_socket
		                                                                                // la read scrive nel buffer il contenuto della connect_socket, e ritorn il numero di byte letti
			write(fileno(stdout), buffer, retcode);                                     // quindi vengono effettuale letture fin quando al retcode non viene assegnato il valore 0
        
		if (retcode == 0)		                                                        // ad ogni iterazione di read() viene scritto il risultato nel file di output stdout con la write()
			printf("\nClient connection closed\n");                                     // dopo il ciclo se il retcode è uguale a 0 o se il risultato è diverso da 0 si stampa un errore 
        else                                                                            // e si chiude la connect_socket con la close().
            perror("errore su read");
		close(connect_socket);
	}                                                               
                                                                                         // qui si arriva per errore in accept() o terminated==TRUE
	cleanup();                                                                           // alla fine del ciclo richiama la funzione di cleanup()
	return(0);
}

void cleanup()                                                      // la funzione di cleanup() chiude la connect_socket e la server_socket 
{	close(connect_socket); close(server_socket);                    // elimina con la funzione unlink() il pathname dal filesystem,restituendo un errore in caso
	if (unlink(NOMESOCK) < 0)                                       // di esito negativo. 
		{perror("removing socket");}        
	printf("Terminated\n");
	exit(0);
}

#define MOREMSG "need another message to terminate\n"               
void terminazione(int signo)                                        // funzione per il signal handler, per terminare il server da tastiera. Questa funzione viene
{                                                                   // eseguita con il CTRL-C , si assegna alla variabile terminate valore 1 e si richiama la 
	printf("Signal handler started. Terminating ...\n");	        // funzione di cleanup. Si potrebbe non usare la funzione cleanup con un return; che 
	terminated=1;                                                   // riporta il codice al main dove adesso la variabile terminated blocca il ciclo di
                                                                    // accettazione delle connessioni. Sono due modalità alternative per terminiare l'esecuzione
//	printf(MOREMSG); return;                                        // ma con la prima il server ha bisogno di un altro messaggio per terminare, altrimenti
	cleanup();					                                    // rimarrà bloccato in accept(). Richiamando la funzione cleaup non sarà necessario 
}                                                                   // un ulteriore messaggio.

