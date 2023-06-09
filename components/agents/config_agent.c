// SPDX-License-Identifier: BSD-3-Clause OR Apache-2.0
#include "agents/config_agent.h"
#include "agents/config_parser.h"

#include "ud3tn/bundle_processor.h"
#include "ud3tn/common.h"
#include "ud3tn/config.h"
#include "ud3tn/eid.h"

#include "platform/hal_io.h"
#include "platform/hal_types.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct config_parser parser;

struct config_agent_params {
	const char *local_eid;
	bool allow_remote_configuration;
};

static void router_command_send(struct router_command *cmd, void *param)
{
	ASSERT(cmd != NULL);

	QueueIdentifier_t bp_queue = param;

	bundle_processor_inform(
		bp_queue,
		NULL,
		BP_SIGNAL_PROCESS_ROUTER_COMMAND,
		NULL,
		NULL,
		NULL,
		cmd
	);
}

static void callback(struct bundle_adu data, void *param)
{
	struct config_agent_params *const ca_param = param;

	if (!ca_param->allow_remote_configuration) {
		char *const node_id = get_node_id(data.source);

		if (!node_id || strncmp(ca_param->local_eid, node_id,
					strlen(ca_param->local_eid)) != 0) {
			LOGF("ConfigAgent: Dropped config message from foreign endpoint \"%s\"",
			     data.source);
			free(node_id);
			return;
		}
		free(node_id);
	}

	config_parser_reset(&parser);
	config_parser_read(
		&parser,
		data.payload,
		data.length
	);
	bundle_adu_free_members(data);
}

int config_agent_setup(
	QueueIdentifier_t bundle_processor_signaling_queue,
	const char *local_eid,
	bool allow_remote_configuration)
{
	const int is_ipn = get_eid_scheme(local_eid) == EID_SCHEME_IPN;

	ASSERT(config_parser_init(&parser, &router_command_send,
				  bundle_processor_signaling_queue));

	struct config_agent_params *const ca_param = malloc(
		sizeof(struct config_agent_params)
	);
	ca_param->local_eid = local_eid;
	ca_param->allow_remote_configuration = allow_remote_configuration;

	return bundle_processor_perform_agent_action(
		bundle_processor_signaling_queue,
		BP_SIGNAL_AGENT_REGISTER,
		is_ipn ? AGENT_ID_CONFIG_IPN : AGENT_ID_CONFIG_DTN,
		callback,
		ca_param,
		false
	);
}
