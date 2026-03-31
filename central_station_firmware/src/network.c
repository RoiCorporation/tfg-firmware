#include "pico/stdlib.h"
#include "mongoose.h"
#include "network.h"


static const char *s_url = "mqtt://mqtt.thingsboard.cloud:1883";


/**
 * @brief Configure the WiFi connection.
 * 
 * @param data struct containing the WiFi credentials.
 */
void wifi_setconfig(void *data) {
    struct mg_tcpip_driver_pico_w_data *wifi_data =
        (struct mg_tcpip_driver_pico_w_data *) data;
    struct mg_wifi_data *wifi = &wifi_data->wifi;
    wifi->ssid = WIFI_SSID;
    wifi->pass = WIFI_PASSWORD;
}


/**
 * @brief Configure the MQTT connection event handler.
 * 
 * @param connection struct that acts as the MQTT connection manager.
 * @param ev code of the event.
 * @param ev_data data created by that event.
 */
void mqtt_ev_handler(struct mg_connection *connection, int ev, void *ev_data) {
    network_ctx_t *ctx = (network_ctx_t *) connection->fn_data;

    if (ev == MG_EV_OPEN) {
        MG_INFO(("%lu CREATED", connection->id));

    }
    else if (ev == MG_EV_ERROR) {
        MG_ERROR(("%lu ERROR %s", connection->id, (char *) ev_data));

    }
    else if (ev == MG_EV_MQTT_OPEN) {
        int code = *(int *) ev_data;

        if (code == 0) {
            MG_INFO(("MQTT CONNECTED"));
        }
        else {
            MG_ERROR(("MQTT rejected, code=%d", code));
            connection->is_closing = 1;
        }

    }
    else if (ev == MG_EV_CLOSE) {
        MG_INFO(("MQTT CLOSED"));
        if (ctx && ctx->mqtt_connection == connection) {
            ctx->mqtt_connection = NULL;
        }
    }
}


/**
 * @brief Configure the method that keeps the connection alive.
 * 
 * @param arg struct containing the necessary fields for the MQTT
 * network.
 */
void mqtt_timer_fn(void *arg) {
    network_ctx_t *network_context = (network_ctx_t *) arg;

    if (network_context->mqtt_connection == NULL) {
        struct mg_mqtt_opts opts = {
            .clean = true,
            .version = 4,
            .client_id = mg_str(TB_CLIENT_ID),
            .user = mg_str(TB_USERNAME),
            .pass = mg_str(TB_PASSWORD)
        };

        network_context->mqtt_connection = mg_mqtt_connect(
            network_context->connection_manager, s_url, &opts, 
            mqtt_ev_handler, network_context
        );
    }
    else {
        mg_mqtt_ping(network_context->mqtt_connection);
    }
}


/**
 * @brief Create an MQTT protocol "Publish" operation and send the environmental
 * readings of a given station as the message's payload.
 * 
 * @param connection struct that acts as the MQTT connection manager.
 * @param station_readings struct containing the environmental data read by the
 * station.
 */
void publish_environmental_readings(
    struct mg_connection *connection,
    ambient_info_t station_readings
) {
    char payload[512];

    mg_snprintf(
        payload, sizeof(payload),
        "{\"station_id\":\"%s\", \"temperature\":%.2f, \"humidity\":%.2f, \"light_intensity\":%.2f, \"air_pressure\":%.2f, \"iaq\":%.2f, \"carbon_monoxide_concentration\":%.2f, \"methane_concentration\":%.2f, \"propane_concentration\":%.2f, \"alcohol_concentration\":%.2f, \"hydrogen_gas_concentration\":%.2f}", 
        station_readings.station_id,
        station_readings.temperature,
        station_readings.humidity,
        station_readings.light_intensity,
        station_readings.air_pressure,
        station_readings.air_quality_index,
        station_readings.carbon_monoxide_concentration,
        station_readings.methane_concentration,
        station_readings.propane_concentration,
        station_readings.alcohol_concentration,
        station_readings.hydrogen_gas_concentration
    );

    struct mg_mqtt_opts pub = {
        .topic = mg_str(MQTT_TOPIC),
        .message = mg_str(payload),
        .qos = 1
    };

    mg_mqtt_pub(connection, &pub);
    MG_INFO(("%lu PUBLISHED -> %s", connection->id, payload));
}
