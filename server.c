#include "server.h"

static gearman_return_t _listen_init(gearmand_st *gearmand);

static void _listen_accept(int fd, void *arg);

int main(int argc, char *argv[])
{
  char c;
  in_port_t port= GEARMAN_DEFAULT_TCP_PORT;
  int backlog= 32;

  gearmand_st *gearmand;

  while ((c = getopt(argc, argv, "b:hp:")) != EOF)
  {
    switch(c)
    {
    case 'b':
      backlog= atoi(optarg);
      break;

    case 'p':
      port= (in_port_t)atoi(optarg);
      break;

    case 'h':
    default:
      printf("\nusage: %s [-h] [-p <port>]\n", argv[0]);
      printf("\t-b <backlog> - number of backlog connections for listen\n");
      printf("\t-h           - print this help menu\n");
      printf("\t-p <port>    - port for server to listen on\n");
      return 1;
    }
  }

  gearmand= malloc(sizeof(gearmand_st));
  if (gearmand == NULL)
    return 1;

  memset(gearmand, 0, sizeof(gearmand_st));
  
  gearmand->listen_fd= -1;
  gearmand->backlog= GEARMAN_DEFAULT_BACKLOG;
  gearmand->port= port;

  gearmand->ret = _listen_init(gearmand);
  if (gearmand->ret != GEARMAN_SUCCESS)
    return gearmand->ret;

  _listen_accept(gearmand->listen_fd, gearmand);
  return 0;
}

static gearman_return_t _listen_init(gearmand_st *gearmand)
{
  struct sockaddr_in sa;
  int opt= 1;

  memset(&sa, 0, sizeof(sa));
  sa.sin_family = AF_INET;
  sa.sin_port= htons(gearmand->port);
  sa.sin_addr.s_addr = INADDR_ANY;

  gearmand->listen_fd= socket(sa.sin_family, SOCK_STREAM, 0);
  if (gearmand->listen_fd == -1)
  {
    printf("_listen_init socket:%d",
                      errno);
    return GEARMAN_ERRNO;
  }

  if (setsockopt(gearmand->listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt,
                 sizeof(opt)) == -1)
  {
    printf("_listen_init setsockopt:%d",
                      errno);
    return GEARMAN_ERRNO;
  }

  if (bind(gearmand->listen_fd, (struct sockaddr *)(&sa), sizeof(sa)) == -1)
  {
    printf("_listen_init bind:%d",
                      errno);
    return GEARMAN_ERRNO;
  }

  if (listen(gearmand->listen_fd, gearmand->backlog) == -1)
  {
    printf("_listen_init listen:%d",
                      errno);
    return GEARMAN_ERRNO;
  }

  printf("Listening on port %u\n", gearmand->port);

  return GEARMAN_SUCCESS;
}

static void _listen_accept(int fd, void *arg)
{
  gearmand_st *gearmand= (gearmand_st *)arg;
  gearmand_con_st *dcon;
  socklen_t sa_len;

  dcon= malloc(sizeof(gearmand_con_st));
  if (dcon == NULL)
  {
    printf("_listen_accept malloc");
    gearmand->ret= GEARMAN_MEMORY_ALLOCATION_FAILURE;
    return;
  }

  sa_len= sizeof(dcon->sa);
  
  while(1)
  {
    dcon->fd= accept(fd, (struct sockaddr *)(&(dcon->sa)), &sa_len);
    if (dcon->fd == -1)
    {
      free(dcon);
      printf("_listen_accept accept:%d", errno);
      gearmand->ret= GEARMAN_ERRNO;;
      return;
    }
    break;
  }

  dcon->gearmand= gearmand;
  ssize_t read_size;
  char buf[8192];

  while (1)
  {
    read_size= read(dcon->fd, buf, 8192);
    if (read_size == 0)
    {
      return;
    }
    else if (read_size == -1)
    {
      if (errno == EAGAIN)
      {
        // there should do more
        continue;
      }
      else if (errno == EINTR)
        continue;
      return;
    }
    break;
  }

  buf[read_size] = '\0';
  printf("recv from client: %s\n", buf);
}