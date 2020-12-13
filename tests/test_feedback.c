#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ndm/feedback.h>
#include "test.h"

#define ESEP_			NDM_FEEDBACK_ENV_SEPARATOR
#define OUTPUT_			"ndm_feedback.output"

int main(int argc, char *argv[])
{
	bool feedback_done = false;
	const char *args[] =
	{
		argv[0],
		"arg1",
		"arg2",
		"arg3",
		NULL
	};

	if (argc == 4 &&
		strcmp(argv[1], "arg1") == 0 &&
		strcmp(argv[2], "arg2") == 0 &&
		strcmp(argv[3], "arg3") == 0)
	{
		/* is a child process? */

		int error = ~0;
		FILE *fp = fopen(OUTPUT_, "w");

		if (fp != NULL) {
			fprintf(fp,
				"%s=%s\n"
				"%s=%s\n"
				"%s=%s\n"
				"%s=%s\n",
				"env0", getenv("env0"),
				"env1", getenv("env1"),
				"env2", getenv("env2"),
				"env3", getenv("env3"));
			error = fclose(fp);
		}

		exit((fp == NULL || error) ? EXIT_FAILURE : EXIT_SUCCESS);
	}

	unlink(OUTPUT_);

	NDM_TEST(setenv("env0", "global", 1) == 0);
	NDM_TEST(setenv("env2", "global", 1) == 0);

	feedback_done = ndm_feedback(
			200, args,
			"%s=%s" ESEP_
			"%s=%i" ESEP_
			"%s=%i",
			"env1", "val1",
			"env2", -100,
			"env3", 100);

	if (!feedback_done) {
		NDM_TEST(feedback_done);
	} else {
		char env[32];
		FILE *fp = fopen(OUTPUT_, "r");

		NDM_TEST_BREAK_IF(fp == NULL);

		fgets(env, sizeof(env), fp);
		NDM_TEST(strcmp(env, "env0=global\n") == 0);

		fgets(env, sizeof(env), fp);
		NDM_TEST(strcmp(env, "env1=val1\n") == 0);

		fgets(env, sizeof(env), fp);
		NDM_TEST(strcmp(env, "env2=-100\n") == 0);

		fgets(env, sizeof(env), fp);
		NDM_TEST(strcmp(env, "env3=100\n") == 0);

		fclose(fp);
	}

	return NDM_TEST_RESULT;
}

