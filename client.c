#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

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


void ispis(int sockfd)
{
	char a[100];
	char spojnica[300];

	printf("Unesi alterego agenta za kojeg zelis vise informacija: \n");
	scanf("%s", a);
	if(strcmp(a, "ENDE")==0)
	{
		write(sockfd, a, strlen(a));
		printf("ENDE\n");
		exit(1);
	}
	cezar(key, a);

	write(sockfd, a, strlen(a));

	int n = read(sockfd, spojnica, 300);
	spojnica[n]=0;

	printf("Sifrovano: %s\n", spojnica);
	desifrovanje(key, spojnica);
	printf("Desifrovano: %s\n", spojnica);
}


void uparivanje(int sockfd)
{
	char serverski[300];
	printf("NEEDINFO\n");
	write(sockfd,"NEEDINFO",strlen("NEEDINFO"));
	int m = read(sockfd, serverski, 300);
	serverski[m]=0;
	if(strcmp(serverski, "YOUCANGETINFO")== 0)
	{
	
		printf("%s\n", serverski);
		while(1)
		{
			ispis(sockfd);
		}
	}
	else
		printf("ERROR!!!\n");

}




int main(int argc, char *argv[])
{
    int sockfd = 0;
    struct sockaddr_in serv_addr;
    char * line = NULL;
    size_t len = 0;
    int nread;
    /* klijentska aplikacija se poziva sa ./ime_aplikacija ip_adresa_servera key */	
    if(argc != 3)
    {
        printf("\n Usage: %s <ip of server> <key>\n",argv[0]);
        return 1;
    }
	key = atoi(argv[2]);

    /* kreiraj socket za komunikaciju sa serverom */
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
       perror("\n Error : Could not create socket \n");
       return 1;
    }
    memset(&serv_addr, 0, sizeof(serv_addr));
   
    /* kao i podatke neophodne za komunikaciju sa serverom */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5001);

    /* inet_pton konvertuje ip adresu iz stringa u format
	neophodan za serv_addr strukturu */	
    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return 1;
    }

    /* povezi se sa serverom definisanim preko ip adrese i porta */
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\n Error : Connect Failed \n");
        return 1;
    }

    printf("Connected to server... Send message to server, or type 'quit' to exit\n");

    /* udji u petlju u kojoj ces slati poruke server sve dok ne posaljes “quit” */
	uparivanje(sockfd);

    close(sockfd);
    return 0;
}
