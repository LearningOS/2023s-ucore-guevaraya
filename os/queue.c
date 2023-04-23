#include "queue.h"
#include "defs.h"

void init_queue(struct queue *q)
{
	q->front = q->tail = 0;
	q->empty = 1;
}

void push_queue(struct queue *q, int value)
{
	if (!q->empty && q->front == q->tail) {
		panic("queue shouldn't be overflow");
	}
	q->empty = 0;
	q->data[q->tail] = value;
	q->tail = (q->tail + 1) % NPROC;
}

int pop_queue(struct queue *q)
{
	if (q->empty)
		return -1;
	int value = q->data[q->front];
	q->front = (q->front + 1) % NPROC;
	if (q->front == q->tail)
		q->empty = 1;
	return value;
}
const Stride_t BIG_STRIDE = 64;
int cmp(Stride_t a, Stride_t b) {
    // YOUR CODE HERE
     if(a == b) 
	 	return 0;
	 else if(a > b)
	 	return 1;
	return -1;
}
/* find the lowest stride of proc 
return proc id*/
int find_low_stride_queue(struct queue *q, struct proc* p)
{
	int index = q->front;
	if(index == q->tail)return -1;
	int low_index = index;
	debugf("front:%d tail:%d", q->front, q->tail);
	debugf("index:%d value:%d pid:%d stride:%d, tail:%d", index, q->data[index],
					p[q->data[index]].pid, p[q->data[index]].stride, q->tail);
	index =  (index+1)% NPROC;
	while(index != q->tail){
		
		debugf("index:%d value:%d pid:%d stride:%d, tail:%d", index, q->data[index],
						p[q->data[index]].pid, p[q->data[index]].stride, q->tail);
		if(cmp(p[q->data[low_index]].stride, p[q->data[index]].stride) > 0){
			low_index = index;
		}
		index =  (index+1)% NPROC;
	}
	p[q->data[low_index]].stride += BIG_STRIDE/p[q->data[low_index]].priority;
	return low_index;
}
int pop_prior_queue(struct queue *q, struct proc* p)
{
	if (q->empty)
		return -1;
	int low_index = find_low_stride_queue(q, p);
	int pop_value;
	int value = q->data[q->front];
	debugf("find low_index:%d, pid:%d, stride:%d", low_index, q->data[low_index], p[q->data[low_index]].stride);
	if(q->data[low_index] > 200){
		panic("invalid low_index");
	}
	if(low_index != q->front){
		pop_value = q->data[low_index];
		q->data[low_index] = value;
		value = pop_value;
	} 
	q->front = (q->front + 1) % NPROC;


	if (q->front == q->tail)
		q->empty = 1;
	return value;
}

