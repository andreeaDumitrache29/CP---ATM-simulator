#include "lib.h"

#define BUFLEN 100
#define MAX_CLIENTS	10

/** functia intoarce 1 daca exista deja un client cu numarul de card dat
 	logat in momentul acesta si 0 in caz contrar.
 	am presupus ca un singur user poate fi logat la un moment dat pe un socket (intr-un proces) **/
int checkOpenSession(user* vect, int nr_card, int N){
	for(int i = 0; i < N; i++){
		if(vect[i].socket != 0 && vect[i].nr_card == nr_card){
			return 1;
		}
	}
	return 0;
}
/** functia intoarce userul logat pe socketul primit ca parametru **/
user getClient(user* vect, int socket, int N){
	user u;
	for(int i = 0; i < N; i++){
		if(vect[i].socket == socket){
			u = vect[i];
			break;
		}
	}
	return u;
}
/** functia intoarce userul logat cu numarul de card primit ca parametru **/
user getCl(user* vect, int nr_card, int N){
	user u;
	for(int i = 0; i < N; i++){
		if(vect[i].nr_card == nr_card){
			u = vect[i];
			break;
		}
	}
	return u;
}

int main(int argc, char *argv[]){
	/** deschidem fisierul de unde vom lua informatiile despre utilizatori **/
	char* filename = malloc(strlen(argv[2]));
	strcpy(filename, argv[2]);
	int N = 0; char buf[BUFLEN];
	FILE* file = fopen(filename, "r");
	if(!file){
		printf("Eroare la deschiderea fisierului\n");
		return 0;
	}
	/** citim numarul de useri **/
	fgets(buf, BUFLEN, file);
	N = atoi(buf);
	user u;
	/** vector de structuri de tip user in care vom retine fiecare user **/
	user* client_vect = malloc(N * sizeof(user));
	if(!client_vect){
		printf("Eroare alocare memorie\n");
		return 0;
	}
	/** citim informatiile pentru fiecare user si il introducem in vectorul de useri **/
	for(int i = 0; i < N; i++){
		memset(buf, 0, BUFLEN);
		memset(&u, 0, sizeof(u));	
		fscanf(file, "%s", u.nume);
		fscanf(file, "%s", u.prenume);
		fscanf(file, "%d", &u.nr_card);
		fscanf(file, "%d", &u.pin);
		fscanf(file, "%s", u.parola_secreta);
		fscanf(file, "%lf", &u.sold);
		u.blocat = 0;
		u.socket = 0;
		client_vect[i] = u;
	}
	/** date pentru conexiunea tcp **/
	int sockfd, new_sockfd, port;
	unsigned int cli;
    char buffer[BUFLEN];
    struct sockaddr_in server, client;
    int n, i;

    /** golim multimea de descriptori de citire read_fds si multimea temporara tmp_fds **/
    fd_set read_fds;	
    fd_set tmp_fds;	
    int fdmax;		
    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);
     
    /** Deschidere socket pentru conexiunea tcp **/
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
     	printf("Eroare socket\n");
     	return 0;
    }
     
    port = atoi(argv[1]);
    /** Setarea campurilor structurii **/
    memset((char *) &server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);
     
    /** asignam adresa specificata de strctura socket-ului de tcp **/
    if (bind(sockfd, (struct sockaddr *) &server, sizeof(struct sockaddr)) < 0) {
        printf("Eroare bind\n");
        return 0;
    }

    struct sockaddr_in udp_struct, client_struct;
	
	/** Deschidere socket **/
	int sockudp = socket(PF_INET, SOCK_DGRAM, 0);
	
	/**Setare structura sockaddr_in pentru a asculta pe portul respectiv **/
	memset(&udp_struct, 0, sizeof(udp_struct));
	udp_struct.sin_family = AF_INET;
	udp_struct.sin_port = htons(atoi(argv[1]));

	/** Legare proprietati de socket **/
	if (bind(sockudp, (struct sockaddr*)(&udp_struct), sizeof(udp_struct)) < 0){
		printf("eroare bind udp\n");
		return 0;
	}
	 socklen_t t;
	 /** setam porturile cu care vom comunica **/
     listen(sockfd, MAX_CLIENTS);
     FD_SET(sockfd, &read_fds);
     FD_SET(sockudp, &read_fds);
     FD_SET(0, &read_fds);
     fdmax = sockudp;

     /** date folosite **/
     char login_msg[] = "login";
     char logout_msg[] = "logout";
     char listsold_msg[] = "listsold";
     char getmoney_msg[] = "getmoney";
     char putmoney_msg[] = "putmoney ";
     char unlock_msg[] = "unlock";
     char quit_msg[] = "quit";

     int sock;
     int okSold = 0;
     int okLogout = 0;
     int okMoney = 0;
     int okPut = 0;
     int okUnlock = 0;
     int nr_card = 0;
     int okQuit = 0;

     /** bucla pirmire mesaje **/
     while (1) {
		tmp_fds = read_fds; 
		/** realizarea multiplexarii **/
		if (select(fdmax + 1, &tmp_fds, NULL, NULL, NULL) == -1) {
			printf("Eroare select\n");
			return 0;
		}
		
		/** parcurgem lista tutror socket-ilor **/
		for(sock = 0; sock <= fdmax; sock++) {
			if (FD_ISSET(sock, &tmp_fds)) {
				/** am primit un mesaj pe un socket inactiv, deci facem accept**/
				if (sock == sockfd) {
					cli = sizeof(client);
					if ((new_sockfd = accept(sockfd, (struct sockaddr *)&client, &cli)) == -1) {
						printf("Eroare accept\n");
						return 0;
					} 
					else {
						/** adaug noul socket intors de accept() la multimea socket-ilor de la care astept mesaje **/
						FD_SET(new_sockfd, &read_fds);
						if (new_sockfd > fdmax) { 
							fdmax = new_sockfd;
						}
					}
					printf("Noua conexiune de la %s, port %d, socket_client %d\n ", inet_ntoa(client.sin_addr), ntohs(client.sin_port), new_sockfd);
				}
				/** am primit un mesaj de la stdin **/
				else if(sock == 0){
					/** am presupus ca mesajul primit nu poate fi decat cel de quit**/
					memset(buffer, 0, BUFLEN);
					fgets(buffer, BUFLEN-1, stdin);
					/** trimitem mesajul tuturor clientilor pentru a-i anunta ca server-ul se va inchide **/
					for(int j = 5; j <= fdmax; j++){
						send(j, buffer, BUFLEN, 0);
					}
					/** inchidem socket-ii si fisierul si apoi incheiem executia programului **/
					close(sockudp);
					close(sockfd);
					fclose(file);
					return 0;
				}
				/** am primit un mesaj pe socketul de udp **/
				else if(sock == sockudp){
					/** apelam recvfrom pentru a primi mesajul **/
					t = sizeof(client_struct);
					recvfrom(sockudp, buffer, BUFLEN, 0, (struct sockaddr*)(&client_struct), &t);
					printf("server primeste: %s\n", buffer);
					char* raspuns = calloc(BUFLEN, sizeof(char));

					/** verificam daca am primit o comanda de unlock **/
					okUnlock = checkString(unlock_msg, buffer);
					if(okUnlock == 0){
						/** daca da, extragem numarul de card si verificam daca cardul respectiv exista si daca era intr-adevar blocat **/
						nr_card = atoi(buffer + 6);
						printf("card care solicita unlock: %d\n", nr_card);
						strcpy(raspuns, "UNLOCK> Trimite parola secreta\n");
						int i = 0;
						for(i = 0; i < N; i++){
							if(nr_card == client_vect[i].nr_card){
								/** cardul nu era blocat => operatia de unlock nu are sens **/
								if(client_vect[i].blocat < 3){
									strcpy(raspuns, "UNLOCK> -6 : Operatie esuata\n");
								}
								break;
							}
						}
						/** am parcurs tot vectorul de useri fara a gasi numarul de card primit, deci acesta nu exista **/
						if(i == N){
							strcpy(raspuns, "UNLOCK> -4 : Numar card inexistent\n");
						}
					}
					else{
						/** nu am primit o comanda de unlock, deci mesajul reprezinta o parola **/
						char *parola_secreta = calloc(strlen(raspuns) - 6, 1);
						/** extragem parola si numarul de card **/
						parola_secreta = buffer + 7;
						char p[6];
						for(int j = 0; j < strlen(buffer); j++){
							p[j] = buffer[j];
						}
						nr_card = atoi(p);
						/** parcurgem vectorul de useri si il cautam pe cel cu numarul de card primit**/
						for(i = 0; i < N; i++){
							if(nr_card == client_vect[i].nr_card){
								printf("parola corecta: %s parola primita: %s\n", client_vect[i].parola_secreta, parola_secreta);
								/** verificam daca parola primita coincide cu cea din fisierul de useri **/
								if(strcmp(client_vect[i].parola_secreta, parola_secreta) == 0){
									/** daca am primit o parola corecta deblocam cardul respectiv **/
									strcpy(raspuns, "UNLOCK> Client deblocat\n");
									client_vect[i].blocat = 0;
								}else{
									/** daca nu, deblocarea esueaza **/
									strcpy(raspuns, "UNLOCK> -7 : Deblocare esuata\n");
								}
							}
						}
					}
					/** trimitem inapoi clientului raspunsul corespunzator **/
					sendto(sockudp, raspuns, BUFLEN, 0, (struct sockaddr*)(&client_struct), t);
				}
					
				else {
					/** am primit date pe unul din socket-ii cu care vorbesc cu clientii **/
					memset(buffer, 0, BUFLEN);
					/** facem receive pentru a vedea mesajul primit **/
					if ((n = recv(sock, buffer, sizeof(buffer), 0)) <= 0) {
						if (n == 0) {
							/** conexiunea s-a inchis daca recv intoarce 0 **/
							printf("server: socket %d hung up\n", sock);
						} else {
							printf("Eroare recv\n");
							return 0;
						}
						/** daca a avut loc o eroare la receive, atunci eliminam socket-ul curent din multimea
						de socketi pe care ii ascultam **/
						close(sock); 
						FD_CLR(sock, &read_fds); 
					} 
					
					else {
						/** am primit cu succes un mesaj **/
						/**resetam markerii pentru a verifica ce mesaj am primit **/
						int okLogin = 0;
						int pin, nr_card;
						okLogout = 0;
						okSold = 0;
						okMoney = 0;
						okPut = 0;
						okQuit = 0;
						char raspuns[BUFLEN];
				    	memset(raspuns, 0, BUFLEN);
						printf ("Am primit de la clientul de pe socketul %d, mesajul: %s\n", sock, buffer);
						
						/** verificam daca am primit comanda de login **/
				    	okLogin = checkString(login_msg, buffer);
				    	if(okLogin == 0){
				    		/** extragem pin-ul si numarul de card primite cu comanda login **/
				    		char p[7];
				    		int ok = 0;
				    		for(int j = 0; j <= 6; j++){
				    			p[j] = buffer[5+j];
				    		}
				    		nr_card = atoi(p);
				    		pin = atoi(buffer + 12);
				    		/** verificam daca userul cu numarul de card respectiv este deja logat **/
				    		if(checkOpenSession(client_vect, nr_card, N) == 1){
		    					strcpy(raspuns, "ATM> -2 : Sesiune deja deschisa\n");
		    				}else{
		    					/** parcurgem vectorul de useri pentru a -l gasi pe cel cu numarul de card primit **/
		    					for(i = 0; i < N; i++){
		    						/** daca am gasit respectiv user verificam daca am primit un pin corect **/
					    			if(client_vect[i].nr_card == nr_card){
					    				ok = 1;
					    				if(client_vect[i].pin == pin){ 
					    					/** daca da, construim reply-ul catre client **/
					    					strcpy(raspuns, "ATM> Welcome ");
					    					strcat(raspuns, client_vect[i].nume);
					    					strcat(raspuns, " ");
					    					strcat(raspuns, client_vect[i].prenume);
					    					strcat(raspuns, "\n");
					    					/** retinem socket - ul pe care este logat user-ul si resetam numarul de incercari
					    					 	esuate  de login la 0 pentru userul acesta**/
					    					client_vect[i].socket = sock;
					    					client_vect[i].blocat = 0;
					    					break;
					    				}else{
					    					/** am primit un pin gresit, deci trimitem inapoi clientului un mesaj 
					    					corespunzator **/
					    					strcpy(raspuns, "ATM> -3 : Pin gresit\n");
					    					client_vect[i].blocat ++;
					    					break;
					    				}
					    			}
				    			}
		    				}

				    		/** daca am parcurs tot vectorul de useri fara a gasi numarul
				    			de card primit atunci acesta nu exista **/
				    		if(i == N){
				    			strcpy(raspuns, "ATM> -4 : Numar card inexistent\n");
				    		}
				    		/** verificam daca numarul de incercari esuate login pentru numarul de card primit
				    		    a ajuns sau depaseste 3, caz in care cardul este blocat si clientul trebuie sa
				    		    fie notificat in mod corespunzator **/
				    		if(ok == 1 && getCl(client_vect, nr_card, N).blocat >= 3){
				    			strcpy(raspuns, "ATM> -5 : Card blocat\n");
				    		}
				    		/** trimitem raspunsul clientului **/
				    		send(sock, raspuns, BUFLEN, 0);
				    	}

				    	/** verificam daca am primit comanda de logout **/
				    	okLogout = checkString(logout_msg, buffer);
				    	if(okLogout == 0){
				    		/** verificam daca cardul nu este blocat **/
				    		if(getClient(client_vect, sock, N).blocat <= 3){
				    			strcpy(raspuns, "ATM> Deconectare de la bancomat\n");
				    			/** cautam userul logat in procesul curent **/
				    			for(int j = 0; j < N; j++){
									if(client_vect[j].socket == sock){
										/** resetam socket-ul pe care era logat userul **/
										client_vect[j].socket = 0;;
										break;
									}
								}
				    		}
				    		else{
				    			strcpy(raspuns, "ATM> -5 : Card blocat\n");
				    		}
				    		/** trimitem raspunsul clientului **/
				    		send(sock, raspuns, BUFLEN, 0);
				    	}

				    	/** verificam daca avem de efectuat o operatie de tip listsold **/
				    	okSold = checkString(listsold_msg, buffer);
				    	if(okSold == 0){
				    		/** verificam daca userul de pe socket-ul curent este blocat **/
				    		if(getClient(client_vect, sock, N).blocat < 3){
				    			/** daca nu, extragem sold-ul acestuia si construim reply-ul corespunzator pentru client **/
				    			memset(raspuns, 0, BUFLEN);
			    				strcpy(raspuns, "ATM> ");
			    				sprintf(buffer, "%.2f\n", getClient(client_vect, sock, N).sold);
			    				strcat(raspuns, buffer);
			    			}else{
				    			strcpy(raspuns, "ATM> -5 : Card blocat\n");
				    		}
				    		/** trimitem raspunsul clientului **/
				    		send(sock, raspuns, BUFLEN, 0);
				    	}

				    	/** verificam daca avem de efectuat o operatie de tipul getmoney **/
				    	okMoney = checkString(getmoney_msg, buffer);
				    	if(okMoney == 0){
				    		/** verificam daca userul de pe socket-ul curent este blocat**/
				    		if(getClient(client_vect, sock, N).blocat < 3){
				    			/** daca nu, extragem suma trimisa de catre client **/
				    			memset(raspuns, 0, BUFLEN);
				    			int sum = atoi(buffer + 8);
				    			/** verificam ca suma sa fie multiplu de 10**/
				    			if(sum % 10 != 0){
				    				strcpy(raspuns, "ATM> -9 : Suma nu este multiplu de 10\n");
				    			}else{
				    				/** daca suma este multilpu de 10 verificam caaceasta sa fie mai mica sau egala
				    					cu sold-ul disponibil al user-ului **/
				    				if(sum >= getClient(client_vect, sock, n).sold){
				    					strcpy(raspuns, "ATM> -8 : Fonduri insuficiente\n");	
				    				}else{
				    					/** daca conditiile sunt indeplinite construim reply-ul corespunzator pentru client **/
				    					strcpy(raspuns, "ATM> Suma ");
				    					sprintf(buffer, "%d", sum);
				    					strcat(raspuns, buffer);
				    					strcat(raspuns, " retrasa cu succes\n");
				    					/** cautam userul logat pe socketul curent **/
				    					for(int j = 0; j < N; j++){
				    						if(client_vect[j].socket == sock){
				    							/** suma extrasa este scazuta din sold **/
				    							client_vect[j].sold -= sum;
				    							break;
				    						}
				    					}
				    				}
				    			}
				    			
				    		}else{
				    			strcpy(raspuns, "ATM> -5 : Card blocat\n");
				    		}
				    		/** trimitem raspunsul clientului **/
				    		send(sock, raspuns, BUFLEN, 0);
				    	}

				    	/** verificam daca avem de efectuat o operatie de tipul putmoney **/
				    	okPut = checkString(putmoney_msg, buffer);
				    	if(okPut == 0){
				    		/** verificam daca userul logat pe socketul curent este blocat **/
				    		if(getClient(client_vect, sock, N).blocat < 3){
				    			/** daca nu, extragem suma primita ca un doubl cu 2 zecimale **/
				    			memset(raspuns, 0, BUFLEN);
				    			double sum = atof(buffer + 8);
				    			/** construim reply-ul pentru client **/	
		    					strcpy(raspuns, "ATM> Suma ");
		    					sprintf(buffer, "%.2f", sum);
		    					strcat(raspuns, buffer);
		    					strcat(raspuns, " depusa cu succes\n");
		    					/** cautam clientul logat pe socket-ul curent si adunam suma primita la sold-ul acestuia**/
		    					for(int j = 0; j < N; j++){
		    						if(client_vect[j].socket == sock){
		    							client_vect[j].sold += sum;
		    							break;
		    						}
		    					}
				    		}else{
				    			strcpy(raspuns, "ATM> -5 : Card blocat\n");
				    		}
				    		/** trimitem raspuns clientului **/
				    		send(sock, raspuns, BUFLEN, 0);
				    	}

				    	/** verificam daca am primit un mesaj de quit **/
				    	okQuit = checkString(quit_msg, buffer);
				    	if(okQuit == 0){
				    		/** cautam userul logat pe socket-ul curent **/
				    		for(int j = 0; j < N; j++){
    						if(client_vect[j].socket == sock){
    							/** resetam socket-ul pentru user-ul respectiv **/
    							client_vect[j].socket = 0;
    							break;
    						}
    					}	/**	scoatm socketul care urmeaza sa se inchida din lista de socketi cu care comunicam **/
				    		FD_CLR(sock, &read_fds);
				    		break;
				    	}
					}
				} 
			}
		}
     }
}