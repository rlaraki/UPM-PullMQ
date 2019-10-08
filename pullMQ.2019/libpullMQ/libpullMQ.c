#include "comun.h"
//#include "pullMQ.h"
#include "stdlib.h"
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h> 
#include <stdbool.h>
#include <stdio.h>   
#include <stdlib.h>
#include <string.h>
#include <unistd.h>    
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>


#include "pullMQ.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>		
#include <string.h>		
#include <sys/socket.h> 
#include <arpa/inet.h>  
#include <netdb.h>
#include <stdio.h>
#include "../libpullMQ/pullMQ.h"

#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <netdb.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
 
 #define MAX_NAME 2900
 
struct sockaddr_in 
{ 
   short int sin_family;        //Protocolo para la conexión
   unsigned short int sin_port; //Puerto para la conexión
   struct in_addr sin_addr;     //Estructura a la dirección IP
   unsigned char sin_zero[8];   //Relleno
};

struct in_addr
{
   unsigned   long s_addr; //Contiene la dirección IP del host
};

struct sockaddr 
{ 
   unsigned short sa_family;  //Protocolo
   char sa_data[14];          //Dirección IP del host  
};
int createMQ(const char *cola) {
	
 writerq(cola, CREATE, NULL,0,NULL,NULL, 0);
}
int destroyMQ(const char *cola){ 
	writerq(cola, DESTROY, NULL,0,NULL,NULL, 0);
}
int put(const char *cola, const void *mensaje, size_t tam){
	writerq(cola, PUT, mensaje,0,NULL,NULL, 0);
}
int get(const char *cola, void **mensaje, size_t *tam, int blocking){
	writerq(cola, CREATE, NULL,0,mensaje, tam, blocking);
}

int writerq(const char* cola, const int cop,const void *mensajeput, 
						size_t tamput,void **mensajeget, size_t *tamget , int blocking ){
	size_t lncola = strlen(cola);
	
	Staticrequest r = {cop, lncola , tamput, blocking};
	int s = get_socket();
	if(send(s,&r, sizeof(r), 0)<0){
		return -1;
	}		
	if(send(s,cola, lncola, 0)<0){
		return -1;
	}	
	if(cop == PUT){
		if(send(s, mensajeput, tamput, 0)<0){
			return -1;
		}
	}

	if(cop == GET){
		if(send(s, mensajeget, *tamget,0)<0) {
			return -1;
		}
		if(send(s, tamget, sizeof(size_t), 0)<0){
			return -1;
		}
	}
	size_t reply_len = 0;
	uint32_t reply_len32 = 0;
	if (recv(s, &reply_len32, sizeof(size_t), MSG_WAITALL) < 0)
	{
		return -1;
	}

	if ((reply_len = ntohl(reply_len32)) < 0)
	{
		return 0;
	}

	char *reply = malloc(reply_len);
	if (recv(s, reply, reply_len, MSG_WAITALL) < 0)
	{
		return -1;
	}

	free(reply);
close(s);

}

int get_socket(){
int s, leido;
	struct sockaddr_in dir;
	struct hostent *host_info;
	
	if ((s=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("error creando socket");
		return 1;
	}

	struct hostent *he;
	char *host = getenv("BROKER_HOST");
	int port = atoi(getenv("BROKER_PORT"));

	host_info=gethostbyname(atoi(host));
	memcpy(&dir.sin_addr.s_addr, host_info->h_addr, host_info->h_length);
	dir.sin_port=htons(port);
	dir.sin_family=PF_INET;
	if (connect(s, (struct sockaddr *)&dir, sizeof(dir)) < 0) {
		perror("error en connect");
		close(s);
		return 1;
	}
		
		return s;
}


