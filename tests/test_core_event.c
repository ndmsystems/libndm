#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <ndm/xml.h>
#include <ndm/core.h>
#include <ndm/poll.h>
#include <ndm/time.h>
#include <ndm/macro.h>
#include "test.h"

#define MAX_HANDLED_EVENTS		10

#define KEY_ESC					27

int main()
{
	struct ndm_core_event_connection_t *econn =
		ndm_core_event_connection_open(NDM_CORE_DEFAULT_TIMEOUT);
	char key = 0;
	ssize_t n = 0;
	struct termios old_term;
	struct termios new_term;

	NDM_TEST_BREAK_IF(econn == NULL);

	tcgetattr(STDIN_FILENO, &old_term);
	new_term = old_term;
	new_term.c_lflag &= (unsigned char) ~(ICANON | ECHO | ISIG);
	new_term.c_cc[VMIN] = 1;
	new_term.c_cc[VTIME] = 0;
	tcsetattr(STDIN_FILENO, TCSANOW, &new_term);

	printf("waiting for core events, press ESC to terminate...\n");

	do {
		struct pollfd pfd[2];

		pfd[0].fd = STDIN_FILENO;
		pfd[0].events = POLLIN | POLLERR | POLLNVAL | POLLHUP;
		pfd[0].revents = 0;

		pfd[1].fd = ndm_core_event_connection_fd(econn);
		pfd[1].events = POLLIN | POLLERR | POLLNVAL | POLLHUP;
		pfd[1].revents = 0;

		n = ndm_poll(pfd, NDM_ARRAY_SIZE(pfd), -1);

		if (n > 0) {
			if (pfd[0].revents & POLLIN) {
				n = read(STDIN_FILENO, &key, sizeof(key));
			}

			if (key != KEY_ESC && n >= 0) {
				if (ndm_core_event_connection_has_events(econn) ||
					pfd[1].revents & POLLIN)
				{
					size_t events_handled = 0;

					do {
						struct ndm_core_event_t *e =
							ndm_core_event_connection_get(econn);

						if (e == NULL) {
							NDM_TEST(false);
							n = -1;
						} else {
							const struct timespec raise_time =
								ndm_core_event_raise_time(e);
							const struct ndm_xml_node_t *r =
								ndm_core_event_root(e);
							const struct ndm_xml_node_t *v =
								ndm_xml_node_first_child(r, NULL);

							printf("event: \"%s\" at %li.%06li%s\n",
								ndm_core_event_type(e),
								(long) raise_time.tv_sec,
								(long) raise_time.tv_nsec/NDM_TIME_MSEC,
								(v == NULL) ? "." : ", first level tags:");

							while (v != NULL) {
								printf("\t%s: \"%s\"\n",
									ndm_xml_node_name(v),
									ndm_xml_node_value(v));
								v = ndm_xml_node_next_sibling(v, NULL);
							}

							ndm_core_event_free(&e);
							++events_handled;
						}
					} while (
						events_handled < MAX_HANDLED_EVENTS &&
						ndm_core_event_connection_has_events(econn));
				} else
				if (pfd[0].revents & (POLLERR | POLLNVAL | POLLHUP)) {
					NDM_TEST(false);
					n = -1;
				} else
				if (pfd[1].revents & (POLLERR | POLLNVAL)) {
					NDM_TEST(false);
					n = -1;
				} else
				if (pfd[1].revents & POLLHUP) {
					printf("event: core disconnected.\n");
					key = KEY_ESC;
				}
			}
		}
	} while (key != KEY_ESC && n >= 0);

	NDM_TEST(ndm_core_event_connection_close(&econn));

	tcsetattr(STDIN_FILENO, TCSANOW, &old_term);

	return NDM_TEST_RESULT;
}
