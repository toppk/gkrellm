// SPDX-License-Identifier: GPL-3.0-or-later WITH GPL-3.0-linking-source-exception

/* GKrellM
|  Copyright (C) 1999-2010 Bill Wilson
|
|  Author:  Stefan Gehn    stefan+gkrellm@srcbox.net
|
|  Latest versions might be found at:  http://gkrellm.net
*/

#ifndef GK_LOG_P_H
#define GK_LOG_P_H

#include <glib.h>

typedef void (*GkrellmLogFunc) (GLogLevelFlags log_level, const gchar *message);
typedef gboolean (*GkrellmLogInitFunc) (void);
typedef gboolean (*GkrellmLogCleanupFunc) (void);

/**
 * Installs a log handler and adds default logging behaviour.
 *  
 * All GLib log functions and macros (i..e g_log(), g_debug(), g_warning() etc)
 * are handled by our log handler from now on
 **/
void gkrellm_log_init(void);

/**
 * Removes our log handler reverting to default GLib log behaviour.
 **/ 
void gkrellm_log_cleanup(void);

/**
 * Registers another log function.
 * 
 * Calls function @p init if it's not NULL and registers function @p log if
 * the (optional) call to @p init was successful.
 *    
 * @note This is mainly used by gkrellmd to register an additional
 *       syslog handler. 
 **/   
gboolean gkrellm_log_register(
	GkrellmLogFunc log,
	GkrellmLogInitFunc init,
	GkrellmLogCleanupFunc cleanup);

/** 
 * Unregisters a log function
 * 
 * Also calls the GkrellmLogCleanupFunc passed together with @p log
 * in gkrellm_log_register().
 **/ 
gboolean gkrellm_log_unregister(GkrellmLogFunc log);

/**
 * Enables or disables logging into a file
 * 
 * @param filename path to logfile or NULL to disable logging into a file
 **/ 
void gkrellm_log_set_filename(const gchar* filename);

#endif //GK_LOG_P_H
