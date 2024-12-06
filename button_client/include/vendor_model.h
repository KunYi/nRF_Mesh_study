#ifndef VENDOR_MODEL_H
#define VENDOR_MODEL_H

#include <zephyr/bluetooth/mesh.h>

#define BT_MESH_VENDOR_COMPANY_ID    0x0059  /* Nordic Semiconductor ASA */
#define BT_MESH_VENDOR_MODEL_ID_CLI  0x000A
#define BT_MESH_VENDOR_MODEL_ID_SRV  0x000B

#define BT_MESH_VENDOR_OP_LED_SET       BT_MESH_MODEL_OP_3(0x01, BT_MESH_VENDOR_COMPANY_ID)
#define BT_MESH_VENDOR_OP_LED_GET       BT_MESH_MODEL_OP_3(0x02, BT_MESH_VENDOR_COMPANY_ID)
#define BT_MESH_VENDOR_OP_LED_STATUS    BT_MESH_MODEL_OP_3(0x03, BT_MESH_VENDOR_COMPANY_ID)
#define BT_MESH_VENDOR_OP_BUTTON_PRESS  BT_MESH_MODEL_OP_3(0x04, BT_MESH_VENDOR_COMPANY_ID)

#define BT_MESH_VENDOR_MSG_MAXLEN_MESSAGE 32

#define LED_OFF 0x00
#define LED_ON  0x01

struct led_status {
    uint8_t led_index;
    uint8_t led_state;
};

struct button_press {
    uint8_t button_index;
    uint8_t button_state;
};

struct bt_mesh_vendor_model_srv;
struct bt_mesh_vendor_model_cli;

/* Server handlers */
struct vendor_model_srv_handlers {
    void (*const led_set)(struct bt_mesh_vendor_model_srv *srv,
                         struct bt_mesh_msg_ctx *ctx,
                         uint8_t led_index,
                         uint8_t led_state);
    void (*const led_get)(struct bt_mesh_vendor_model_srv *srv,
                         struct bt_mesh_msg_ctx *ctx,
                         uint8_t led_index);
    void (*const button_pressed)(struct bt_mesh_vendor_model_srv *srv,
                               struct bt_mesh_msg_ctx *ctx,
                               struct button_press *press);
};

/* Client handlers */
struct vendor_model_cli_handlers {
    void (*const led_status)(struct bt_mesh_vendor_model_cli *cli,
                           struct bt_mesh_msg_ctx *ctx,
                           struct led_status *status);
};

/* Server model context */
struct bt_mesh_vendor_model_srv {
    struct bt_mesh_model *model;
    const struct vendor_model_srv_handlers handlers;
    uint8_t led_states[4];
};

/* Client model context */
struct bt_mesh_vendor_model_cli {
    struct bt_mesh_model *model;
    const struct vendor_model_cli_handlers handlers;
};

/* Server API */
int bt_mesh_vendor_model_srv_led_status_send(struct bt_mesh_vendor_model_srv *srv,
                                          struct bt_mesh_msg_ctx *ctx,
                                          struct led_status *status);

/* Client API */
int bt_mesh_vendor_model_cli_led_set(struct bt_mesh_vendor_model_cli *cli,
                                   uint8_t led_index,
                                   uint8_t led_state);
int bt_mesh_vendor_model_cli_led_get(struct bt_mesh_vendor_model_cli *cli,
                                   uint8_t led_index);
int bt_mesh_vendor_model_cli_button_press(struct bt_mesh_vendor_model_cli *cli,
                                        struct button_press *press);

/* Model Definitions */
#define BT_MESH_VENDOR_MODEL_SRV_DEFINE(_name, _handlers) \
    static struct bt_mesh_vendor_model_srv _name = { \
        .handlers = _handlers, \
    }

#define BT_MESH_VENDOR_MODEL_CLI_DEFINE(_name, _handlers) \
    static struct bt_mesh_vendor_model_cli _name = { \
        .handlers = _handlers, \
    }

#define BT_MESH_VND_MODEL(_company, _id, _model, _pub) \
    BT_MESH_MODEL_VND_CB(_company, _id, _model.handlers.op, _pub, &_model, NULL, NULL)

#endif /* VENDOR_MODEL_H */
