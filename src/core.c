#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ndm/sys.h>
#include <ndm/xml.h>
#include <ndm/core.h>
#include <ndm/time.h>
#include <ndm/poll.h>
#include <ndm/pool.h>
#include <ndm/dlist.h>
#include <ndm/macro.h>
#include <ndm/stdio.h>
#include <ndm/string.h>
#include <ndm/ip_sockaddr.h>

#define NDM_CORE_SOCKET_								"/var/run/ndm.core.socket"
#define NDM_CORE_EVENT_SOCKET_							"/var/run/ndm.event.socket"

/**
 * A default agent name should be an empty string
 * to prevent NDM agent changing.
 **/
#define NDM_CORE_DEFAULT_AGENT_							""

#define NDM_CORE_STATIC_BUFFER_SIZE_					1024

#define NDM_CORE_CONNECTION_BUFFER_SIZE_				8192

#define NDM_CORE_REQUEST_BINARY_STATIC_SIZE_			2048
#define NDM_CORE_REQUEST_STATIC_SIZE_					2048
#define NDM_CORE_REQUEST_DYNAMIC_BLOCK_SIZE_			1024

#define NDM_CORE_REQUEST_STATIC_COMMAND_BUFFER_SIZE_	512

#define NDM_CORE_REQUEST_ATTR_NAME_						"name"
#define NDM_CORE_REQUEST_ATTR_PREFIX_					'@'

#define NDM_CORE_RESPONSE_STATIC_SIZE_					1024
#define NDM_CORE_RESPONSE_INITIAL_BUFFER_SIZE_			2048
#define NDM_CORE_RESPONSE_DYNAMIC_BLOCK_SIZE_			4096

#define NDM_CORE_RESPONSE_ID_INITIALIZER_				0

#define NDM_CORE_RESPONSE_STATIC_PATH_BUFFER_SIZE_		256

#define NDM_CORE_EVENT_CONNECTION_BUFFER_SIZE_			4096
#define NDM_CORE_EVENT_INITIAL_BUFFER_SIZE_				1024
#define NDM_CORE_EVENT_DYNAMIC_BLOCK_SIZE_				1024

#define NDM_CORE_MESSAGE_STRING_MAX_SIZE_				512
#define NDM_CORE_MESSAGE_IDENT_MAX_SIZE_				128
#define NDM_CORE_MESSAGE_SOURCE_MAX_SIZE_				128

#define NDM_CORE_CTRL_NODE_								0
#define NDM_CORE_CTRL_ATTR_								1
#define NDM_CORE_CTRL_SIBL_								2
#define NDM_CORE_CTRL_END_								3

NDM_BUILD_ASSERT(message_string_size, NDM_CORE_MESSAGE_STRING_MAX_SIZE_ > 0);
NDM_BUILD_ASSERT(message_ident_size, NDM_CORE_MESSAGE_IDENT_MAX_SIZE_ > 0);
NDM_BUILD_ASSERT(message_source_size, NDM_CORE_MESSAGE_SOURCE_MAX_SIZE_ > 0);

typedef uint8_t ndm_core_ctrl_t;
typedef uint32_t ndm_core_size_t;
typedef uint64_t ndm_core_response_id_t;

struct ndm_core_buffer_t
{
	uint8_t *start;
	uint8_t *bound;
	uint8_t *getp;
	uint8_t *putp;
};

struct ndm_core_cache_entry_t
{
	struct ndm_dlist_entry_t list;
	struct ndm_core_response_t *response;
	struct timespec expiration_time;
	struct ndm_core_cache_t *owner;
	size_t request_size;
	uint8_t request[];
};

struct ndm_core_cache_t
{
	const int ttl_msec;
	const size_t max_size;
	struct timespec next_expiration_time;
	size_t size;
	struct ndm_dlist_entry_t entries;
};

struct ndm_core_message_t
{
	bool received;
	char string[NDM_CORE_MESSAGE_STRING_MAX_SIZE_];
	char ident[NDM_CORE_MESSAGE_IDENT_MAX_SIZE_];
	char source[NDM_CORE_MESSAGE_SOURCE_MAX_SIZE_];
	ndm_code_t code;
	enum ndm_core_response_type_t type;
	ndm_core_response_id_t response_id;
};

struct ndm_core_t
{
	int fd;
	int timeout;
	const char *agent;
	uint8_t buffer_storage[NDM_CORE_CONNECTION_BUFFER_SIZE_];
	struct ndm_core_buffer_t buffer;
	struct ndm_core_cache_t cache;
	struct ndm_core_message_t last_message;
	ndm_core_response_id_t response_id;
};

struct ndm_core_response_t
{
	uint8_t buffer[NDM_CORE_RESPONSE_INITIAL_BUFFER_SIZE_];
	struct ndm_xml_document_t doc;
	struct ndm_xml_node_t *root;
	ndm_core_response_id_t id;
};

struct ndm_core_event_connection_t
{
	int fd;
	int timeout;
	uint8_t buffer_storage[NDM_CORE_EVENT_CONNECTION_BUFFER_SIZE_];
	struct ndm_core_buffer_t buffer;
};

struct ndm_core_event_t
{
	uint8_t buffer[NDM_CORE_EVENT_INITIAL_BUFFER_SIZE_];
	struct ndm_xml_document_t doc;
	struct ndm_xml_node_t *root;
	const char *type;
	struct timespec raise_time;
};

/**
 * Core message functions.
 **/

static inline void __ndm_core_message_init(
		struct ndm_core_message_t *message)
{
	message->received = false;
	*message->string = '\0';
	*message->source = '\0';
	*message->ident = '\0';
	message->code = 0;
	message->type = NDM_CORE_INFO;
	message->response_id = NDM_CORE_RESPONSE_ID_INITIALIZER_;
}

static inline void __ndm_core_message_append_dots(
		char *dst,
		const size_t dst_size)
{
	static const char TRAILING_DOTS_[] = "...";

	snprintf(dst + dst_size - sizeof(TRAILING_DOTS_),
		sizeof(TRAILING_DOTS_), "%s", TRAILING_DOTS_);
}

static void __ndm_core_message_printf(
		char *dst,
		const size_t dst_size,
		const char *const src)
{
	const int size = snprintf(dst, dst_size, "%s", src);

	if ((size_t) size >= dst_size) {
		__ndm_core_message_append_dots(dst, dst_size);
	}
}

static void __ndm_core_message_update(
		struct ndm_core_message_t *message,
		struct ndm_core_response_t *response)
{
	if (response->id != message->response_id) {
		const struct ndm_xml_node_t *response_node =
			ndm_core_response_root(response);
		const struct ndm_xml_node_t *message_node = NULL;

		__ndm_core_message_init(message);

		message->response_id = response->id;

		if ((message_node = ndm_xml_node_first_child(
				response_node, "info")) != NULL ||
			(message_node = ndm_xml_node_first_child(
				response_node, "message")) != NULL)
		{
			message->type = NDM_CORE_INFO;
		} else
		if ((message_node = ndm_xml_node_first_child(
				response_node, "warning")) != NULL)
		{
			message->type = NDM_CORE_WARNING;
		} else
		if ((message_node = ndm_xml_node_first_child(
				response_node, "error")) != NULL)
		{
			message->type = NDM_CORE_ERROR;
		} else
		if ((message_node = ndm_xml_node_first_child(
				response_node, "critical")) != NULL)
		{
			message->type = NDM_CORE_CRITICAL;
		}

		if (message_node != NULL) {
			char *p = message->string;
			char *pend = p + sizeof(message->string);
			const char *m = (const char *) ndm_xml_node_value(message_node);
			const struct ndm_xml_attr_t *code_attr =
				ndm_xml_node_first_attr(message_node, "code");
			const struct ndm_xml_attr_t *ident_attr =
				ndm_xml_node_first_attr(message_node, "ident");
			const struct ndm_xml_attr_t *source_attr =
				ndm_xml_node_first_attr(message_node, "source");

			message->received = true;

			__ndm_core_message_printf(
				message->ident, sizeof(message->ident),
				(ident_attr == NULL) ?
				"" : ndm_xml_attr_value(ident_attr));

			__ndm_core_message_printf(
				message->source, sizeof(message->source),
				(source_attr == NULL) ?
				"" : ndm_xml_attr_value(source_attr));

			if (code_attr != NULL) {
				char end;

				if (sscanf(ndm_xml_attr_value(code_attr),
						"%" NDM_CODE_SCNu "%c", &message->code, &end) != 1)
				{
					/* failed to parse a message code */
					message->code = 0;
				}
			}

			/* a message string may contain argument references */

			do {
				if (((uint8_t) *(m + 0)) == 0xee &&
					((uint8_t) *(m + 1)) == 0x80 &&
					((uint8_t) *(m + 2)) >= 0x80)
				{
					/* UTF-8 codes 0xee8080-0xee80ff are arg. references */
					size_t arg_index = (size_t) (*(m + 2) - '\x80');
					struct ndm_xml_node_t *arg_node =
						ndm_xml_node_first_child(message_node, "argument");

					/* skip an argument reference */
					m += 3;

					while (arg_node != NULL && arg_index > 0) {
						arg_node = ndm_xml_node_next_sibling(
							arg_node, "argument");
						--arg_index;
					}

					if (arg_node == NULL) {
						/* ignore invalid argument reference */
					} else {
						const int arg_size = snprintf(
							p, (size_t) (pend - p), "%s",
							ndm_xml_node_value(arg_node));

						if (arg_size < 0) {
							/* ignore failed snprintf() call */
							*p = '\0';
						} else {
							p += NDM_MIN(pend - p, (ptrdiff_t) arg_size);
						}
					}
				} else {
					*p = *m;
					++p;
					++m;
				}
			} while (*m != '\0' && p < pend);

			if (p == pend) {
				--p;
				*p = '\0';

				/* message was truncated, append trailing dots */
				__ndm_core_message_append_dots(
					message->string,
					sizeof(message->string));
			} else {
				*p = '\0';
			}
		}
	}
}

/**
 * Core buffer functions.
 **/

static void __ndm_core_buffer_init(
		struct ndm_core_buffer_t *buffer,
		uint8_t *buffer_start,
		const size_t buffer_size)
{
	buffer->start = buffer_start;
	buffer->bound = buffer_start + buffer_size;
	buffer->getp = buffer->start;
	buffer->putp = buffer->start;
}

static inline bool __ndm_core_buffer_is_empty(
		const struct ndm_core_buffer_t *buffer)
{
	return buffer->getp == buffer->putp;
}

static bool __ndm_core_buffer_read_all(
		struct ndm_core_buffer_t *buffer,
		const int fd,
		const struct timespec *deadline,
		void *data,
		const size_t data_size)
{
	uint8_t *p = data;
	size_t s = data_size;
	bool error = false;

	while (s != 0 && !error) {
		if (__ndm_core_buffer_is_empty(buffer)) {
			/* there is no data in a buffer */
			const int delay = (int) ndm_time_left_monotonic_msec(deadline);

			buffer->putp = buffer->getp = buffer->start;

			if (delay < 0) {
				error = true;
				errno = ETIMEDOUT;
			} else {
				struct pollfd pfd =
				{
					.fd = fd,
					.events = POLLIN | POLLERR | POLLHUP | POLLNVAL,
					.revents = 0
				};
				ssize_t n = ndm_poll(&pfd, 1, delay);

				if (n > 0) {
					n = recv(fd, buffer->putp,
						(size_t) (buffer->bound - buffer->start), 0);

					if (n > 0) {
						/* NDM_HEX_DUMP(buffer->putp, n); */
						buffer->putp += n;
					} else
					if (n == 0) {
						error = true;
						errno = ECONNRESET;
					}
				}

				if (n < 0) {
					error = true;
				} else
				if (ndm_sys_is_interrupted()) {
					error = true;
					errno = EINTR;
				}
			}
		}

		if (!error) {
			const size_t copied = NDM_MIN(s,
				(size_t) (buffer->putp - buffer->getp));

			memcpy(p, buffer->getp, copied);
			p += copied;
			s -= copied;
			buffer->getp += copied;
		}
	}

	return !error;
}

/**
 * Common core functions.
 **/

static inline bool __ndm_core_read_ctrl(
		struct ndm_core_buffer_t *buffer,
		const int fd,
		const struct timespec *deadline,
		ndm_core_ctrl_t *ctrl,
		enum ndm_xml_node_type_t *type)
{
	uint8_t data;
	bool done = false;

	if (__ndm_core_buffer_read_all(
			buffer, fd, deadline, &data, sizeof(data)))
	{
		*ctrl = (ndm_core_ctrl_t) ((data >> 6) & 0x03);
		*type = (enum ndm_xml_node_type_t) (data & 0x3f);

		if (*type > NDM_XML_NODE_TYPE_PI) {
			errno = EBADMSG;
		} else {
			done = true;
		}
	}

	return done;
}

static bool __ndm_core_read_string(
		struct ndm_core_buffer_t *buffer,
		const int fd,
		const struct timespec *deadline,
		struct ndm_xml_document_t *doc,
		const char **s)
{
	bool done = false;
	ndm_core_size_t size = 0;

	if (__ndm_core_buffer_read_all(
			buffer, fd, deadline, &size, sizeof(size)))
	{
		size = ntohl(size);

		if (size == 0) {
			*s = "";
			done = true;
		} else {
			char *p = ndm_xml_document_alloc(doc, size + 1);

			if (p != NULL &&
				__ndm_core_buffer_read_all(
					buffer, fd, deadline, p, size))
			{
				p[size] = '\0';
				*s = p;
				done = true;
			}
		}
	}

	return done;
}

static bool __ndm_core_read_xml_children(
		const int fd,
		struct ndm_core_buffer_t *buffer,
		const struct timespec *deadline,
		struct ndm_xml_node_t *root_parent)
{
	struct ndm_xml_document_t *doc = ndm_xml_node_document(root_parent);
	struct ndm_xml_node_t *node = root_parent;
	struct ndm_xml_node_t *root = NULL;
	size_t ctrl_index = 0;
	bool error = false;
	bool stopped = false;

	do {
		ndm_core_ctrl_t ctrl = NDM_CORE_CTRL_END_;
		enum ndm_xml_node_type_t type = NDM_XML_NODE_TYPE_ELEMENT;
		struct ndm_xml_node_t *parent = ndm_xml_node_parent(node);

		++ctrl_index;

		if (!__ndm_core_read_ctrl(buffer, fd, deadline, &ctrl, &type)) {
			error = true;
		} else
		if (ctrl == NDM_CORE_CTRL_NODE_ || ctrl == NDM_CORE_CTRL_SIBL_) {
			struct ndm_xml_node_t *new_node = NULL;
			const char *name;
			const char *value;

			if (type == NDM_XML_NODE_TYPE_DOCUMENT) {
				if (ctrl_index > 1 ||
					node != root_parent ||
					ndm_xml_node_type(node) != NDM_XML_NODE_TYPE_DOCUMENT)
				{
					/* XML document node in a middle of a stream or
					 * a given root node is not a document node */
					errno = EBADMSG;
					error = true;
				} else
				if (ctrl != NDM_CORE_CTRL_NODE_) {
					/* invalid control for a document node */
					errno = EBADMSG;
					error = true;
				} else
				if (!__ndm_core_read_string(buffer,
						fd, deadline, doc, &name) ||
					!__ndm_core_read_string(buffer,
						fd, deadline, doc, &value))
				{
					error = true;
				} else {
					ndm_xml_node_set_name(node, name);
					ndm_xml_node_set_value(node, value);
					root = node;
				}
			} else
			if (ctrl == NDM_CORE_CTRL_SIBL_ && parent == NULL) {
				errno = EBADMSG;
				error = true;
			} else
			if (!__ndm_core_read_string(buffer, fd, deadline, doc, &name) ||
				!__ndm_core_read_string(buffer, fd, deadline, doc, &value) ||
				(new_node = ndm_xml_document_alloc_node(
					doc, type, name, value)) == NULL)
			{
				/* failed to read or allocate new node contents */
				error = true;
			} else
			if ((*name == '\0' &&
				 (type == NDM_XML_NODE_TYPE_ELEMENT ||
				  type == NDM_XML_NODE_TYPE_PI)) ||
				(*name != '\0' &&
				 (type == NDM_XML_NODE_TYPE_DATA ||
				  type == NDM_XML_NODE_TYPE_CDATA ||
				  type == NDM_XML_NODE_TYPE_COMMENT ||
				  type == NDM_XML_NODE_TYPE_DECLARATION ||
				  type == NDM_XML_NODE_TYPE_DOCTYPE)))
			{
				error = true;
				errno = EBADMSG;
			} else {
				if (ctrl == NDM_CORE_CTRL_NODE_) {
					ndm_xml_node_append_child(node, new_node);
					parent = node;
				} else {
					ndm_xml_node_append_child(parent, new_node);
				}

				node = new_node;

				if (root == NULL) {
					root = node;
				}

				if (ndm_xml_node_type(node) == NDM_XML_NODE_TYPE_DATA &&
					*ndm_xml_node_value(parent) == '\0')
				{
					ndm_xml_node_set_value(parent, ndm_xml_node_value(node));
				}
			}
		} else
		if (ctrl == NDM_CORE_CTRL_ATTR_) {
			if (parent == NULL) {
				/* failed to add an attribute to the root node */
				error = true;
				errno = EBADMSG;
			} else {
				struct ndm_xml_attr_t *new_attr;
				const char *name;
				const char *value;

				if (!__ndm_core_read_string(
						buffer, fd, deadline, doc, &name) ||
					!__ndm_core_read_string(
						buffer, fd, deadline, doc, &value) ||
					(new_attr = ndm_xml_document_alloc_attr(
						doc, name, value)) == NULL)
				{
					/* failed to read or allocate a new node attribute */
					error = true;
				} else
				if (*name == '\0') {
					error = true;
					errno = EBADMSG;
				} else {
					ndm_xml_node_append_attr(node, new_attr);
				}
			}
		} else
		if (ctrl == NDM_CORE_CTRL_END_) {
			stopped = (node == root) || (ndm_xml_node_parent(node) == root);
			node = parent;
		} else {
			/* bad binary XML command */
			error = true;
			errno = EBADMSG;
		}
	} while (!error && !stopped);

	return !error;
}

/**
 * Core event connection functions.
 **/

struct ndm_core_event_connection_t *ndm_core_event_connection_open(
		const int timeout)
{
	struct ndm_core_event_connection_t *connection =
		malloc(sizeof(*connection));

	if (connection == NULL) {
		errno = ENOMEM;
	} else {
		bool connected = false;

		if ((connection->fd = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
		{
			/* failed to open a new UNIX domain socket */

		} else {
			struct sockaddr_un sa;
			int len = 0;

			sa.sun_family = AF_UNIX;
			len = snprintf(sa.sun_path, sizeof(sa.sun_path), "%s",
				NDM_CORE_EVENT_SOCKET_);

			if (len < 0 || len >= sizeof(sa.sun_path)) {
				errno = ENOBUFS;
			} else {
				len += (int)offsetof(struct sockaddr_un, sun_path) + 1;

				if (connect(connection->fd,
					(struct sockaddr *) &sa,
					(socklen_t)len) == 0)
				{
					connected = true;
					connection->timeout = timeout;
					__ndm_core_buffer_init(&connection->buffer,
						connection->buffer_storage,
						sizeof(connection->buffer_storage));
				}
			}
		}

		if (!connected) {
			close(connection->fd);
			free(connection);
			connection = NULL;
		}
	}

	return connection;
}

bool ndm_core_event_connection_close(
		struct ndm_core_event_connection_t **connection)
{
	int n = 0;

	if (connection != NULL && *connection != NULL) {
		do {
			n = close((*connection)->fd);
		} while (n != 0 && errno == EINTR);

		free(*connection);
		*connection = NULL;
	}

	return (n == 0) ? true : false;
}

int ndm_core_event_connection_fd(
		const struct ndm_core_event_connection_t *connection)
{
	return connection->fd;
}

bool ndm_core_event_connection_has_events(
		struct ndm_core_event_connection_t *connection)
{
	return !__ndm_core_buffer_is_empty(&connection->buffer);
}

struct ndm_core_event_t *ndm_core_event_connection_get(
		struct ndm_core_event_connection_t *connection)
{
	struct ndm_core_event_t *event = malloc(sizeof(*event));

	if (event == NULL) {
		errno = ENOMEM;
	} else {
		bool done = false;
		struct timespec deadline;
		struct ndm_xml_node_t *root = NULL;

		ndm_time_get_monotonic_plus_msec(&deadline, connection->timeout);

		ndm_xml_document_init(
			&event->doc, event->buffer, sizeof(event->buffer),
			NDM_CORE_EVENT_DYNAMIC_BLOCK_SIZE_);

		/* read a whole document with a root node */

		if ((root = ndm_xml_document_alloc_root(&event->doc)) != NULL &&
			__ndm_core_read_xml_children(
				connection->fd, &connection->buffer, &deadline, root))
		{
			struct ndm_xml_attr_t *event_type;
			struct ndm_xml_attr_t *raise_time;

			event->root = ndm_xml_node_first_child(
				ndm_xml_document_root(&event->doc), "event");

			if (event->root == NULL ||
				(event_type = ndm_xml_node_first_attr(
					event->root, "class")) == NULL ||
				*(event->type = ndm_xml_attr_value(event_type)) == '\0' ||
				(raise_time = ndm_xml_node_first_attr(
					event->root, "raise_time")) == NULL ||
				*ndm_xml_attr_value(raise_time) == '\0')
			{
				errno = EBADMSG;
			} else {
				const char *const raise_time_value =
					ndm_xml_attr_value(raise_time);
				char end;
				long seconds = 0;
				long milliseconds = 0;

				if ((strchr(raise_time_value, '.') == NULL &&
					 sscanf(raise_time_value, "%ld%c",
						&seconds, &end) == 1) ||
					 sscanf(raise_time_value, "%ld.%ld%c",
						&seconds, &milliseconds, &end) == 2)
				{
					event->raise_time.tv_sec = (time_t) seconds;
					event->raise_time.tv_nsec = milliseconds*NDM_TIME_MSEC;

					done = true;
				} else {
					errno = EBADMSG;
				}
			}
		}

		if (!done) {
			ndm_core_event_free(&event);
		}
	}

	return event;
}

const struct ndm_xml_node_t *ndm_core_event_root(
		const struct ndm_core_event_t *event)
{
	return event->root;
}

const char *ndm_core_event_type(
		const struct ndm_core_event_t *event)
{
	return event->type;
}

struct timespec ndm_core_event_raise_time(
		const struct ndm_core_event_t *event)
{
	return event->raise_time;
}

void ndm_core_event_free(
		struct ndm_core_event_t **event)
{
	if (event != NULL && *event != NULL) {
		ndm_xml_document_clear(&(*event)->doc);
		free(*event);
		*event = NULL;
	}
}

/**
 * Core cache functions.
 **/

static struct ndm_core_response_t *__ndm_core_response_copy(
		const struct ndm_core_response_t *response) NDM_ATTR_WUR;

static inline size_t __ndm_core_response_size(
		const struct ndm_core_response_t *response) NDM_ATTR_WUR;

static inline size_t __ndm_core_cache_entry_size(
		const size_t request_size,
		const struct ndm_core_response_t *response)
{
	return
		request_size +
		sizeof(struct ndm_core_cache_entry_t) +
		__ndm_core_response_size(response);
}

static void __ndm_core_cache_entry_remove(
		struct ndm_core_cache_entry_t *e)
{
	ndm_dlist_remove(&e->list);
	e->owner->size -=
		__ndm_core_cache_entry_size(e->request_size, e->response);
	ndm_core_response_free(&e->response);
	free(e);
}

static void __ndm_core_cache_init(
		struct ndm_core_cache_t *cache,
		const int ttl_msec,
		const size_t max_size)
{
	*((int *) &cache->ttl_msec) = ttl_msec;
	*((size_t *) &cache->max_size) = max_size;
	ndm_time_get_max(&cache->next_expiration_time);
	cache->size = 0;
	ndm_dlist_init(&cache->entries);
}

static inline void __ndm_core_cache_remove_last(
		struct ndm_core_cache_t *cache)
{
	__ndm_core_cache_entry_remove(
		ndm_dlist_entry(cache->entries.prev,
			struct ndm_core_cache_entry_t, list));
}

void ndm_core_cache_clear(
		struct ndm_core_t *core,
		const bool remove_all)
{
	struct ndm_core_cache_t *cache = &core->cache;

	if (remove_all) {
		while (!ndm_dlist_is_empty(&cache->entries)) {
			__ndm_core_cache_remove_last(cache);
		}

		ndm_time_get_max(&cache->next_expiration_time);
	} else {
		/* remove expired entries only */
		struct timespec now;

		ndm_time_get_monotonic(&now);

		if (ndm_time_less(&cache->next_expiration_time, &now)) {
			struct ndm_core_cache_entry_t *e;
			struct ndm_core_cache_entry_t *n;

			ndm_time_get_max(&cache->next_expiration_time);

			ndm_dlist_foreach_entry_safe(e,
				struct ndm_core_cache_entry_t,
				list, &cache->entries, n)
			{
				if (ndm_time_less(&e->expiration_time, &now)) {
					__ndm_core_cache_entry_remove(e);
				} else
				if (ndm_time_greater(
						&cache->next_expiration_time,
						&e->expiration_time))
				{
					cache->next_expiration_time = e->expiration_time;
				}
			}
		}
	}
}

static bool __ndm_core_cache_get(
		struct ndm_core_cache_t *cache,
		const uint8_t *request,
		const size_t request_size,
		const bool copy_cached_response,
		bool *response_copied,
		struct ndm_core_response_t **response)
{
	struct ndm_core_cache_entry_t *i;
	struct ndm_core_cache_entry_t *n;
	struct ndm_core_cache_entry_t *e = NULL;
	struct timespec now;
	bool error = false;

	ndm_time_get_monotonic(&now);

	ndm_dlist_foreach_entry_safe(i,
		struct ndm_core_cache_entry_t,
		list, &cache->entries, n)
	{
		if (ndm_time_less(&i->expiration_time, &now)) {
			/* remove and free an expired entry */
			__ndm_core_cache_entry_remove(i);
		} else
		if (i->request_size == request_size &&
			memcmp(i->request, request, request_size) == 0)
		{
			e = i;

			break;
		}
	}

	*response = NULL;

	if (response_copied != NULL) {
		*response_copied = false;
	}

	if (e != NULL) {
		/* cache hit */

		/* move a found entry to a head of a list */
		ndm_dlist_remove(&e->list);
		ndm_dlist_insert_after(&cache->entries, &e->list);

		if (copy_cached_response) {
			*response = __ndm_core_response_copy(e->response);

			if (*response == NULL) {
				error = true;
			}

			if (response_copied != NULL) {
				*response_copied = (*response != NULL);
			}
		} else {
			/* no real response copy */
			*response = e->response;
		}
	}

	return !error;
}

static void __ndm_core_cache(
		struct ndm_core_cache_t *cache,
		const uint8_t *request,
		const size_t request_size,
		const struct ndm_core_response_t *response)
{
	/* there is no check of duplicating entries here due to performance
	 * reasons; make sure that this function called only
	 * when __ndm_core_cache_get() failed because of a cache miss */

	/* try to add a new entry */
	const size_t need_size =
		__ndm_core_cache_entry_size(request_size, response);

	if (cache->max_size >= need_size) {
		/* the response can be cached */
		struct ndm_core_cache_entry_t *e = NULL;

		while (cache->max_size - cache->size < need_size) {
			__ndm_core_cache_remove_last(cache);
		}

		e = malloc(sizeof(*e) + request_size);

		if (e != NULL) {
			if ((e->response = __ndm_core_response_copy(response)) == NULL) {
				/* failed to cache */
				free(e);
			} else {
				ndm_dlist_init(&e->list);
				e->owner = cache;
				e->request_size = request_size;
				memcpy(e->request, request, request_size);

				ndm_time_get_monotonic(&e->expiration_time);
				ndm_time_add_msec(&e->expiration_time, cache->ttl_msec);

				e->owner->size += need_size;

				if (ndm_dlist_is_empty(&cache->entries)) {
					cache->next_expiration_time = e->expiration_time;
				}

				ndm_dlist_insert_after(&cache->entries, &e->list);
			}
		}
	}
}

/**
 * Core connection functions.
 **/

struct ndm_core_t *ndm_core_open(
		const char *const agent,
		const int cache_ttl_msec,
		const size_t cache_max_size)
{
	struct ndm_core_t *core = malloc(sizeof(*core));
	const char *current_agent =
		((agent == NULL) || (*agent == '\0')) ?
		NDM_CORE_DEFAULT_AGENT_ : agent;

	if (core == NULL) {
		errno = ENOMEM;
	} else {
		bool connected = false;

		__ndm_core_buffer_init(
			&core->buffer,
			core->buffer_storage,
			sizeof(core->buffer_storage));

		__ndm_core_cache_init(&core->cache, cache_ttl_msec, cache_max_size);
		__ndm_core_message_init(&core->last_message);

		if ((core->agent = ndm_string_dup(current_agent)) == NULL) {
			errno = ENOMEM;
		} else {
			if ((core->fd = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
				/* failed to open a new UNIX stream socket */
			} else {
				struct sockaddr_un sa;
				int len = 0;

				sa.sun_family = AF_UNIX;
				len = snprintf(sa.sun_path, sizeof(sa.sun_path), "%s",
					NDM_CORE_SOCKET_);

				if (len < 0 || len >= sizeof(sa.sun_path)) {
					errno = ENOBUFS;
					close(core->fd);
					core->fd = -1;
				} else {
					len += (int)offsetof(struct sockaddr_un, sun_path) + 1;

					if (connect(core->fd,
						(struct sockaddr *) &sa,
						(socklen_t)len) != 0) {
						/* failed to parse a defined core address or connect */
						close(core->fd);
						core->fd = -1;
					} else {
						connected = true;
						core->timeout = NDM_CORE_DEFAULT_TIMEOUT;
						core->response_id = NDM_CORE_RESPONSE_ID_INITIALIZER_;
					}
				}
			}
		}

		if (!connected) {
			ndm_core_close(&core);
		}
	}

	return core;
}

bool ndm_core_close(
		struct ndm_core_t **core)
{
	int n = 0;

	if (core != NULL && *core != NULL) {
		struct ndm_core_t *c = *core;

		if (c->fd >= 0) {
			do {
				n = close(c->fd);
			} while (n != 0 && errno == EINTR);
		}

		ndm_core_cache_clear(c, true);
		free((void *) c->agent);
		free(c);
		*core = NULL;
	}

	return (n == 0) ? true : false;
}

int ndm_core_fd(
		const struct ndm_core_t *core)
{
	return core->fd;
}

void ndm_core_set_timeout(
		struct ndm_core_t *core,
		const int timeout)
{
	core->timeout = timeout;
}

int ndm_core_get_timeout(
		const struct ndm_core_t *core)
{
	return core->timeout;
}

const char *ndm_core_agent(
		const struct ndm_core_t *core)
{
	return core->agent;
}

static inline size_t __ndm_core_request_store_ctrl(
		uint8_t **p,
		const uint8_t *end,
		const ndm_core_ctrl_t ctrl,
		const enum ndm_xml_node_type_t node_type)
{
	size_t size = sizeof(ndm_core_ctrl_t);

	if (p != NULL) {
		if (*p == NULL || end - *p < size) {
			size = 0;
			*p = NULL;
		} else {
			*((ndm_core_ctrl_t *) *p) =
				((ndm_core_ctrl_t) (ctrl << 6)) |
				((ndm_core_ctrl_t) node_type);
			(*p)++;
		}
	}

	return size;
}

static inline size_t __ndm_core_request_store_str(
		uint8_t **p,
		const uint8_t *end,
		const char *const str,
		const size_t str_size)
{
	size_t size = sizeof(ndm_core_size_t) + str_size;

	if (p != NULL) {
		if (*p == NULL || end - *p < size) {
			size = 0;
			*p = NULL;
		} else {
			*((ndm_core_size_t *) (*p)) =
				(ndm_core_size_t) htonl((uint32_t) str_size);
			(*p) += sizeof(ndm_core_size_t);
			memcpy(*p, str, str_size);
			(*p) += str_size;
		}
	}

	return size;
}

static inline size_t __ndm_core_request_store_base(
		uint8_t **p,
		const uint8_t *end,
		const ndm_core_ctrl_t ctrl,
		const enum ndm_xml_node_type_t node_type,
		const char *const name,
		const size_t name_size,
		const char *const value,
		const size_t value_size)
{
	return
		__ndm_core_request_store_ctrl(p, end, ctrl, node_type) +
		__ndm_core_request_store_str(p, end, name, name_size) +
		__ndm_core_request_store_str(p, end, value, value_size);
}

static size_t __ndm_core_request_store_node(
		const struct ndm_xml_node_t *node,
		const ndm_core_ctrl_t node_ctrl,
		uint8_t **p,
		const uint8_t *end)
{
	size_t size = __ndm_core_request_store_base(
		p, end, node_ctrl, ndm_xml_node_type(node),
		ndm_xml_node_name(node), ndm_xml_node_name_size(node),
		ndm_xml_node_value(node), ndm_xml_node_value_size(node));
	struct ndm_xml_attr_t *attr = ndm_xml_node_first_attr(node, NULL);
	struct ndm_xml_node_t *child = ndm_xml_node_first_child(node, NULL);
	struct ndm_xml_node_t *first_child = child;

	while (attr != NULL) {
		size += __ndm_core_request_store_base(
			p, end, NDM_CORE_CTRL_ATTR_, 0,
			ndm_xml_attr_name(attr), ndm_xml_attr_name_size(attr),
			ndm_xml_attr_value(attr), ndm_xml_attr_value_size(attr));
		attr = ndm_xml_attr_next(attr, NULL);
	}

	while (child != NULL) {
		size += __ndm_core_request_store_node(child,
			(ndm_core_ctrl_t) ((child == first_child) ?
			NDM_CORE_CTRL_NODE_ : NDM_CORE_CTRL_SIBL_),
			p, end);
		child = ndm_xml_node_next_sibling(child, NULL);
	}

	if (ndm_xml_node_first_child(node, NULL) != NULL) {
		size += __ndm_core_request_store_ctrl(p, end, NDM_CORE_CTRL_END_, 0);
	}

	return size;
}

static size_t __ndm_core_request_store(
		const struct ndm_xml_node_t *root,
		uint8_t *buffer,
		const size_t buffer_size)
{
	size_t size = 0;
	uint8_t *start = buffer;
	uint8_t **p = (start == NULL) ? NULL : &start;

	if (root != NULL) {
		size = __ndm_core_request_store_node(
			root, NDM_CORE_CTRL_NODE_,
			p, start + buffer_size);

		if (p != NULL && *p == NULL) {
			/* internal error */
			size = 0;
		}
	}

	return size;
}

static struct ndm_core_response_t *__ndm_core_do_request(
		struct ndm_core_t *core,
		const enum ndm_core_cache_mode_t cache_mode,
		const bool copy_cached_response,
		struct ndm_xml_node_t *request,
		bool *response_copied)
{
	struct timespec deadline;
	const size_t request_size = __ndm_core_request_store(request, NULL, 0);
	uint8_t request_static_buffer[NDM_CORE_REQUEST_BINARY_STATIC_SIZE_];
	uint8_t *buffer = request_static_buffer;
	struct ndm_core_response_t *response = NULL;

	ndm_time_get_monotonic_plus_msec(&deadline, core->timeout);

	if (request_size == 0) {
		/* empty request */
		errno = EBADMSG;
	} else
	if (request_size > sizeof(request_static_buffer) &&
		(buffer = malloc(request_size)) == NULL)
	{
		errno = ENOMEM;
	} else
	if (__ndm_core_request_store(
			request, buffer, request_size) != request_size)
	{
		/* internal error occurred */
		errno = EILSEQ;
	} else {
		/* a request sequence is ready */

		if (cache_mode == NDM_CORE_MODE_CACHE &&
			!__ndm_core_cache_get(&core->cache, buffer,
				request_size, copy_cached_response,
				response_copied, &response))
		{
			/* failed to copy a response from a cache */
			errno = ENOMEM;
		} else
		if (response == NULL) {
			/* cache miss or noncached mode */

			ssize_t n = 0;
			size_t offs = 0;

			do {
				const int delay = (int)
					ndm_time_left_monotonic_msec(&deadline);

				if (delay < 0) {
					errno = ETIMEDOUT;
					n = -1;
				} else {
					struct pollfd pfd =
					{
						.fd = core->fd,
						.events = POLLOUT | POLLERR | POLLHUP | POLLNVAL,
						.revents = 0
					};

					n = ndm_poll(&pfd, 1, delay);

					if (n > 0) {
						n = send(
							core->fd, buffer + offs,
							request_size - offs, 0);

						if (n > 0) {
							/* NDM_HEX_DUMP(
								buffer + offs,
								request_size - offs); */
							offs += (size_t) n;
						}
					}

					if (ndm_sys_is_interrupted()) {
						errno = EINTR;
						n = -1;
					}
				}
			} while (offs != request_size && n >= 0);

			if (offs == request_size) {
				/* the request sequence was sent */
				if ((response = malloc(sizeof(*response))) == NULL) {
					errno = ENOMEM;
				} else {
					struct ndm_xml_node_t *root = NULL;

					ndm_xml_document_init(&response->doc,
						response->buffer, sizeof(response->buffer),
						NDM_CORE_RESPONSE_DYNAMIC_BLOCK_SIZE_);

					if ((root = ndm_xml_document_alloc_root(
							&response->doc)) == NULL ||
						!__ndm_core_read_xml_children(
							core->fd, &core->buffer, &deadline, root) ||
						(response->root = ndm_xml_node_first_child(
							root, "response")) == NULL)
					{
						ndm_core_response_free(&response);
					} else {
						response->id = ++core->response_id;

						if (cache_mode == NDM_CORE_MODE_CACHE &&
							!ndm_core_response_is_continued(response))
						{
							__ndm_core_cache(&core->cache,
								buffer, request_size, response);
						}

						/* this allocated response copy should be freed */
						if (response_copied != NULL) {
							*response_copied = true;
						}
					}
				}
			}
		}

		if (response != NULL) {
			__ndm_core_message_update(&core->last_message, response);
		}
	}

	if (buffer != request_static_buffer) {
		free(buffer);
	}

	return response;
}

static struct ndm_xml_node_t *__ndm_core_request_document_init(
		struct ndm_xml_document_t *doc,
		uint8_t *buffer,
		const size_t buffer_size,
		const char *const agent)
{
	struct ndm_xml_node_t *root = NULL;
	struct ndm_xml_node_t *request_node = NULL;

	ndm_xml_document_init(doc, buffer, buffer_size,
		NDM_CORE_REQUEST_DYNAMIC_BLOCK_SIZE_);

	if ((root = ndm_xml_document_alloc_root(doc)) == NULL ||
		(request_node = ndm_xml_node_append_child_str(
			root, "request", NULL)) == NULL ||
		ndm_xml_node_append_attr_str(request_node, "agent", agent) == NULL)
	{
		ndm_xml_document_clear(doc);
		request_node = NULL;
	}

	return request_node;
}

bool ndm_core_authenticate(
		struct ndm_core_t *core,
		const char *const user,
		const char *const password,
		const char *const realm,
		const char *const tag,
		bool *authenticated)
{
	bool done = false;
	uint8_t request_buffer[NDM_CORE_REQUEST_STATIC_SIZE_];
	struct ndm_xml_document_t request;
	struct ndm_xml_node_t *hello_node = NULL;
	struct ndm_xml_node_t *request_node =
		__ndm_core_request_document_init(&request,
			request_buffer, sizeof(request_buffer),
			core->agent);

	*authenticated = false;

	if (request_node != NULL &&
		(hello_node = ndm_xml_node_append_child_str(
			request_node, "hello", password)) != NULL &&
		ndm_xml_node_append_attr_str(hello_node, "name", user) != NULL &&
		ndm_xml_node_append_attr_str(hello_node, "realm", realm) != NULL &&
		ndm_xml_node_append_attr_str(hello_node, "tag", tag) != NULL)
	{
		struct ndm_core_response_t *response = __ndm_core_do_request(
			core, NDM_CORE_MODE_NO_CACHE, true, request_node, NULL);

		if (response != NULL) {
			const struct ndm_xml_node_t *response_node =
				ndm_core_response_root(response);

			*authenticated =
				(ndm_xml_node_first_child(response_node, "prompt") != NULL);

			ndm_core_response_free(&response);
			done = true;
		}
	}

	ndm_xml_document_clear(&request);

	return done;
}

bool ndm_core_find_command(
		struct ndm_core_t *core,
		const char *const command,
		bool* found)
{
	bool done = false;
	uint8_t request_buffer[NDM_CORE_REQUEST_STATIC_SIZE_];
	struct ndm_xml_document_t request;
	struct ndm_xml_node_t *probe_node = NULL;
	struct ndm_xml_node_t *request_node =
		__ndm_core_request_document_init(&request,
			request_buffer, sizeof(request_buffer),
			core->agent);

	*found = false;

	if (request_node != NULL &&
		(probe_node = ndm_xml_node_append_child_str(
			request_node, "probe", NULL)) != NULL &&
		ndm_xml_node_append_attr_str(probe_node, "name", command) != NULL)
	{
		struct ndm_core_response_t *response = __ndm_core_do_request(
			core, NDM_CORE_MODE_NO_CACHE, true, request_node, NULL);

		if (response != NULL) {
			const struct ndm_xml_node_t *response_node =
				ndm_core_response_root(response);
			const struct ndm_xml_node_t *probe_resp_node =
				ndm_xml_node_first_child(response_node, "probe");
			struct ndm_xml_attr_t *found_attr = NULL;

			if ( probe_resp_node == NULL ||
				(found_attr = ndm_xml_node_first_attr(
					probe_resp_node, "found")) == NULL)
			{
				errno = EBADMSG;
			} else {
				const char *const found_val = ndm_xml_attr_value(found_attr);

				if (strcasecmp(found_val, "yes") == 0) {
					done = true;
					*found = true;
				} else
				if (strcasecmp(found_val, "no") == 0) {
					done = true;
				} else {
					errno = EBADMSG;
				}
			}

			ndm_core_response_free(&response);
		}
	}

	ndm_xml_document_clear(&request);

	return done;
}

static struct ndm_core_response_t *__ndm_core_request(
		struct ndm_core_t *core,
		const enum ndm_core_request_type_t request_type,
		const enum ndm_core_cache_mode_t cache_mode,
		const bool copy_cached_response,
		bool *response_copied,
		const char *const command_args[],
		const char *const command_format,
		va_list ap)
{
	uint8_t request_buffer[NDM_CORE_REQUEST_STATIC_SIZE_];
	struct ndm_xml_document_t request;
	struct ndm_xml_node_t *request_node =
		__ndm_core_request_document_init(&request,
			request_buffer, sizeof(request_buffer),
			core->agent);
	struct ndm_core_response_t *response = NULL;

	if (request_node != NULL) {
		char command_buffer[NDM_CORE_REQUEST_STATIC_COMMAND_BUFFER_SIZE_];
		char *command = command_buffer;
		va_list aq;

		va_copy(aq, ap);
		ndm_vabsprintf(
			command_buffer, sizeof(command_buffer),
			&command, command_format, aq);
		va_end(aq);

		if (command != NULL) {
			if (request_type != NDM_CORE_REQUEST_CONFIG  &&
				request_type != NDM_CORE_REQUEST_EXECUTE &&
				request_type != NDM_CORE_REQUEST_PARSE)
			{
				errno = EINVAL;
			} else {
				bool has_args = false;
				struct ndm_xml_node_t *command_node = NULL;

				command_node = ndm_xml_node_append_child_str(
					request_node,
					(request_type == NDM_CORE_REQUEST_CONFIG)  ? "config"  :
					(request_type == NDM_CORE_REQUEST_EXECUTE) ? "command" :
																 "parse",
					(request_type == NDM_CORE_REQUEST_PARSE)   ? command   :
																 NULL);

				if (command_node == NULL) {
					errno = ENOMEM;
				} else
				if ((request_type == NDM_CORE_REQUEST_CONFIG ||
					 request_type == NDM_CORE_REQUEST_EXECUTE) &&
					 ndm_xml_node_append_attr_str(command_node,
						NDM_CORE_REQUEST_ATTR_NAME_, command) == NULL)
				{
					errno = ENOMEM;
					command_node = NULL;
				} else
				if (command_args != NULL) {
					size_t i = 0;

					while (
						command_node != NULL &&
						command_args[i] != NULL &&
						command_args[i + 1] != NULL)
					{
						const char *const name = command_args[i];
						const char *const value = command_args[i + 1];

						if (*name == NDM_CORE_REQUEST_ATTR_PREFIX_) {
							if (strcmp(
									name +
									sizeof(NDM_CORE_REQUEST_ATTR_PREFIX_),
									NDM_CORE_REQUEST_ATTR_NAME_) == 0)
							{
								/* reserved attribute */
								errno = EINVAL;
								command_node = NULL;
							} else
							if (ndm_xml_node_append_attr_str(
									command_node, name, value) == NULL)
							{
								errno = ENOMEM;
								command_node = NULL;
							}
						} else
						if (ndm_xml_node_append_child_str(
								command_node, name, value) == NULL)
						{
							errno = ENOMEM;
							command_node = NULL;
						} else {
							has_args = true;
						}

						i += 2;
					}

					if (command_node != NULL &&
						command_args[i] != NULL)
					{
						if (command_args[i + 1] == NULL) {
							errno = EINVAL;
						} else {
							/* ENOMEM or EINVAL */
						}

						command_node = NULL;
					}
				}

				if (command_node != NULL) {
					if (request_type == NDM_CORE_REQUEST_PARSE &&
						has_args)
					{
						/* no arguments allowed for "parse" request */
						errno = EINVAL;
						command_node = NULL;
					} else {
						response = __ndm_core_do_request(core,
							cache_mode, copy_cached_response,
							request_node, response_copied);
					}
				}
			}

			if (command != command_buffer) {
				free(command);
			}
		}
	}

	ndm_xml_document_clear(&request);

	return response;
}

static struct ndm_core_response_t *__ndm_core_request_cf(
		struct ndm_core_t *core,
		const enum ndm_core_request_type_t request_type,
		const enum ndm_core_cache_mode_t cache_mode,
		const bool copy_cached_response,
		bool *response_copied,
		const char *const command_args[],
		const char *const command_format,
		...)
{
	va_list ap;
	struct ndm_core_response_t *response = NULL;

	va_start(ap, command_format);
	response = __ndm_core_request(
		core, request_type, cache_mode, copy_cached_response,
		response_copied, command_args, command_format, ap);
	va_end(ap);

	return response;
}

struct ndm_core_response_t *ndm_core_request(
		struct ndm_core_t *core,
		const enum ndm_core_request_type_t request_type,
		const enum ndm_core_cache_mode_t cache_mode,
		const char *const command_args[],
		const char *const command_format,
		...)
{
	va_list ap;
	struct ndm_core_response_t *response = NULL;

	va_start(ap, command_format);
	response = __ndm_core_request(
		core, request_type, cache_mode, true,
		NULL, command_args, command_format, ap);
	va_end(ap);

	return response;
}

static struct ndm_core_response_t *__ndm_core_get_one_tag(
		struct ndm_core_t *core,
		const enum ndm_core_cache_mode_t cache_mode,
		const char *const tag,
		const char *const value)
{
	struct ndm_core_response_t *response = NULL;
	uint8_t request_buffer[NDM_CORE_REQUEST_STATIC_SIZE_];
	struct ndm_xml_document_t request;
	struct ndm_xml_node_t *request_node =
		__ndm_core_request_document_init(&request,
			request_buffer, sizeof(request_buffer),
			core->agent);

	if (request_node != NULL &&
		ndm_xml_node_append_child_str(request_node, tag, value) != NULL)
	{
		response = __ndm_core_do_request(core,
			cache_mode, true, request_node, NULL);
	}

	ndm_xml_document_clear(&request);

	return response;
}

struct ndm_core_response_t *ndm_core_get_help(
		struct ndm_core_t *core,
		const enum ndm_core_cache_mode_t cache_mode,
		const char *const command)
{
	return __ndm_core_get_one_tag(core,
		cache_mode, "help", command);
}

struct ndm_core_response_t *ndm_core_continue(
		struct ndm_core_t *core)
{
	return __ndm_core_get_one_tag(core,
		NDM_CORE_MODE_NO_CACHE, "continue", "");
}

struct ndm_core_response_t *ndm_core_break(
		struct ndm_core_t *core)
{
	return __ndm_core_get_one_tag(core,
		NDM_CORE_MODE_NO_CACHE, "break", "");
}

bool ndm_core_last_message_received(
		struct ndm_core_t *core)
{
	return core->last_message.received;
}

enum ndm_core_response_type_t ndm_core_last_message_type(
		struct ndm_core_t *core)
{
	return core->last_message.type;
}

const char *ndm_core_last_message_string(
		struct ndm_core_t *core)
{
	return core->last_message.string;
}

const char *ndm_core_last_message_ident(
		struct ndm_core_t *core)
{
	return core->last_message.ident;
}

const char *ndm_core_last_message_source(
		struct ndm_core_t *core)
{
	return core->last_message.source;
}

ndm_code_t ndm_core_last_message_code(
		struct ndm_core_t *core)
{
	return core->last_message.code;
}

void ndm_core_response_free(
		struct ndm_core_response_t **response)
{
	if (response != NULL && *response != NULL) {
		ndm_xml_document_clear(&(*response)->doc);
		free(*response);
		*response = NULL;
	}
}

bool ndm_core_response_is_ok(
		const struct ndm_core_response_t *response)
{
	const enum ndm_core_response_type_t type =
		ndm_core_response_type(response);

	return type == NDM_CORE_INFO || type == NDM_CORE_WARNING;
}

enum ndm_core_response_type_t ndm_core_response_type(
		const struct ndm_core_response_t *response)
{
	enum ndm_core_response_type_t type = NDM_CORE_INFO;

	if (ndm_xml_node_first_child(response->root, "error") != NULL) {
		type = NDM_CORE_ERROR;
	} else
	if (ndm_xml_node_first_child(response->root, "critical") != NULL) {
		type = NDM_CORE_CRITICAL;
	} else
	if (ndm_xml_node_first_child(response->root, "warning") != NULL) {
		type = NDM_CORE_WARNING;
	} else {
		/* NDM_CORE_INFO */
	}

	return type;
}

bool ndm_core_response_is_continued(
		const struct ndm_core_response_t *response)
{
	return ndm_xml_node_first_child(response->root, "continued") != NULL;
}

const struct ndm_xml_node_t *ndm_core_response_root(
		const struct ndm_core_response_t *response)
{
	return response->root;
}

static struct ndm_core_response_t *__ndm_core_response_copy(
		const struct ndm_core_response_t *response)
{
	struct ndm_core_response_t *copy = malloc(sizeof(*response));

	if (copy != NULL) {
		bool error = false;

		copy->id = response->id;

		ndm_xml_document_init(&copy->doc,
			copy->buffer, sizeof(copy->buffer),
			NDM_CORE_RESPONSE_DYNAMIC_BLOCK_SIZE_);

		if (!ndm_xml_document_copy(&copy->doc, &response->doc)) {
			error = true;
			errno = ENOMEM;
		} else
		if ((copy->root = ndm_xml_node_first_child(
				ndm_xml_document_root(&copy->doc), "response")) == NULL)
		{
			error = true;
			errno = EINVAL;
		}

		if (error) {
			ndm_xml_document_clear(&copy->doc);
			free(copy);
			copy = NULL;
		}
	}

	return copy;
}

static inline size_t __ndm_core_response_size(
		const struct ndm_core_response_t *response)
{
	return (response == NULL) ?
		0 : sizeof(*response) + ndm_xml_document_size(&response->doc);
}

static inline void __ndm_core_response_find_end(char **p)
{
	char *q = *p;

	while (*q != '/' && *q != NDM_CORE_REQUEST_ATTR_PREFIX_ && *q != '\0') {
		++q;
	}

	*p = q;
}

static enum ndm_core_response_error_t __ndm_core_response_first_node(
		const struct ndm_xml_node_t *node,
		const struct ndm_xml_node_t **value,
		char *value_path)
{
	enum ndm_core_response_error_t e = NDM_CORE_RESPONSE_ERROR_OK;
	char *p = value_path;

	*value = node;

	do {
		const char *node_name = p;

		__ndm_core_response_find_end(&p);

		if (*p == NDM_CORE_REQUEST_ATTR_PREFIX_) {
			/* no attribute reference allowed here */
			e = NDM_CORE_RESPONSE_ERROR_SYNTAX;
		} else {
			if (*p == '/') {
				*p = '\0';
				++p;
			}

			if (*node_name != '\0' &&
				(*value = ndm_xml_node_first_child(
					*value, node_name)) == NULL)
			{
				/* there is no such a child */
				e = NDM_CORE_RESPONSE_ERROR_NOT_FOUND;
			}
		}
	} while (*p != '\0' && e == NDM_CORE_RESPONSE_ERROR_OK);

	return e;
}

enum ndm_core_response_error_t ndm_core_response_first_node(
		const struct ndm_xml_node_t *node,
		const struct ndm_xml_node_t **value,
		const char *const value_path_format,
		...)
{
	enum ndm_core_response_error_t e = NDM_CORE_RESPONSE_ERROR_SYSTEM;
	char path_buffer[NDM_CORE_RESPONSE_STATIC_PATH_BUFFER_SIZE_];
	char *value_path = path_buffer;
	va_list ap;

	va_start(ap, value_path_format);
	ndm_vabsprintf(
		path_buffer, sizeof(path_buffer),
		&value_path, value_path_format, ap);
	va_end(ap);

	if (value_path != NULL) {
		e = __ndm_core_response_first_node(node, value, value_path);

		if (value_path != path_buffer) {
			free(value_path);
		}
	}

	return e;
}

static enum ndm_core_response_error_t __ndm_core_response_first_str(
		const struct ndm_xml_node_t *node,
		const char **value,
		const char *const path_format,
		va_list ap)
{
	enum ndm_core_response_error_t e = NDM_CORE_RESPONSE_ERROR_SYSTEM;
	char path_buffer[NDM_CORE_RESPONSE_STATIC_PATH_BUFFER_SIZE_];
	char *path = path_buffer;
	int path_size = -1;
	va_list aq;

	va_copy(aq, ap);
	path_size = ndm_vabsprintf(
		path_buffer, sizeof(path_buffer),
		&path, path_format, aq);
	va_end(aq);

	if (path != NULL) {
		const struct ndm_xml_node_t *n = node;
		char *p = path + path_size - 1;
		char *attr_name = NULL;

		while (
			path < p &&
			*p != NDM_CORE_REQUEST_ATTR_PREFIX_ &&
			*p != '/')
		{
			--p;
		}

		if (path <= p && *p == NDM_CORE_REQUEST_ATTR_PREFIX_) {
			*p = '\0';
			attr_name = p + 1;
		}

		e = __ndm_core_response_first_node(node, &n, path);

		if (e == NDM_CORE_RESPONSE_ERROR_OK) {
			if (attr_name == NULL) {
				*value = ndm_xml_node_value(n);
			} else {
				struct ndm_xml_attr_t *a =
					ndm_xml_node_first_attr(n, attr_name);

				if (a == NULL) {
					e = NDM_CORE_RESPONSE_ERROR_NOT_FOUND;
				} else {
					*value = ndm_xml_attr_value(a);
				}
			}
		}

		if (path != path_buffer) {
			free(path);
		}
	}

	return e;
}

enum ndm_core_response_error_t ndm_core_response_first_str(
		const struct ndm_xml_node_t *node,
		const char **value,
		const char *const path_format,
		...)
{
	va_list ap;
	enum ndm_core_response_error_t e = NDM_CORE_RESPONSE_ERROR_OK;

	va_start(ap, path_format);
	e = __ndm_core_response_first_str(node, value, path_format, ap);
	va_end(ap);

	return e;
}

#define NDM_CORE_RESPONSE_FIRST_INTEGER_(tabbr, type)						\
static enum ndm_core_response_error_t __ndm_core_response_first_##tabbr(	\
		const struct ndm_xml_node_t *node,									\
		type *value,														\
		const char *const path_format,										\
		va_list ap)															\
{																			\
	const char *str_value = NULL;											\
	enum ndm_core_response_error_t e =										\
		__ndm_core_response_first_str(node, &str_value, path_format, ap);	\
																			\
	if (e == NDM_CORE_RESPONSE_ERROR_OK &&									\
		!ndm_int_parse_##tabbr(str_value, value))							\
	{																		\
		e = NDM_CORE_RESPONSE_ERROR_FORMAT;									\
	}																		\
																			\
	return e;																\
}																			\
																			\
enum ndm_core_response_error_t ndm_core_response_first_##tabbr(				\
		const struct ndm_xml_node_t *node,									\
		type *value,														\
		const char *const path_format,										\
		...)																\
{																			\
	va_list ap;																\
	enum ndm_core_response_error_t e = NDM_CORE_RESPONSE_ERROR_OK;			\
																			\
	va_start(ap, path_format);												\
	e = __ndm_core_response_first_##tabbr(node, value, path_format, ap);	\
	va_end(ap);																\
																			\
	return e;																\
}																			\
																			\
static enum ndm_core_response_error_t __ndm_core_response_first_u##tabbr(	\
		const struct ndm_xml_node_t *node,									\
		unsigned type *value,												\
		const char *const path_format,										\
		va_list ap)															\
{																			\
	const char *str_value = NULL;											\
	enum ndm_core_response_error_t e =										\
		__ndm_core_response_first_str(node, &str_value, path_format, ap);	\
																			\
	if (e == NDM_CORE_RESPONSE_ERROR_OK &&									\
		!ndm_int_parse_u##tabbr(str_value, value))							\
	{																		\
		e = NDM_CORE_RESPONSE_ERROR_FORMAT;									\
	}																		\
																			\
	return e;																\
}																			\
																			\
enum ndm_core_response_error_t ndm_core_response_first_u##tabbr(			\
		const struct ndm_xml_node_t *node,									\
		unsigned type *value,												\
		const char *const path_format,										\
		...)																\
{																			\
	va_list ap;																\
	enum ndm_core_response_error_t e = NDM_CORE_RESPONSE_ERROR_OK;			\
																			\
	va_start(ap, path_format);												\
	e = __ndm_core_response_first_u##tabbr(node, value, path_format, ap);	\
	va_end(ap);																\
																			\
	return e;																\
}

NDM_CORE_RESPONSE_FIRST_INTEGER_(char, char)
NDM_CORE_RESPONSE_FIRST_INTEGER_(short, short)
NDM_CORE_RESPONSE_FIRST_INTEGER_(int, int)
NDM_CORE_RESPONSE_FIRST_INTEGER_(long, long)
NDM_CORE_RESPONSE_FIRST_INTEGER_(llong, long long)

static enum ndm_core_response_error_t __ndm_core_response_first_bool(
		const struct ndm_xml_node_t *node,
		const bool parse_value,
		bool *value,
		const char *const path_format,
		va_list ap)
{
	const char *str_value = NULL;
	enum ndm_core_response_error_t e =
		__ndm_core_response_first_str(node, &str_value, path_format, ap);

	if (!parse_value) {
		/* try to find only */
		if (e == NDM_CORE_RESPONSE_ERROR_OK) {
			/* a tag found */
			*value = true;
		} else
		if (e == NDM_CORE_RESPONSE_ERROR_NOT_FOUND) {
			/* no tag found */
			*value = false;
			e = NDM_CORE_RESPONSE_ERROR_OK;
		}
	} else
	if (e == NDM_CORE_RESPONSE_ERROR_OK) {
		long l;

		if (ndm_int_parse_long(str_value, &l)) {
			*value = (l == 0) ? false : true;
		} else
		if (strcasecmp(str_value, "yes") == 0 ||
			strcasecmp(str_value, "true") == 0 ||
			strcasecmp(str_value, "up") == 0 ||
			strcasecmp(str_value, "on") == 0)
		{
			*value = true;
		} else
		if (strcasecmp(str_value, "no") == 0 ||
			strcasecmp(str_value, "false") == 0 ||
			strcasecmp(str_value, "down") == 0 ||
			strcasecmp(str_value, "off") == 0)
		{
			*value = false;
		} else {
			e = NDM_CORE_RESPONSE_ERROR_FORMAT;
		}
	}

	return e;
}

enum ndm_core_response_error_t ndm_core_response_first_bool(
		const struct ndm_xml_node_t *node,
		const bool parse_value,
		bool *value,
		const char *const path_format,
		...)
{
	va_list ap;
	enum ndm_core_response_error_t e = NDM_CORE_RESPONSE_ERROR_OK;

	va_start(ap, path_format);
	e = __ndm_core_response_first_bool(node,
		parse_value, value, path_format, ap);
	va_end(ap);

	return e;
}

/**
 * The highest level core functions.
 **/

enum ndm_core_response_error_t ndm_core_request_break(
		struct ndm_core_t *core)
{
	struct ndm_core_response_t *response = ndm_core_break(core);
	enum ndm_core_response_error_t e = NDM_CORE_RESPONSE_ERROR_SYSTEM;

	if (response != NULL) {
		const struct ndm_xml_node_t *response_root =
			ndm_core_response_root(response);

		e = (ndm_xml_node_first_child(response_root, "prompt") == NULL) ?
			NDM_CORE_RESPONSE_ERROR_NOT_FOUND :
			NDM_CORE_RESPONSE_ERROR_OK;

		ndm_core_response_free(&response);
	}

	return e;
}

static bool __ndm_core_request_succeeded(
		struct ndm_core_t *core)
{
	bool ok = true;

	if (!ndm_core_last_message_received(core)) {
		/* no core message received, this is a valid response */
	} else
	if ((ndm_core_last_message_type(core) == NDM_CORE_ERROR) ||
		(ndm_core_last_message_type(core) == NDM_CORE_CRITICAL))
	{
		ok = false;
	}

	return ok;
}

enum ndm_core_response_error_t ndm_core_request_send(
		struct ndm_core_t *core,
		const enum ndm_core_request_type_t request_type,
		const char *const command_args[],
		const char *const command_format,
		...)
{
	va_list ap;
	struct ndm_core_response_t *response = NULL;
	enum ndm_core_response_error_t e = NDM_CORE_RESPONSE_ERROR_OK;

	va_start(ap, command_format);
	response = __ndm_core_request(
		core, request_type, NDM_CORE_MODE_NO_CACHE,
		true, NULL, command_args, command_format, ap);
	va_end(ap);

	if (response == NULL) {
		e = NDM_CORE_RESPONSE_ERROR_SYSTEM;
	} else {
		if (!__ndm_core_request_succeeded(core)) {
			e = NDM_CORE_RESPONSE_ERROR_MESSAGE;
		}

		ndm_core_response_free(&response);
	}

	return e;
}

/**
 * Functions with command formatting.
 **/

enum ndm_core_response_error_t ndm_core_request_first_str_alloc_cf(
		struct ndm_core_t *core,
		const enum ndm_core_request_type_t request_type,
		const enum ndm_core_cache_mode_t cache_mode,
		char **value,
		const char *const value_path,
		const char *const command_args[],
		const char *const command_format,
		...)
{
	va_list ap;
	bool response_copied = false;
	struct ndm_core_response_t *response = NULL;
	enum ndm_core_response_error_t e = NDM_CORE_RESPONSE_ERROR_OK;

	va_start(ap, command_format);
	response = __ndm_core_request(core, request_type, cache_mode,
		(cache_mode != NDM_CORE_MODE_CACHE), &response_copied,
		command_args, command_format, ap);
	va_end(ap);

	if (response == NULL) {
		e = NDM_CORE_RESPONSE_ERROR_SYSTEM;
	} else {
		if (!__ndm_core_request_succeeded(core)) {
			e = NDM_CORE_RESPONSE_ERROR_MESSAGE;
		} else {
			const char *response_value = NULL;

			e = ndm_core_response_first_str(
				ndm_core_response_root(response),
				&response_value, "%s", value_path);

			if (e == NDM_CORE_RESPONSE_ERROR_OK &&
				(*value = ndm_string_dup(response_value)) == NULL)
			{
				e = NDM_CORE_RESPONSE_ERROR_SYSTEM;
			}
		}

		if (response_copied) {
			ndm_core_response_free(&response);
		}
	}

	return e;
}

enum ndm_core_response_error_t ndm_core_request_first_str_buffer_cf(
		struct ndm_core_t *core,
		const enum ndm_core_request_type_t request_type,
		const enum ndm_core_cache_mode_t cache_mode,
		char *value,
		const size_t value_buffer_size,
		size_t *value_size,
		const char *const value_path,
		const char *const command_args[],
		const char *const command_format,
		...)
{
	va_list ap;
	bool response_copied = false;
	struct ndm_core_response_t *response = NULL;
	enum ndm_core_response_error_t e = NDM_CORE_RESPONSE_ERROR_OK;

	va_start(ap, command_format);
	response = __ndm_core_request(core, request_type, cache_mode,
		(cache_mode != NDM_CORE_MODE_CACHE), &response_copied,
		command_args, command_format, ap);
	va_end(ap);

	if (response == NULL) {
		e = NDM_CORE_RESPONSE_ERROR_SYSTEM;
	} else {
		if (!__ndm_core_request_succeeded(core)) {
			e = NDM_CORE_RESPONSE_ERROR_MESSAGE;
		} else {
			const char *response_value = NULL;

			e = ndm_core_response_first_str(
				ndm_core_response_root(response),
				&response_value, "%s", value_path);

			if (e == NDM_CORE_RESPONSE_ERROR_OK) {
				const int size = snprintf(
					value, value_buffer_size,
					"%s", response_value);

				if (size < 0) {
					e = NDM_CORE_RESPONSE_ERROR_SYSTEM;
				} else
				if (value_size != NULL) {
					*value_size = (size_t) size;
				}

				if (size >= (int) value_buffer_size) {
					/* a buffer contains truncated content;
					 * *value_size is a real content length */
					e = NDM_CORE_RESPONSE_ERROR_BUFFER_SIZE;
				}
			}
		}

		if (response_copied) {
			ndm_core_response_free(&response);
		}
	}

	return e;
}

enum ndm_core_response_error_t ndm_core_request_first_int_cf(
		struct ndm_core_t *core,
		const enum ndm_core_request_type_t request_type,
		const enum ndm_core_cache_mode_t cache_mode,
		int *value,
		const char *const value_path,
		const char *const command_args[],
		const char *const command_format,
		...)
{
	va_list ap;
	bool response_copied = false;
	struct ndm_core_response_t *response = NULL;
	enum ndm_core_response_error_t e = NDM_CORE_RESPONSE_ERROR_OK;

	va_start(ap, command_format);
	response = __ndm_core_request(core, request_type, cache_mode,
		(cache_mode != NDM_CORE_MODE_CACHE), &response_copied,
		command_args, command_format, ap);
	va_end(ap);

	if (response == NULL) {
		e = NDM_CORE_RESPONSE_ERROR_SYSTEM;
	} else {
		e = __ndm_core_request_succeeded(core) ?
			ndm_core_response_first_int(
				ndm_core_response_root(response),
				value, "%s", value_path) :
			NDM_CORE_RESPONSE_ERROR_MESSAGE;

		if (response_copied) {
			ndm_core_response_free(&response);
		}
	}

	return e;
}

enum ndm_core_response_error_t ndm_core_request_first_uint_cf(
		struct ndm_core_t *core,
		const enum ndm_core_request_type_t request_type,
		const enum ndm_core_cache_mode_t cache_mode,
		unsigned int *value,
		const char *const value_path,
		const char *const command_args[],
		const char *const command_format,
		...)
{
	va_list ap;
	bool response_copied = false;
	struct ndm_core_response_t *response = NULL;
	enum ndm_core_response_error_t e = NDM_CORE_RESPONSE_ERROR_OK;

	va_start(ap, command_format);
	response = __ndm_core_request(core, request_type, cache_mode,
		(cache_mode != NDM_CORE_MODE_CACHE), &response_copied,
		command_args, command_format, ap);
	va_end(ap);

	if (response == NULL) {
		e = NDM_CORE_RESPONSE_ERROR_SYSTEM;
	} else {
		e = __ndm_core_request_succeeded(core) ?
			ndm_core_response_first_uint(
				ndm_core_response_root(response),
				value, "%s", value_path) :
			NDM_CORE_RESPONSE_ERROR_MESSAGE;

		if (response_copied) {
			ndm_core_response_free(&response);
		}
	}

	return e;
}

enum ndm_core_response_error_t ndm_core_request_first_bool_cf(
		struct ndm_core_t *core,
		const enum ndm_core_request_type_t request_type,
		const enum ndm_core_cache_mode_t cache_mode,
		const bool parse_value,
		bool *value,
		const char *const value_path,
		const char *const command_args[],
		const char *const command_format,
		...)
{
	va_list ap;
	bool response_copied = false;
	struct ndm_core_response_t *response = NULL;
	enum ndm_core_response_error_t e = NDM_CORE_RESPONSE_ERROR_OK;

	va_start(ap, command_format);
	response = __ndm_core_request(core, request_type, cache_mode,
		(cache_mode != NDM_CORE_MODE_CACHE), &response_copied,
		command_args, command_format, ap);
	va_end(ap);

	if (response == NULL) {
		e = NDM_CORE_RESPONSE_ERROR_SYSTEM;
	} else {
		e = __ndm_core_request_succeeded(core) ?
			ndm_core_response_first_bool(
				ndm_core_response_root(response),
				parse_value, value, "%s", value_path) :
			NDM_CORE_RESPONSE_ERROR_MESSAGE;

		if (response_copied) {
			ndm_core_response_free(&response);
		}
	}

	return e;
}

/**
 * Functions with path formatting.
 **/

enum ndm_core_response_error_t ndm_core_request_first_str_alloc_pf(
		struct ndm_core_t *core,
		const enum ndm_core_request_type_t request_type,
		const enum ndm_core_cache_mode_t cache_mode,
		const char *const command_args[],
		const char *const command,
		char **value,
		const char *const value_path_format,
		...)
{
	bool response_copied = false;
	struct ndm_core_response_t *response = NULL;
	enum ndm_core_response_error_t e = NDM_CORE_RESPONSE_ERROR_OK;

	response = __ndm_core_request_cf(
		core, request_type, cache_mode,
		(cache_mode != NDM_CORE_MODE_CACHE), &response_copied,
		command_args, "%s", command);

	if (response == NULL) {
		e = NDM_CORE_RESPONSE_ERROR_SYSTEM;
	} else {
		if (!__ndm_core_request_succeeded(core)) {
			e = NDM_CORE_RESPONSE_ERROR_MESSAGE;
		} else {
			va_list ap;
			const char *response_value = NULL;

			va_start(ap, value_path_format);
			e = __ndm_core_response_first_str(
				ndm_core_response_root(response),
				&response_value, value_path_format, ap);
			va_end(ap);

			if (e == NDM_CORE_RESPONSE_ERROR_OK &&
				(*value = ndm_string_dup(response_value)) == NULL)
			{
				e = NDM_CORE_RESPONSE_ERROR_SYSTEM;
			}
		}

		if (response_copied) {
			ndm_core_response_free(&response);
		}
	}

	return e;
}

enum ndm_core_response_error_t ndm_core_request_first_str_buffer_pf(
		struct ndm_core_t *core,
		const enum ndm_core_request_type_t request_type,
		const enum ndm_core_cache_mode_t cache_mode,
		const char *const command_args[],
		const char *const command,
		char *value,
		const size_t value_buffer_size,
		size_t *value_size,
		const char *const value_path_format,
		...)
{
	bool response_copied = false;
	struct ndm_core_response_t *response = NULL;
	enum ndm_core_response_error_t e = NDM_CORE_RESPONSE_ERROR_OK;

	response = __ndm_core_request_cf(core, request_type, cache_mode,
		(cache_mode != NDM_CORE_MODE_CACHE), &response_copied,
		command_args, "%s", command);

	if (response == NULL) {
		e = NDM_CORE_RESPONSE_ERROR_SYSTEM;
	} else {
		if (!__ndm_core_request_succeeded(core)) {
			e = NDM_CORE_RESPONSE_ERROR_MESSAGE;
		} else {
			va_list ap;
			const char *response_value = NULL;

			va_start(ap, value_path_format);
			e = __ndm_core_response_first_str(
				ndm_core_response_root(response),
				&response_value, value_path_format, ap);
			va_end(ap);

			if (e == NDM_CORE_RESPONSE_ERROR_OK) {
				const int size = snprintf(
					value, value_buffer_size,
					"%s", response_value);

				if (size < 0) {
					e = NDM_CORE_RESPONSE_ERROR_SYSTEM;
				} else
				if (value_size != NULL) {
					*value_size = (size_t) size;
				}

				if (size >= (int) value_buffer_size) {
					/* a buffer contains truncated content;
					 * *value_size is a real content length */
					e = NDM_CORE_RESPONSE_ERROR_BUFFER_SIZE;
				}
			}
		}

		if (response_copied) {
			ndm_core_response_free(&response);
		}
	}

	return e;
}

enum ndm_core_response_error_t ndm_core_request_first_int_pf(
		struct ndm_core_t *core,
		const enum ndm_core_request_type_t request_type,
		const enum ndm_core_cache_mode_t cache_mode,
		const char *const command_args[],
		const char *const command,
		int *value,
		const char *const value_path_format,
		...)
{
	bool response_copied = false;
	struct ndm_core_response_t *response = NULL;
	enum ndm_core_response_error_t e = NDM_CORE_RESPONSE_ERROR_OK;

	response = __ndm_core_request_cf(core, request_type, cache_mode,
		(cache_mode != NDM_CORE_MODE_CACHE), &response_copied,
		command_args, "%s", command);

	if (response == NULL) {
		e = NDM_CORE_RESPONSE_ERROR_SYSTEM;
	} else {
		va_list ap;

		va_start(ap, value_path_format);
		e = __ndm_core_request_succeeded(core) ?
			__ndm_core_response_first_int(
				ndm_core_response_root(response),
				value, value_path_format, ap) :
			NDM_CORE_RESPONSE_ERROR_MESSAGE;
		va_end(ap);

		if (response_copied) {
			ndm_core_response_free(&response);
		}
	}

	return e;
}

enum ndm_core_response_error_t ndm_core_request_first_uint_pf(
		struct ndm_core_t *core,
		const enum ndm_core_request_type_t request_type,
		const enum ndm_core_cache_mode_t cache_mode,
		const char *const command_args[],
		const char *const command,
		unsigned int *value,
		const char *const value_path_format,
		...)
{
	bool response_copied = false;
	struct ndm_core_response_t *response = NULL;
	enum ndm_core_response_error_t e = NDM_CORE_RESPONSE_ERROR_OK;

	response = __ndm_core_request_cf(core, request_type, cache_mode,
		(cache_mode != NDM_CORE_MODE_CACHE), &response_copied,
		command_args, "%s", command);

	if (response == NULL) {
		e = NDM_CORE_RESPONSE_ERROR_SYSTEM;
	} else {
		va_list ap;

		va_start(ap, value_path_format);
		e = __ndm_core_request_succeeded(core) ?
			__ndm_core_response_first_uint(
				ndm_core_response_root(response),
				value, value_path_format, ap) :
			NDM_CORE_RESPONSE_ERROR_MESSAGE;
		va_end(ap);

		if (response_copied) {
			ndm_core_response_free(&response);
		}
	}

	return e;
}

enum ndm_core_response_error_t ndm_core_request_first_bool_pf(
		struct ndm_core_t *core,
		const enum ndm_core_request_type_t request_type,
		const enum ndm_core_cache_mode_t cache_mode,
		const char *const command_args[],
		const char *const command,
		const bool parse_value,
		bool *value,
		const char *const value_path_format,
		...)
{
	bool response_copied = false;
	struct ndm_core_response_t *response = NULL;
	enum ndm_core_response_error_t e = NDM_CORE_RESPONSE_ERROR_OK;

	response = __ndm_core_request_cf(core, request_type, cache_mode,
		(cache_mode != NDM_CORE_MODE_CACHE), &response_copied,
		command_args, "%s", command);

	if (response == NULL) {
		e = NDM_CORE_RESPONSE_ERROR_SYSTEM;
	} else {
		va_list ap;

		va_start(ap, value_path_format);
		e = __ndm_core_request_succeeded(core) ?
			__ndm_core_response_first_bool(
				ndm_core_response_root(response),
				parse_value, value, value_path_format, ap) :
			NDM_CORE_RESPONSE_ERROR_MESSAGE;
		va_end(ap);

		if (response_copied) {
			ndm_core_response_free(&response);
		}
	}

	return e;
}

