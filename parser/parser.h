/*-------------------------------------------------------------------------
 *
 * parser.h
 *		Definitions for the "raw" parser (lex and yacc phases only)
 *
 *
 * Portions Copyright (c) 2003-2008, PgPool Global Development Group
 * Portions Copyright (c) 1996-2005, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/parser/parser.h,v 1.19 2004/12/31 22:03:38 pgsql Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef PARSER_H
#define PARSER_H

#include "pg_list.h"
#include "parsenodes.h"

extern List *raw_parser(const char *str);
extern void free_parser(void);
extern int filtered_base_yylex(void);

#endif   /* PARSER_H */
