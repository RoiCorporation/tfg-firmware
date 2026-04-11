#ifndef NETWORK_H
#define NETWORK_H


#include "mongoose.h"
#include "central_station_firmware.h"


void wifi_setconfig(void *data);
void mqtt_timer_fn(void *arg);
int8_t mqtt_is_ready(network_ctx_t *network_context);
void publish_environmental_readings(
    struct mg_connection *connection,
    ambient_info_t station_readings
);


#endif
