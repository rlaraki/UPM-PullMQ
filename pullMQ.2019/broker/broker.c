


#include "comun.h"
#include <stdio.h>
//#include "../libpullMQ/pullMQ.h"

#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdbool.h>
#include <unistd.h>    
#include <netinet/in.h>

#define MAX_NAME 2900 




/*struct in_addr
{
   unsigned   long s_addr; //Contiene la dirección IP del host
};*/

/*struct sockaddr 
{ 
   unsigned short sa_family;  //Protocolo
   char sa_data[14];          //Dirección IP del host  
};*/


Queues allqueues;
int allqueues_empty = 1;


int main(int argc, char *argv[]){
    if(argc!=2) {
        fprintf(stderr, "Uso: %s puerto\n", argv[0]);
        return 1;
    }
    
    int s, s_conec, leido;
	unsigned int tam_dir;
	struct sockaddr_in dir, dir_cliente;
	//char buf[TAM]; pas de buffer.
	int opcion=1;
	
	if ((s=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("error creando socket");
		return 1;
	}
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opcion, sizeof(opcion))<0){
                perror("error en setsockopt");
                return 1;
        }
    dir.sin_addr.s_addr=INADDR_ANY;
	dir.sin_port=htons(atoi(argv[1]));
	dir.sin_family=PF_INET;
	if (bind(s, (struct sockaddr *)&dir, sizeof(dir)) < 0) {
		perror("error en bind");
		close(s);
		return 1;
	}
	if (listen(s, 5) < 0) {
		perror("error en listen");
		close(s);
		return 1;
	}
	
	while (1) {
		tam_dir=sizeof(dir_cliente);
		if ((s_conec=accept(s, (struct sockaddr *)&dir_cliente, &tam_dir))<0){
			perror("error en accept");
			close(s);
			return 1;
		}

	Staticrequest sr;
	if(recv(s_conec,&sr,sizeof(Staticrequest), 0) <0){
		return -1;
	}
	size_t lncola = sr.queue_name_ln;
	char* queue_name = malloc(lncola);
	if(recv(s_conec,queue_name,lncola, 0) <0){
		return -1;
	}

	void*msg;
	size_t msg_ln;


	int processing = -1;


		
	switch(sr.cop){
		case CREATE : 
		processing = createMQ(queue_name);
		case DESTROY : 
		processing = destroyMQ(queue_name);
		case PUT :
		msg = malloc(sr.msg_ln);
		if(recv(s_conec,msg,msg_ln, 0)< 0){
		return -1;
		}
		processing = put(queue_name, msg,msg_ln);
		case GET : 
		msg = malloc(sr.msg_ln);
		if(recv(s_conec,msg,msg_ln, 0)< 0){
		return -1;
		}
		processing = get(queue_name, &msg, &msg_ln, sr.block);
	}
	char* msgresp = malloc(msg_ln);
	size_t* msg_lnres = malloc(sizeof(size_t));

	if(sr.cop == GET && processing >0){
		memcpy(msgresp, msg, msg_ln);
		memcpy(msg_lnres, &msg_ln, sizeof(msg_ln));
	}

	if(send(s_conec, msgresp, msg_ln, 0)<0){
		return -1;
	}
	if(send(s_conec, msg_lnres, sizeof(msg_ln), 0)<0){
		return -1;
	}

	if(sr.cop == PUT){
		free(msg);
	}
	free(queue_name);
	free(msgresp);
	free(msg_lnres);
		
	close(s_conec);
}
close(s);
return 1;
}

int create_queue(Queue *q, const char* queue_name){
	
	Queue* newq = (Queue*)malloc(sizeof(Queue));
	if(newq != NULL){
	newq->front = NULL;
	newq->last = NULL;
	newq->name = (char *)malloc(strlen(queue_name));
	strcpy(newq->name, queue_name);
	newq->size = 0;
	*q = *newq;
	return 1;
}  else return -1;
}

int enqueue(Queue * q, struct Node* node){
	if((q == NULL) || (node ==NULL)){
		return -1;
	}
	node->prev = NULL;
	if(q->size ==0){
		q->front = node;
		q->last = node;
		
	} else {
		q->last->prev = node;
		q->last = node;
	}
	q->size++;
	
	return 1;
}

struct Node* dequeue(Queue* q , void **msg, size_t *tam){
	if((q->size == 0) || (q == NULL)){
		return 0;
	}
	struct Node* node;
	node = q->front;
	*msg = (void*)node->msg;
	*tam = node->msg_ln;
	q->front = (q->front)->prev;
	q->size--;
	return node;
}

int destruct_queue(Queue * q){
	struct Node* front = NULL;
	if(q == NULL){
		return -1;
	}
	
	while(q->size !=0){
		front = dequeue(q, NULL, NULL);
		free(front->msg);
		front->msg = NULL;
		free(front);
		front = NULL;	
	}
	free(q->name);
	q->name = NULL;
	free(q);
	q = NULL;
	return 1;
}

int index_in_array(const char* cola){
	for(int i =0; i<allqueues.size; ++i){
		if (strcmp(allqueues.array[i].name, cola) == 0){
			return i;
		}
	}
	return -1;
}

int createMQ(const char* cola){
	if(allqueues_empty){
		allqueues.array = (Queue *)malloc(0);
		allqueues.size = 0;
		allqueues_empty = -1;
	}
	if(index_in_array(cola) >=0){
		return -1;
	}
	
	Queue q;
	if(create_queue(&q, cola) < 0){
		return -1;
	}
	allqueues.array = (Queue *)realloc(allqueues.array, (allqueues.size+1) * sizeof(Queue));
	if(allqueues.array <0){
		return -1;
	}
	allqueues.size++;
	allqueues.array[allqueues.size - 1] = q;
	return 1;
}

int destroyMQ(const char* cola){
	if(allqueues_empty){
		return -1;
	}
	int index = -1;
	if((index = index_in_array(cola)) < 0){
		return -1;
	}
	
	Queue q = allqueues.array[index];
	destruct_queue(&q);
	allqueues.size--;
	
	Queue* newqueues = NULL;
	do{
	newqueues = calloc(allqueues.size, sizeof(Queue)); 
	} while(newqueues == NULL);

    memcpy(newqueues, allqueues.array, index * sizeof(Queue)); 
    
	if (index < allqueues.size){
		memcpy(&(newqueues[index]), &(allqueues.array[index+1]),(allqueues.size - index) * sizeof(Queue)); 
	}	
    free (allqueues.array);
	allqueues.array = newqueues;
	return 0;
}

int put(const char *cola, const void *mensaje, size_t tam){
	if(allqueues_empty){
		return -1;
	}
	int index  = -1;
	if((index = index_in_array(cola)) < 0){
		return -1;
	}
	Queue q = allqueues.array[index];
	struct Node node = {mensaje, tam, NULL};
	int res = -1;
	if(enqueue(&q,&node)<0){
	return -1;	
	} 
	allqueues.array[index] = q;
	return 1;
}

int get(const char *cola, void **mensaje, size_t *tam, int blocking){
	if(allqueues_empty){
		return -1;
	}
	int index  = -1;
	if((index = index_in_array(cola)) < 0){
		return -1;
	}
	Queue q = allqueues.array[index];
	if(dequeue(&q,mensaje,tam) <0){
		return -1;
	}
	allqueues.array[index]=q;
	return 1;
}



	
	

