// SPDX-License-Identifier: BSD-3-Clause OR Apache-2.0
#include "cla/cla.h"
#include "cla/cla_contact_tx_task.h"

#include "bundle6/parser.h"
#include "bundle7/parser.h"

#include "platform/hal_config.h"
#include "platform/hal_io.h"
#include "platform/hal_semaphore.h"
#include "platform/hal_task.h"

#include "ud3tn/bundle.h"
#include "ud3tn/common.h"
#include "ud3tn/router_task.h"
#include "ud3tn/task_tags.h"

#include <stdlib.h>


static inline void report_bundle(QueueIdentifier_t signaling_queue,
				 struct bundle *bundle,
				 char *cla_addr,
				 enum router_signal_type type)
{
	struct bundle_tx_result *info = malloc(
		sizeof(struct bundle_tx_result)
	);

	//info->bundle = cur->data->bundle_ptr;
	info->bundle = bundle;
	info->peer_cla_addr = cla_addr;

	struct router_signal signal = {
		.type = type,
		.data = info,
	};

	hal_queue_push_to_back(signaling_queue, &signal);
}

// BPv7 5.4-4 / RFC5050 5.4-5
static void prepare_bundle_for_forwarding(struct bundle *bundle)
{
	struct bundle_block_list **blocks = &bundle->blocks;

	// BPv7 5.4-4: "If the bundle has a Previous Node block ..., then that
	// block MUST be removed ... before the bundle is forwarded."
	while (*blocks != NULL) {
		if ((*blocks)->data->type == BUNDLE_BLOCK_TYPE_PREVIOUS_NODE) {
			// Replace the first occurrence of the previous node
			// block by its successor and free it.
			*blocks = bundle_block_entry_free(*blocks);
			break;
		}
		blocks = &(*blocks)->next;
	}

	const uint8_t dwell_time_ms = hal_time_get_timestamp_ms() -
		bundle->reception_timestamp_ms;

	// BPv7 5.4-4: "If the bundle has a bundle age block ... at the last
	// possible moment ... the bundle age value MUST be increased ..."
	if (bundle_age_update(bundle, dwell_time_ms) == UD3TN_FAIL)
		LOGF("TX: Bundle %p age block update failed!", bundle);
}

static void cla_contact_tx_task(void *param)
{
	struct cla_link *link = param;
	struct cla_contact_tx_task_command cmd;

	enum ud3tn_result s;
	void const *cla_send_packet_data =
		link->config->vtable->cla_send_packet_data;
	QueueIdentifier_t router_signaling_queue =
		link->config->bundle_agent_interface->router_signaling_queue;

	while (link->active) {
		if (hal_queue_receive(link->tx_queue_handle,
				      &cmd, -1) == UD3TN_FAIL)
			continue;
		else if (cmd.type == TX_COMMAND_FINALIZE || !cmd.bundle)
			break;

		struct bundle *b = cmd.bundle;

		prepare_bundle_for_forwarding(b);
		LOGF(
			"TX: Sending bundle %p via CLA %s",
			b,
			link->config->vtable->cla_name_get()
		);
		link->config->vtable->cla_begin_packet(
			link,
			bundle_get_serialized_size(b),
			cmd.cla_address
		);
		s = bundle_serialize(
			b,
			cla_send_packet_data,
			(void *)link
		);
		link->config->vtable->cla_end_packet(link);

		if (s == UD3TN_OK) {
			report_bundle(
				router_signaling_queue,
				b,
				cla_get_cla_addr_from_link(link),
				ROUTER_SIGNAL_TRANSMISSION_SUCCESS
			);
		} else {
			report_bundle(
				router_signaling_queue,
				b,
				cla_get_cla_addr_from_link(link),
				ROUTER_SIGNAL_TRANSMISSION_FAILURE
			);
		}

		free(cmd.cla_address);
	}

	// Lock the queue before we start to free it
	hal_semaphore_take_blocking(link->tx_queue_sem);

	// Consume the rest of the queue
	while (hal_queue_receive(link->tx_queue_handle, &cmd, 0) != UD3TN_FAIL) {
		if (cmd.type == TX_COMMAND_BUNDLES) {
			report_bundle(
				router_signaling_queue,
				cmd.bundle,
				cla_get_cla_addr_from_link(link),
				ROUTER_SIGNAL_TRANSMISSION_FAILURE
			);
			free(cmd.cla_address);
		}
	}

	Task_t tx_task_handle = link->tx_task_handle;

	// After releasing the semaphore, link may become invalid.
	hal_semaphore_release(link->tx_task_sem);
	hal_task_delete(tx_task_handle);
}

enum ud3tn_result cla_launch_contact_tx_task(struct cla_link *link)
{
	static uint8_t ctr = 1;
	static char tname_buf[6];

	tname_buf[0] = 't';
	tname_buf[1] = 'x';
	snprintf(tname_buf + 2, sizeof(tname_buf) - 2, "%hhu", ctr++);

	hal_semaphore_take_blocking(link->tx_task_sem);
	link->tx_task_handle = hal_task_create(
		cla_contact_tx_task,
		tname_buf,
		CONTACT_TX_TASK_PRIORITY,
		link,
		CONTACT_TX_TASK_STACK_SIZE,
		(void *)CONTACT_TX_TASK_TAG
	);

	return link->tx_task_handle ? UD3TN_OK : UD3TN_FAIL;
}

void cla_contact_tx_task_request_exit(QueueIdentifier_t queue)
{
	struct cla_contact_tx_task_command command = {
		.type = TX_COMMAND_FINALIZE,
		.bundle = NULL,
		.cla_address = NULL,
	};

	ASSERT(queue != NULL);
	hal_queue_push_to_back(queue, &command);
}
