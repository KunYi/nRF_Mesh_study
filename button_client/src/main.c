#include <zephyr/kernel.h>
#include <zephyr/bluetooth/mesh.h>
#include <zephyr/drivers/gpio.h>
#include "vendor_model.h"
#include "device_config.h"

/* Device UUID */
static const uint8_t dev_uuid[16] = DEV_UUID;

/* Health Server */
static struct bt_mesh_health_srv_cb health_srv_cb = HEALTH_SRV_CB;

static struct bt_mesh_health_srv health_srv = {
    .cb = &health_srv_cb,
};

static struct bt_mesh_model_pub health_pub = HEALTH_PUB;

/* Provisioning */
static const struct bt_mesh_prov prov = {
    .uuid = dev_uuid,
};

/* Button handling */
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static struct k_work button_work;

/* Forward declaration of vendor client */
static struct bt_mesh_vendor_model_cli vendor_client;

static void button_pressed_work_handler(struct k_work *work)
{
    struct button_press press = {
        .button_index = 0,
        .button_state = 1
    };
    bt_mesh_vendor_model_cli_button_press(&vendor_client, &press);
}

/* Vendor Model handlers */
static void handle_led_status(struct bt_mesh_vendor_model_cli *cli,
                           struct bt_mesh_msg_ctx *ctx,
                           struct led_status *status)
{
    printk("LED %d is %s\n", status->led_index,
           status->led_state == LED_ON ? "on" : "off");
}

static const struct vendor_model_cli_handlers cli_handlers = {
    .led_status = handle_led_status,
};

/* Initialize the Vendor Model Client */
BT_MESH_VENDOR_MODEL_CLI_DEFINE(vendor_client, cli_handlers);

/* Element Definition */
static struct bt_mesh_model models[] = {
    BT_MESH_MODEL_CFG_SRV,
    BT_MESH_MODEL_HEALTH_SRV(&health_srv, &health_pub),
    BT_MESH_MODEL_VND_CB(BT_MESH_VENDOR_COMPANY_ID,
                      BT_MESH_VENDOR_MODEL_ID_CLI,
                      vendor_cli_op,
                      NULL,
                      &vendor_client,
                      NULL),
};

static struct bt_mesh_elem elements[] = {
    BT_MESH_ELEM(0, models, BT_MESH_MODEL_NONE),
};

static const struct bt_mesh_comp comp = {
    .cid = BT_MESH_VENDOR_COMPANY_ID,
    .elem = elements,
    .elem_count = ARRAY_SIZE(elements),
};

static void button_pressed(const struct device *dev, struct gpio_callback *cb,
                         uint32_t pins)
{
    k_work_submit(&button_work);
}

static void configure_button(void)
{
    int ret;

    if (!gpio_is_ready_dt(&button)) {
        printk("Error: button device %s is not ready\n",
               button.port->name);
        return;
    }

    ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
    if (ret != 0) {
        printk("Error %d: failed to configure %s pin %d\n",
               ret, button.port->name, button.pin);
        return;
    }

    ret = gpio_pin_interrupt_configure_dt(&button,
                                        GPIO_INT_EDGE_TO_ACTIVE);
    if (ret != 0) {
        printk("Error %d: failed to configure interrupt on %s pin %d\n",
               ret, button.port->name, button.pin);
        return;
    }

    static struct gpio_callback button_cb_data;
    gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
    gpio_add_callback(button.port, &button_cb_data);
}

int main(void)
{
    int err;

    printk("Initializing...\n");

    k_work_init(&button_work, button_pressed_work_handler);
    configure_button();

    /* Initialize the Bluetooth Mesh Stack */
    err = bt_mesh_init(&prov, &comp);
    if (err) {
        printk("Bluetooth mesh init failed (err %d)\n", err);
        return 0;
    }

    /* Enable provisioning */
    err = bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);
    if (err) {
        printk("Failed to enable provisioning (err %d)\n", err);
        return 0;
    }

    printk("Mesh initialized\n");

    return 0;
}
