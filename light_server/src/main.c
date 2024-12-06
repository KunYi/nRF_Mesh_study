#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/mesh.h>
#include <dk_buttons_and_leds.h>
#include "vendor_model.h"
#include "device_config.h"

#define LED_MSG "LED state changed\n"

static void led_set_handler(struct bt_mesh_vendor_model_srv *srv,
                          struct bt_mesh_msg_ctx *ctx,
                          uint8_t led_index,
                          uint8_t led_state)
{
    if (led_index >= 4) {
        return;
    }

    /* Set the physical LED state */
    dk_set_led(led_index, led_state == LED_ON);
    
    /* Store state */
    srv->led_states[led_index] = led_state;
    
    /* Send status back */
    struct led_status status = {
        .led_index = led_index,
        .led_state = led_state
    };
    bt_mesh_vendor_model_srv_led_status_send(srv, ctx, &status);

    printk("LED %d set to %s\n", led_index, led_state == LED_ON ? "ON" : "OFF");
}

static void led_get_handler(struct bt_mesh_vendor_model_srv *srv,
                          struct bt_mesh_msg_ctx *ctx,
                          uint8_t led_index)
{
    if (led_index >= 4) {
        return;
    }

    struct led_status status = {
        .led_index = led_index,
        .led_state = srv->led_states[led_index]
    };
    bt_mesh_vendor_model_srv_led_status_send(srv, ctx, &status);

    printk("LED %d state requested\n", led_index);
}

static void button_handler(struct bt_mesh_vendor_model_srv *srv,
                         struct bt_mesh_msg_ctx *ctx,
                         struct button_press *press)
{
    printk("Button %d %s\n", 
           press->button_index,
           press->button_state == BUTTON_PRESSED ? "pressed" : "released");
}

/* Define server handlers */
static const struct bt_mesh_vendor_model_srv_handlers srv_handlers = {
    .led_set = led_set_handler,
    .led_get = led_get_handler,
    .button_pressed = button_handler,
};

/* Define server model */
BT_MESH_VENDOR_MODEL_SRV_DEFINE(vendor_server, srv_handlers);

/* Health Server */
struct bt_mesh_health_srv_cb health_srv_cb = {
    .attn_on = NULL,
    .attn_off = NULL,
};

BT_MESH_HEALTH_PUB_DEFINE(health_pub, 0);
struct bt_mesh_health_srv health_srv = {
    .cb = &health_srv_cb,
};

/* Node composition */
static struct bt_mesh_model models[] = {
    BT_MESH_MODEL_CFG_SRV,
    BT_MESH_MODEL_HEALTH_SRV(&health_srv, &health_pub),
    BT_MESH_VND_MODEL(BT_MESH_VENDOR_COMPANY_ID,
                      BT_MESH_VENDOR_MODEL_ID_SRV,
                      vendor_srv_op,
                      NULL,
                      &vendor_server)
};

static struct bt_mesh_elem elements[] = {
    BT_MESH_ELEM(0, models, BT_MESH_MODEL_NONE),
};

static const struct bt_mesh_comp comp = {
    .cid = BT_MESH_VENDOR_COMPANY_ID,
    .elem = elements,
    .elem_count = ARRAY_SIZE(elements),
};

/* Provisioning callbacks */
static void prov_complete(uint16_t net_idx, uint16_t addr)
{
    printk("Provisioning completed. Net idx 0x%04x, addr 0x%04x\n", net_idx, addr);
}

static void prov_reset(void)
{
    bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);
}

const struct bt_mesh_prov prov = {
    .uuid = dev_uuid,
    .output_size = 0,
    .output_actions = 0,
    .output_number = 0,
    .complete = prov_complete,
    .reset = prov_reset,
};

static void bt_ready(int err)
{
    if (err) {
        printk("Bluetooth init failed (err %d)\n", err);
        return;
    }

    printk("Bluetooth initialized\n");

    err = bt_mesh_init(&prov, &comp);
    if (err) {
        printk("Mesh initialization failed (err %d)\n", err);
        return;
    }

    printk("Mesh initialized\n");
}

void main(void)
{
    int err;

    printk("Initializing Light Server...\n");

    /* Initialize LEDs */
    err = dk_leds_init();
    if (err) {
        printk("LEDs init failed (err %d)\n", err);
        return;
    }

    /* Initialize Bluetooth */
    err = bt_enable(bt_ready);
    if (err) {
        printk("Bluetooth init failed (err %d)\n", err);
        return;
    }

    /* Enable provisioning */
    bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);

    printk("Light server initialized\n");
}
