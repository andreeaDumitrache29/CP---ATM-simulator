#include "lib.h"

#define BUFLEN 100
#define MAX_CLIENTS	10

int main(int argc, char *argv[])
{	/** datele necesare *multiplexarii comunicatiei */
    int sockfd, n;
    struct sockaddr_in serv_addr;
    fd_set read_fds;
    fd_set tmp_fds;	
    int fdmax;
    /** golim multimea de descriptori de citire read_fds si multimea temporara tmp_fds **/
    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);

    /** construim numele fisierului de log curent si il deschidem pentru scriere **/
    char filename[] = "client-";
    int pid = getpid();
	char*aux = calloc(6, sizeof(char));
	if(aux == NULL){
		printf("%s", "Eroare alocare memorie\n");
        return 0; 
	}
	sprintf(aux, "%d", pid);
	strcat(filename, aux);
	strcat(aux, ".log");
	printf("%s\n", filename);
	int fd = open(filename, O_WRONLY | O_CREAT, 0777);
	char* mesaj = calloc(50, sizeof(char));
	if(mesaj == NULL){
        printf("%s", "Eroare alocare memorie\n");
        return 0; 
	}

	/** realizam conxiunea tcp **/
    char buffer[BUFLEN]; 
    /** Deschidere socket pentru conexiunea tcp **/   
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
    	memset(mesaj, 0, 50);
    	strcpy(mesaj, "-10 : Eroare la apel socket");
        printf("%s", mesaj);
        write(fd, mesaj, strlen(mesaj));
        return 0;
    }

    /** Setarea structurii pentru a specifica unde trimit datele **/
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[1], &serv_addr.sin_addr);

   /** cerere de conexiune catre server **/
    if (connect(sockfd,(struct sockaddr*) &serv_addr,sizeof(serv_addr)) < 0) {
    	memset(mesaj, 0, 50);
    	strcpy(mesaj, "-10 : Eroare la apel connect");
        printf("%s", mesaj);
        write(fd, mesaj, strlen(mesaj)); 
        return 0;
    }

    /** setam socket-ii pe care vrem sa le ascultam **/
    FD_SET(sockfd, &read_fds);
    FD_SET(0, &read_fds);
    fdmax = sockfd;
    int i = 0; 

    /** structura pentru conexiunea udp **/
	struct sockaddr_in udp_struct;

	/*Deschidere socket pentru conexiunea udp*/
	int sockfd_udp = socket(PF_INET, SOCK_DGRAM, 0);
	if(sockfd_udp < 0){
		memset(mesaj, 0, 50);
    	strcpy(mesaj, "-10 : Eroare la apel socket");
        printf("%s", mesaj);
        write(fd, mesaj, strlen(mesaj)); 
        return 0;
	}

	/** marcam si socket-ul de tcp ca fiind unul dintre socket-ii pe care vrem sa le ascultam **/
	FD_SET(sockfd_udp, &read_fds);
	
	/*Setare structurii pentru a specifica unde trimit datele*/
	memset(&udp_struct, 0, sizeof(udp_struct));
	udp_struct.sin_family = AF_INET;
	udp_struct.sin_port = htons(atoi(argv[2]));
	struct in_addr s;
	inet_aton(argv[1], &s);
	udp_struct.sin_addr = s;

	/** date folosite **/
    int login = 0;
    int okLogin = 0;
    int okLogout = 0;
    int okSold = 0;
    int okMoney = 0;
    int okPut = 0;
    int okQuit = 0;
    int okUnlock = 0;
    
    /** variabila  nr_card retine numarul de card al userului curent logat
    	variabila parola indica daca avem de primit o parola secreta pentru udp **/
    int nr_card;
    int parola = 0;

    char login_msg[] = "login";
    char login_succes[] = "ATM> Welcome";
    char logout_msg[] = "logout";
    char listsold_msg[] = "listsold";
    char getmoney_msg[] = "getmoney";
    char putmoney_msg[] = "putmoney ";
    char unlock_msg[] = "unlock";
    char quit_msg[] = "quit";
    
    /** bucla pirmire mesaje **/
    while(1){
    	tmp_fds = read_fds;
    	/** apelul select pentru realizarea multiplexarii comunicatiei **/ 
		if (select(fdmax + 1, &tmp_fds, NULL, NULL, NULL) == -1) {
			memset(mesaj, 0, 50);
	    	strcpy(mesaj, "-10 : Eroare la apel select");
	        printf("%s", mesaj);
	        write(fd, mesaj, strlen(mesaj)); 
		}
		/** markeri pentru a sti ce operatie avem de efectuat in cazul primirii unui mesaj de la stdin **/
		okLogin = 0;
		okLogout = 0;
		okSold = 0;
		okMoney = 0;
		okPut = 0;
		okUnlock = 0;
		okQuit = 0;
		/** portul 0 este activ, deci am primit mesaj de la stdin **/
		if(FD_ISSET(0, &tmp_fds)){
			/** citim mesajul primit si il scriem in fisierul de log **/
			memset(buffer, 0 , BUFLEN);
	    	fgets(buffer, BUFLEN-1, stdin);
	    	write(fd, buffer, strlen(buffer));
	    	
	    	okLogin = checkString(login_msg, buffer);
	    	
	    	/** am primit mesaj de login **/
	    	if(okLogin == 0){
	    		/** login = 1 => avem deja un proces de login in desfasurare in procesul curent **/
		    	if(login == 1){
		    		memset(mesaj, 0, 50);
			    	strcpy(mesaj, "-2 : Sesiune deja deschisa\n");
			        printf("%s", mesaj);
			        write(fd, mesaj, strlen(mesaj)); 
		    	}else{
		    		/** extragem numarul de card pentru a fi folosit in caz de unlock **/
		    		char p[7];
		    		for(int j = 0; j <= 6; j++){
		    			p[j] = buffer[5+j];
		    		}
		    		nr_card = atoi(p);
		    		/** trimitem la server informatiile de la client pentru login **/
		    		n = send(sockfd,buffer,strlen(buffer), 0);
			    	if (n < 0) {
			    		memset(mesaj, 0, 50);
				    	strcpy(mesaj, "-10 : Eroare la apel send\n");
				        printf("%s", mesaj);
				        write(fd, mesaj, strlen(mesaj)); 
				        return 0;
			    	}
		    	}
	    	}

	    	/** verificam daca avem de faut logout **/
	    	okLogout = checkString(logout_msg, buffer);
	    	if(okLogout == 0){
	    		/** login = 0 => niciun client nu este autentificat, deci nu putem efectua operatia de logout **/
	    		if(login == 0){
	    			memset(mesaj, 0, 50);
			    	strcpy(mesaj, "-1 : Clientul nu este autentificat\n");
			        printf("%s", mesaj);
			        write(fd, mesaj, strlen(mesaj)); 
	    		}else{
	    			/** trimitem serverului mesajul de logout si facem login = 0 ca sa stim ca nu mai avem niciun client logat **/
	    			n = send(sockfd,buffer,strlen(buffer), 0);
	    			login = 0;
			    	if (n < 0){
			    		memset(mesaj, 0, 50);
				    	strcpy(mesaj, "-10 : Eroare la apel send\n");
				        printf("%s", mesaj);
				        write(fd, mesaj, strlen(mesaj)); 
				        return 0;
			    	}
	    		}
	    	}

	    	/** verificam daca avem de efectuat o operatie de tipul listsold **/
	    	okSold = checkString(listsold_msg, buffer);
	    	if(okSold == 0){
	    		if(login == 0){
	    			/** login = 0 => niciun client nu este autentificat, deci nu putem efectua operatia de listsold **/
	    			memset(mesaj, 0, 50);
			    	strcpy(mesaj, "-1 : Clientul nu este autentificat\n");
			        printf("%s", mesaj);
			        write(fd, mesaj, strlen(mesaj)); 
	    		}else{
	    			/** trimitem serverului mesajul primit**/
	    			n = send(sockfd,buffer,strlen(buffer), 0);
			    	if (n < 0){
			    		memset(mesaj, 0, 50);
				    	strcpy(mesaj, "-10 : Eroare la apel send\n");
				        printf("%s", mesaj);
				        write(fd, mesaj, strlen(mesaj)); 
				        return 0;
			    	}
	    		}
	    	}
	    	/** verificam daca avem de efectuat o operatie de tip getmoney **/
	    	okMoney = checkString(getmoney_msg, buffer);
	    	if(okMoney == 0){
	    		if(login == 0){
	    			/** login = 0 => niciun client nu este autentificat, deci nu putem efectua operatia de getmoney **/
	    			memset(mesaj, 0, 50);
			    	strcpy(mesaj, "-1 : Clientul nu este autentificat\n");
			        printf("%s", mesaj);
			        write(fd, mesaj, strlen(mesaj)); 
	    		}else{
	    			/** trimitem serverului mesajul primit **/
	    			n = send(sockfd,buffer,strlen(buffer), 0);
			    	if (n < 0) {
			    		memset(mesaj, 0, 50);
				    	strcpy(mesaj, "-10 : Eroare la apel send\n");
				        printf("%s", mesaj);
				        write(fd, mesaj, strlen(mesaj)); 
				        return 0;
			    	}
	    		}
	    	}
	    	/** verificam daca avem de efectut o operatie de tip putmoney**/
	    	okPut = checkString(putmoney_msg, buffer);
	    	if(okPut == 0){
	    		if(login == 0){
	    			/** login = 0 => niciun client nu este autentificat, deci nu putem efectua operatia de putmoney **/
	    			memset(mesaj, 0, 50);
			    	strcpy(mesaj, "-1 : Clientul nu este autentificat\n");
			        printf("%s", mesaj);
			        write(fd, mesaj, strlen(mesaj)); 
	    		}else{
	    			/** trimitem serverului informatia primita **/
	    			n = send(sockfd,buffer,strlen(buffer), 0);
			    	if (n < 0){
			    		memset(mesaj, 0, 50);
				    	strcpy(mesaj, "-10 : Eroare la apel send\n");
				        printf("%s", mesaj);
				        write(fd, mesaj, strlen(mesaj)); 
				        return 0;
			    	}
	    		}
	    	}

	    	/** verificam daca avem de efectuat o operatie de quit.
	    		in acest caz nu am mai tinut cont de faptul ca s-ar putea sa nu fie niciun client logat **/
	    	okQuit = checkString(quit_msg, buffer);
	    	if(okQuit == 0){
	    		/** trimitem mesajul de logout serverului si iesim din bucla de primire mesaje **/
    			login = 0;
    			n = send(sockfd,buffer,strlen(buffer), 0);
		    	if (n < 0) {
		    		memset(mesaj, 0, 50);
			    	strcpy(mesaj, "-10 : Eroare la apel send\n");
			        printf("%s", mesaj);
			        write(fd, mesaj, strlen(mesaj)); 
			        return 0;
		    	}
		    	/** scriem mesajul "Clientul se inchide" in fisierul de log pentru a marca operatia de quit **/
		        memset(mesaj, 0, 50);
		        strcpy(mesaj, "Clientul se inchide\n");
		        write(fd, mesaj, strlen(mesaj));
		        break;
	    	}

	    	/** verificam daca avem de efectuat o operatie de unlock **/
	    	okUnlock = checkString(unlock_msg, buffer);
	    	if(okUnlock == 0){
	    		char *aux = calloc(BUFLEN, 1);
	    		if(aux == NULL){
	    			memset(mesaj, 0, 50);
			    	strcpy(mesaj, "Eroare alocare memorie\n");
			        printf("%s", mesaj);
			        write(fd, mesaj, strlen(mesaj));
			        return 0; 
	    		}
	    		/** copiem mesajul primit intr-un sir auxiliar**/
	    		for(i = 0; i < strlen(buffer); i++){
	    			if(buffer[i] == '\n'){
	    				break;
	    			}else{
	    				aux[i] = buffer[i];
	    			}
	    		}
	    		char* auxstr = calloc(7, sizeof(char));
	    		if(auxstr == NULL){
	    			memset(mesaj, 0, 50);
			    	strcpy(mesaj, "Eroare alocare memorie\n");
			        printf("%s", mesaj);
			        write(fd, mesaj, strlen(mesaj));
			        return 0; 
	    		}
	    		/** construim mesajul pentru server care este de forma: numar_card unlock**/
	    		sprintf(auxstr, "%d", nr_card);
	    		strcat(aux, " ");
	    		strcat(aux, auxstr);
	    		/** trimitem mesajul folosind coneixiunea udp **/
				n = sendto(sockfd_udp, aux, BUFLEN, 0, (struct sockaddr*)(&udp_struct), sizeof(udp_struct));
		    	if (n < 0) {
		    		memset(mesaj, 0, 50);
			    	strcpy(mesaj, "-10 : Eroare la apel sendto\n");
			        printf("%s", mesaj);
			        write(fd, mesaj, strlen(mesaj)); 
			        return 0;
			    }
			    memset(buffer, 0 , BUFLEN);
				socklen_t t = sizeof(udp_struct);
				/** asteptam sa primim raspuns de la server **/
				recvfrom(sockfd_udp, buffer, BUFLEN, 0, (struct sockaddr*)(&udp_struct), &t);
				printf("%s", buffer);
				/** daca am primit mesajul de introducere parola secreta, atunci variabila parola devine 1
				pentru a sti la primirea urmatorului mesaj, care nu este o comanda valida, de la stdin ca 
				acesta reprezinta o parola pentru unlock**/
				if(strcmp(buffer, "UNLOCK> Trimite parola secreta\n") == 0){
					parola = 1;
				}else{
					parola = 0;
				}
				/** scriem in fisier mesajul primit si continuam bucla de primire mesaje **/
				write(fd, buffer, strlen(buffer));
				continue;
	    	}
	    	/**parola = 1 => mesajul primit de la stdin este o parola secreta pentru unlock**/
	    	if(parola == 1){
	    		char *auxstr = malloc(BUFLEN);
	    		if(auxstr == NULL){
	    			memset(mesaj, 0, 50);
			    	strcpy(mesaj, "Eroare alocare memorie\n");
			        printf("%s", mesaj);
			        write(fd, mesaj, strlen(mesaj));
			        return 0; 
	    		}
	    		/** salvam mesjaul primit intr-un sir auxiliar **/
	    		for(i = 0; i < strlen(buffer); i++){
	    			if(buffer[i] == '\n'){
	    				break;
	    			}else{
	    				auxstr[i] = buffer[i];
	    			}
	    		}
	    		char *aux = calloc(BUFLEN, 1);
	    		if(aux == NULL){
	    			memset(mesaj, 0, 50);
			    	strcpy(mesaj, "Eroare alocare memorie\n");
			        printf("%s", mesaj);
			        write(fd, mesaj, strlen(mesaj));
			        return 0; 
	    		}
	    		/** construim mesajul de trimis serverului: numar_card parola_secreta **/
	    		sprintf(aux, "%d", nr_card);
	    		strcat(aux, " ");
	    		strcat(aux, auxstr);
	    		/** trimtiem mesajul folosind conexiunea udp **/
	    		n = sendto(sockfd_udp, aux, BUFLEN, 0, (struct sockaddr*)(&udp_struct), sizeof(udp_struct));
		    	if (n < 0) {
		    		memset(mesaj, 0, 50);
			    	strcpy(mesaj, "-10 : Eroare la apel sendto\n");
			        printf("%s", mesaj);
			        write(fd, mesaj, strlen(mesaj));
			        return 0; 
		    	}
			    memset(buffer, 0 , BUFLEN);
				socklen_t t;
				/** asteptam raspunsul de la server **/
				recvfrom(sockfd_udp, buffer, BUFLEN, 0, (struct sockaddr*)(&udp_struct), &t);
				parola = 0;
				/** afisam mesajul primit **/
				printf("%s", buffer);
				write(fd, buffer, strlen(buffer));
	    	}
	    	
		}
		else{
			/** primim un mesaj de la server **/
			memset(buffer, 0 , BUFLEN);
			recv(sockfd, buffer, BUFLEN, 0);
			printf("%s", buffer);
			write(fd, buffer, strlen(buffer));

			/** verificam daca serverul a trimis un mesaj de quit, caz in care se inchide si clientul curent **/
			if(strcmp(buffer, "quit\n") == 0){
				memset(mesaj, 0, 50);
		    	strcpy(mesaj, "Serverul se inchide\n");
		        printf("%s", mesaj);
		        write(fd, mesaj, strlen(mesaj)); 
		        login = 0;
	    		break;
	    	}

			/** verificam daca s-a facut login cu succes **/
			for(i = 0; i < strlen(login_succes); i++){
	    		if(buffer[i] != login_succes[i]){
	    			break;
	    		}
	    	}

	    	/** daca s-a facult login cu succes retinem acest lucru folosind variabila login
	    		pentru a nu mai permite alte logari **/
	    	if(i == strlen(login_succes)){
	    		login = 1;
	    	}	    	
		}
    }
    close(sockfd);
    close(sockfd_udp);
    close(fd);
    return 0;
}
