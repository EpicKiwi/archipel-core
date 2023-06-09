// SPDX-License-Identifier: BSD-3-Clause OR Apache-2.0
/*
 * hal_config.h
 *
 * Description: contains platform-specific configuration information
 *
 */

#ifndef HAL_CONFIG_H_INCLUDED
#define HAL_CONFIG_H_INCLUDED

#include <sys/socket.h>


/* THREAD CONFIGURATION */

#define CONTACT_RX_TASK_PRIORITY 2
#define ROUTER_TASK_PRIORITY 2
#define BUNDLE_PROCESSOR_TASK_PRIORITY 2
#define CONTACT_MANAGER_TASK_PRIORITY 1
#define CONTACT_TX_TASK_PRIORITY 3
#define CONTACT_LISTEN_TASK_PRIORITY 2
#define CONTACT_MANAGEMENT_TASK_PRIORITY 2

/* 0 means inheriting the stack size from the parent task */
#define DEFAULT_TASK_STACK_SIZE 0
#define CONTACT_RX_TASK_STACK_SIZE 0
#define CONTACT_MANAGER_TASK_STACK_SIZE 0
#define CONTACT_TX_TASK_STACK_SIZE 0
#define CONTACT_LISTEN_TASK_STACK_SIZE 0
#define CONTACT_MANAGEMENT_TASK_STACK_SIZE 0


/* IO CONFIGURATION */

#define APPLICATION_AGENT_RX_BUFFER_SIZE 512


/* PLATFORM CONFIGURATION */

#define LINUX_SPECIFIC_API 0

#endif /* HAL_CONFIG_H_INCLUDED */
