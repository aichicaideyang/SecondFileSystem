#ifndef SERVER_SUPPORT_H
#define SERVER_SUPPORT_H
#include "common.h"
void init_help_info();
void *client_thread(void *client_fd);
int deal_trans_bytes(const int &trans_bytes, const string info, int mod = 0);
void initialize_listen(struct sigaction &action, int &listen_fd, int &sin_size, struct sockaddr_in &server, struct sockaddr_in &client);
void init_user_table();
#endif