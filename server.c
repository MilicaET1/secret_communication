#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define LEN 10

int key;
//sifrovanje i desifrovanje koriscenjem cezarove enkripcije
void cezar(int key, char *buffer)
{
	int duzina = strlen(buffer);
	for(int i=0; i<duzina; i++)
	{
		buffer[i] = buffer[i]+key;
		if(buffer[i]>'Z')
			buffer[i] = buffer[i]-'Z'+'A'-1;
	}
}
void desifrovanje(int key, char *buffer)
{
	int duzina = strlen(buffer);
	for(int i=0; i<duzina; i++)
	{
		buffer[i] = buffer[i]-key;
		if(buffer[i]<'A')
			buffer[i] = buffer[i]+'Z'-'A'+1;
	}
}


struct Agent
{

	char ime[100];
	char prezime[100];
	char alterego[100];
	char lokacija[100];

};
struct Agent baza[LEN] = {
{"DUSAN", "POPOV", "JAMESbOND", "SRBIJA"},
{"JOVANA", "JOVANOVIC", "SKALI", "ITALIJA"},
{"MILOS", "MILSOEVIC", "IZZY", "NORVESKA"},
{"ZIKA", "ZIKIC","BOSKOJAKOVLJEVIC","SRBIJA"},
{"JOVA", "JOVIC", "DESISTI", "BIH"},
{"NIKOLINA", "NIKIC", "WONDERWOMAN", "SAD"},
{"BATA", "ZIVOINOVIC", "BETMEN", "TURSKA"},
{"BELA", "VAJDA", "ROBIN", "KINA"},
{"TOM", "TOM", "SUPERMAN", "KOLORADO"},
{"ARON", "VAJDA", "SPIDERMAN", "LA"}
};

int nadji(char *alterego)
{
	for (int i = 0; i < LEN; i++)
	{
		if(strcmp(alterego, baza[i].alterego)==0) 
			return i;
	}
	return -1;
}



/*ovo je jezgro serverske funkcionalnosti-ova funkcija se poziva
nakon uspostavljanja veze sa klijentom kako bi klijentu bilo
omoguceno da komunicira sa serverom. */
void doprocessing (int sock)
{

	int m;
	char buffer2[256];
	buffer2[255]=0;
	m=read(sock, buffer2, 255);
	buffer2[m]=0;
	printf("%s\n", buffer2);
	if(strcmp(buffer2, "NEEDINFO") != 0)
	{
		printf("ERROR!!!\n");
		exit(1);
	}
	write(sock, "YOUCANGETINFO", strlen("YOUCANGETINFO"));

	char buffer[256];
	buffer[255]= 0;
	int n;

	while (1)
	{
		char spojnica[300];
		n =read(sock,buffer, 255);
		buffer[n]=0;
		if(strcmp(buffer, "ENDE")== 0)
		{
			printf("ENDE");
			exit(1);
		}

		if(n==0)
		{
			printf("ERROR!!!\n");
			exit(1);
		}
		printf("%s\n", buffer);
		desifrovanje(key, buffer);
		printf("%s\n", buffer);
		int i = nadji(buffer);

		if(i != -1)
			sprintf(spojnica,"%s%s%s",baza[i].ime, baza[i].prezime, baza[i].lokacija);
		else
			sprintf(spojnica,"error\n");
		cezar(key, spojnica);

		write(sock, spojnica, strlen(spojnica));
	}
}



/* glavni program serverske aplikacije */
int main( int argc, char *argv[] )
{
	int sockfd, newsockfd, portno, clilen;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	int  n;

	if(argc != 2)
	{
		printf("\n Usage: %s <key>\n", argv[0]);
		return 1;
	}
	key = atoi(argv[1]);
	/* najpre se poziva uvek socket() funkcija da se registruje socket:
	AF_INET je neophodan kada se zahteva komunikacija bilo koja
	dva host-a na Internetu;
	Drugi argument definise tip socket-a i moze biti SOCK_STREAM ili SOCK_DGRAM:
	SOCK_STREAM odgovara npr. TCP komunikaciji, dok SOCK_DGRAM kreira npr. UDP kanal
	Treci argument je zapravo protokol koji se koristi: najcesce se stavlja 0 sto znaci da
	OS sam odabere podrazumevane protokole za dati tip socket-a (TCP za SOCK_STREAM
	ili UDP za SOCK_DGRAM)  */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		perror("ERROR opening socket");
		exit(1);
	}

	/* Inicijalizacija strukture socket-a */
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = 5001;
	serv_addr.sin_family = AF_INET; //mora biti AF_INET
	/* ip adresa host-a. INADDR_ANY vraca ip adresu masine na kojoj se startovao server */
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	/* broj porta-ne sme se staviti kao broj vec se mora konvertovati u
	tzv. network byte order funkcijom htons*/
	serv_addr.sin_port = htons(portno);



	/* Sada bind-ujemo adresu sa prethodno kreiranim socket-om */
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("ERROR on binding");
		exit(1);
	}
	printf("Server started.. waiting for clients ...\n");

	/* postavi prethodno kreirani socket kao pasivan socket
	koji ce prihvatati zahteve za konekcijom od klijenata
	koriscenjem accept funkcije */
	listen(sockfd,5); //maksimalno 5 klijenata moze da koristi moje usluge
	clilen = sizeof(cli_addr);

	while (1)
	{
		/*ovde ce cekati sve dok ne stigne zahtev za konekcijom od prvog klijenta*/
		newsockfd = accept(sockfd,
				(struct sockaddr *) &cli_addr, &clilen);
		printf("Client connected...\n");

		if (newsockfd < 0)
		{
			perror("ERROR on accept");
			exit(1);
		}

		/* Kreiraj child proces sa ciljem da mozes istovremeno da
		komuniciras sa vise klijenata */
		int  pid = fork();
		if (pid < 0)
		{
			perror("ERROR on fork");
		exit(1);
		}
		if (pid == 0)  
		{
			/* child proces ima pid 0 te tako mozemo znati da li
		se ovaj deo koda izvrsava u child ili parent procesu */
			close(sockfd);
			doprocessing(newsockfd);
			exit(0);
		}
		else
		{
		/*ovo je parent proces koji je samo zaduzen da
		delegira poslove child procesima-stoga ne moras
		da radis nista vec samo nastavi da osluskujes
		nove klijente koji salju zahtev za konekcijom*/		
			close(newsockfd);
		}
	} /* end of while */
}



