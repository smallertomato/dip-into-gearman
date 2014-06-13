#include "client.h"

static gearman_return_t _con_init(gearman_client_st *client, gearman_client_con_st *con);
gearman_return_t _con_send(gearman_client_st *client, gearman_client_con_st *con, char* data);

gearman_return_t gearman_client_add_server(gearman_client_st *client,
                                           const char *host, in_port_t port)
{
    strncpy(client->host, host == NULL ? GEARMAN_DEFAULT_TCP_HOST : host,
          NI_MAXHOST);
    client->host[NI_MAXHOST - 1]= 0;
    
    client->port= port == 0 ? GEARMAN_DEFAULT_TCP_PORT : port;
    return GEARMAN_SUCCESS;
}

gearman_client_st *gearman_client_create(gearman_client_st *client)
{
    if (client == NULL)
    {
        client= malloc(sizeof(gearman_client_st));
        if (client == NULL)
            return NULL;
    }
    else
    {
        memset(client, 0, sizeof(gearman_client_st));
    }

    gearman_client_con_st *con;
    client->con = gearman_client_con_create(client, con);
    
    if (client->con == NULL)
    {
        free(client);
        return NULL;
    }
    return client;
}

void gearman_client_free(gearman_client_st *client)
{
  if (client->con != NULL)
    free(client->con);
  
  if (client != NULL)
    free(client);
}

gearman_client_con_st* gearman_client_con_create(gearman_client_st *client, gearman_client_con_st *con)
{
    if (con == NULL)
    {
        con = malloc(sizeof(gearman_client_con_st));
        if (con == NULL)
            return NULL;
    }
    else
        memset(con, 0, sizeof(gearman_client_con_st));

    return con;
}

static gearman_return_t _con_init(gearman_client_st *client, gearman_client_con_st *con)
{
    struct addrinfo ai;
    int ret;
    char port_str[NI_MAXSERV];
    sprintf(port_str, "%u", client->port);

    memset(&ai, 0, sizeof(struct addrinfo));
    ai.ai_flags= (AI_V4MAPPED | AI_ADDRCONFIG);
    ai.ai_family= AF_UNSPEC;
    ai.ai_socktype= SOCK_STREAM;
    ai.ai_protocol= IPPROTO_TCP;

    ret= getaddrinfo(client->host, port_str, &ai, &(con->addrinfo));
    if (ret != 0)
    {
        printf("_con_init getaddrinfo:%s", gai_strerror(ret));
        return GEARMAN_GETADDRINFO;
    }

    con->fd= socket(con->addrinfo->ai_family,
            con->addrinfo->ai_socktype,
            con->addrinfo->ai_protocol);

    if (con->fd == -1)
    {
        return GEARMAN_ERRNO;
    }

    while (1)
    {
        ret= connect(con->fd, con->addrinfo->ai_addr,
                     con->addrinfo->ai_addrlen);
        if (ret == 0)
        {
          break;
        }

        if (errno == EAGAIN || errno == EINTR)
          continue;

        if (errno == EINPROGRESS)
        {
          break;
        }

        if (errno == ECONNREFUSED || errno == ENETUNREACH || errno == ETIMEDOUT)
        {
          con->addrinfo= con->addrinfo->ai_next;
          break;
        }

        return GEARMAN_ERRNO;
    }
    return GEARMAN_SUCCESS;
}

gearman_return_t _con_send(gearman_client_st *client, gearman_client_con_st *con, char* data)
{
    ssize_t write_size;
    write_size = write(con->fd, data, 1024);

    if (write_size == 0)
    {
        return GEARMAN_EOF;
    }
    else if (write_size == -1)
    {
        return GEARMAN_ERRNO;
    }

    return GEARMAN_SUCCESS;
}

gearman_return_t main(int argc, char const *argv[])
{
    char c;
    in_port_t port= GEARMAN_DEFAULT_TCP_PORT;
    char * host = GEARMAN_DEFAULT_TCP_HOST;

    gearman_client_st * client;
    client = gearman_client_create(client);

    if(client == NULL)
    {
        return GEARMAN_MEMORY_ALLOCATION_FAILURE;
    }

    if(GEARMAN_SUCCESS != gearman_client_add_server(client, host, port))
    {
        return GEARMAN_ERRNO;
    }

    if(GEARMAN_SUCCESS != _con_init(client, client->con))
    {
        return GEARMAN_ERRNO;
    }

    char * data = "love you, china";

    if ( GEARMAN_SUCCESS != _con_send(client, client->con, data))
        return GEARMAN_ERRNO;
    printf("send %s\n", data);
    return 0;
}