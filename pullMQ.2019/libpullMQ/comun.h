/*
 * Incluya en este fichero todas las definiciones que pueden
 * necesitar compartir el broker y la biblioteca, si es que las hubiera.
 */

#include <stddef.h>

#define CREATE 1
#define DESTROY 2
#define PUT 3
#define GET 4

#define MAX_NAME 2900
#define MAX_MSG 2900



struct Node1 
{ 
  int data; 
  struct Node1 *next; 
}; 
struct Node {
	const void *msg;
	size_t msg_ln;
	struct Node *prev;
};

typedef struct {
	int cop;
	size_t queue_name_ln;
	size_t msg_ln;
	int block;
} Staticrequest;

typedef struct {
	struct Node* front;
	struct Node* last;
	const char* name;
	unsigned int size;
}Queue;

typedef struct 
{
	Queue* array;
	int size;
} Queues;

