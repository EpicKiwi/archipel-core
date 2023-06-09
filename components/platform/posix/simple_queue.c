// SPDX-License-Identifier: BSD-3-Clause OR Apache-2.0
/*
 * simple_queue.c
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
#include "platform/posix/simple_queue.h"

#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

static char *increment(
	char *current, char *abs_start, char *abs_end, unsigned int item_size)
{

	char *val = current + item_size;

	if (val > abs_end)
		val = abs_start;

	return val;
}

static char *decrement(
	char *current, char *abs_start, char *abs_end, unsigned int item_size)
{

	char *val = current - item_size;

	if (val < abs_start)
		val = abs_end - item_size;

	return val;
}

Queue_t *queueCreate(unsigned int queue_length, unsigned int item_size)
{
	if (queue_length == 0 || item_size == 0) {
		// if one of the values is zero, creating a queue
		// makes no sense
		exit(EXIT_FAILURE);
	}

	// allocate memory to store managment data
	Queue_t *queue = malloc(sizeof(Queue_t));

	// initialise the queue's semaphore
	sem_init(&queue->semaphore, 0, 1);
	sem_init(&queue->sem_pop, 0, 0);
	sem_init(&queue->sem_push, 0, queue_length);

	queue->item_length = queue_length;
	queue->item_size = item_size;

	// allocate enough memory to store the actual items
	queue->abs_start = malloc(queue->item_length * queue->item_size);
	queue->abs_end = queue->abs_start + (
		queue->item_length * queue->item_size);

	queue->current_start = queue->abs_start;
	queue->current_end = queue->abs_start;

	return queue;
}

void queueDelete(Queue_t *queue)
{
	// get the semaphore
	sem_wait(&queue->semaphore);

	// free the data memory
	free(queue->abs_start);

	// destroy the semaphore
	sem_destroy(&queue->semaphore);

	// free the memory ressources for the management data
	free(queue);
}

void queueReset(Queue_t *queue)
{
	sem_wait(&queue->semaphore);

	// reset both current-pointers to the absolute start pointer
	// old values will be simply overwritten when new items are stored
	queue->current_start = queue->abs_start;
	queue->current_end = queue->abs_start;

	sem_destroy(&queue->sem_pop);
	sem_destroy(&queue->sem_push);

	sem_init(&queue->sem_pop, 0, 0);
	sem_init(&queue->sem_push, 0, queue->item_length);

	sem_post(&queue->semaphore);
}

unsigned int queueItemsWaiting(Queue_t *queue)
{
	int value;

	sem_getvalue(&queue->sem_pop, &value);
	if (value < 0)
		exit(EXIT_FAILURE); // Bad error -> abort
	else
		return (unsigned int)value;
}

uint8_t queuePop(Queue_t *queue, void *targetBuffer, int timeout)
{
	struct timespec ts;
	int errsv;

	if (timeout >= 0) {
		if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
			exit(EXIT_FAILURE);

		ts.tv_sec += timeout / 1000;
		ts.tv_nsec += (timeout % 1000) * 1000000;

		if (ts.tv_nsec >= 1000000000) {
			ts.tv_sec += 1;
			ts.tv_nsec = (ts.tv_nsec%1000000000);
		}

		if (sem_timedwait(&(queue->sem_pop), &ts) == -1) {
			errsv = errno;

			if (errsv != ETIMEDOUT) {
				printf("unhandled error occured - value: %d\n",
				       errsv);
				exit(EXIT_FAILURE);
			}

			return EXIT_FAILURE;
		}
	} else {
		sem_wait(&queue->sem_pop);
	}

	sem_wait(&queue->semaphore);

	if (queue->current_start >= queue->abs_end)
		queue->current_start = queue->abs_start;

	memcpy(targetBuffer, queue->current_start, queue->item_size);

	queue->current_start = increment(
		queue->current_start, queue->abs_start,
		queue->abs_end, queue->item_size);

	sem_post(&queue->sem_push);
	sem_post(&queue->semaphore);

	return EXIT_SUCCESS;
}


uint8_t queuePush(Queue_t *queue, const void *item, int timeout, bool force)
{
	struct timespec ts;
	int errsv;

	// forcefully replace the last element of the queue
	if (force && sem_trywait(&queue->sem_push) == -1) {

		sem_wait(&queue->semaphore);

		memcpy(decrement(
				queue->current_end, queue->abs_start,
				queue->abs_end, queue->item_size),
			item, queue->item_size);

		sem_post(&queue->semaphore);

		return EXIT_SUCCESS;
	} else if (timeout >= 0) {
		if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
			exit(EXIT_FAILURE);

		ts.tv_sec += timeout / 1000;
		ts.tv_nsec += (timeout % 1000) * 1000000;
		if (ts.tv_nsec >= 1000000000) {
			ts.tv_sec += 1;
			ts.tv_nsec = (ts.tv_nsec%1000000000);
		}

		if (sem_timedwait(&(queue->sem_push), &ts) == -1) {
			errsv = errno;

			if (errsv != ETIMEDOUT) {
				printf("unhandled error occured - value: %d\n",
				       errsv);
				exit(EXIT_FAILURE);
			}

			return EXIT_FAILURE;
		}
	} else if (!force) {
		sem_wait(&queue->sem_push);
	}

	sem_wait(&queue->semaphore);

	if (queue->current_end >= queue->abs_end)
		queue->current_end = queue->abs_start;

	memcpy(queue->current_end, item, queue->item_size);

	queue->current_end = increment(
		queue->current_end, queue->abs_start,
		queue->abs_end, queue->item_size);

	sem_post(&queue->sem_pop);
	sem_post(&queue->semaphore);

	return EXIT_SUCCESS;
}
