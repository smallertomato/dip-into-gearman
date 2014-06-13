#ifndef __GEARMAN_SERVER_H__
#define __GEARMAN_SERVER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

/* Defines. */
#define GEARMAN_DEFAULT_TCP_HOST "127.0.0.1"
#define GEARMAN_DEFAULT_TCP_PORT 7003
#define GEARMAN_DEFAULT_BACKLOG 32

/**
 * Return codes.
 */
typedef enum
{
  GEARMAN_SUCCESS,
  GEARMAN_ERRNO,
  GEARMAN_MEMORY_ALLOCATION_FAILURE,
  GEARMAN_GETADDRINFO,
  GEARMAN_EOF,
  GEARMAN_MAX_RETURN /* Always add new error code before */
} gearman_return_t;

typedef struct gearman_client_con gearman_client_con_st;
typedef struct gearman_client_st gearman_client_st;

struct gearman_client_con
{
  int fd;
  struct addrinfo *addrinfo;
  gearman_client_st *client;
};

struct gearman_client_st
{
  char host[NI_MAXHOST];
  in_port_t port;

  gearman_client_con_st * con;
  gearman_return_t ret;
};



gearman_client_st *gearman_client_create(gearman_client_st *client);
void gearman_client_free(gearman_client_st *client);

gearman_return_t gearman_client_add_server(gearman_client_st *client, const char *host, in_port_t port);
gearman_client_con_st* gearman_client_con_create(gearman_client_st *client, gearman_client_con_st *con);

#ifdef __cplusplus
}
#endif

#endif /* __GEARMAN_SERVER_H__ */

