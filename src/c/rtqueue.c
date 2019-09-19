/* rtqueue.c
This file is a part of 'rtqueue'
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

'rtqueue' is a simple FIFO linked list intended to hold 
JACK sample data for process()'ing.

Copyright 2014 murray foster */

#include <stdio.h>
#include <stdlib.h>
#include <jack/jack.h>
#include <pthread.h>

int dequeue_is_waiting = 0;
pthread_mutex_t dequeue_is_waiting_mutex;
pthread_cond_t dequeue_is_waiting_cond;

int enqueue_is_waiting = 0;
pthread_mutex_t enqueue_is_waiting_mutex;
pthread_cond_t enqueue_is_waiting_cond;

typedef struct rtqueue_item {

  size_t size;
  unsigned char *data;
} rtqueue_item_t;

typedef struct queue
{
  int head;
  int tail;
  int recordlimit;
  int records;
  unsigned char **queue;
} rtqueue_t;

rtqueue_t *
rtqueue_init(int recordlimit)
{
  rtqueue_t *rtq = (rtqueue_t *) malloc(sizeof(rtqueue_t));
  rtq->queue = malloc(sizeof(unsigned char) * (recordlimit + 1));
  rtq->head = 0;
  rtq->tail = 0;
  rtq->recordlimit = recordlimit;
  return rtq;
}

rtqueue_item_t *
rtqueue_item_init(unsigned char *data, size_t size)
{
  rtqueue_item_t *rtq_item = malloc(sizeof(rtqueue_item_t));
  rtq_item->data = data;
  rtq_item->size = size;
  return rtq_item;
}

int
rtqueue_numrecords(rtqueue_t *rtq)
{
  return rtq->records;
}

int
rtqueue_isfull(rtqueue_t *rtq)
{
  /* if queue is full, return 1 */
  if ((rtq->tail + 1) % (rtq->recordlimit + 1) == rtq->head)
    return 1;
  else
    return 0;
}

int
rtqueue_isempty(rtqueue_t *rtq)
{
  /* if queue is empty, return 1 */
  if (rtq->head == rtq->tail)
    return 1;
  else
    return 0;
}

int
rtqueue_enq(rtqueue_t *rtq, rtqueue_item_t *data)
{
  /* if queue is full, wait */
  if ((rtq->tail + 1) % (rtq->recordlimit + 1) == rtq->head)
    {
      enqueue_is_waiting = 1;
      pthread_mutex_lock(&enqueue_is_waiting_mutex);
      pthread_cond_wait(&enqueue_is_waiting_cond, &enqueue_is_waiting_mutex);
      pthread_mutex_unlock(&enqueue_is_waiting_mutex); 
      enqueue_is_waiting = 0;
    }

  
  
  rtq->queue[rtq->tail] = data;
  rtq->tail = (rtq->tail + 1) % (rtq->recordlimit + 1);
  rtq->records+=1;

  if (dequeue_is_waiting)
    pthread_cond_signal(&dequeue_is_waiting_cond);

  return 0;
}

rtqueue_item_t*
rtqueue_deq(rtqueue_t *rtq)
{
  rtqueue_item_t *data;

  /* if queue is empty, wait */
  while (rtq->head == rtq->tail)
    {
      dequeue_is_waiting = 1;
      pthread_mutex_lock(&dequeue_is_waiting_mutex);
      pthread_cond_wait(&dequeue_is_waiting_cond, &dequeue_is_waiting_mutex);
      pthread_mutex_unlock(&dequeue_is_waiting_mutex); 
      dequeue_is_waiting = 0;
    }

  /* dequeue and return data at the head */
  data = rtq->queue[rtq->head];
  rtq->head = (rtq->head + 1) % (rtq->recordlimit + 1);
  rtq->records-=1;

  if (enqueue_is_waiting)
    pthread_cond_signal(&enqueue_is_waiting_cond);

  return data;
}
