#include <kore/kore.h>
#include <string.h>
#include "db.h"

static MYSQL *g_db = NULL;

int
db_init(const char *host, const char *user, const char *pass,
        const char *dbname, unsigned int port)
{
    g_db = mysql_init(NULL);
    if (g_db == NULL) {
        kore_log(LOG_ERR, "mysql_init failed");
        return KORE_RESULT_ERROR;
    }

    /* Optional: auto-reconnect is deprecated in some versions; handle errors instead */
    /* mysql_options(g_db, MYSQL_OPT_RECONNECT, &(my_bool){1}); */

    if (mysql_real_connect(g_db, host, user, pass, dbname, port, NULL, 0) == NULL) {
        kore_log(LOG_ERR, "mysql_real_connect failed: %s", mysql_error(g_db));
        mysql_close(g_db);
        g_db = NULL;
        return KORE_RESULT_ERROR;
    }

    /* Optional: set charset */
    if (mysql_set_character_set(g_db, "utf8mb4") != 0) {
        kore_log(LOG_WARNING, "mysql_set_character_set: %s", mysql_error(g_db));
    }

    return KORE_RESULT_OK;
}

void
db_cleanup(void)
{
    if (g_db != NULL) {
        mysql_close(g_db);
        g_db = NULL;
    }
}

int
db_get_user_email(int user_id, char *out, size_t out_len)
{
    /* Prepared statement example */
    const char *sql = "SELECT email FROM users WHERE id = ? LIMIT 1";

    MYSQL_STMT *stmt = mysql_stmt_init(g_db);
    if (!stmt) {
        kore_log(LOG_ERR, "mysql_stmt_init: %s", mysql_error(g_db));
        return KORE_RESULT_ERROR;
    }

    if (mysql_stmt_prepare(stmt, sql, (unsigned long)strlen(sql)) != 0) {
        kore_log(LOG_ERR, "mysql_stmt_prepare: %s", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return KORE_RESULT_ERROR;
    }

    MYSQL_BIND bind_param[1];
    memset(bind_param, 0, sizeof(bind_param));
    bind_param[0].buffer_type = MYSQL_TYPE_LONG;
    bind_param[0].buffer = (char *)&user_id;

    if (mysql_stmt_bind_param(stmt, bind_param) != 0) {
        kore_log(LOG_ERR, "mysql_stmt_bind_param: %s", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return KORE_RESULT_ERROR;
    }

    if (mysql_stmt_execute(stmt) != 0) {
        kore_log(LOG_ERR, "mysql_stmt_execute: %s", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return KORE_RESULT_ERROR;
    }

    /* Bind result */
    MYSQL_BIND bind_res[1];
    memset(bind_res, 0, sizeof(bind_res));

    char email_buf[256];
    unsigned long email_len = 0;
    bool is_null = 0;

    bind_res[0].buffer_type = MYSQL_TYPE_STRING;
    bind_res[0].buffer = email_buf;
    bind_res[0].buffer_length = sizeof(email_buf);
    bind_res[0].length = &email_len;
    bind_res[0].is_null = &is_null;

    if (mysql_stmt_bind_result(stmt, bind_res) != 0) {
        kore_log(LOG_ERR, "mysql_stmt_bind_result: %s", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return KORE_RESULT_ERROR;
    }

    int fetch_rc = mysql_stmt_fetch(stmt);
    if (fetch_rc == 0 && !is_null) {
        /* Copy to output safely */
        size_t n = (email_len < out_len - 1) ? email_len : out_len - 1;
        memcpy(out, email_buf, n);
        out[n] = '\0';
        mysql_stmt_close(stmt);
        return KORE_RESULT_OK;
    }

    mysql_stmt_close(stmt);
    return KORE_RESULT_ERROR; /* Not found or error */
}