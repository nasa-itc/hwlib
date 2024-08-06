#include "libsocket.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>

// Creates an endpoint for communication
// Binds stream, server sockets to localhost and port number
//
// Inputs:
//      socket_info->address_family
//      socket_info->type
//      socket_info->port_num (used for stream sockets)
//      socket_info->block
//
// Outputs:
//      socket_info->sockfd
//      socket_info->created
//      socket_info->bound  
int32_t socket_create(socket_info_t* socket_info)
{
    return SOCKET_SUCCESS;
}

// Listens on a connection on a socket
//
// Inputs:
//      socket_info->bound
//      socket_info->sockfd
//
// Outputs:
//      socket_info->listening
int32_t socket_listen(socket_info_t* socket_info)
{
    return SOCKET_SUCCESS;
}

// Accepts a connection on a socket
//
// Inputs:
//      socket_info->listening
//      socket_info->sockfd
//
// Outputs:
//      socket_info->connected
//      socket_info->sockfd
int32_t socket_accept(socket_info_t* socket_info)
{
    return SOCKET_SUCCESS;
} 

// Initiates a connection to a remote ip address and port number
//
// Inputs:
//      socket_info->created
//      socket_info->category
//      socket_info->address_family
//      socket_info->sockfd
//      remote_ip_address (the remote ip address)
//      remote_port_num (the remote port number)
//
// Outputs:
//      socket_info->connected
int32_t socket_connect(socket_info_t* socket_info, char* remote_ip_address, int remote_port_num)
{
    return SOCKET_SUCCESS;
}

int32_t socket_send(socket_info_t* socket_info, uint8_t* buffer, size_t buflen, size_t* bytes_sent, char* remote_ip_address, int remote_port_num)
{
    return SOCKET_SUCCESS;
}

int32_t socket_recv(socket_info_t* socket_info, uint8_t* buffer, size_t buflen, size_t* bytes_recvd)
{
    return SOCKET_SUCCESS;
}

int32_t socket_close(socket_info_t* socket_info)
{
    return SOCKET_SUCCESS;
}
