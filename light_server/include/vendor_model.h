#ifndef VENDOR_MODEL_H__
#define VENDOR_MODEL_H__

#include <zephyr/bluetooth/mesh.h>

/* Company ID and Model IDs */
#define BT_MESH_VENDOR_COMPANY_ID    0x0059 /* Nordic Semiconductor ASA */
#define BT_MESH_VENDOR_MODEL_ID_SRV  0x0000
#define BT_MESH_VENDOR_MODEL_ID_CLI  0x0001

/* Operation codes for messages */
#define BT_MESH_VENDOR_OP_LED_SET     BT_MESH_MODEL_OP_3(0x00, BT_MESH_VENDOR_COMPANY_ID)
#define BT_MESH_VENDOR_OP_LED_GET     BT_MESH_MODEL_OP_3(0x01, BT_MESH_VENDOR_COMPANY_ID)
#define BT_MESH_VENDOR_OP_LED_STATUS  BT_MESH_MODEL_OP_3(0x02, BT_MESH_VENDOR_COMPANY_ID)
#define BT_MESH_VENDOR_OP_BUTTON_PRESS BT_MESH_MODEL_OP_3(0x03, BT_MESH_VENDOR_COMPANY_ID)

/* Maximum message length */
#define BT_MESH_VENDOR_MSG_MAXLEN_MESSAGE 4

/* LED states */
#define LED_OFF 0x00
#define LED_ON  0x01

/* Button states */
#define BUTTON_PRESSED  0x01
#define BUTTON_RELEASED 0x00

/* Message structures */
struct led_status {
    uint8_t led_index;
    uint8_t led_state;
};

struct button_press {
    uint8_t button_index;
    uint8_t button_state;
};

/* Forward declarations */
struct bt_mesh_vendor_model_cli;
struct bt_mesh_vendor_model_srv;

/* Operation arrays for the models */
extern const struct bt_mesh_model_op vendor_srv_op[];
extern const struct bt_mesh_model_op vendor_cli_op[];

/* Vendor Model Client API */
struct bt_mesh_vendor_model_cli_handlers {
    void (*led_status)(struct bt_mesh_vendor_model_cli *cli,
                      struct bt_mesh_msg_ctx *ctx,
                      struct led_status *status);
};

struct bt_mesh_vendor_model_cli {
    struct bt_mesh_model *model;
    struct bt_mesh_vendor_model_cli_handlers handlers;
};

/* Vendor Model Server API */
struct bt_mesh_vendor_model_srv_handlers {
    void (*led_set)(struct bt_mesh_vendor_model_srv *srv,
                   struct bt_mesh_msg_ctx *ctx,
                   uint8_t led_index,
                   uint8_t led_state);
    void (*led_get)(struct bt_mesh_vendor_model_srv *srv,
                   struct bt_mesh_msg_ctx *ctx,
                   uint8_t led_index);
    void (*button_pressed)(struct bt_mesh_vendor_model_srv *srv,
                          struct bt_mesh_msg_ctx *ctx,
                          struct button_press *press);
};

struct bt_mesh_vendor_model_srv {
    struct bt_mesh_model *model;
    struct bt_mesh_vendor_model_srv_handlers handlers;
    uint8_t led_states[4];  /* State storage for 4 LEDs */
};

/* Helper macros */
#define BT_MESH_VENDOR_MODEL_CLI_DEFINE(_name, _handlers) \
    static struct bt_mesh_vendor_model_cli _name = { \
        .handlers = _handlers \
    }

#define BT_MESH_VENDOR_MODEL_SRV_DEFINE(_name, _handlers) \
    static struct bt_mesh_vendor_model_srv _name = { \
        .handlers = _handlers \
    }

/* Client API functions */
int bt_mesh_vendor_model_cli_led_set(struct bt_mesh_vendor_model_cli *cli,
                                   uint8_t led_index,
                                   uint8_t led_state);
int bt_mesh_vendor_model_cli_led_get(struct bt_mesh_vendor_model_cli *cli,
                                   uint8_t led_index);
int bt_mesh_vendor_model_cli_button_press(struct bt_mesh_vendor_model_cli *cli,
                                        struct button_press *press);

/* Server API functions */
int bt_mesh_vendor_model_srv_led_status_send(struct bt_mesh_vendor_model_srv *srv,
                                          struct bt_mesh_msg_ctx *ctx,
                                          struct led_status *status);

#endif /* VENDOR_MODEL_H__ */
