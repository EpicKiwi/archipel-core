// SPDX-License-Identifier: BSD-3-Clause OR Apache-2.0
#include "ud3tn/init.h"
#include "ud3tn/cmdline.h"

#include "platform/hal_io.h"
#include "platform/hal_platform.h"
#include "platform/hal_task.h"

#include "unity_fixture.h"

#include <stdio.h>
#include <stdlib.h>

void testud3tn(void);

static int test_errors;

void test_task(void *args)
{
	static const char *argv[1] = { "testud3tn" };

	LOG("Starting testsuite...");
	hal_io_message_printf("\n");

	/* Disable the logger spamming out output */
	/* Start Unity */
	test_errors = UnityMain(1, argv, testud3tn);

	hal_io_message_printf("\n");
	if (!test_errors) {
		LOG("uD3TN unittests succeeded.");
		exit(EXIT_SUCCESS);
	} else {
		LOGF("uD3TN unittests resulted in %d error(s).", test_errors);
		exit(EXIT_FAILURE);
	}
}

int main(void)
{
	char *argv[1] = {"<undefined>"};

	init(1, argv);

	hal_task_create(test_task, "test_task", 0, NULL, 0, NULL);

	return start_os();
}
