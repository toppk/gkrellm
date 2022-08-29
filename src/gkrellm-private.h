

#ifndef GKRELLM_PRIVATE_H
#define GKRELLM_PRIVATE_H

#if defined(GKRELLM_SERVER)
#include "gkrellm-private-server.h"
#else
#include "gkrellm-private-client.h"
#endif

/* Debugs for debug_level	*/

#define DEBUG_SYSDEP 0x1
#define DEBUG_SERVER 0x2
#define DEBUG_MAIL 0x10
#define DEBUG_NET 0x20
#define DEBUG_TIMER 0x40
#define DEBUG_SENSORS 0x80
#define DEBUG_NO_LIBSENSORS 0x100
#define DEBUG_INET 0x800
#define DEBUG_CLIENT 0x1000
#define DEBUG_GUI 0x2000
#define DEBUG_POSITION 0x4000
#define DEBUG_BATTERY 0x8000
#define DEBUG_CHART_TEXT 0x10000
#define DEBUG_PLUGIN 0x20000

#endif
