/* -*-pgsql-c-*- */
/*
 * $Header: /cvsroot/pgpool/pgpool-II/pool_signal.c,v 1.1.1.1 2006/09/08 03:36:08 t-ishii Exp $
 *
 * pgpool: a language independent connection pool server for PostgreSQL 
 * written by Tatsuo Ishii
 *
 * Portions Copyright (c) 2003-2006, PgPool Global Development Group
 * Portions Copyright (c) 2003-2004, PostgreSQL Global Development Group
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

/*
 * Signal stuff. Stolen from PostgreSQL source code.
 */

#include "config.h"
#include "pool_signal.h"
#include <signal.h>

#ifdef HAVE_SIGPROCMASK
sigset_t	UnBlockSig,
			BlockSig,
			AuthBlockSig;

#else
int			UnBlockSig,
			BlockSig,
			AuthBlockSig;
#endif

/*
 * Initialize BlockSig, UnBlockSig, and AuthBlockSig.
 *
 * BlockSig is the set of signals to block when we are trying to block
 * signals.  This includes all signals we normally expect to get, but NOT
 * signals that should never be turned off.
 *
 * AuthBlockSig is the set of signals to block during authentication;
 * it's essentially BlockSig minus SIGTERM, SIGQUIT, SIGALRM.
 *
 * UnBlockSig is the set of signals to block when we don't want to block
 * signals (is this ever nonzero??)
 */
void
poolinitmask(void)
{
#ifdef HAVE_SIGPROCMASK
	sigemptyset(&UnBlockSig);
	sigfillset(&BlockSig);
	sigfillset(&AuthBlockSig);

	/*
	 * Unmark those signals that should never be blocked. Some of these
	 * signal names don't exist on all platforms.  Most do, but might as
	 * well ifdef them all for consistency...
	 */
#ifdef SIGTRAP
	sigdelset(&BlockSig, SIGTRAP);
	sigdelset(&AuthBlockSig, SIGTRAP);
#endif
#ifdef SIGABRT
	sigdelset(&BlockSig, SIGABRT);
	sigdelset(&AuthBlockSig, SIGABRT);
#endif
#ifdef SIGILL
	sigdelset(&BlockSig, SIGILL);
	sigdelset(&AuthBlockSig, SIGILL);
#endif
#ifdef SIGFPE
	sigdelset(&BlockSig, SIGFPE);
	sigdelset(&AuthBlockSig, SIGFPE);
#endif
#ifdef SIGSEGV
	sigdelset(&BlockSig, SIGSEGV);
	sigdelset(&AuthBlockSig, SIGSEGV);
#endif
#ifdef SIGBUS
	sigdelset(&BlockSig, SIGBUS);
	sigdelset(&AuthBlockSig, SIGBUS);
#endif
#ifdef SIGSYS
	sigdelset(&BlockSig, SIGSYS);
	sigdelset(&AuthBlockSig, SIGSYS);
#endif
#ifdef SIGCONT
	sigdelset(&BlockSig, SIGCONT);
	sigdelset(&AuthBlockSig, SIGCONT);
#endif
#ifdef SIGTERM
	sigdelset(&AuthBlockSig, SIGTERM);
#endif
#ifdef SIGQUIT
	sigdelset(&AuthBlockSig, SIGQUIT);
#endif
#ifdef SIGALRM
	sigdelset(&AuthBlockSig, SIGALRM);
#endif
#else
	UnBlockSig = 0;
	BlockSig = sigmask(SIGHUP) | sigmask(SIGQUIT) |
		sigmask(SIGTERM) | sigmask(SIGALRM) |
		sigmask(SIGINT) | sigmask(SIGUSR1) |
		sigmask(SIGUSR2) | sigmask(SIGCHLD) |
		sigmask(SIGWINCH) | sigmask(SIGFPE);
	AuthBlockSig = sigmask(SIGHUP) |
		sigmask(SIGINT) | sigmask(SIGUSR1) |
		sigmask(SIGUSR2) | sigmask(SIGCHLD) |
		sigmask(SIGWINCH) | sigmask(SIGFPE);
#endif
}


/* Win32 signal handling is in backend/port/win32/signal.c */
#ifndef WIN32

/* 
 * We need to check actually the system has the posix signals or not, but...
 */
#define HAVE_POSIX_SIGNALS
/*
 * Set up a signal handler
 */
pool_sighandler_t
pool_signal(int signo, pool_sighandler_t func)
{
#if !defined(HAVE_POSIX_SIGNALS)
	return signal(signo, func);
#else
	struct sigaction act,
				oact;

	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if (signo != SIGALRM)
		act.sa_flags |= SA_RESTART;
#ifdef SA_NOCLDSTOP
	if (signo == SIGCHLD)
		act.sa_flags |= SA_NOCLDSTOP;
#endif
	if (sigaction(signo, &act, &oact) < 0)
		return SIG_ERR;
	return oact.sa_handler;
#endif   /* !HAVE_POSIX_SIGNALS */
}

#endif   /* WIN32 */
