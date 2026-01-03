#ifndef DB_H
#define DB_H

#include <mysql/mysql.h>

int  db_init(const char *host, const char *user, const char *pass,
             const char *dbname, unsigned int port);
void db_cleanup(void);

/* Example query helper (simple) */
int db_get_user_email(int user_id, char *out, size_t out_len);

#endif