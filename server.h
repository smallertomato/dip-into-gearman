#ifndef __GEARMAN_SERVER_H__
#define __GEARMAN_SERVER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
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
  GEARMAN_MAX_RETURN /* Always add new error code before */
} gearman_return_t;

struct gearmand
{
  in_port_t port;
  int backlog;
  int listen_fd;
  gearman_return_t ret;
};
typedef struct gearmand gearmand_st;

struct gearmand_con
{
  int fd;
  struct sockaddr_in sa;
  gearmand_st *gearmand;
};
typedef struct gearmand_con gearmand_con_st;


#ifdef __cplusplus
}
#endif

#endif /* __GEARMAN_SERVER_H__ */

