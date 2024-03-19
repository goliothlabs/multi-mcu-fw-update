/*
 * Copyright (c) 2023 Golioth, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(fw_update_sample, LOG_LEVEL_DBG);

#include <golioth/client.h>
#include <golioth/fw_update.h>
#include <samples/common/sample_credentials.h>
#include <string.h>
#include <zephyr/kernel.h>

#include <samples/common/net_connect.h>

#include "pm_config.h"
#include <zephyr/mgmt/mcumgr/mgmt/mgmt.h>
#include <zephyr/mgmt/mcumgr/grp/img_mgmt/img_mgmt.h>
#include <zephyr/mgmt/mcumgr/grp/img_mgmt/img_mgmt_client.h>
#include <zephyr/mgmt/mcumgr/smp/smp.h>
#include <zephyr/mgmt/mcumgr/smp/smp_client.h>
#include <zephyr/mgmt/mcumgr/transport/smp.h>
#include <mgmt/mcumgr/transport/smp_internal.h>

#define AUX_IMAGE_SLOT 1
#define OTA_NAME_MAIN_CPU "main"
#define OTA_NAME_AUXILIARY_CPU "aux-mcu"

// Current firmware version; update in prj.conf or via build argument
static const char *_current_version = CONFIG_MCUBOOT_IMGTOOL_SIGN_VERSION;

static struct golioth_fw_update_config ota_config_main_cpu = {
        .current_version = CONFIG_MCUBOOT_IMGTOOL_SIGN_VERSION,
        .fw_package_name = OTA_NAME_MAIN_CPU,
};

static struct golioth_fw_update_config ota_config_aux_cpu = {
        .current_version = CONFIG_MCUBOOT_IMGTOOL_SIGN_VERSION,
        .fw_package_name = OTA_NAME_AUXILIARY_CPU,
};

static K_SEM_DEFINE(connected, 0, 1);

static void on_client_event(struct golioth_client *client,
                            enum golioth_client_event event,
                            void *arg)
{
    bool is_connected = (event == GOLIOTH_CLIENT_EVENT_CONNECTED);
    if (is_connected)
    {
        k_sem_give(&connected);
    }
    LOG_INF("Golioth client %s", is_connected ? "connected" : "disconnected");
}

void upload_image_to_aux(void)
{

	LOG_INF("Sending the image to AUX MCU");

	static struct img_mgmt_client img_client;
	static struct smp_client_object smp_client;
	struct mcumgr_image_data image_list;
	struct mcumgr_image_upload img_upload;
	int image_list_size = 1;

	int err = 0;
	struct image_version ver;
	uint8_t hash[32];
	uint32_t flags = 0;
	int32_t aux_image_size;

	aux_image_size = get_image_size();

	err = img_mgmt_read_info(AUX_IMAGE_SLOT, &ver, hash, &flags);

	err = smp_client_object_init(&smp_client, SMP_SERIAL_TRANSPORT);
	if(err) {
		LOG_ERR("SMP Client Object init error: %d", err);
	}

	img_mgmt_client_init(&img_client, &smp_client, image_list_size, &image_list);

	err = img_mgmt_client_upload_init(&img_client, aux_image_size, AUX_IMAGE_SLOT, NULL);
	if(err) {
		LOG_ERR("SMP Client Object init error: -%d", err);
	}

	err = img_mgmt_client_upload(&img_client, (uint8_t *)(PM_MCUBOOT_SECONDARY_ADDRESS), aux_image_size, &img_upload);
	if(err) {
		LOG_ERR("IMG MGMT CLIENT UPLOAD ERR: -%d", err);
	}

	LOG_INF("Confirming the AUX image..");
	struct mcumgr_image_state img_state;
	err = img_mgmt_client_state_write(&img_client, hash, true, &img_state);
	if(err) {
		LOG_ERR("CLient State Write error: -%d", err);
	}

	LOG_INF("Reset the AUX MCU ...");
}

int main(void)
{
    LOG_DBG("Start FW Update sample");

    net_connect();

    /* Note: In production, you would provision unique credentials onto each
     * device. For simplicity, we provide a utility to hardcode credentials as
     * kconfig options in the samples.
     */
    const struct golioth_client_config *client_config = golioth_sample_credentials_get();

    struct golioth_client *client = golioth_client_create(client_config);

    golioth_client_register_event_callback(client, on_client_event, NULL);

    golioth_fw_update_init_with_config(client, &ota_config_aux_cpu);

    k_sem_take(&connected, K_FOREVER);

    /* No while(true) loop needed, the Golioth client thread will handle updates */
    golioth_fw_update_register_callback(upload_image_to_aux, NULL);

    return 0;
}
