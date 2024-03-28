..
   Copyright (c) 2022-2023 Golioth, Inc.
   SPDX-License-Identifier: Apache-2.0

Golioth Multi MCU OTA Updates
#############################

Overview
********

An experimental project based on the Golioth fw_update_ sample that demonstrates
how to update an auxiliary MCU from the main MCU utilizing SMP (`Simple Management Protocol`_).

The main MCU is nRF9160 DK running an SMP Client and is responsible for downloading
the auxiliary MCUs image and transfering it over UART interface.
The auxiliary MCU is the nRF52840 DK, running the `SMP Server sample`_.

Hardware
========

Both the nRF9160 DK and nRF52840 DK use ``uart1`` interface for communication.

Connect nRF9160 DK and nRF52840 DK using wires:

.. list-table::
   :header-rows: 1

   * - nRF9160 DK
     - nRF52840 DK
    
   * - P0.00 (RX)
     - P1.02 (TX)

   * - P0.01 (TX)
     - P1.01 (RX) 
  
   * - GND
     - GND


Requirements
============

* Golioth credentials
* Network connectivity

Local set up
************

Do not clone this repo using git. Zephyr's ``west`` meta tool should be used to
set up your local workspace.

Install the Python virtual environment (recommended)
====================================================

.. code-block:: shell

   cd ~
   mkdir golioth-multi-mcu-update
   python -m venv golioth-multi-mcu-update/.venv
   source golioth-multi-mcu-update/.venv/bin/activate
   pip install wheel west

Use ``west`` to initialize and install
======================================

.. code-block:: shell

   cd ~/golioth-multi-mcu-update
   west init -m git@github.com:goliothlabs/multi-mcu-fw-update.git .
   west update
   west zephyr-export
   pip install -r deps/zephyr/scripts/requirements.txt

Apply Golioth Firmware SDK Patch
================================

Apply the patch to add experimental support for multi MCU updates to Golioth
Firmware SDK.

.. code-block:: shell

   cd ~/golioth-multi-mcu-update/deps/zephyr
   git am ../../app/0001-Zephyr-SMP-SVR-Example.patch

Apply Zephyr SMP Server Sample Patch
====================================

Apply the patch to add support for nRF52840 DK to the SMP Server sample.

.. code-block:: shell

   cd ~/golioth-multi-mcu-update/deps/modules/lib/golioth-firmware-sdk
   git am ../../../../app/0001-Golioth-Firmware-SDK-patch.patch

Creating a Release in the Golioth Console
=========================================

Build an application with MCUBoot for nRF52840 DK.

When creating the Artifact let the Package name be: ``aux-mcu``.

Building and Running
********************

Building the application for nRF9160 DK
=======================================

.. code-block:: shell

   $ (.venv) cd ~/golioth-multi-mcu-update
   $ (.venv) west build -p -b nrf9160dk_nrf9160_ns app --

Building the application for nRF52840 DK
========================================

.. code-block:: shell
    
    $ (.venv) cd ~/golioth-multi-mcu-update/deps/zephyr/samples/subsys/mgmt/mcumgr/smp_svr
    $ (.venv) west build -p -b nrf52840dk_nrf52840 -- -DEXTRA_CONF_FILE='overlay-serial.conf'


Authentication specific configuration
=====================================

PSK based auth - Hardcoded
--------------------------

Configure the following Kconfig options based on your Golioth
credentials:

* GOLIOTH_SAMPLE_PSK_ID - PSK ID of registered device
* GOLIOTH_SAMPLE_PSK - PSK of registered device

by chaning these lines in the configuration file:

.. code-block:: shell

    CONFIG_GOLIOTH_SAMPLE_PSK_ID="psk-id"
    CONFIG_GOLIOTH_SAMPLE_PSK="psk"

PSK based auth - Runtime
------------------------

We provide an option for setting Golioth credentials through the Zephyr
shell. This is based on the Zephyr Settings subsystem.

Add the following options to ``prj.conf``:

.. code-block:: shell

    CONFIG_GOLIOTH_SAMPLE_PSK_ID="psk-id"
    CONFIG_GOLIOTH_SAMPLE_PSK="psk"
    CONFIG_GOLIOTH_SAMPLE_HARDCODED_CREDENTIALS=n
    CONFIG_FLASH=y
    CONFIG_FLASH_MAP=y
    CONFIG_NVS=y
    CONFIG_SETTINGS=y
    CONFIG_SETTINGS_RUNTIME=y
    CONFIG_GOLIOTH_SAMPLE_PSK_SETTINGS=y
    CONFIG_GOLIOTH_SAMPLE_WIFI_SETTINGS=y
    CONFIG_GOLIOTH_SAMPLE_SETTINGS_AUTOLOAD=y
    CONFIG_GOLIOTH_SAMPLE_SETTINGS_SHELL=y


At runtime, configure PSK-ID and PSK using the device shell based on your
Golioth credentials:

.. code-block:: shell

    uart:~$ settings set golioth/psk-id <my-psk-id@my-project>
    uart:~$ settings set golioth/psk <my-psk>
    uart:-$ kernel reboot cold

Certificate based auth
----------------------

Configure the following Kconfig options based on your Golioth
credentials:

* CONFIG_GOLIOTH_AUTH_METHOD_CERT - use certificate-based
    authentication
* CONFIG_GOLIOTH_SAMPLE_HARDCODED_CRT_PATH - device certificate
* CONFIG_GOLIOTH_SAMPLE_HARDCODED_KEY_PATH - device private key

by adding these lines to configuration file (e.g. `prj.conf`):

.. code-block:: shell

    CONFIG_GOLIOTH_AUTH_METHOD_CERT=y
    CONFIG_GOLIOTH_SAMPLE_HARDCODED_CRT_PATH="keys/device.crt.der"
    CONFIG_GOLIOTH_SAMPLE_HARDCODED_KEY_PATH="keys/device.key.der"

Sample output
=================


A sample output from the serial console is found below.

The device initially reports firmware version `1.2.3` and about 90
seconds later it receives notification of a new firmware release
(`1.2.4`) from Golioth which triggers the update process.

After a successful update the new version number will be printed out on
the serial terminal and displayed on the Golioth web console.

nRF9160 DK output
-----------------

.. code-block:: shell

    *** Booting nRF Connect SDK v2.5.2 ***
    [00:00:00.513,336] <dbg> fw_update_sample: main: Start FW Update sample
    [00:00:00.513,366] <inf> golioth_samples: Waiting to obtain IP address
    [00:00:06.040,618] <inf> lte_monitor: Network: Searching
    [00:00:08.239,471] <inf> lte_monitor: Network: Registered (home)
    [00:00:08.240,570] <inf> golioth_mbox: Mbox created, bufsize: 1232, num_items: 10, item_size: 112
    [00:00:08.241,333] <inf> golioth_fw_update: Current firmware version: aux-mcu - 1.2.3
    [00:00:09.939,239] <inf> golioth_coap_client_zephyr: Golioth CoAP client connected
    [00:00:09.939,544] <inf> fw_update_sample: Golioth client connected
    [00:00:09.939,544] <inf> golioth_coap_client_zephyr: Entering CoAP I/O loop
    [00:00:10.202,636] <inf> golioth_fw_update: Waiting to receive OTA manifest
    [00:00:10.606,109] <inf> golioth_fw_update: Received OTA manifest
    [00:00:10.606,170] <inf> golioth_fw_update: Current version = 1.2.3, Target version = 3.0.1
    [00:00:10.606,201] <inf> golioth_fw_update: State = Downloading
    [00:00:10.856,567] <inf> golioth_fw_update: Image size = 71943
    [00:00:10.856,689] <inf> fw_block_processor: Downloading block index 0 (1/71)
    [00:00:11.926,605] <inf> mcuboot_util: Image index: 0, Swap type: none
    [00:00:11.926,635] <inf> golioth_fw_zephyr: swap type: none
    [00:00:12.024,475] <inf> fw_block_processor: Downloading block index 1 (2/71)
    [00:00:12.889,892] <inf> fw_block_processor: Downloading block index 2 (3/71)
    ...
    
    [00:00:58.114,654] <inf> fw_block_processor: Downloading block index 69 (70/71)
    [00:00:58.778,228] <inf> fw_block_processor: Downloading block index 70 (71/71)
    [00:01:00.005,859] <inf> golioth_fw_update: Download took 49149 ms
    [00:01:00.005,889] <inf> fw_block_processor: Block Latency Stats:
    [00:01:00.005,889] <inf> fw_block_processor:    Min: 434 ms
    [00:01:00.005,920] <inf> fw_block_processor:    Ave: %.3f ms
    [00:01:00.005,920] <inf> fw_block_processor:    Max: 1070 ms
    [00:01:00.005,950] <inf> fw_block_processor: Total bytes written: 71943
    [00:01:00.095,275] <inf> golioth_fw_update: State = Downloaded
    [00:01:00.578,552] <inf> golioth_fw_update: State = Updating
    [00:01:00.858,551] <inf> fw_update_sample: Sending the image to AUX MCU
    [00:01:13.976,562] <inf> fw_update_sample: Confirming the AUX image..
    [00:01:14.016,448] <inf> mcumgr_grp_img_client: User configured image list buffer size 1 can't store all info
    [00:01:14.016,510] <inf> fw_update_sample: Reset the AUX MCU ...


nRF52840 DK output
------------------

.. code-block:: shell

    *** Booting nRF Connect SDK v2.5.2 ***
    I: Starting bootloader
    I: Primary image: magic=good, swap_type=0x3, copy_done=0x1, image_ok=0x1
    I: Secondary image: magic=unset, swap_type=0x1, copy_done=0x3, image_ok=0x3
    I: Boot source: none
    I: Image index: 0, Swap type: none
    I: Bootloader chainload address offset: 0xc000
    *** Booting nRF Connect SDK v2.5.2 ***

After reseting the board:

.. code-block:: shell
    *** Booting nRF Connect SDK v2.5.2 ***
    I: Starting bootloader
    I: Primary image: magic=good, swap_type=0x3, copy_done=0x1, image_ok=0x1
    I: Secondary image: magic=good, swap_type=0x3, copy_done=0x3, image_ok=0x1
    I: Boot source: none
    I: Image index: 0, Swap type: perm
    I: Starting swap using move algorithm.
    I: Bootloader chainload address offset: 0xc000
    I: Jumping to the first image slot
    
    [00:00:01.001,861] <inf> hello_update: Hello World! nrf52840dk_nrf52840
    [00:00:02.002,014] <inf> hello_update: Hello World! nrf52840dk_nrf52840
    [00:00:03.002,227] <inf> hello_update: Hello World! nrf52840dk_nrf52840
    [00:00:04.002,380] <inf> hello_update: Hello World! nrf52840dk_nrf52840


.. _fw_update sample: https://github.com/golioth/golioth-firmware-sdk/tree/main/examples/zephyr/fw_update

.. _Simple Management Protocol: https://docs.zephyrproject.org/latest/services/device_mgmt/smp_protocol.html

.. _SMP Server sample: https://docs.zephyrproject.org/latest/samples/subsys/mgmt/mcumgr/smp_svr/README.html
