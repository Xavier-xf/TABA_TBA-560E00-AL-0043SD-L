#include "queue.h"
#include "ak_mem.h"
#include <memory.h>
#define QUEUE_MAX 64


struct queue_node
{
	struct queue_node* prev;
	struct queue_node* next;
	queue_data data;
};


typedef struct
{
	struct queue_node* head;
	struct queue_node* tail;
	int index;
}queue;



static queue queue_msg[AUDIO_CHANNEL_TOTAL];


static void node_data_init(queue_data* node)
{
	node->addres = NULL;
	node->size = 0;
	node->ts = 0;
	node->is_idr_frame = false;
}



static struct queue_node* create_init_node(void)
{
	struct queue_node* head = (struct queue_node*)ak_mem_alloc(MODULE_ID_APP, sizeof(struct queue_node));
	head->next = head->prev = NULL;
	node_data_init(&head->data);

	struct queue_node* node;
	struct queue_node* node_temp = head;
	
	int i = 0;
	for( i = 0 ; i < QUEUE_MAX ; i++)
	{
		node = (struct queue_node*)ak_mem_alloc(MODULE_ID_APP, sizeof(struct queue_node));
		node->prev = node_temp;
		node->next = NULL;

		node_temp->next = node;
		node_data_init(&node->data);
		node_temp = node;
	}
	node->next = head;
	return head;
}


#if 0
static bool queue_full_check(queue*q)
{
	if(q->index >= QUEUE_MAX)
	{
		return true;
	}
	return false;
}


static bool queue_empty_check(queue*q)
{
	if(q->index <= 0)
	{
		return true;
	}
	return false;
}
#endif
static bool queue_init(queue* q)
{
	q->head = create_init_node();
	q->tail = q->head;
	q->index = 0;
	return true;
}

static bool queue_push(queue*q,queue_data* data)
{
	struct queue_node* node = q->tail;
	/*******************************************************************
	在数据地址为空或者已存在的数据大小小于需要存储的大小，则重新分配大小
	********************************************************************/
	if(node->data.addres != NULL)/*||(data->size > node->data.size))*/
	{
		ak_mem_free(node->data.addres);
	}
	if(q == &queue_msg[QUEUE_CHANNEL_VIDEO_1])
	{
		node->data.size = data->size + 4;
	}
	else
	{
		node->data.size = data->size;
	}
	node->data.addres = (unsigned char*)ak_mem_alloc(MODULE_ID_APP, node->data.size);
	if(q == &queue_msg[QUEUE_CHANNEL_VIDEO_1])
	{
		unsigned char node_head[4] = {0,0,0,1};
		memcpy(node->data.addres,node_head,4);
		memcpy(&(node->data.addres[4]),data->addres,data->size);
	}
	else
	{
		memcpy(node->data.addres,data->addres,data->size);
	}
	node->data.ts = data->ts;
	node->data.is_idr_frame = data->is_idr_frame;
	node->data.ts = data->ts;

	/****************************************
	在环形的队列中，尾和头相连了，则头部则下移
	****************************************/
	if(node->next == q->head)
	{
		struct queue_node* head = q->head;
		q->head = head->next;
		node->next = head;
	}
	else
	{
		q->index++;
	}
	q->tail = node->next;
//	printf("push:%d:pecakget addres:%p,size:%d idr:%d\n\r",q->index,node->data.addres,node->data.size,node->data.is_idr_frame);
	return true;
}


static queue_data* queue_pop(queue*q)
{
	if(q->tail == q->head)
	{
		return NULL;
	}
	
	struct queue_node* node = q->head;
	queue_data* pdata = &(q->head->data);	
	q->head = node->next;
	q->index--;
	
//	printf("pop:%d:pecakget addres:%p,size:%d idr:%d\n\r",q->index,node->data.addres,node->data.size,node->data.is_idr_frame);
	return pdata;
}


bool queue_data_init(QUEUE_CHANNEL ch)
{
	queue_init(&queue_msg[ch]);
	return true;
}
bool queue_data_release(QUEUE_CHANNEL ch)
{
	struct queue_node* node = queue_msg[ch].head;
	while(1)
	{
		if(node->data.addres)
		{
			ak_mem_free(node->data.addres);
		}
		node_data_init(&(node->data));
		if(node == queue_msg[ch].tail)
		{
			break;
		}
		node = node->next;
	}
	queue_msg[ch].head = queue_msg[ch].tail;
	queue_msg[ch].index = 0;
	return true;
}

bool queue_data_push(QUEUE_CHANNEL ch,unsigned char* addres,int len,unsigned long ts,bool is_idr)
{
	queue_data data;
	data.addres = addres;
	data.size = len;
	data.ts = ts;
	data.is_idr_frame = is_idr;
	queue_push(&queue_msg[ch],&data);
	return true;
}

queue_data* queue_data_pop(QUEUE_CHANNEL ch)
{
	return queue_pop(&queue_msg[ch]);
}

bool queue_data_full_check(QUEUE_CHANNEL ch)
{
	return queue_msg[ch].index == QUEUE_MAX?true:false;
}

bool queue_data_empty_check(QUEUE_CHANNEL ch)
{
	return queue_msg[ch].index<2?true:false;
}


