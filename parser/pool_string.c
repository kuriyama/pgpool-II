/* -*-pgsql-c-*- */
/*
 * $Header: /cvsroot/pgpool/pgpool-II/parser/pool_string.c,v 1.3 2007/09/14 13:36:22 y-asaba Exp $
 *
 * Copyright (c) 2006-2007, pgpool Global Development Group
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
 */
#include "pool.h"
#include "pool_parser.h"

#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "pool_memory.h"
#include "pool_string.h"
#include "value.h"

/* String Library */
String *init_string(char *str)
{
	String *string = palloc(sizeof(String));
	int size;

	if (string == NULL)
	{
		pool_error("init_string: palloc failed: %s", strerror(errno));
		child_exit(1);
	}

	size = (strlen(str) + 1) / STRING_SIZE + 1;
	string->size = size;
	string->data = palloc(STRING_SIZE * size);
	if (string->data == NULL)
	{
		pool_error("init_string: palloc failed: %s", strerror(errno));
		pfree(string);
		child_exit(1);
	}

	memset(string->data, 0, STRING_SIZE * size);

	if (str == NULL)
	{
		string->len = 0;
	}
	else
	{
		string->len = strlen(str);
		memcpy(string->data, str, string->len);
	}
	
	return string;
}

void string_append_string(String *string, String *data)
{
	string_append_char(string, data->data);
}

void string_append_char(String *string, char *append_data)
{
	int len = strlen(append_data);
	
	if (string->len + len + 1 > string->size * STRING_SIZE)
	{
		int size, old_size;
		size = (string->len + len + 1) / STRING_SIZE + 1;
		old_size = string->size; 
		string->size = size;
		string->data = repalloc(string->data, string->size * STRING_SIZE);
		if (string->data == NULL)
		{
			pool_error("string_append_char: realloc failed: %s", strerror(errno));
			child_exit(1);
		}
		memset(string->data + (old_size * STRING_SIZE),
			   0, STRING_SIZE * (string->size - old_size));
	}
	memcpy(string->data + string->len, append_data, len);
	string->len += len;
}

void free_string(String *string)
{
	pfree(string->data);
	pfree(string);
}

String *copy_string(String *string)
{
	String *copy = palloc(sizeof(String));

	if (copy == NULL)
	{
		pool_error("copy_string: palloc failed: %s", strerror(errno));
		child_exit(1);
	}
	copy->size = string->size;
	copy->len = string->len;
	copy->data = palloc(string->size * STRING_SIZE);
	if (copy->data == NULL)
	{
		pool_error("copy_string: palloc failed: %s", strerror(errno));
		pfree(copy);
		child_exit(1);
	}		
	memcpy(copy->data, string->data, string->size * STRING_SIZE);
	
	return copy;
}
