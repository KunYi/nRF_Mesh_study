#include <zephyr/bluetooth/mesh.h>
#include "vendor_model.h"

/* Forward declarations of message handlers */
static int handle_led_set(const struct bt_mesh_model *model,
                        struct bt_mesh_msg_ctx *ctx,
                        struct net_buf_simple *buf);
static int handle_led_get(const struct bt_mesh_model *model,
                        struct bt_mesh_msg_ctx *ctx,
                        struct net_buf_simple *buf);
static int handle_led_status(const struct bt_mesh_model *model,
                           struct bt_mesh_msg_ctx *ctx,
                           struct net_buf_simple *buf);
static int handle_button_press(const struct bt_mesh_model *model,
                             struct bt_mesh_msg_ctx *ctx,
                             struct net_buf_simple *buf);

/* Operation arrays for the models */
const struct bt_mesh_model_op vendor_srv_op[] = {
    { BT_MESH_VENDOR_OP_LED_SET, 2, handle_led_set },
    { BT_MESH_VENDOR_OP_LED_GET, 1, handle_led_get },
    { BT_MESH_VENDOR_OP_BUTTON_PRESS, 2, handle_button_press },
    BT_MESH_MODEL_OP_END,
};

const struct bt_mesh_model_op vendor_cli_op[] = {
    { BT_MESH_VENDOR_OP_LED_STATUS, 2, handle_led_status },
    BT_MESH_MODEL_OP_END,
};

/* Message handlers */
static int handle_led_set(const struct bt_mesh_model *model,
                        struct bt_mesh_msg_ctx *ctx,
                        struct net_buf_simple *buf)
{
    struct bt_mesh_vendor_model_srv *srv = model->user_data;
    uint8_t led_index = net_buf_simple_pull_u8(buf);
    uint8_t led_state = net_buf_simple_pull_u8(buf);

    if (srv->handlers.led_set) {
        srv->handlers.led_set(srv, ctx, led_index, led_state);
    }

    /* Store the LED state */
    if (led_index < 4) {
        srv->led_states[led_index] = led_state;
    }

    /* Send status message back */
    struct led_status status = {
        .led_index = led_index,
        .led_state = led_state
    };
    bt_mesh_vendor_model_srv_led_status_send(srv, ctx, &status);
    
    return 0;
}

static int handle_led_get(const struct bt_mesh_model *model,
                        struct bt_mesh_msg_ctx *ctx,
                        struct net_buf_simple *buf)
{
    struct bt_mesh_vendor_model_srv *srv = model->user_data;
    uint8_t led_index = net_buf_simple_pull_u8(buf);

    if (srv->handlers.led_get) {
        srv->handlers.led_get(srv, ctx, led_index);
    }

    /* Send status message back */
    struct led_status status = {
        .led_index = led_index,
        .led_state = (led_index < 4) ? srv->led_states[led_index] : LED_OFF
    };
    bt_mesh_vendor_model_srv_led_status_send(srv, ctx, &status);
    
    return 0;
}

static int handle_led_status(const struct bt_mesh_model *model,
                           struct bt_mesh_msg_ctx *ctx,
                           struct net_buf_simple *buf)
{
    struct bt_mesh_vendor_model_cli *cli = model->user_data;
    struct led_status status;

    status.led_index = net_buf_simple_pull_u8(buf);
    status.led_state = net_buf_simple_pull_u8(buf);

    if (cli->handlers.led_status) {
        cli->handlers.led_status(cli, ctx, &status);
    }
    
    return 0;
}

static int handle_button_press(const struct bt_mesh_model *model,
                            struct bt_mesh_msg_ctx *ctx,
                            struct net_buf_simple *buf)
{
    struct bt_mesh_vendor_model_srv *srv = model->user_data;
    struct button_press press;

    press.button_index = net_buf_simple_pull_u8(buf);
    press.button_state = net_buf_simple_pull_u8(buf);

    if (srv->handlers.button_pressed) {
        srv->handlers.button_pressed(srv, ctx, &press);
    }
    
    return 0;
}

/* Client API Implementation */
int bt_mesh_vendor_model_cli_led_set(struct bt_mesh_vendor_model_cli *cli,
                                   uint8_t led_index,
                                   uint8_t led_state)
{
    struct net_buf_simple *msg = NET_BUF_SIMPLE(BT_MESH_VENDOR_MSG_MAXLEN_MESSAGE);

    net_buf_simple_init(msg, 0);
    net_buf_simple_add_u8(msg, led_index);
    net_buf_simple_add_u8(msg, led_state);

    struct bt_mesh_msg_ctx ctx = {
        .addr = 0xC000,  /* Group address */
        .app_idx = 0,    /* Use first application key */
        .send_ttl = BT_MESH_TTL_DEFAULT,
    };

    return bt_mesh_model_send(cli->model, &ctx, msg, NULL, NULL);
}

int bt_mesh_vendor_model_cli_led_get(struct bt_mesh_vendor_model_cli *cli,
                                   uint8_t led_index)
{
    struct net_buf_simple *msg = NET_BUF_SIMPLE(BT_MESH_VENDOR_MSG_MAXLEN_MESSAGE);

    net_buf_simple_init(msg, 0);
    net_buf_simple_add_u8(msg, led_index);

    struct bt_mesh_msg_ctx ctx = {
        .addr = 0xC000,  /* Group address */
        .app_idx = 0,    /* Use first application key */
        .send_ttl = BT_MESH_TTL_DEFAULT,
    };

    return bt_mesh_model_send(cli->model, &ctx, msg, NULL, NULL);
}

int bt_mesh_vendor_model_cli_button_press(struct bt_mesh_vendor_model_cli *cli,
                                        struct button_press *press)
{
    struct net_buf_simple *msg = NET_BUF_SIMPLE(BT_MESH_VENDOR_MSG_MAXLEN_MESSAGE);

    net_buf_simple_init(msg, 0);
    net_buf_simple_add_u8(msg, press->button_index);
    net_buf_simple_add_u8(msg, press->button_state);

    struct bt_mesh_msg_ctx ctx = {
        .addr = 0xC000,  /* Group address */
        .app_idx = 0,    /* Use first application key */
        .send_ttl = BT_MESH_TTL_DEFAULT,
    };

    return bt_mesh_model_send(cli->model, &ctx, msg, NULL, NULL);
}

/* Server API Implementation */
int bt_mesh_vendor_model_srv_led_status_send(struct bt_mesh_vendor_model_srv *srv,
                                          struct bt_mesh_msg_ctx *ctx,
                                          struct led_status *status)
{
    struct net_buf_simple *msg = NET_BUF_SIMPLE(BT_MESH_VENDOR_MSG_MAXLEN_MESSAGE);

    net_buf_simple_init(msg, 0);
    net_buf_simple_add_u8(msg, status->led_index);
    net_buf_simple_add_u8(msg, status->led_state);

    return bt_mesh_model_send(srv->model, ctx, msg, NULL, NULL);
}
