// SPDX-License-Identifier: GPL-3.0-or-later WITH GPL-3.0-linking-source-exception

/* GKrellM
|  Copyright (C) 1999-2019 Bill Wilson
|
|  Author:  Bill Wilson    billw@gkrellm.net
|
|  Latest versions might be found at:  http://gkrellm.net
*/
 
#if !defined(WIN32)
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
// Enable IPV6 on win32 if we target win xp or newer
#if defined(IPPROTO_IPV6) && (_WIN32_WINNT > 0x0500)
#define INET6
#endif
#endif

#if defined(__linux__)
#if defined(IPPROTO_IPV6)
#define INET6
#endif
#endif /* __linux__ */

#if defined(__FreeBSD__) || defined(__DragonFly__)
#include <osreldate.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/sysctl.h>
#include <net/route.h>
#if defined(__KAME__) && !defined(INET6)
#define INET6
#endif
#endif  /* __FreeBSD__ */

#if defined(__APPLE__)
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/sysctl.h>
#include <net/route.h>
#define INET6
#endif /* __APPLE__ */

#if defined(__NetBSD__) || defined(__OpenBSD__)
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#endif	/* __NetBSD__ || __OpenBSD__ */

#if defined(__solaris__)
/* IPv6 ? */
#include <netconfig.h>
#if defined(NC_INET6)
#define INET6
#endif
#endif /* __solaris__ */


  /* Values for state.
  */
#define	TCP_DEAD	0
#define	TCP_ALIVE	1

typedef struct
	{
	gint			state;
	gint			family;
	gint			local_port;
	struct in_addr	remote_addr;
#if defined(INET6)
	struct in6_addr	remote_addr6;
#endif
	gint			remote_port;
	gint			new_hit;
	gboolean		is_udp;
	}
	ActiveTCP;

