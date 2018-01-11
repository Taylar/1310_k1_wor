#ifndef EXTFLASH_RINGQ_H__
#define EXTFLASH_RINGQ_H__ 



#define	SENSOR_DATA_LENGTH_MAX			24
#define EXTFLASH_QUEUE_MAX 				20  


  
typedef struct ringq{  
	uint8_t 	head; /* the head, the queue take direction*/  
	uint8_t 	tail; /* the head, the queue add direction*/   
	uint8_t 	tag ; /* 0:empty   1: full*/  
	uint8_t 	size ; /* the size of the queue */  
	uint8_t		space[EXTFLASH_QUEUE_MAX][SENSOR_DATA_LENGTH_MAX]; /* the queue space */  
}extflash_queue_s;  
  

#define ExtflashRingQueueIsEmpty(q) ( (q->head == q->tail) && (q->tag == 0))  

#define ExtflashRingQueueIsFull(q) ( (q->head == q->tail) && (q->tag == 1))  

void ExtflashRingQueueInit(extflash_queue_s * p_queue);

bool ExtflashRingQueuePush(extflash_queue_s * p_queue, uint8_t *data);

bool ExtflashRingQueuePoll(extflash_queue_s * p_queue, uint8_t * p_data);

  

  



#endif	// EXTFLASH_RINGQ_H__
