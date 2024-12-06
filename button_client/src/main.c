#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/mesh.h>
#include <dk_buttons_and_leds.h>
#include "vendor_model.h"
#include "device_config.h"

/* Health Server */
struct bt_mesh_health_srv_cb health_srv_cb = {
    .attn_on = NULL,
    .attn_off = NULL,
};

BT_MESH_HEALTH_PUB_DEFINE(health_pub, 0);
struct bt_mesh_health_srv health_srv = {
    .cb = &health_srv_cb,
};

/* LED status handler */
static void led_status_handler(struct bt_mesh_vendor_model_cli *cli,
                             struct bt_mesh_msg_ctx *ctx,
                             struct led_status *status)
{
    printk("LED %d is now %s\n",
           status->led_index,
           status->led_state == LED_ON ? "ON" : "OFF");

    /* Mirror the server's LED state on local LEDs */
    dk_set_led(status->led_index, status->led_state == LED_ON);
}

/* Button handler */
static void button_handler(uint32_t button_state, uint32_t has_changed)
{
    uint32_t button;

    for (button = 0; button < 4; button++) {
        if (has_changed & BIT(button)) {
            struct button_press press = {
                .button_index = button,
                .button_state = (button_state & BIT(button)) ?
                               BUTTON_PRESSED : BUTTON_RELEASED
            };

            /* Send button press to server */
            bt_mesh_vendor_model_cli_button_press(&vendor_client, &press);

            /* If button is pressed, toggle corresponding LED */
            if (press.button_state == BUTTON_PRESSED) {
                static uint8_t led_states[4] = {0};
                led_states[button] = !led_states[button];
                bt_mesh_vendor_model_cli_led_set(&vendor_client,
                                               button,
                                               led_states[button]);
            }
        }
    }
}

/* Define client handlers */
static const struct bt_mesh_vendor_model_cli_handlers cli_handlers = {
    .led_status = led_status_handler,
};

/* Define client model */
BT_MESH_VENDOR_MODEL_CLI_DEFINE(vendor_client, cli_handlers);

/* Node composition */
static struct bt_mesh_model models[] = {
    BT_MESH_MODEL_CFG_SRV,
    BT_MESH_MODEL_HEALTH_SRV(&health_srv, &health_pub),
    BT_MESH_VND_MODEL(BT_MESH_VENDOR_COMPANY_ID,
                      BT_MESH_VENDOR_MODEL_ID_CLI,
                      vendor_cli_op,
                      NULL,
                      &vendor_client)
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

    printk("Initializing Button Client...\n");

    /* Initialize buttons with callback */
    err = dk_buttons_init(button_handler);
    if (err) {
        printk("Buttons init failed (err %d)\n", err);
        return;
    }

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

    printk("Button client initialized\n");
}
