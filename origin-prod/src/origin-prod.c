#include <kore/kore.h>
#include <kore/http.h>

#include <assets.h>

#include "db/db.h"

/* ------------------- Prototypes -------------------- */
/* ---------------------- Pages ---------------------- */
int		home_page(struct http_request *);
int		waitlist_page(struct http_request *);
/* ---------------------- Media ---------------------- */
int		serve_favicon(struct http_request *);
int		serve_home_image(struct http_request *);
/* ----------------------  API  ---------------------- */
int waitlist_submission(struct http_request *req);
/* ---------------------- Kore  ---------------------- */
void	kore_parent_configure(int argc, char** argv);
int		init(int state);
void	kore_worker_teardown(void);


/* ------------------ Implementation ----------------- */
int
init(int state)
{
    (void)state;

    /* Pull these from config/env in real apps */
    if (db_init("127.0.0.1", "appuser", "apppass", "appdb", 3306) != KORE_RESULT_OK)
        return KORE_RESULT_ERROR;

    return KORE_RESULT_OK;
}

void
kore_worker_teardown(void)
{
    db_cleanup();
}

void 
kore_parent_configure(int argc, char** argv)
{
    kore_log(LOG_INFO, "╔══════════════════════════════════════════════════════════╗");
    kore_log(LOG_INFO, "║                                                          ║");
    kore_log(LOG_INFO, "║   🪶  Icarus Social Media Server                          ║");
    kore_log(LOG_INFO, "║                                                          ║");
    kore_log(LOG_INFO, "║   Local:    http://localhost:5000                        ║");
    kore_log(LOG_INFO, "║   Network:  http://<your-ip>:5000                        ║");
    kore_log(LOG_INFO, "║                                                          ║");
    kore_log(LOG_INFO, "║   To find your IP, run: hostname -I (Linux)              ║");
    kore_log(LOG_INFO, "║                     or: ipconfig (Windows)               ║");
    kore_log(LOG_INFO, "║                     or: ifconfig (Mac)                   ║");
    kore_log(LOG_INFO, "║                                                          ║");
    kore_log(LOG_INFO, "║   Press Ctrl+C to stop the server                        ║");
    kore_log(LOG_INFO, "║                                                          ║");
    kore_log(LOG_INFO, "╚══════════════════════════════════════════════════════════╝");
}

int 
home_page(struct http_request *req)
{
	//const char html[] = "<html><body>Hello, world!</body></html>";

	http_response_header(req, "content-type", "text/html; charset=utf-8");
	http_response(req, 200, asset_index_html, asset_len_index_html);

	return (KORE_RESULT_OK);
}

int
serve_favicon(struct http_request *req) 
{
	http_response_header(req, "content-type", "image/x-icon");
	http_response(req, 200, asset_icarus_logo_ico, asset_len_icarus_logo_ico);

	return (KORE_RESULT_OK);
}

int
serve_home_image(struct http_request *req)
{
	http_response_header(req, "content-type", "image/jpeg");
	http_response(req, 200, asset_Gowy_icaro_prado_jpg, asset_len_Gowy_icaro_prado_jpg);

	return (KORE_RESULT_OK);
}

int
waitlist_page(struct http_request *req)
{
	http_response_header(req, "content-type", "text/html");
	http_response(req, 200, asset_waitlist_html, asset_len_waitlist_html);

	return (KORE_RESULT_OK);
}

int
waitlist_submission(struct http_request *req)
{
	struct kore_json json;
	struct kore_json_item *name, *role, *email, *source, *ts;
	if (req->method != HTTP_METHOD_POST) { http_response(req, 405, NULL, 0); return KORE_RESULT_OK; }

	kore_json_init(&json, req->http_body->data, req->http_body->length);

	/* Error handle */
	if (kore_json_parse(&json) != KORE_RESULT_OK) {
		kore_log(LOG_NOTICE, "JSON parse error: %s", kore_json_strerror());
		kore_json_cleanup(&json);
		http_response(req, 400, "Invalid JSON\n", 13);
	}

	/* Extract values upon success */
	name = kore_json_find_string(json.root, "name"); role = kore_json_find_string(json.root, "role");
	email = kore_json_find_string(json.root, "email"); ts = kore_json_find_string(json.root, "ts");
	source = kore_json_find_string(json.root, "source");

	if (name == NULL || role == NULL || 
		email == NULL || ts == NULL ||
		source == NULL) 
	{
		kore_json_cleanup(&json);
		http_response(req, 400, "Missing field(s)\n", 18);
		return (KORE_RESULT_OK);
	}

	kore_log(LOG_DEBUG, "Got JSON values:\n\t| Name: %s\n\t| Role: %s\n\t| Email: %s\n\t| Time-Stamp: %s\n %%Source%%: %s",
			 name->data.string, role->data.string, email->data.string, ts->data.string, source->data.string);

	kore_json_cleanup(&json);
	http_response(req, 204, NULL, 0);
	return (KORE_RESULT_OK);
}
