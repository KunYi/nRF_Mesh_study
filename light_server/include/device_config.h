#ifndef DEVICE_CONFIG_H
#define DEVICE_CONFIG_H

#include <zephyr/bluetooth/mesh.h>

/* Device UUID for provisioning */
static uint8_t dev_uuid[16] = { 0xdd, 0xdd };

/* Health Server */
extern struct bt_mesh_health_srv health_srv;
extern struct bt_mesh_health_srv_cb health_srv_cb;
extern struct bt_mesh_model_pub health_pub;

/* Provisioning */
extern const struct bt_mesh_prov prov;

/* Vendor Model */
extern const struct bt_mesh_model_op vendor_srv_op[];
extern struct bt_mesh_vendor_model_srv vendor_server;

#endif /* DEVICE_CONFIG_H */
