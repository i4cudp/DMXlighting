/*
*
@file		socket.h
@brief	define function of socket API 
*
*/

#ifndef	_SOCKET_H_
#define	_SOCKET_H_
#include "psptypes.h"
#include "printf.h"

#define SOCK0  0
#define SOCK1  1
#define SOCK2  2
#define SOCK3  3
#define SOCK4  4
#define SOCK5  5
#define SOCK6  6
#define SOCK7  7

typedef uint_8 SOCKET;

extern uint_8 socket(SOCKET s, uint_8 protocol, uint_16 port, uint_8 flag); // Opens a socket(TCP or UDP or IP_RAW mode)
extern void close(SOCKET s); // Close socket
extern uint_8 connect(SOCKET s, uint_8 * addr, uint_16 port); // Establish TCP connection (Active connection)
extern void disconnect(SOCKET s); // disconnect the connection
extern uint_8 listen(SOCKET s);	// Establish TCP connection (Passive connection)
extern uint_16 send(SOCKET s, const uint_8 * buf, uint_16 len); // Send data (TCP)
extern uint_16 recv(SOCKET s, uint_8 * buf, uint_16 len);	// Receive data (TCP)
extern uint_16 sendto(SOCKET s, const uint_8 * buf, uint_16 len, uint_8 * addr, uint_16 port); // Send data (UDP/IP RAW)
extern uint_16 recvfrom(SOCKET s, uint_8 * buf, uint_16 len, uint_8 * addr, uint_16  *port); // Receive data (UDP/IP RAW)

#ifdef __MACRAW__
void macraw_open(void);
uint_16 macraw_send( const uint_8 * buf, uint_16 len ); //Send data (MACRAW)
uint_16 macraw_recv( uint_8 * buf, uint_16 len ); //Recv data (MACRAW)
#endif

#endif
/* _SOCKET_H_ */
