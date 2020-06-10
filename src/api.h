#ifndef api_h_INCLUDED
#define api_h_INCLUDED

typedef struct {
    int socket;
    int index;
} client_t;

void API_init(void);
void api_check_events();
void api_receive(client_t *api_client);
void api_close_connection(client_t *api_client);
void api_send_response(client_t *api_client, char *response);
void api_error(const char* error);

#endif // api_h_INCLUDED

