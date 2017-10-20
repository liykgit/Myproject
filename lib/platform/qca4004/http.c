
#include "plat.h"
#include "http.h"

int vg_http_client_startup(unsigned char *domain, unsigned short port)
{
    return http_client_startup(domain, port);
}

int vg_http_client_post(char *url, unsigned int content_length, post_callback_t post_cb, get_callback_t get_cb)
{
    return http_client_post(url, content_length, post_cb, get_cb);
}

int vg_http_client_get(char *url, get_callback_t get_cb)
{
    return http_client_get(url, get_cb);

}

int vg_http_client_stop(void)
{
    return http_client_stop();
}
