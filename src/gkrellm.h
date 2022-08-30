// SPDX-License-Identifier: GPL-3.0-or-later WITH GPL-3.0-linking-source-exception

/* GKrellM
|  Copyright (C) 1999-2019 Bill Wilson
|
|  Author:  Bill Wilson    billw@gkrellm.net
|
|  Latest versions might be found at:  http://gkrellm.net
*/

#ifndef GKRELLM_H
#define GKRELLM_H

#if defined(GKRELLM_SERVER)

#if !defined(PACKAGE)
#define PACKAGE "gkrellmd"
#endif
#include "gkrellm-server.h"

#else

#if !defined(PACKAGE)
#define PACKAGE "gkrellm"
#endif
#include "gkrellm-client.h"

#endif

/* Internationalization support.
 */
#if defined(ENABLE_NLS)
#include <libintl.h>
#undef _
#define _(String) dgettext(PACKAGE, String)
#if defined(gettext_noop)
#define N_(String) gettext_noop(String)
#else
#define N_(String) (String)
#endif /* gettext_noop */
#else
#define _(String) (String)
#define N_(String) (String)
#define textdomain(String) (String)
#define gettext(String) (String)
#define dgettext(Domain, String) (String)
#define dcgettext(Domain, String, Type) (String)
#define bindtextdomain(Domain, Directory) (Domain)
#endif /* ENABLE_NLS */
#endif
