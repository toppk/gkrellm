// SPDX-License-Identifier: GPL-3.0-or-later WITH GPL-3.0-linking-source-exception

/* GKrellM
|  Copyright (C) 1999-2010 Bill Wilson
|
|  Author:  Stefan Gehn    stefan+gkrellm@srcbox.net
|
|  Latest versions might be found at:  http://gkrellm.net
*/

#ifndef GK_LOG_H
#define GK_LOG_H

#include <glib.h>

/**
 * @brief Prints our and/or logs a debug message.
 *  
 * If a logfile was set @see gkrellm_log_set_filename() the message will
 * be logged into the logfile as well.
 **/    
void gkrellm_debug(guint debug_level, const gchar *format, ...);
void gkrellm_debugv(guint debug_level, const gchar *format, va_list arg);

#endif //GK_LOG_H
