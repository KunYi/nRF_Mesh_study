#include <zephyr/bluetooth/mesh.h>
#include <zephyr/sys/util.h>
#include "vendor_model.h"

/* Message handlers */
static int handle_led_status(const struct bt_mesh_model *model,
                           struct bt_mesh_msg_ctx *ctx,
                           struct net_buf_simple *buf)
{
    if (!model->user_data) {
        return -EINVAL;
    }
    
    struct bt_mesh_vendor_model_cli *cli = model->user_data;
    struct led_status status;

    if (!cli->handlers.led_status) {
        return -EINVAL;
    }

    status.led_index = net_buf_simple_pull_u8(buf);
    status.led_state = net_buf_simple_pull_u8(buf);

    cli->handlers.led_status(cli, ctx, &status);
    return 0;
}

/* Operation arrays for the models */
const struct bt_mesh_model_op vendor_cli_op[] = {
    { BT_MESH_VENDOR_OP_LED_STATUS, 2, handle_led_status },
    BT_MESH_MODEL_OP_END,
};

/* Client API Implementation */
int bt_mesh_vendor_model_cli_led_set(struct bt_mesh_vendor_model_cli *cli,
                                   uint8_t led_index,
                                   uint8_t led_state)
{
    if (!cli || !cli->model) {
        return -EINVAL;
    }

    BT_MESH_MODEL_BUF_DEFINE(msg, BT_MESH_VENDOR_OP_LED_SET,
                            BT_MESH_VENDOR_MSG_MAXLEN_MESSAGE);
    
    bt_mesh_model_msg_init(&msg, BT_MESH_VENDOR_OP_LED_SET);
    net_buf_simple_add_u8(&msg, led_index);
    net_buf_simple_add_u8(&msg, led_state);

    struct bt_mesh_msg_ctx ctx = {
        .addr = BT_MESH_ADDR_ALL_NODES,
        .app_idx = cli->model->keys[0],
        .send_ttl = BT_MESH_TTL_DEFAULT,
    };

    return bt_mesh_model_send(cli->model, &ctx, &msg, NULL, NULL);
}

int bt_mesh_vendor_model_cli_led_get(struct bt_mesh_vendor_model_cli *cli,
                                   uint8_t led_index)
{
    if (!cli || !cli->model) {
        return -EINVAL;
    }

    BT_MESH_MODEL_BUF_DEFINE(msg, BT_MESH_VENDOR_OP_LED_GET,
                            BT_MESH_VENDOR_MSG_MAXLEN_MESSAGE);
    
    bt_mesh_model_msg_init(&msg, BT_MESH_VENDOR_OP_LED_GET);
    net_buf_simple_add_u8(&msg, led_index);

    struct bt_mesh_msg_ctx ctx = {
        .addr = BT_MESH_ADDR_ALL_NODES,
        .app_idx = cli->model->keys[0],
        .send_ttl = BT_MESH_TTL_DEFAULT,
    };

    return bt_mesh_model_send(cli->model, &ctx, &msg, NULL, NULL);
}

int bt_mesh_vendor_model_cli_button_press(struct bt_mesh_vendor_model_cli *cli,
                                        struct button_press *press)
{
    if (!cli || !cli->model || !press) {
        return -EINVAL;
    }

    BT_MESH_MODEL_BUF_DEFINE(msg, BT_MESH_VENDOR_OP_BUTTON_PRESS,
                            BT_MESH_VENDOR_MSG_MAXLEN_MESSAGE);
    
    bt_mesh_model_msg_init(&msg, BT_MESH_VENDOR_OP_BUTTON_PRESS);
    net_buf_simple_add_u8(&msg, press->button_index);
    net_buf_simple_add_u8(&msg, press->button_state);

    struct bt_mesh_msg_ctx ctx = {
        .addr = BT_MESH_ADDR_ALL_NODES,
        .app_idx = cli->model->keys[0],
        .send_ttl = BT_MESH_TTL_DEFAULT,
    };

    return bt_mesh_model_send(cli->model, &ctx, &msg, NULL, NULL);
}
