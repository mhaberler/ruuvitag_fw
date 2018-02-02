#include "application_service_if.h"
#include <stdint.h>

#include "nrf_ble_es.h"
#include "bsp.h"
#include "bsp_board_config.h"
#include "bluetooth_board_config.h"

#define NRF_LOG_MODULE_NAME "SERVICE"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

static ble_dfu_t m_dfus;    /**< Structure for DFU service. */

/**@brief Function for handling Eddystone events.
 *
 * @param[in] evt Eddystone event to handle.
 */
static void on_es_evt(nrf_ble_es_evt_t evt)
{
    switch(evt)
    {
        case NRF_BLE_ES_EVT_ADVERTISEMENT_SENT:
            //bsp_board_led_invert(NON_CONNECTABLE_ADV_LED_PIN);
            break;
        
        case NRF_BLE_ES_EVT_CONNECTABLE_ADV_STARTED:
            bsp_board_led_on(CONNECTABLE_ADV_LED_PIN);
            break;

        default:
            break;
    }
}

static void ble_dfu_evt_handler(ble_dfu_t * p_dfu, ble_dfu_evt_t * p_evt)
{
    switch (p_evt->type)
    {
        case BLE_DFU_EVT_INDICATION_DISABLED:
            NRF_LOG_INFO("Indication for BLE_DFU is disabled\r\n");
            break;

        case BLE_DFU_EVT_INDICATION_ENABLED:
            NRF_LOG_INFO("Indication for BLE_DFU is enabled\r\n");
            break;

        case BLE_DFU_EVT_ENTERING_BOOTLOADER:
            NRF_LOG_INFO("Device is entering bootloader mode!\r\n");
            break;
        default:
            NRF_LOG_INFO("Unknown event from ble_dfu\r\n");
            break;
    }
}

/**@brief Function for initializing the Services generated by Bluetooth Developer Studio.
 *
 *
 * @return      NRF_SUCCESS on successful initialization of services, otherwise an error code.
 */
uint32_t application_services_init(void)
{
    nrf_ble_es_init(on_es_evt);

    uint32_t       err_code = NRF_SUCCESS;

    // Initialize the Device Firmware Update Service.
    ble_dfu_init_t dfus_init;
    memset(&dfus_init, 0, sizeof(dfus_init));

    dfus_init.evt_handler                               = ble_dfu_evt_handler;
    dfus_init.ctrl_point_security_req_write_perm        = SEC_SIGNED;
    dfus_init.ctrl_point_security_req_cccd_write_perm   = SEC_SIGNED;

    err_code |= ble_dfu_init(&m_dfus, &dfus_init);
    NRF_LOG_INFO("DFU Init status: %s\r\n", (uint32_t)ERR_TO_STR(err_code));

    //Initialize DIS
    ble_dis_init_t    dis_init;
    memset(&dis_init, 0, sizeof(dis_init));

    // Create pseudo-unique name. Note: This should be disabled 
    unsigned int mac0 =  NRF_FICR->DEVICEID[0];
    unsigned int mac1 =  NRF_FICR->DEVICEID[1];
    char serial[SERIAL_LENGTH];
    uint8_t index = 0;
    sprintf(&serial[index], "%x", mac0);
    index+=4;
    sprintf(&serial[index], "%x", mac1);
    index+=4;
    serial[index++] = 0x00;
    NRF_LOG_DEBUG("SET Serial %s\r\n", (uint32_t)serial);
    ble_srv_ascii_to_utf8(&dis_init.manufact_name_str, INIT_MANUFACTURER); 
    ble_srv_ascii_to_utf8(&dis_init.model_num_str, INIT_MODEL);           
    ble_srv_ascii_to_utf8(&dis_init.serial_num_str, serial); 
    ble_srv_ascii_to_utf8(&dis_init.hw_rev_str, INIT_HWREV); 
    ble_srv_ascii_to_utf8(&dis_init.fw_rev_str, INIT_FWREV); 
    ble_srv_ascii_to_utf8(&dis_init.sw_rev_str, INIT_SWREV);


    //TODO: Find out proper values
    memset(&dis_init.p_sys_id, 0x00, sizeof(ble_dis_sys_id_t));
    memset(&dis_init.p_reg_cert_data_list, 0x00, sizeof(ble_dis_reg_cert_data_list_t));
    //dis_init.ble_dis_ieee_11073_20601_regulatory_certification_data_list_initial_value.data.list_len = 1;
    //dis_init.ble_dis_ieee_11073_20601_regulatory_certification_data_list_initial_value.data.p_list = m_dis_ieee_11073_20601_regulatory_certification_data_list_initial_value_data_arr; 
    //dis_init.ble_dis_pnp_id_initial_value.vendor_id_source.vendor_id_source = VENDOR_ID_SOURCE_BLUETOOTH_SIG_ASSIGNED_COMPANY_IDENTIFIER_VALUE_FROM_THE_ASSIGNED_NUMBERS_DOCUMENT; 
    memset(&dis_init.p_pnp_id, 0x00, sizeof(ble_dis_pnp_id_t));
    // Security           
    /**< Read permissions. */
    dis_init.dis_attr_md.read_perm.sm = 1;  /**< Security Mode (1 or 2), 0 for no permissions at all. */               
    dis_init.dis_attr_md.read_perm.lv = 1;  /**< Level (1, 2, 3 or 4), 0 for no permissions at all. */

    err_code |= ble_dis_init(&dis_init);
    NRF_LOG_INFO("DIS init, status %d\r\n", err_code);
    if (err_code != NRF_SUCCESS)
    {
        NRF_LOG_ERROR("Failed to init DIS\r\n");
        return err_code;
    }
    return NRF_SUCCESS;
}

/** Return pointer to BLE dfu service **/
ble_dfu_t* get_dfu(void)
{
  return &m_dfus;
}
