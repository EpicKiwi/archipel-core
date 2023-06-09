// SPDX-License-Identifier: BSD-3-Clause OR Apache-2.0
/*
 * simple_queue.h
 *
 * Description: simple and lightweight implementation of message-queues in C.
 *
 * Copyright (c) 2016, Robert Wiewel
 *
 * This file has been initially provided under the BSD 3-clause license and
 * is now provided in agreement with the original author as part of uD3TN
 * under the terms and conditions of either the Apache 2.0 or the BSD 3-clause
 * license. See the LICENSE file in the project root for details.
 *
 */

#ifndef SIMPLE_QUEUE_H_INCLUDED
#define SIMPLE_QUEUE_H_INCLUDED

#include <semaphore.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief The Queue_t struct Data structure holding all queue-related info
 */
typedef struct {
	// absolut length of the queue (in items)
	int item_length;
	// size of one single item (in bytes)
	int item_size;

	// provides mutual exclusion for the given queue
	sem_t semaphore;

	// these semaphores circumvent busy waiting
	sem_t sem_pop;
	sem_t sem_push;

	// abs_start and abs_end are pointers to the first and last
	// byte of the reserved memory
	char *abs_start;
	char *abs_end;

	// current_start and current_end point to the first and last currently
	// used elements in the queue
	char *current_start;
	char *current_end;

} Queue_t;

/**
 * @brief queueCreate Creates a new queue structure, i.e. allocates the
 *			necessary memory ressources
 * @param queue_length absolute length of the queue (in items)
 * @param item_size size of a single item (in bytes)
 * @return returns a pointer to the queue "object"
 */
Queue_t *queueCreate(unsigned int queue_length, unsigned int item_size);

/**
 * @brief queueDelete Delete a queue structure, i.e. frees the memory
 *			ressources
 * @param queue The pointer to the queue structure
 */
void queueDelete(Queue_t *queue);

/**
 * @brief queueReset Reset a given queue, i.e. empty the data section
 * @param queue The pointer to the queue structure
 */
void queueReset(Queue_t *queue);

/**
 * @brief queueItemsWaiting Returns the number of items that are currently in
 *				the list
 * @param queue	The pointer to the queue structure
 * @return The number of items in the list
 */
unsigned int queueItemsWaiting(Queue_t *queue);

/**
 * @brief queuePush Pushes an item to the end of the queue
 * @param queue The pointer to the queue structure
 * @param item A pointer to the item that should be queued
 * @param timeout Defines how long the queuing should be tried
 *			(in milliseconds)
 * @param force Defines if the last element of the queue should be replaced
 *		forcefully (only applied when queue is full)
 * @return Exitcode, if queueing was successfull
 */
uint8_t queuePush(Queue_t *queue, const void *item, int timeout, bool force);

/**
 * @brief queuePop Pops the upmost element of the queue
 * @param queue The pointer to the queue structure
 * @param targetBuffer The memory location where the queued item should be
 *			copied to
 * @param timeout Defines how long the dequeuing should be tried
 *			(in milliseconds)
 * @return Exitcode, if dequeueing was successfull
 */
uint8_t queuePop(Queue_t *queue, void *targetBuffer, int timeout);

#endif /* SIMPLE_QUEUE_H_INCLUDED */
