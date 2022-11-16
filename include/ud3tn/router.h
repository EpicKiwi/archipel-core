// SPDX-License-Identifier: BSD-3-Clause OR Apache-2.0
#ifndef ROUTER_H_INCLUDED
#define ROUTER_H_INCLUDED

#include "ud3tn/bundle.h"
#include "ud3tn/config.h"
#include "ud3tn/node.h"
#include "ud3tn/routing_table.h"

#include <stddef.h>
#include <stdint.h>

struct router_config {
	size_t global_mbs;
	uint16_t fragment_min_payload;
	uint8_t router_min_contacts_htab;
	uint8_t router_min_contacts_nbf;
};

struct fragment_route {
	uint32_t payload_size;
	struct contact *contact;
	uint8_t preemption_improved;
};

/* With MAX_FRAGMENTS = 3 and MAX_CONTACTS = 5: 92 bytes on stack */
struct router_result {
	struct fragment_route fragment_results[ROUTER_MAX_FRAGMENTS];
	int32_t fragments;
	uint8_t preemption_improved;
};

#define ROUTER_BUNDLE_PRIORITY(bundle) (bundle_get_routing_priority(bundle))
#define ROUTER_CONTACT_CAPACITY(contact, prio) \
	(contact_get_cur_remaining_capacity(contact, prio))

struct router_config router_get_config(void);
void router_update_config(struct router_config config);

struct contact_list *router_lookup_destination(char *dest);
uint8_t router_calculate_fragment_route(
	struct fragment_route *res, uint32_t size,
	struct contact_list *contacts, uint32_t preprocessed_size,
	enum bundle_routing_priority priority, uint64_t exp_time,
	struct contact **excluded_contacts, uint8_t excluded_contacts_count);

struct router_result router_get_first_route(struct bundle *bundle);
struct router_result router_try_reuse(
	struct router_result route, struct bundle *bundle);

enum ud3tn_result router_add_bundle_to_contact(
	struct contact *contact, struct bundle *b);
enum ud3tn_result router_remove_bundle_from_contact(
	struct contact *contact, struct bundle *bundle);

#endif /* ROUTER_H_INCLUDED */
