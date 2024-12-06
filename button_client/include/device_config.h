#ifndef DEVICE_CONFIG_H
#define DEVICE_CONFIG_H

#include <zephyr/bluetooth/mesh.h>
#include "vendor_model.h"

/* Device UUID for provisioning */
#define DEV_UUID { 0xcc, 0xcc }

/* Health Server */
#define HEALTH_SRV_CB { \
    .attn_on = NULL, \
    .attn_off = NULL, \
}

#define HEALTH_PUB { \
    .msg = NULL, \
}

/* Model Operation Arrays */
extern const struct bt_mesh_model_op vendor_cli_op[];

#endif /* DEVICE_CONFIG_H */
