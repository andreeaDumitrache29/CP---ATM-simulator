#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <netdb.h>
#include <fcntl.h>

/** structura care retine informatiile despre un client: nume, prenume, numarul de card, 
	pin-ul, parola pentru unlock, sold-ul, numarul de incercari de logare esuate si socket-ul pe care este logat **/
typedef struct 
{   char nume[12];
	char prenume[12];
	int nr_card;
	int pin;
	char parola_secreta[16];
	double sold;
	int blocat;
	int socket;
}user;

/** functie care intoarce 0 daca primele n caractere din sirul 1 corespund cu sirul al doilea, 
	unde n = numarul de caractere din sirul 2 **/
int checkString(char* string, char*buffer){
	for(int i = 0; i < strlen(string); i++){
		if(buffer[i] != string[i]){
			return 1;
		}
	}
	return 0;
}
/** functie care afiseaza un client **/
void showUser(user u){
	printf("%s %s %d %d %s %lf %d %d\n", u.nume, u.prenume, u.nr_card, u.pin, u.parola_secreta, u.sold, u.blocat, u.socket);
}