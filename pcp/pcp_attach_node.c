/*
 * $Header: /cvsroot/pgpool/pgpool-II/pcp/pcp_attach_node.c,v 1.2 2008/01/29 01:56:37 y-asaba Exp $
 *
 * pgpool: a language independent connection pool server for PostgreSQL 
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2008	PgPool Global Development Group
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that copyright notice and this permission
 * notice appear in supporting documentation, and that the name of the
 * author not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. The author makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * Client program to send "attach node" command.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pcp.h"

static void usage(void);
static void myexit(ErrorCode e);

int
main(int argc, char **argv)
{
	long timeout;
	char host[MAX_DB_HOST_NAMELEN];
	int port;
	char user[MAX_USER_PASSWD_LEN];
	char pass[MAX_USER_PASSWD_LEN];
	int nodeID;

	if (argc == 2 && (strcmp(argv[1], "-h") == 0) )
	{
		usage();
		exit(0);
	}

	if (argc != 7)
	{
		errorcode = INVALERR;
		pcp_errorstr(errorcode);
		myexit(errorcode);
	}

	timeout = atol(argv[1]);
	if (timeout < 0) {
		errorcode = INVALERR;
		pcp_errorstr(errorcode);
		myexit(errorcode);
	}

	if (strlen(argv[2]) >= MAX_DB_HOST_NAMELEN)
	{
		errorcode = INVALERR;
		pcp_errorstr(errorcode);
		myexit(errorcode);
	}
	strcpy(host, argv[2]);

	port = atoi(argv[3]);
	if (port <= 1024 || port > 65535)
	{
		errorcode = INVALERR;
		pcp_errorstr(errorcode);
		myexit(errorcode);
	}

	if (strlen(argv[4]) >= MAX_USER_PASSWD_LEN)
	{
		errorcode = INVALERR;
		pcp_errorstr(errorcode);
		myexit(errorcode);
	}
	strcpy(user, argv[4]);

	if (strlen(argv[5]) >= MAX_USER_PASSWD_LEN)
	{
		errorcode = INVALERR;
		pcp_errorstr(errorcode);
		myexit(errorcode);
	}
	strcpy(pass, argv[5]);

	nodeID = atoi(argv[6]);
	if (nodeID < 0 || nodeID > MAX_NUM_BACKENDS)
	{
		errorcode = INVALERR;
		pcp_errorstr(errorcode);
		myexit(errorcode);
	}
		
	pcp_set_timeout(timeout);

	if (pcp_connect(host, port, user, pass))
	{
		pcp_errorstr(errorcode);
		myexit(errorcode);
	}

	if (pcp_attach_node(nodeID))
	{
		pcp_errorstr(errorcode);
		pcp_disconnect();
		myexit(errorcode);
	}

	pcp_disconnect();

	return 0;
}

static void
usage(void)
{
	fprintf(stderr, "pcp_attach_node - attach a node from pgpool-II\n\n");
	fprintf(stderr, "Usage: pcp_attach_node timeout hostname port# username password nodeID\n");
	fprintf(stderr, "Usage: pcp_attach_node -h\n\n");
	fprintf(stderr, "  timeout  - connection timeout value in seconds. command exits on timeout\n");
	fprintf(stderr, "  hostname - pgpool-II hostname\n");
	fprintf(stderr, "  port#    - pgpool-II port number\n");
	fprintf(stderr, "  username - username for PCP authentication\n");
	fprintf(stderr, "  password - password for PCP authentication\n");
	fprintf(stderr, "  nodeID   - ID of a node to be attached\n");
	fprintf(stderr, "  -h       - print this help\n");
}

static void
myexit(ErrorCode e)
{
	if (e == INVALERR)
	{
		usage();
		exit(e);
	}

	exit(e);
}
