#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <WebServer.h>

extern WebServer server;

void start_web_interface();
void web_interface_handle_client();

#endif