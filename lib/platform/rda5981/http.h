#ifndef _HTTP_H_
#define _HTTP_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
* @fn typedef void (*get_callback_t)(unsigned char *buf, unsigned int len)
* @brief response data get callback function, user can get the response data in this function .
* @param buf buffer store response data.
* @param len length of data.
*/
typedef void (*get_callback_t)(unsigned char *buf, unsigned int len);

/**
* @fn typedef void (*post_callback_t)(unsigned char **send_buf, unsigned int *buf_len)
* @brief POST callback function, user should give the data that want to post in this function.
* @param send_buf a pointer to pointer to the send buffer.
* @param buf_len a pointer to get the send buffer length.
* in this function, user should define a buf, just like
* >    char buf[64]
* then define the function:
* > void post_cb(unsigned char **send_buf, unsigned int *buf_len);
* and then, in the callback funtion, point send_buf to the buf define above:
* >    *send_buf = buf
* and give the buf length:
* >    *buf_len = 10;
*/
typedef void (*post_callback_t)(unsigned char **send_buf, unsigned int *buf_len);


/**
* @fn int http_client_startup(unsigned char *domain, unsigned short port)
* @brief Start a http client, connect to domain:port.
* @param domain host name or host ip of the http server.
* @param port port number of the http server.
* @return 0 success, -1 failed
*/
int vg_http_client_startup(unsigned char *domain, unsigned short port);

/**
* @fn int http_client_post(char *url, unsigned int content_length, post_callback_t post_cb, get_callback_t get_cb)
* @brief Post data to the url.
* @param url POST url.
* @param content_length content length.
* @param post_cb post callback function, user should give the data that want to post in this function
* @param get_cb response data get callback function, user can get the response data in this function
* @return 0 success, -1 failed
*/
int vg_http_client_post(char *url, unsigned int content_length, post_callback_t post_cb, get_callback_t respond_cb);

/**
* @fn int http_client_get(char *url, get_callback_t get_cb)
* @brief get data or page from the url.
* @param url GET url.
* @param get_cb response data get callback function, user can get the response data in this function.
* @return 0 success, -1 failed
*/
int vg_http_client_get(char *url, get_callback_t get_cb);

/**
* @fn int http_client_stop(void)
* @brief Stop the http client, free the memory.
* @param none.
* @return 0 success, -1 failed
*/
int vg_http_client_stop(void);

#ifdef __cplusplus
}
#endif

#endif
