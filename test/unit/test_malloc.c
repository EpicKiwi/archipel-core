// SPDX-License-Identifier: BSD-3-Clause OR Apache-2.0
#include "ud3tn/config.h"

#include "unity_fixture.h"

#include <stdlib.h>

TEST_GROUP(malloc);

TEST_SETUP(malloc)
{
}

TEST_TEAR_DOWN(malloc)
{
}

TEST(malloc, alloc_huge)
{
	char *mem = malloc(1024);

	TEST_ASSERT_NOT_NULL(mem);
	free(mem);
	mem = malloc(BUNDLE_MAX_SIZE);
	TEST_ASSERT_NOT_NULL(mem);
	free(mem);
	mem = malloc(65537);
	TEST_ASSERT_NOT_NULL(mem);
	free(mem);
}

TEST_GROUP_RUNNER(malloc)
{
	RUN_TEST_CASE(malloc, alloc_huge);
}
