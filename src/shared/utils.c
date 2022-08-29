/* GKrellM
|  Copyright (C) 1999-2019 Bill Wilson
|
|  Author:  Bill Wilson    billw@gkrellm.net
|  Latest versions might be found at:  http://gkrellm.net
|
|
|  GKrellM is free software: you can redistribute it and/or modify it
|  under the terms of the GNU General Public License as published by
|  the Free Software Foundation, either version 3 of the License, or
|  (at your option) any later version.
|
|  GKrellM is distributed in the hope that it will be useful, but WITHOUT
|  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
|  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
|  License for more details.
|
|  You should have received a copy of the GNU General Public License
|  along with this program. If not, see http://www.gnu.org/licenses/
|
|
|  Additional permission under GNU GPL version 3 section 7
|
|  If you modify this program, or any covered work, by linking or
|  combining it with the OpenSSL project's OpenSSL library (or a
|  modified version of that library), containing parts covered by
|  the terms of the OpenSSL or SSLeay licenses, you are granted
|  additional permission to convey the resulting work.
|  Corresponding Source for a non-source form of such a combination
|  shall include the source code for the parts of OpenSSL used as well
|  as that of the covered work.
*/

#if !defined(WIN32)
#include <sys/socket.h>
#include <utime.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include "../gkrellm.h"
#include "../gkrellm-private.h"

/* ===============  string utility functions ================= */

gboolean
gkrellm_dup_string(gchar **dst, gchar *src)
{
	if (!dst || (!*dst && !src))
		return FALSE;
	if (*dst)
	{
		if (src && !strcmp(*dst, src))
			return FALSE;
		g_free(*dst);
	}
	*dst = g_strdup(src);
	return TRUE;
}

static gboolean
any(gchar c, gchar *s)
{
	while (*s)
		if (c == *s++)
			return TRUE;
	return FALSE;
}

/* Return a duplicated token from a string.  "*string" points to the source
|  string and is updated to point to the string remaining after the
|  found token.  If there is no next token, return an empty dupped string
|  (not a NULL pointer) and leave *string unchanged.
|  Unlike strtok(): args are not modified, gkrellm_token() can be used on
|  constant strings, delimeter identity is not lost, and it's thread safe.
|  Only the caller's initial string pointer is modified.
*/
gchar *
gkrellm_dup_token(gchar **string, gchar *delimeters)
{
	gchar *str, *s, *delims;
	gboolean quoted = FALSE;

	if (!string || !*string)
		return g_strdup("");

	str = *string;
	delims = delimeters ? delimeters : " \t\n";
	while (any(*str, delims))
		++str;

	if (*str == '"')
	{
		quoted = TRUE;
		++str;
		for (s = str; *s && *s != '"'; ++s)
			;
	}
	else
		for (s = str; *s && !any(*s, delims); ++s)
			;

	*string = (quoted && *s) ? s + 1 : s;
	return g_strndup(str, s - str);
}

/* Cut out an optionally quoted string.  This is destructive to the src.
 */
gchar *
gkrellm_cut_quoted_string(gchar *src, gchar **endptr)
{
	gchar *s;

	while (*src == ' ' || *src == '\t')
		++src;
	if (*src == '"')
	{
		s = strchr(++src, '"');
		if (s == NULL)
		{
			if (endptr)
				*endptr = src;
			g_warning(_("Unterminated quote\n"));
			return NULL;
		}
		*s = '\0';
		if (endptr)
			*endptr = s + 1;
	}
	else
	{
		for (s = src; *s != '\0' && *s != ' ' && *s != '\t'; ++s)
			;
		if (endptr)
			*endptr = *s ? s + 1 : s;
		*s = '\0';
	}
	return src;
}

/* If there is a line in the gstring ('\n' delimited) copy it to the
|  line buffer including the newline and erase it from the gstring.
*/
gboolean
gkrellm_getline_from_gstring(GString **gstring, gchar *line, gint size)
{
	GString *gstr = *gstring;
	gchar *s;
	gint len, n;

	if (gstr && gstr->str && (s = strchr(gstr->str, '\n')) != NULL)
	{
		n = len = s - gstr->str + 1;
		if (n >= size)
			n = size - 1; /* Truncate the line to fit */
		strncpy(line, gstr->str, n);
		line[n] = '\0';
		*gstring = g_string_erase(gstr, 0, len);
		return TRUE;
	}
	return FALSE;
}

/* ===============  list utility functions ================= */

void gkrellm_free_glist_and_data(GList **list_head)
{
	GList *list;

	if (*list_head == NULL)
		return;

	/* could use g_list_foreach(*list_head, (G_FUNC)g_free, NULL);
	 */
	for (list = *list_head; list; list = list->next)
		if (list->data)
			g_free(list->data);
	g_list_free(*list_head);
	*list_head = NULL;
}

GList *
gkrellm_string_in_list(GList *list, gchar *s)
{
	if (!s)
		return NULL;
	for (; list; list = list->next)
	{
		if (!strcmp((gchar *)list->data, s))
			return list;
	}
	return NULL;
}

gint gkrellm_string_position_in_list(GList *list, gchar *s)
{
	gint i, n = -1;

	if (!s)
		return -1;
	for (i = 0; list; list = list->next, ++i)
	{
		if (!strcmp((gchar *)list->data, s))
		{
			n = i;
			break;
		}
	}
	return n;
}

/* ===============  file utility functions ================= */
gchar *
gkrellm_homedir(void)
{
	gchar *homedir;

	homedir = (gchar *)g_get_home_dir();
	if (!homedir)
		homedir = ".";
	return homedir;
}

gboolean
gkrellm_make_home_subdir(gchar *subdir, gchar **path)
{
	gchar *dir;
	gint result = FALSE;

	dir = g_build_path(G_DIR_SEPARATOR_S, gkrellm_homedir(), subdir, NULL);
	if (!g_file_test(dir, G_FILE_TEST_IS_DIR))
	{
		if (g_mkdir(dir, 0755) < 0)
			g_warning(_("Cannot create directory: %s\n"), dir);
		else
			result = TRUE;
	}
	if (path)
		*path = dir;
	else
		g_free(dir);
	return result;
}

gint gkrellm_connect_to(gchar *server, gint server_port)
{
	gint fd = -1;
#ifdef HAVE_GETADDRINFO
	gint rv = 0;
	struct addrinfo hints, *res, *res0;
	gchar portnumber[6];
#else
	struct hostent *addr;
	struct sockaddr_in s;
#endif

#ifdef HAVE_GETADDRINFO
	snprintf(portnumber, sizeof(portnumber), "%d", server_port);
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;
	if ((rv = getaddrinfo(server, portnumber, &hints, &res0)) != 0)
		return -1;

	for (res = res0; res; res = res->ai_next)
	{
		if ((fd = socket(res->ai_family, res->ai_socktype,
						 res->ai_protocol)) < 0)
			continue;
		gkrellm_debug(DEBUG_CLIENT, "\t[gkrellm_connect_to: (%d,%d,%d) %s:%d]\n",
					  res->ai_family, res->ai_socktype,
					  res->ai_protocol, server, server_port);
		if (connect(fd, res->ai_addr, res->ai_addrlen) >= 0)
			break;
#ifdef WIN32
		closesocket(fd);
#else
		close(fd);
#endif
		fd = -1;
	}
	freeaddrinfo(res0);
#else
	gkrellm_debug(DEBUG_CLIENT, "\t[gkrellm_connect_to: %s:%d]\n", server,
				  server_port);
	addr = gethostbyname(server);
	if (addr)
	{
		fd = socket(AF_INET, SOCK_STREAM, 0);
		if (fd >= 0)
		{
			memset(&s, 0, sizeof(s));
			memcpy(&s.sin_addr.s_addr, addr->h_addr, addr->h_length);
			s.sin_family = AF_INET;
			s.sin_port = htons(server_port);
			if (connect(fd, (struct sockaddr *)&s, sizeof(s)) < 0)
			{
				gkrellm_debug(DEBUG_CLIENT, "gkrellm_connect_to(); connect() failed\n");
#ifdef WIN32
				closesocket(fd);
#else
				close(fd);
#endif
				fd = -1;
			}
		}
	}
	else
	{
		gkrellm_debug(DEBUG_CLIENT, "gkrellm_connect_to(); gethostbyname() failed\n");
	}
#endif
	if (fd < 0)
		return -1;

	return fd;
}