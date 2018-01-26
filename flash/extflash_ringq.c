/*
* @Author: zxt
* @Date:   2018-01-11 10:34:13
* @Last Modified by:   zxt
* @Last Modified time: 2018-01-26 10:41:14
*/
#include "../general.h"


/***** Function definitions *****/
//***********************************************************************************
// brief:   Init the extflash ring queue parameter
// 
// parameter: 
//***********************************************************************************
void ExtflashRingQueueInit(extflash_queue_s * p_queue)
{  
   p_queue->size = EXTFLASH_QUEUE_MAX ;  
     
   p_queue->head = 0;  
   p_queue->tail = 0;  
     
   p_queue->tag = 0;  
}  




//***********************************************************************************
// brief: add the data to queue   
// 
// parameter: 
// return:
//      true:   add success
//      false:  add fail
//***********************************************************************************
bool ExtflashRingQueuePush(extflash_queue_s * p_queue, uint8_t *data)
{  
    if(ExtflashRingQueueIsFull(p_queue))
    {  
        return false;  
    }  
        
    memcpy(p_queue->space[p_queue->tail], data, SENSOR_DATA_LENGTH_MAX);
     
    p_queue->tail = (p_queue->tail + 1) % p_queue->size ;  
     
    /* the queue is full*/  
    if(p_queue->tail == p_queue->head)
    {  
       p_queue->tag = 1;  
    }

    return true;
}  
  
//***********************************************************************************
// brief: take the data from queue   
// 
// parameter: 
// return:
//      true:   take success
//      false:  take fail
//***********************************************************************************
bool ExtflashRingQueuePoll(extflash_queue_s * p_queue, uint8_t * data)
{  
    if(ExtflashRingQueueIsEmpty(p_queue))  
    {  
        return false;
    }  
    
    memcpy(data, p_queue->space[p_queue->head], SENSOR_DATA_LENGTH_MAX);

    p_queue->head = (p_queue->head + 1) % p_queue->size ;  
     
    /* the queue is empty*/  
    if(p_queue->tail == p_queue->head)  
    {  
        p_queue->tag = 0;  
    }
    return true;
}  



