/* -*-pgsql-c-*- */
/*
 * $Header: /cvsroot/pgpool/pgpool-II/pool_ssl.c,v 1.2 2010/01/31 02:22:24 t-ishii Exp $
 *
 * pgpool: a language independent connection pool server for PostgreSQL
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2010	PgPool Global Development Group
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
 * pool_ssl.c: ssl negotiation functions
 *
 */

#include "config.h"
#include "pool.h"

#ifdef USE_SSL

#include <arpa/inet.h> /* for htonl() */

/* Major/minor codes to negotiate SSL prior to startup packet */
#define NEGOTIATE_SSL_CODE ( 1234<<16 | 5679 )

/* enum flag for differentiating server->client vs client->server SSL */
enum ssl_conn_type { ssl_conn_clientserver, ssl_conn_serverclient };

/* perform per-connection ssl initialization.  returns nonzero on error */
static int init_ssl_ctx(POOL_CONNECTION *cp, enum ssl_conn_type conntype);

/* attempt to negotiate a secure connection */
void pool_ssl_negotiate_clientserver(POOL_CONNECTION *cp) {
	int ssl_packet[2] = { htonl(sizeof(int)*2), htonl(NEGOTIATE_SSL_CODE) };
	char server_response;

	cp->ssl_active = -1;

	if ( (!pool_config->ssl) || init_ssl_ctx(cp, ssl_conn_clientserver))
		return;

	pool_debug("pool_ssl: sending client->server SSL request");
	pool_write_and_flush(cp, ssl_packet, sizeof(int)*2);
	pool_read(cp, &server_response, 1);
	pool_debug("pool_ssl: client->server SSL response: %c", server_response);

	switch (server_response) {
		case 'S':
			SSL_set_fd(cp->ssl, cp->fd);
			if (SSL_connect(cp->ssl) < 0) {
				pool_error("pool_ssl: SSL_connect failed: %ld", ERR_get_error());
			} else {
				cp->ssl_active = 1;
			}
			break;
		case 'N':
			pool_error("pool_ssl: server doesn't want to talk SSL");
			break;
		default:
			pool_error("pool_ssl: unhandled response: %c", server_response);
			break;
	}
}


/* attempt to negotiate a secure connection */
void pool_ssl_negotiate_serverclient(POOL_CONNECTION *cp) {

	cp->ssl_active = -1;

	if ( (!pool_config->ssl) || init_ssl_ctx(cp, ssl_conn_serverclient)) {
		/* write back an "SSL reject" response before returning */
		pool_write_and_flush(cp, "N", 1);
	} else {
		/* write back an "SSL accept" response */
		pool_write_and_flush(cp, "S", 1);

		SSL_set_fd(cp->ssl, cp->fd);
		if (SSL_accept(cp->ssl) < 0) {
			pool_error("pool_ssl: SSL_accept failed: %ld", ERR_get_error());
		} else {
			cp->ssl_active = 1;
		}
	}
}

void pool_ssl_close(POOL_CONNECTION *cp) {
	if (cp->ssl) { 
		SSL_shutdown(cp->ssl); 
		SSL_free(cp->ssl); 
	} 

	if (cp->ssl_ctx) 
		SSL_CTX_free(cp->ssl_ctx);
}

int pool_ssl_read(POOL_CONNECTION *cp, void *buf, int size) {
	return SSL_read(cp->ssl, buf, size);
}

int pool_ssl_write(POOL_CONNECTION *cp, const void *buf, int size) {
	return SSL_write(cp->ssl, buf, size);
}

static int init_ssl_ctx(POOL_CONNECTION *cp, enum ssl_conn_type conntype) {
	int error = 0;
	char *cacert = NULL, *cacert_dir = NULL;

	/* initialize SSL members */
	cp->ssl_ctx = SSL_CTX_new(TLSv1_method());
	if (! cp->ssl_ctx) {
		pool_error("pool_ssl: SSL_CTX_new failed: %ld", ERR_get_error());
		error = -1;
	}

	if ( conntype == ssl_conn_serverclient) {
		if ( (!error) && SSL_CTX_use_certificate_file(cp->ssl_ctx, 
													  pool_config->ssl_cert,
													  SSL_FILETYPE_PEM) <= 0) {
			pool_error("pool_ssl: SSL cert failure: %ld", ERR_get_error());
			error = -1;
		}

		if ( (!error) && SSL_CTX_use_PrivateKey_file(cp->ssl_ctx, 
													 pool_config->ssl_key, 
													 SSL_FILETYPE_PEM) <= 0) {
			pool_error("pool_ssl: SSL key failure: %ld", ERR_get_error());
			error = -1;
		}
	} else {
		/* set extra verification if ssl_ca_cert or ssl_ca_cert_dir are set */
		if (strlen(pool_config->ssl_ca_cert))
			cacert = pool_config->ssl_ca_cert;
		if (strlen(pool_config->ssl_ca_cert_dir))
			cacert_dir = pool_config->ssl_ca_cert_dir;
    
		if ( (!error) && (cacert || cacert_dir) ) {
			if (! SSL_CTX_load_verify_locations(cp->ssl_ctx, cacert, cacert_dir)) {
				pool_error("pool_ssl: SSL CA load error: %ld", ERR_get_error());   
				error = -1;
			} else {
				SSL_CTX_set_verify(cp->ssl_ctx, SSL_VERIFY_PEER, NULL);
			}
		}

	}

	if (! error) {
		cp->ssl = SSL_new(cp->ssl_ctx);
		if (! cp->ssl) {
			pool_error("pool_ssl: SSL_new failed: %ld", ERR_get_error());
			error = -1;
		}
	}

	return error;
}

#else /* USE_SSL: wrap / no-op ssl functionality if it's not available */

void pool_ssl_negotiate_serverclient(POOL_CONNECTION *cp) {
	pool_debug("pool_ssl: SSL requested but SSL support is not available");
	pool_write_and_flush(cp, "N", 1);
	cp->ssl_active = -1;
}

void pool_ssl_negotiate_clientserver(POOL_CONNECTION *cp) {
	pool_debug("pool_ssl: SSL requested but SSL support is not available");
	cp->ssl_active = -1;
}

void pool_ssl_close(POOL_CONNECTION *cp) { return; }

int pool_ssl_read(POOL_CONNECTION *cp, void *buf, int size) {
	pool_error("pool_ssl: SSL i/o called but SSL support is not available");
	notice_backend_error(cp->db_node_id);
	child_exit(1);
	return -1; /* never reached */
}

int pool_ssl_write(POOL_CONNECTION *cp, const void *buf, int size) {
	pool_error("pool_ssl: SSL i/o called but SSL support is not available");
	notice_backend_error(cp->db_node_id);
	child_exit(1);
	return -1; /* never reached */
}

#endif /* USE_SSL */
