#ifndef NETWORK_H
#define NETWORK_H


#include "mongoose.h"
#include "central_station_firmware.h"


typedef struct {
    struct mg_mgr *mgr;
    struct mg_connection *mqtt_conn;
    ambient_info_t environmental_readings;
} network_ctx_t;

void wifi_setconfig(void *data);
void mqtt_timer_fn(void *arg);
void sensor_readings_timer(void *arg);
void publish_environmental_readings(
    struct mg_connection *connection,
    ambient_info_t station_readings
);


#endif
