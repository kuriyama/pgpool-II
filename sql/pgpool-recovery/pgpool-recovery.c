/* -*-pgsql-c-*- */
/*
 * $Header: /cvsroot/pgpool/pgpool-II/sql/pgpool-recovery/pgpool-recovery.c,v 1.2 2007/10/23 07:32:20 y-asaba Exp $
 *
 * pgpool-recovery: exec online recovery script from SELECT statement.
 *
 * Copyright (c) 2003-2007	PgPool Global Development Group
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

#include "postgres.h"
#include "fmgr.h"
#include "miscadmin.h"
#include "executor/spi.h"
#include "funcapi.h"

#define RECOVERY_FILE "pgpool_recovery"
#define REMOTE_START_FILE "pgpool_remote_start"

#include <stdlib.h>

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

PG_FUNCTION_INFO_V1(pgpool_recovery);
PG_FUNCTION_INFO_V1(pgpool_remote_start);

extern Datum pgpool_recovery(PG_FUNCTION_ARGS);
extern Datum pgpool_remote_start(PG_FUNCTION_ARGS);

static char recovery_script[1024];

Datum
pgpool_recovery(PG_FUNCTION_ARGS)
{
	int r;
	char *remote_host = DatumGetCString(DirectFunctionCall1(textout,
															PointerGetDatum(PG_GETARG_TEXT_P(0))));
	char *remote_data_directory = DatumGetCString(DirectFunctionCall1(textout,
																	  PointerGetDatum(PG_GETARG_TEXT_P(1))));

	sprintf(recovery_script, "%s/%s %s %s %s",
			DataDir, RECOVERY_FILE, DataDir, remote_host,
			remote_data_directory);
	elog(DEBUG1, "recovery_script: %s", recovery_script);
	r = system(recovery_script);

	if (r != 0)
	{
		elog(ERROR, "pgpool_recovery failed");
	}

	PG_RETURN_BOOL(true);
}


Datum
pgpool_remote_start(PG_FUNCTION_ARGS)
{
	int r;
	char *remote_host = DatumGetCString(DirectFunctionCall1(textout,
															PointerGetDatum(PG_GETARG_TEXT_P(0))));
	char *remote_data_directory = DatumGetCString(DirectFunctionCall1(textout,
																	  PointerGetDatum(PG_GETARG_TEXT_P(1))));

	sprintf(recovery_script, "%s/%s %s %s", DataDir, REMOTE_START_FILE,
			remote_host, remote_data_directory);
	elog(DEBUG1, "recovery_script: %s", recovery_script);
	r = system(recovery_script);

	if (r != 0)
	{
		elog(ERROR, "pgpool_remote_start failed");
	}

	PG_RETURN_BOOL(true);
}
