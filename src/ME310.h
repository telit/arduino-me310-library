/*Copyright (C) 2021 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    ME310.h

  @brief
    Driver Library for ME310 Telit Modem

  @details
    The library contains a single class that implements a C++ interface to all ME310 AT Commands.
    It makes it easy to build Arduino applications that use the full power of ME310 module

  @version
    2.6.0

  @note
    Dependencies:
    Arduino.h
    ATCommandDataParsing.h

  @author

  @date
    28/10/2020
*/
#ifndef __ME310__H
#define __ME310__H

/* Include files ================================================================================*/
#include "Arduino.h"
#include <vector>

namespace me310
{

   #define ME310_BUFFSIZE 3100 ///< Exchange buffer size
   #define ME310_SEND_BUFFSIZE 1500

   #define F(A) A

   #define _READ(A,B,C) return_t read_ ## A(tout_t aTimeout = C){return read_send_wait(F(B),OK_STRING,aTimeout);}
   #define _TEST(A,B,C) return_t test_ ## A(tout_t aTimeout = C){return test_send_wait(F(B),OK_STRING,aTimeout);}
   #define _READ_TEST(A,B,C) _READ(A,B,C) _TEST(A,B,C)

   /*! \class ME310
      \brief Implements the ME310 driver with synchronous calls
      \details
      The library implements all AT commands described in ME310 AT command reference manual.\n
      The library implements synchronous calls, but has virtual callbacks for intercepting async messages.\n
      The last parameter of each call is the timeout between the command and the expected answer.\n
      Each function returns a code of type return_t that informs if the call was correcly executed or not or if the timeout is reached.\n
      Most functions have a read_<i>function_name</i> and/or a test_<i>function_name</i> that implements the ATcommand? and ATcommand=? respectively.\n
      These auxiliary function are generated with \_READ, \_TEST or \_READ_TEST macros.\n
      The generated functions have only the timeout as argument and return a return_t code.\n

      The strings returned by the modem are stored in the class memory buffer and can be requested using an index.\n
      A null return value means end of string buffer.\n
   */
   class ME310
   {
      public:

      /*! \enum Return value
         \brief Enum of return value  methods
      */
      typedef enum {
         RETURN_VALID = 0,      ///< Return value if valid return code detected
         RETURN_ERROR = 1,      ///< Return value if ERROR or CME ERROR return code detected
         RETURN_DATA  = 2,      ///< Return value for generic data detected
         RETURN_NO_CARRIER = 3, ///< Return value if no carrier return code detected

         RETURN_TOUT = -1,      ///< Return value on timeout
         RETURN_CONTINUE = -2,  ///< Return value to continue parsing modem answer
         RETURN_ASYNC = -3      ///< Return Value for async call
      } return_t;


      /*! \enum Timeout
         \brief Timeout value in milliseconds
      */
      typedef enum
      {

         TOUT_0MS =   0, TOUT_100MS =   100, TOUT_200MS =   200, TOUT_300MS =   300, TOUT_400MS =   400, TOUT_500MS =   500, TOUT_600MS =   600, TOUT_700MS =   700, TOUT_800MS =   800, TOUT_900MS =   900,
         TOUT_1SEC =   1000, TOUT_2SEC =   2000, TOUT_3SEC =   3000, TOUT_4SEC =   4000, TOUT_5SEC =   5000, TOUT_6SEC =   6000, TOUT_7SEC =   7000, TOUT_8SEC =   8000, TOUT_9SEC =   9000,
         TOUT_10SEC = 10000, TOUT_20SEC = 20000, TOUT_30SEC = 30000, TOUT_45SEC = 45000,
         TOUT_1MIN  = 60000, TOUT_2MIN = 120000
      } tout_t;


      #ifdef ARDUINO_TELIT_SAMD_CHARLIE
      ME310(Uart &aSerial = SerialModule);
      #else
      ME310(Uart &aSerial);
      #endif

      ~ME310();

      #ifdef ARDUINO_TELIT_SAMD_CHARLIE
      void powerOn(unsigned int onoff_gpio = ON_OFF);
      #else
      void powerOn(unsigned int onoff_gpio);
      #endif

      void begin(unsigned long baudRate);
      void end();

   // Command Line Prefixes -------------------------------------------------------
      return_t attention(tout_t aTimeout = TOUT_100MS);
      return_t repeat_last_auto(tout_t aTimeout = TOUT_100MS);
      return_t repeat_last(tout_t aTimeout = TOUT_100MS);

   // Generic Modem Control -------------------------------------------------------
      return_t select_interface_style(int value = 2,tout_t aTimeout = TOUT_100MS);
      _READ_TEST(select_interface_style,"AT#SELINT",TOUT_100MS)

      return_t set_factory_config(int value = 0,tout_t aTimeout = TOUT_100MS);
      return_t soft_reset(int value = 0,tout_t aTimeout = TOUT_100MS);
      return_t default_reset_basic_profile_designation(int value = 0,tout_t aTimeout = TOUT_100MS);
      return_t default_reset_full_profile_designation(int value = 0,tout_t aTimeout = TOUT_100MS);
      return_t store_current_configuration(int value = 0,tout_t aTimeout = TOUT_100MS);
      return_t display_internal_phonebook_number(int value = 0,tout_t aTimeout = TOUT_100MS);
      return_t extended_reset(int value = 0,tout_t aTimeout = TOUT_100MS);
      _TEST(extended_reset,"AT#Z",TOUT_100MS)

      return_t display_config_profile(tout_t aTimeout = TOUT_100MS);

      /*return_t country_installation(int value = 59,tout_t aTimeout = TOUT_100MS);
      _READ_TEST(country_installation,"AT+CGI",TOUT_100MS) */

      return_t capabilities_list(tout_t aTimeout = TOUT_100MS);
      _TEST(capabilities_list,"AT+GCAP",TOUT_100MS)

      return_t manufacturer_identification(tout_t aTimeout = TOUT_100MS);
      _TEST(manufacturer_identification,"AT+GMI",TOUT_100MS)

      return_t model_identification(tout_t aTimeout = TOUT_100MS);
      _TEST(model_identification,"AT+GMM",TOUT_100MS)

      return_t revision_identification(tout_t aTimeout = TOUT_100MS);
      _TEST(revision_identification,"AT+GMR",TOUT_100MS)

      return_t serial_number(tout_t aTimeout = TOUT_100MS);
      _TEST(serial_number,"AT+GSN",TOUT_100MS)

      return_t request_manufacturer_identification(tout_t aTimeout = TOUT_100MS);
      _TEST(request_manufacturer_identification,"AT+CGMI",TOUT_100MS)

      return_t request_model_identification(tout_t aTimeout = TOUT_100MS);
      _TEST(request_model_identification,"AT+CGMM",TOUT_100MS)

      return_t request_revision_identification(tout_t aTimeout = TOUT_100MS);
      _TEST(request_revision_identification,"AT+CGMR",TOUT_100MS)

      return_t request_psn_identification(tout_t aTimeout = TOUT_100MS);
      _TEST(request_psn_identification,"AT+CGSN",TOUT_100MS)

      return_t request_manufacturer_identification_echo(tout_t aTimeout = TOUT_100MS);
      _TEST(request_manufacturer_identification_echo,"AT#CGMI",TOUT_100MS)

      return_t request_revision_identification_echo(tout_t aTimeout = TOUT_100MS);
      _TEST(request_revision_identification_echo,"AT#CGMR",TOUT_100MS)

      return_t request_psn_identification_echo(tout_t aTimeout = TOUT_100MS);
      _TEST(request_psn_identification_echo,"AT#CGSN",TOUT_100MS)

      return_t request_product_code(tout_t aTimeout = TOUT_100MS);
      _TEST(request_product_code,"AT#CGMF",TOUT_100MS)

      return_t request_software_package_version(tout_t aTimeout = TOUT_100MS);
      _TEST(request_software_package_version,"AT#SWPKGV",TOUT_100MS)

      return_t phone_activity_status(tout_t aTimeout = TOUT_100MS);
      _TEST(phone_activity_status,"AT+CPAS",TOUT_100MS)

      return_t set_phone_functionality(int fun = 1,int rst = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(set_phone_functionality,"AT+CFUN",TOUT_100MS)

      return_t mobile_equipment_event_reporting(int mode = 0, int keyp = 0, int disp = 0, int ind = 0, int bfr = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(mobile_equipment_event_reporting,"AT+CMER",TOUT_100MS)

      return_t set_voice_mail_number(int mode = 0,const char *number = "", int type = 129, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(set_voice_mail_number,"AT+CSVM",TOUT_100MS)

      return_t mailbox_numbers(int index = 0, const char *number = "", int type = 129, const char *text ="", const char *mboxtype="", tout_t aTimeout = TOUT_100MS);
      _TEST(mailbox_numbers,"AT#MBN",TOUT_100MS)

      return_t message_waiting_indication(int enable = 1, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(message_waiting_indication,"AT#MWI",TOUT_100MS)

      return_t available_at_commands(tout_t aTimeout = TOUT_100MS);
      _TEST(available_at_commands,"AT+CLAC",TOUT_100MS)

      return_t select_language(const char * lan = "en", tout_t aTimeout = TOUT_100MS);
      _READ_TEST(select_language,"AT#LANG",TOUT_100MS)

      return_t report_mobile_equipment_error(int n = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(report_mobile_equipment_error,"AT+CMEE",TOUT_100MS)

      return_t extended_numeric_error_report(tout_t aTimeout = TOUT_100MS);
      _TEST(extended_numeric_error_report,"AT#CEER",TOUT_100MS)

      return_t power_saving_mode_ring_indicator(int n = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(power_saving_mode_ring_indicator,"AT#PSMRI",TOUT_100MS)

      return_t select_te_character_set(const char* chset = "IRA", tout_t aTimeout = TOUT_100MS);
      _READ_TEST(select_te_character_set,"AT+CSCS",TOUT_100MS)

      return_t multiplexing_mode(int mode = 0, int subset = 0, int port_speed = 5, int n1 = 31, int t1=10, int n2=3, int t2=30, int t3 = 10, int k=2, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(multiplexing_mode,"AT+CMUX",TOUT_100MS)

      return_t usb_configuration(int mode = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(usb_configuration,"AT#USBCFG",TOUT_100MS)

      return_t connect_physycal_port_sap(int variant = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(connect_physycal_port_sap,"AT#PORTCFG",TOUT_100MS)

      return_t at_command_delay(int delay = 0, tout_t aTimeout = TOUT_100MS);
      _TEST(at_command_delay,"AT#ATDELAY",TOUT_100MS)

      return_t store_telephone_number(int n = 0, const char *nr = "", tout_t aTimeout = TOUT_100MS);

      return_t display_last_connection_statistics(tout_t aTimeout = TOUT_100MS);

      return_t request_imei_software_version(tout_t aTimeout = TOUT_100MS);
      _TEST(request_imei_software_version,"AT+IMEISV",TOUT_100MS)

      return_t request_model_identification_echo(tout_t aTimeout = TOUT_100MS);
      _TEST(request_model_identification_echo,"AT#CGMM",TOUT_100MS)

      return_t display_current_configuration_profile(tout_t aTimeout = TOUT_100MS);

      return_t set_active_firmware_image(int image_number = 0, int storage_conf = 1, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(set_active_firmware_image, "AT#FWSWITCH", TOUT_100MS)

      return_t ims_pdp_apn_number_set(const char* pdpApnName, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(ims_pdp_apn_number_set,"AT#IMSPDPSET",TOUT_100MS)

      return_t request_telit_id(tout_t aTimeout = TOUT_100MS);
      _TEST(request_telit_id,"AT#TID",TOUT_100MS)

   // S Parameters ----------------------------------------------------------------
      return_t number_rings_auto_answer(int n, tout_t aTimeout = TOUT_100MS);
      _READ(number_rings_auto_answer,"ATS0",TOUT_100MS)

      return_t ring_counter(tout_t aTimeout = TOUT_100MS);
      _READ(ring_counter,"ATS1",TOUT_100MS)

      return_t escape_character(int chr = 43, tout_t aTimeout = TOUT_100MS);
      _READ(escape_character,"ATS2",TOUT_100MS)

      return_t command_line_terminator_character(int chr = 13, tout_t aTimeout = TOUT_100MS);
      _READ(command_line_terminator_character,"ATS3",TOUT_100MS)

      return_t response_formatting_character(int chr = 10, tout_t aTimeout = TOUT_100MS);
      _READ(response_formatting_character,"ATS4",TOUT_100MS)

      return_t command_line_editing_character(int chr = 8, tout_t aTimeout = TOUT_100MS);
      _READ(command_line_editing_character,"ATS5",TOUT_100MS)

      return_t connection_completed_timeout(int tout = 60, tout_t aTimeout = TOUT_100MS);
      _READ(connection_completed_timeout,"ATS7",TOUT_100MS)

      return_t escaper_prompt_delay(int time = 50, tout_t aTimeout = TOUT_100MS);
      _READ(escaper_prompt_delay,"ATS12",TOUT_100MS)

      return_t delay_dtr_off(int time = 5, tout_t aTimeout = TOUT_100MS);
      _READ(delay_dtr_off,"ATS25",TOUT_100MS)

      return_t s_registers_display(tout_t aTimeout = TOUT_100MS);

      return_t carrier_off_firm_time(int n , tout_t aTimeout = TOUT_100MS);

      return_t extended_s_registers_display(tout_t aTimeout = TOUT_100MS);

   // DTE Modem Interface Control -------------------------------------------------

      return_t command_echo(int n = 1, tout_t aTimeout = TOUT_100MS);
      return_t quiet_result_codes(int n = 0, tout_t aTimeout = TOUT_100MS);
      return_t response_format(int n = 1, tout_t aTimeout = TOUT_100MS);
      return_t identification_information(int n = 0, tout_t aTimeout = TOUT_100MS);
      return_t data_carrier_detect_control(int n = 0, tout_t aTimeout = TOUT_100MS);
      return_t data_terminal_ready_control(int n = 0, tout_t aTimeout = TOUT_100MS);
      return_t flow_control(int n = 3, tout_t aTimeout = TOUT_100MS);
      return_t data_set_ready_control(int n = 3, tout_t aTimeout = TOUT_100MS);

      return_t uart_dce_interface_speed(int rate = 115200, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(uart_dce_interface_speed,"AT+IPR",TOUT_100MS)

      return_t dte_modem_local_control_flow(int byDTE = 2, int byDCE = 2, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(dte_modem_local_control_flow,"AT+IFC",TOUT_100MS)

      return_t dte_modem_character_framing(int format = 3, int parity = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(dte_modem_character_framing,"AT+ICF",TOUT_100MS)

      return_t skip_escape_sequence(int mode = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(skip_escape_sequence,"AT#SKIPESC",TOUT_100MS)

      return_t escape_sequence_guard_time(int gt = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(escape_sequence_guard_time,"AT#E2ESC",TOUT_100MS)

      return_t extended_result_codes(int n = 1, tout_t aTimeout = TOUT_100MS);

   // Call Control ----------------------------------------------------------------
      return_t dialup_connection(tout_t aTimeout = TOUT_100MS);
      return_t hang_up(tout_t aTimeout = TOUT_100MS);
      return_t return_online(tout_t aTimeout = TOUT_100MS);

      return_t set_dialing_mode(int mode = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(set_dialing_mode, "AT#DIALMODE", TOUT_100MS)

   // Modulation and Compression Control ------------------------------------------
      return_t line_quality_auto_retain(int n, tout_t aTimeout = TOUT_100MS);

   // Network ---------------------------------------------------------------------
      return_t subscriber_number(tout_t aTimeout = TOUT_100MS);
      _TEST(subscriber_number,"AT+CNUM",TOUT_100MS)

      return_t read_operator_names(tout_t aTimeout = TOUT_100MS);
      _TEST(read_operator_names,"AT+COPN",TOUT_100MS)

      return_t network_registration_status(int mode = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(network_registration_status,"AT+CREG",TOUT_100MS)

      return_t operator_selection(int mode = 0, tout_t aTimeout = TOUT_100MS);
      return_t operator_selection(int mode, int format, const char *oper, int act, tout_t aTimeout = TOUT_100MS);
      return_t operator_selection(int mode, int format, int oper, int act, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(operator_selection,"AT+COPS",TOUT_20SEC)

      return_t facility_lock_unlock(const char * fac, int mode, const char *password, int classType, tout_t aTimeout = TOUT_100MS);
      return_t facility_lock_unlock(const char * fac, int mode, tout_t aTimeout = TOUT_100MS);
      return_t facility_lock_unlock(const char * fac, int mode, const char *password, tout_t aTimeout = TOUT_100MS);
      _TEST(facility_lock_unlock,"AT+CLCK",TOUT_100MS)

      return_t change_facility_password(const char * fac, const char *old_password, const char *new_password, tout_t aTimeout = TOUT_100MS);
      _TEST(change_facility_password,"AT+CPWD",TOUT_100MS)

      return_t list_current_calls(tout_t aTimeout = TOUT_100MS);
      _TEST(list_current_calls, "AT+CLCC", TOUT_100MS)

      return_t preferred_operator_list(int index, int format, const char *oper, int gsm_act, int gsm_compact_cact, int utran_act, int e_utran_actn, tout_t aTimeout = TOUT_100MS);
      return_t preferred_operator_list(tout_t aTimeout = TOUT_100MS);
      _READ_TEST(preferred_operator_list,"AT+CPOL",TOUT_100MS)

      return_t selection_preferred_plmn_list(int list = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(selection_preferred_plmn_list,"AT+CPLS",TOUT_100MS)

      return_t signal_quality(tout_t aTimeout = TOUT_100MS);
      _TEST(signal_quality,"AT+CSQ",TOUT_100MS)

      return_t serving_cell_information(tout_t aTimeout = TOUT_100MS);
      _TEST(serving_cell_information,"AT#SERVINFO",TOUT_100MS)

      return_t lock_to_single_BCCH_ARFCN(int lockedBcch = 1024, int lockedUarfcn = 0, int lockedPsc = 65535, int lockedEarfcn = 0, const char *lockedPci = "FFFF" , tout_t aTimeout = TOUT_100MS);
      _READ_TEST(lock_to_single_BCCH_ARFCN, "AT#BCCHLOCK", TOUT_100MS)

      return_t network_emergency_number_update(int en = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(network_emergency_number_update,"AT#NWEN",TOUT_100MS)

      return_t update_plmn_list(int action, int mcc, int mnc, const char* plmnname, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(update_plmn_list,"AT#PLMNUPDATE",TOUT_100MS)

      return_t plmn_list_selection(int mode = 1, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(plmn_list_selection,"AT#PLMNMODE",TOUT_100MS)

      return_t select_band(int band, int umts_band, int lte_band, int tdscdma_band, int lte_band_over_64, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(select_band,"AT#BND",TOUT_100MS)

      return_t automatic_band_selection(int value = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(automatic_band_selection,"AT#AUTOBND",TOUT_100MS)

      return_t set_subscriber_number(int index, const char *number, const char *alpha, tout_t aTimeout = TOUT_100MS);
      _TEST(SET_subscriber_number,"AT#SNUM",TOUT_100MS)

      return_t extended_numeric_error_report_net(tout_t aTimeout = TOUT_100MS);
      _TEST(extended_numeric_error_report_net,"AT#CEERNET",TOUT_100MS)

      return_t extended_error_report_network_reject_cause(int func = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(extended_error_report_network_reject_cause,"AT#CEERNETEXT",TOUT_100MS)

      return_t ciphering_indication(int mode = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(ciphering_indication,"AT#CIPHIND",TOUT_100MS)

      return_t packet_service_network_type(int mode = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(packet_service_network_type,"AT#PSNT",TOUT_100MS)

      return_t set_encryption_algorithm(int encgsm = 5, int encgprs = 7, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(set_encryption_algorithm,"AT#ENCALG",TOUT_100MS)

      return_t set_mode_operation_eps(int mode = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(set_mode_operation_eps,"AT+CEMODE",TOUT_100MS)

      return_t extended_signal_quality(tout_t aTimeout = TOUT_100MS);
      _TEST(extended_signal_quality,"AT+CESQ",TOUT_100MS)

      return_t enhanced_network_selection(int mode = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(enhanced_network_selection,"AT#ENS",TOUT_100MS)

      return_t select_wireless_network(int value,tout_t aTimeout = TOUT_100MS);
      _READ_TEST(select_wireless_network,"AT+WS46",TOUT_100MS)

      return_t edrx_settings(int mode = 0, tout_t aTimeout = TOUT_100MS);
      return_t edrx_settings(int mode, int acttype, const char * req_edrx, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(edrx_settings,"AT+CEDRXS",TOUT_100MS)

      return_t calling_line_identification_presentation (int enable = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(calling_line_identification_presentation, "AT+CLIP", TOUT_100MS)

      return_t select_iot_technology(int n , tout_t aTimeout = TOUT_100MS);
      _READ_TEST(select_iot_technology,"AT#WS46",TOUT_100MS)

      return_t edrx_read_dynamic_parameters(tout_t aTimeout = TOUT_100MS);
      _TEST(edrx_read_dynamic_parameters,"AT+CEDRXRDP",TOUT_100MS)

      return_t eps_network_registration_status(int mode , tout_t aTimeout = TOUT_100MS);
      _READ_TEST(eps_network_registration_status,"AT+CEREG",TOUT_100MS)

      return_t read_current_network_status(tout_t aTimeout = TOUT_100MS);
      _TEST(read_current_network_status,"AT#RFSTS",TOUT_100MS)

      return_t read_sim_field_spn(tout_t aTimeout = TOUT_100MS);
      _TEST(read_sim_field_spn,"AT#SPN",TOUT_100MS)

      return_t extended_edrx_settings(int mode = 0, int acttype = 2, const char *req_edrx = "0000", const char *reqpagetimewindow = "0000", tout_t aTimeout = TOUT_100MS);
      _READ_TEST(extended_edrx_settings,"AT#CEDRXS",TOUT_100MS)

      return_t cell_monitor(int number = 0, tout_t aTimeout = TOUT_100MS);
      _TEST(cell_monitor,"AT#MONI",TOUT_100MS)

      return_t LTE_frame_information( tout_t aTimeout = TOUT_100MS);
      _READ_TEST(LTE_frame_information, "AT#LTESFN", TOUT_100MS)

      return_t SNR_set_level(int level = 2, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(SNR_set_level, "AT#SNRSET", TOUT_100MS)

      return_t catm1_nbiot_band_setting(int m1_band_1_64, int m1_band_65_128, int nb1_band_1_64, int nb1_band_65_128 , tout_t aTimeout = TOUT_100MS);
      return_t catm1_nbiot_band_setting(tout_t aTimeout = TOUT_100MS);
      _READ_TEST(catm1_nbiot_band_setting,"AT#IOTBND",TOUT_100MS)

      return_t inter_RAT_timer_setting(int irat_timer = 60, int search_alignment = 20, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(inter_RAT_timer_setting, "AT#IRATTIMER", TOUT_100MS)

      return_t enable_NB2_mode(int ena = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(enable_NB2_mode, "AT#NB2ENA", TOUT_100MS)

   // SIM -------------------------------------------------------------------------

      return_t enter_pin(const char* pin,tout_t aTimeout = TOUT_100MS);
      return_t enter_pin(const char* pin,const char* newpin, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(enter_pin,"AT+CPIN",TOUT_100MS)

      return_t display_pin_counter(tout_t aTimeout = TOUT_100MS);
      _TEST(display_pin_counter,"AT#PCT",TOUT_100MS)

      return_t read_iccid(tout_t aTimeout = TOUT_100MS);
      _TEST(read_iccid,"AT+CCID",TOUT_100MS)

      return_t imsi(tout_t aTimeout = TOUT_100MS);
      _TEST(imsi,"AT+CIMI",TOUT_100MS)

      return_t imsi_echo(tout_t aTimeout = TOUT_100MS);
      _TEST(imsi_echo,"AT#CIMI",TOUT_100MS)

      return_t sim_detection_mode(int mode = 2,tout_t aTimeout = TOUT_100MS);
      _READ_TEST(sim_detection_mode,"AT#SIMDET",TOUT_100MS)

      return_t sim_presence_status(int mode = 0,tout_t aTimeout = TOUT_100MS);
      _READ_TEST(sim_presence_status,"AT#SIMPR",TOUT_100MS)

      return_t read_iccid_1(tout_t aTimeout = TOUT_100MS);
      _TEST(read_iccid_1,"AT#CCID",TOUT_100MS)

      return_t query_sim_status(int mode = 0,tout_t aTimeout = TOUT_100MS);
      _READ_TEST(query_sim_status,"AT#QSS",TOUT_100MS)

      return_t restricted_sim_access(int command, int field, int p1, int p2, int p3, const char *data, tout_t aTimeout = TOUT_100MS);
      _TEST(restricted_sim_access,"AT+CRSM",TOUT_100MS)

      return_t generic_sim_access(int length, const char *command, tout_t aTimeout = TOUT_100MS);
      _TEST(generic_sim_access,"AT+CSIM",TOUT_100MS)

      return_t open_logical_channel(const char * dfname, tout_t aTimeout = TOUT_100MS);
      _TEST(open_logical_channel,"AT+CCHO",TOUT_100MS)

      return_t close_logical_channel(int sessionid, tout_t aTimeout = TOUT_100MS);
      _TEST(close_logical_channel,"AT+CCHC",TOUT_100MS)

      return_t generic_uicc_logical_channell_access(int sessionid, int length, const char *command,tout_t aTimeout = TOUT_100MS);
      _TEST(generic_uicc_logical_channell_access,"AT+CGLA",TOUT_100MS)

      return_t read_iccid_2(tout_t aTimeout = TOUT_100MS);
      _TEST(read_iccid_2, "AT+ICCID", TOUT_100MS)

      return_t simin_pin_configuration(int gpiopin, int simindetmode, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(simin_pin_configuration,"AT#SIMINCFG",TOUT_100MS)

      return_t automatic_switch_firmware(int mode = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(automatic_switch_firmware, "AT#FWAUTOSIM", TOUT_100MS)

   // SIM Toolkit -----------------------------------------------------------------

      return_t simtoolkit_interface_action(int mode = 1, int timeout = 2, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(simtoolkit_interface_action,"AT#STIA",TOUT_100MS)

      return_t simtoolkit_get_information(int cmdType, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(simtoolkit_get_information,"AT#STGI",TOUT_100MS)

      return_t simtoolkit_send_response(int cmdType, int userAction, const char *data, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(simtoolkit_send_response,"AT#STSR",TOUT_100MS)

   // SMS & CB --------------------------------------------------------------------

      return_t select_messaging_service(int service = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(select_messaging_service,"AT+CSMS",TOUT_100MS)

      return_t preferred_message_storage(const char * memr = "SE", const char *memw = "SM", const char *mems = "SM", tout_t aTimeout = TOUT_100MS);
      _READ_TEST(preferred_message_storage,"AT+CPMS",TOUT_100MS)

      return_t message_format(int mode = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(message_format,"AT+CMGF",TOUT_100MS)

      return_t service_center_address(const char *number, int type = 145, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(service_center_address,"AT+CSCA",TOUT_100MS)

      return_t set_text_mode_parameters(int fo, const char *vp, int pid, int dcs, tout_t aTimeout = TOUT_100MS);
      return_t set_text_mode_parameters(int fo, int vp, int pid, int dcs, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(set_text_mode_parameters,"AT+CSMP",TOUT_100MS)

      return_t show_text_mode_parameters(int show = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(show_text_mode_parameters,"AT+CSDH",TOUT_100MS)

      return_t save_settings(int profile = 0, tout_t aTimeout = TOUT_100MS);
      _TEST(save_settings,"AT+CSAS",TOUT_100MS)

      return_t restore_settings(int profile = 0, tout_t aTimeout = TOUT_100MS);
      _TEST(restore_settings,"AT+CRES",TOUT_100MS)

      return_t more_message_send(int n = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(more_message_send,"AT+CMMS",TOUT_100MS)

      return_t new_message_indications_TE(int mode = 0, int mt = 0, int bm = 0, int ds= 0, int bfr = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(new_message_indications_TE,"AT+CNMI",TOUT_100MS)

      return_t new_message_ack(int n, int length, tout_t aTimeout = TOUT_100MS);
      return_t new_message_ack(tout_t aTimeout = TOUT_100MS);
      _READ_TEST(new_message_ack,"AT+CNMA",TOUT_100MS)

      return_t list_messages(int stat, tout_t aTimeout = TOUT_100MS);
      return_t list_messages(const char *stat, tout_t aTimeout = TOUT_100MS);
      return_t list_messages(tout_t aTimeout = TOUT_100MS);
      _TEST(list_messages,"AT+CMGL",TOUT_100MS)

      return_t read_message(int index, tout_t aTimeout = TOUT_100MS);
      _TEST(read_message,"AT+CMGR",TOUT_100MS)

      return_t send_short_message(int length, char* data, tout_t aTimeout = TOUT_100MS);
      return_t send_short_message(const char *da, const char *toda, char* data, tout_t aTimeout = TOUT_100MS);
      _TEST(send_short_message,"AT+CMGS",TOUT_100MS)

      return_t send_message_from_storage(const char *index, const char *da, const char *toda,tout_t aTimeout = TOUT_100MS);
      _TEST(send_message_from_storage,"AT+CMSS",TOUT_100MS)

      return_t write_short_message_memory(int length, int stat, char *data, tout_t aTimeout = TOUT_100MS);
      return_t write_short_message_memory(const char *da, int toda, const char *stat, char *data, tout_t aTimeout = TOUT_100MS);
      _TEST(write_short_message_memory,"AT+CMGW",TOUT_100MS)

      return_t delete_message(int index, int delflag = 0,tout_t aTimeout = TOUT_100MS);
      _TEST(delete_message,"AT+CMGD",TOUT_100MS)

      return_t select_service_mo_sms(int service = 1,tout_t aTimeout = TOUT_100MS);
      _READ_TEST(select_service_mo_sms,"AT+CGSMS",TOUT_100MS)

      return_t sms_commands_operation_mode(int mode = 1,tout_t aTimeout = TOUT_100MS);
      _READ_TEST(sms_commands_operation_mode,"AT#SMSMODE",TOUT_100MS)

      return_t report_concatenated_sms_indexes(tout_t aTimeout = TOUT_100MS);
      _TEST(report_concatenated_sms_indexes,"AT#CMGLCONCINDEX",TOUT_100MS)

      return_t sms_ring_indicator(int n = 0,tout_t aTimeout = TOUT_100MS);
      _READ_TEST(sms_ring_indicator,"AT#E2SMSRI",TOUT_100MS)

      return_t sms_overflow(int mode = 0,tout_t aTimeout = TOUT_100MS);
      _READ_TEST(sms_overflow,"AT#SMOV",TOUT_100MS)

      return_t sms_move(int index = 0,tout_t aTimeout = TOUT_100MS);
      _READ_TEST(sms_move,"AT#SMSMOVE",TOUT_100MS)

   // Phonebook -------------------------------------------------------------------

      return_t phonebook_select_memory_storage(const char *storage, const char *password, tout_t aTimeout = TOUT_100MS);
      return_t phonebook_select_memory_storage(const char *storage,tout_t aTimeout = TOUT_100MS);
      _READ_TEST(phonebook_select_memory_storage,"AT+CPBS",TOUT_100MS)

      return_t phonebook_read_entries( int index1, int index2, tout_t aTimeout = TOUT_100MS);
      return_t phonebook_read_entries( int index1, tout_t aTimeout = TOUT_100MS);
      _TEST(phonebook_read_entries,"AT+CPBR",TOUT_100MS)

      return_t phonebook_find_entries(const char *findtext, tout_t aTimeout = TOUT_100MS);
      _TEST(phonebook_find_entries,"AT+CPBF",TOUT_100MS)

      return_t phonebook_write_entry(int index, const char *number, int type, const char *text, const char *group, const char * adnumber, int adtype, const char *secondtext, const char * email, int hidden, tout_t aTimeout = TOUT_100MS);
      return_t phonebook_write_entry(int index, const char *number, int type, const char *text, const char *group, tout_t aTimeout = TOUT_100MS);
      _TEST(phonebook_write_entry,"AT+CPBW",TOUT_100MS)

      return_t phonebook_read_group_entries( int indexFirst, int indexLast, tout_t aTimeout = TOUT_100MS);
      return_t phonebook_read_group_entries( int index, tout_t aTimeout = TOUT_100MS);
      _TEST(phonebook_read_group_entries,"AT#CPBGR",TOUT_100MS)

      return_t phonebook_write_group_entry( int index, const char *text, tout_t aTimeout = TOUT_100MS);
      _TEST(phonebook_write_group_entry,"AT#CPBGW",TOUT_100MS)

      return_t phonebook_delete_all_entries(tout_t aTimeout = TOUT_100MS);
      _TEST(phonebook_delete_all_entries,"AT#CPBD",TOUT_100MS)

   // Time & Alarm ----------------------------------------------------------------

      return_t clock_management(const char *time, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(clock_management,"AT+CCLK",TOUT_100MS)

      return_t alarm_management(const char *time, int index = 0, int type = 1, const char *string ="", const char *recurr="", int silent = 0,tout_t aTimeout = TOUT_100MS);
      _READ_TEST(alarm_management,"AT+CALA",TOUT_100MS)

      return_t alarm_postpone(int sec = 0,tout_t aTimeout = TOUT_100MS);
      _TEST(alarm_postpone,"AT+CAPD",TOUT_100MS)

      return_t setting_date_format(int mode = 1, int auxmode = 1,tout_t aTimeout = TOUT_100MS);
      _READ_TEST(setting_date_format,"AT+CSDF",TOUT_100MS)

      return_t time_zone_reporting(int onoff = 0,tout_t aTimeout = TOUT_100MS);
      _READ_TEST(time_zone_reporting,"AT+CTZR",TOUT_100MS)

      return_t automatic_time_zone_update(int onoff = 0,tout_t aTimeout = TOUT_100MS);
      _READ_TEST(automatic_time_zone_update,"AT+CTZU",TOUT_100MS)

      return_t network_identity_time_zone(int val = 7, int mode = 0,tout_t aTimeout = TOUT_100MS);
      _READ_TEST(network_identity_time_zone,"AT#NITZ",TOUT_100MS)

      return_t clock_management_ext(const char *time,tout_t aTimeout = TOUT_100MS);
      _READ_TEST(clock_management_ext,"AT#CCLK",TOUT_100MS)

      return_t clock_mode(int mode = 0,tout_t aTimeout = TOUT_100MS);
      _READ_TEST(clock_mode,"AT#CCLKMODE",TOUT_100MS)

      return_t wake_alarm_mode(int opmode = 0,tout_t aTimeout = TOUT_100MS);
      _READ_TEST(wake_alarm_mode,"AT#WAKE",TOUT_100MS)

      return_t setting_time_format(int mode = 0,tout_t aTimeout = TOUT_100MS);
      _READ_TEST(setting_time_format,"AT+CSTF",TOUT_100MS)

      return_t alarm_delete(int n = 0,tout_t aTimeout = TOUT_100MS);
      _TEST(alarm_delete,"AT+CALD",TOUT_100MS)

   // Audio -----------------------------------------------------------------------
      return_t microphone_mute_control(int n, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(microphone_mute_control, "AT+CMUT", TOUT_100MS)

      return_t open_audio_loop(int mode = 0,tout_t aTimeout = TOUT_100MS);
      _READ_TEST(open_audio_loop, "AT#OAP", TOUT_100MS)


   // Digital Voice Interface -----------------------------------------------------
      return_t digital_voiceband_interface(int mode = 0, int dviport = 0, int clockmode = 1, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(digital_voiceband_interface, "AT#DV", TOUT_100MS)

   // HW and Radio Control --------------------------------------------------------

      return_t battery_charger_status(tout_t aTimeout = TOUT_100MS);
      _TEST(battery_charger_status,"AT#CBC",TOUT_100MS)

      return_t gpio_control(int pin, int mode = 0, int dir= 0, int save = 0,tout_t aTimeout = TOUT_100MS);
      _READ_TEST(gpio_control,"AT#GPIO",TOUT_100MS)

      return_t alarm_pin_configuration(int pin = 0,tout_t aTimeout = TOUT_100MS);
      _READ_TEST(alarm_pin_configuration,"AT#ALARMPIN",TOUT_100MS)

      return_t stat_led_gpio_setting(int mode = 5,int onDuration = 10, int offDuration = 10, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(stat_led_gpio_setting,"AT#SLED",TOUT_100MS)

      return_t stat_led_gpio_setting_save(tout_t aTimeout = TOUT_100MS);
      _TEST(stat_led_gpio_setting_save,"AT#SLEDSAV",TOUT_100MS)

      return_t adc_read(int adc = 1,int mode = 2, int dir = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(adc_read,"AT#ADC",TOUT_100MS)

      return_t v24_output_pin_configuration(int pin = 0,int mode = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(v24_output_pin_configuration,"AT#V24CFG",TOUT_100MS)

      return_t v24_output_pin_control(int pin = 0,int state = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(v24_output_pin_control,"AT#V24",TOUT_100MS)

      return_t i2c_write(int sdaPin,int sclPin, int deviceId, int registerId, int len, char* data, tout_t aTimeout = TOUT_100MS);
      _TEST(i2c_write,"AT#I2CWR",TOUT_100MS)

      return_t i2c_read(int sdaPin,int sclPin, int deviceId, int registerId, int len, tout_t aTimeout = TOUT_100MS);
      _TEST(i2c_read,"AT#I2CRD",TOUT_100MS)

      return_t i2c_write_read(int sdaPin,int sclPin, int deviceId, int lenwr, int lenrd, char* data, tout_t aTimeout = TOUT_100MS);
      _TEST(i2c_write_read,"AT#I2CCF",TOUT_100MS)

      return_t test_mode_configuration(const char *cmd, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(test_mode_configuration,"AT#TESTMODE",TOUT_100MS)


   // Power Down ------------------------------------------------------------------

      return_t module_reboot(tout_t aTimeout = TOUT_100MS);
      _TEST(module_reboot,"AT#REBOOT",TOUT_100MS)

      return_t periodic_reset(int mode, int delay = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(periodic_reset,"AT#ENHRST",TOUT_100MS)

      return_t software_shutdown(tout_t aTimeout = TOUT_100MS);
      _TEST(software_shutdown,"AT#SHDN",TOUT_100MS)

      return_t system_turnoff(tout_t aTimeout = TOUT_100MS);
      _READ_TEST(system_turnoff,"AT#SYSHALT",TOUT_100MS)

      return_t fast_shutdown_configuration(int enable, int gpio, tout_t aTimeout = TOUT_100MS);
      return_t fast_shutdown_configuration(tout_t aTimeout = TOUT_100MS);
      _READ_TEST(fast_shutdown_configuration,"AT#FASTSHDN",TOUT_100MS)

   // Easy Scan -------------------------------------------------------------------

      return_t network_survey(int s, int e, tout_t aTimeout = TOUT_100MS);
      return_t network_survey(tout_t aTimeout = TOUT_100MS);

      return_t nertwork_survey_numeric_format(int s, int e, tout_t aTimeout = TOUT_100MS);
      return_t nertwork_survey_numeric_format( tout_t aTimeout = TOUT_100MS);

      return_t network_survey_format(int format = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(network_survey_format,"AT#CSURVF",TOUT_100MS)

      return_t network_survey_crlf(int value = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(network_survey_crlf,"AT#CSURVNLF",TOUT_100MS)

      return_t network_survey_extended(int value = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(network_survey_extended,"AT#CSURVEXT",TOUT_100MS)

   // Jamming Detection and Report --------------------------------------------------

      return_t jamming_detect_report(int mode = 0, int sat2G = 45, int carrNum2G = 100, int pRxLevT2G = 15, int p_rssi_t4g = 20, int p_rsrq_t4g = 20, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(jamming_detect_report,"AT#JDRENH2",TOUT_100MS)

      return_t jamming_detect_threshold(int p_rsrp_t4g = 20, int p_rsrq_t4g = 20, int initial_delay = 500, int sampling_number = 100,int p_rssi_s4g = -50, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(jamming_detect_threshold,"AT#JDR4GCFG",TOUT_100MS)

   // Packet Domain ---------------------------------------------------------------

      return_t define_pdp_context(int cid, const char *pdp_type, const char * apn, tout_t aTimeout = TOUT_1SEC);
      _READ_TEST(define_pdp_context,"AT+CGDCONT",TOUT_100MS)

      return_t show_pdp_address(int cid, tout_t aTimeout = TOUT_1SEC);
      return_t show_pdp_address(tout_t aTimeout = TOUT_1SEC);
      _TEST(show_pdp_address,"AT+CGPADDR",TOUT_100MS)

      return_t auto_attach_property(int _auto = 0, tout_t aTimeout = TOUT_1SEC);
      _READ_TEST(auto_attach_property,"AT#AUTOATT",TOUT_100MS)

      return_t multislot_class_control(int _class = 33, int autoattach = 0, tout_t aTimeout = TOUT_1SEC);
      _READ_TEST(multislot_class_control,"AT#MSCLASS",TOUT_100MS)

      return_t ppp_data_connection_auth_type(int type = 3, tout_t aTimeout = TOUT_1SEC);
      _READ_TEST(ppp_data_connection_auth_type,"AT#GAUTH",TOUT_100MS)

      return_t define_pdp_context_auth_params(int cid, int auth_type, const char *username, const char *password, tout_t aTimeout = TOUT_1SEC);
      _READ_TEST(define_pdp_context_auth_params,"AT+CGAUTH",TOUT_100MS)

      return_t pdp_context_read_dynamic_parameters(tout_t aTimeout = TOUT_1SEC);
      return_t pdp_context_read_dynamic_parameters(int cid, tout_t aTimeout = TOUT_1SEC);
      _TEST(pdp_context_read_dynamic_parameters,"AT+CGCONTRDP",TOUT_100MS)

      return_t printing_ip_address_format(int ipv6_addressFormat = 0, int ipv6_subnetNotation = 0, int ipv6_leadingZeros = 0, int ipv6_compressZeros = 0, tout_t aTimeout = TOUT_1SEC);
      _READ_TEST(printing_ip_address_format,"AT+CGPIAF",TOUT_100MS)

      return_t pdp_context_activate(int cid, int stat, tout_t aTimeout = TOUT_1SEC);
      return_t pdp_context_activate(tout_t aTimeout = TOUT_1SEC);
      _READ_TEST(pdp_context_activate,"AT+CGACT",TOUT_100MS)

      return_t packet_domain_event_reporting(int mode = 0, int bfr = 0, tout_t aTimeout = TOUT_1SEC);
      _READ_TEST(packet_domain_event_reporting,"AT+CGEREP",TOUT_100MS)

      return_t ppp_configuration(int mode = 0, tout_t aTimeout = TOUT_1SEC);
      _READ_TEST(ppp_configuration,"AT#PPPCFG",TOUT_100MS)

      return_t gprs_network_registration_status(int mode = 0, tout_t aTimeout = TOUT_1SEC);
      _READ_TEST(gprs_network_registration_status,"AT+CGREG",TOUT_100MS)

      return_t ps_attach_detach(int state, tout_t aTimeout = TOUT_1SEC);
      _READ_TEST(ps_attach_detach,"AT+CGATT",TOUT_100MS)

      return_t sending_originating_data(int cid, int cpdataLength, const char *cpdata, int rai = 0, int typeOfUserData = 0, tout_t aTimeout = TOUT_1SEC);
      _TEST(sending_originating_data,"AT+CSODCP",TOUT_100MS)

      return_t reporting_terminating_data(int reporting = 0, tout_t aTimeout = TOUT_1SEC);
      _READ_TEST(reporting_terminating_data,"AT+CRTDCP",TOUT_100MS)

   // IPEasy ----------------------------------------------------------------------

      return_t context_activation(int cid, int stat, const char *userid, const char *pwd, tout_t aTimeout = TOUT_1SEC);
      return_t context_activation(int cid, int stat, tout_t aTimeout = TOUT_1SEC);
      _READ_TEST(context_activation,"AT#SGACT",TOUT_100MS)

      return_t pdp_context_auth_type(int type = 1, tout_t aTimeout = TOUT_1SEC);
      _READ_TEST(pdp_context_auth_type,"AT#SGACTAUTH",TOUT_100MS)

      return_t pdp_automatic_context_activation(int cid, int retry = 0, int delay = 180, int urcmode = 0, tout_t aTimeout = TOUT_1SEC);
      _READ_TEST(pdp_automatic_context_activation,"AT#SGACTCFG",TOUT_100MS)

      return_t extended_pdp_context_activation(int cid, int abortAttemptEnable = 0, tout_t aTimeout = TOUT_1SEC);
      _READ_TEST(extended_pdp_context_activation,"AT#SGACTCFGEXT",TOUT_100MS)

      return_t socket_configuration(int connId, int cid, int pktSz = 300, int maxTo = 90, int connTo = 600, int txTo = 50, tout_t aTimeout = TOUT_1SEC);
      _READ_TEST(socket_configuration,"AT#SCFG",TOUT_100MS)

      return_t socket_configuration_extended(int connId = 1, int srMode = 0, int recvDataMode = 0, int keepalive = 0, int listenAutoRsp = 0, int sendDataMode = 0, tout_t aTimeout = TOUT_1SEC);
      _READ_TEST(socket_configuration_extended,"AT#SCFGEXT",TOUT_100MS)

      return_t socket_configuration_extended_2(int connId, int bufferStart = 0, int abortConnAttempt= 0, int unusedB = 0, int unusedC = 0, int noCarrierMode = 0, tout_t aTimeout = TOUT_1SEC);
      _READ_TEST(socket_configuration_extended_2,"AT#SCFGEXT2",TOUT_100MS)

      return_t socket_parameters_reset(tout_t aTimeout = TOUT_100MS);
      _TEST(socket_parameters_reset,"AT#SKTRST",TOUT_100MS)

      return_t socket_dial(int connId, int txProt, int rPort, const char *IPaddr, int closureType, int lPort = 0, int connMode = 0, int txTime = 0, int userIpType = 0, tout_t aTimeout = TOUT_1SEC);
      return_t socket_dial(int connId, int txProt, int rPort, const char *IPaddr, tout_t aTimeout = TOUT_1SEC);
      _TEST(socket_dial,"AT#SD",TOUT_100MS)

      return_t socket_restore(int connId, tout_t aTimeout = TOUT_1SEC);
      _TEST(socket_restore,"AT#SO",TOUT_100MS)

      return_t socket_shutdown(int connId, tout_t aTimeout = TOUT_1SEC);
      _TEST(socket_shutdown,"AT#SH",TOUT_100MS)

      return_t socket_listen(int connId, int listenState, int listenPort, int lingerT = 255, tout_t aTimeout = TOUT_1SEC);
      _READ_TEST(socket_listen,"AT#SL",TOUT_100MS)

      return_t socket_listen_UDP(int connId, int listenState, int listenPort, tout_t aTimeout = TOUT_1SEC);
      _READ_TEST(socket_listen_UDP,"AT#SLUDP",TOUT_100MS)

      return_t socket_accept(int connId, int connMode = 0, tout_t aTimeout = TOUT_1SEC);
      _TEST(socket_accept,"AT#SA",TOUT_100MS)

      return_t socket_send_data_command_mode(int connId, char* data,int rai = 1, tout_t aTimeout = TOUT_1SEC);
      _TEST(socket_send_data_command_mode,"AT#SSEND",TOUT_100MS)

      return_t socket_send_data_command_mode_extended(int connId, int bytesToSend, char* data, int rai = 1, tout_t aTimeout = TOUT_1SEC);
      _TEST(socket_send_data_command_mode_extended,"AT#SSENDEXT",TOUT_100MS)

      return_t socket_receive_data_command_mode(int connId, int maxByte, int udpInfo = 0, tout_t aTimeout = TOUT_1SEC);
      _TEST(socket_receive_data_command_mode,"AT#SRECV",TOUT_100MS)

      return_t socket_send_udp_data_specific_remote_host(int connId, const char *remoteIP, int remotePort, int rai, char* data, tout_t aTimeout = TOUT_1SEC);
      _TEST(socket_send_udp_data_specific_remote_host,"AT#SSENDUDP",TOUT_100MS)

      return_t socket_send_udp_data_specific_remote_host_extended(int connId, int bytesToSend, const char *remoteIP, int remotePort, int rai, char* data, tout_t aTimeout = TOUT_1SEC);
      _TEST(socket_send_udp_data_specific_remote_host_extended,"AT#SSENDUDPEXT",TOUT_100MS)

      return_t socket_detect_cause_disconnection(int connId, tout_t aTimeout = TOUT_1SEC);
      _TEST(socket_detect_cause_disconnection,"AT#SLASTCLOSURE",TOUT_100MS)

      return_t socket_status(tout_t aTimeout = TOUT_100MS);
      return_t socket_status(int connId, tout_t aTimeout = TOUT_100MS);
      _TEST(socket_status,"AT#SS",TOUT_100MS)

      return_t socket_info(int connId, tout_t aTimeout = TOUT_1SEC);
      return_t socket_info(tout_t aTimeout = TOUT_1SEC);
      _TEST(socket_info,"AT#SI",TOUT_100MS)

      return_t socket_type(int connId, tout_t aTimeout = TOUT_1SEC);
      return_t socket_type(tout_t aTimeout = TOUT_1SEC);
      _TEST(socket_type,"AT#ST",TOUT_100MS)

      return_t pad_command_features(int mode, tout_t aTimeout = TOUT_1SEC);
      _READ_TEST(pad_command_features,"AT#PADCMD",TOUT_100MS)

      return_t pad_forward_character(int _char = 13, int mode = 0, tout_t aTimeout = TOUT_1SEC);
      _READ_TEST(pad_forward_character,"AT#PADFWD",TOUT_100MS)

      return_t base64_encode(int connId, int enc = 0, int dec = 0, tout_t aTimeout = TOUT_1SEC);
      _READ_TEST(base64_encode,"AT#BASE64",TOUT_100MS)

      return_t firewall_setup(int action, const char *ip_addr = "000.000.000.000",  const char *net_mask = "000.000.000.000", tout_t aTimeout = TOUT_1SEC);
      _READ_TEST(firewall_setup,"AT#FRWL",TOUT_100MS)

      return_t socket_listen_ring_indicator(int n = 0, tout_t aTimeout = TOUT_1SEC);
      _READ_TEST(socket_listen_ring_indicator,"AT#E2SLRI",TOUT_100MS)

      return_t ping_support(int mode = 1, tout_t aTimeout = TOUT_1SEC);
      _READ_TEST(ping_support,"AT#ICMP",TOUT_100MS)

      return_t ping(const char *ipaddr, int retryNum, int len, int timeout, int ttl, tout_t aTimeout);
      return_t ping(const char *ipaddr, tout_t aTimeout = TOUT_1SEC);
      _TEST(ping,"AT#PING",TOUT_100MS)

      return_t dns_query(const char *host_name, tout_t aTimeout = TOUT_1SEC);
      _TEST(dns_query,"AT#QDNS",TOUT_100MS)

      return_t dns_from_network(int cid, tout_t aTimeout = TOUT_1SEC);
      return_t dns_from_network(tout_t aTimeout = TOUT_1SEC);
      _TEST(dns_from_network,"AT#NWDNS",TOUT_100MS)

      return_t ntp(const char *ntpaddress, int ntpport, int updModClock =1, int timeout = 2, int timezone = 4, tout_t aTimeout = TOUT_100MS);
      _TEST(ntp,"AT#NTP",TOUT_100MS)

      return_t configure_ntp_parameters(int cid, int authType = 0, int keyID = 0, char* keysFilePath = "",  tout_t aTimeout = TOUT_100MS);
      _READ_TEST(configure_ntp_parameters,"AT#NTPCFG",TOUT_100MS)

      return_t socket_configuration_extended3(int connId, int immRsp = 0, int closureType = 0, int fastSRing = 0,int ssendTimeout = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(socket_configuration_extended3,"AT#SCFGEXT3",TOUT_100MS)

   // FTPEasy ---------------------------------------------------------------------

      return_t ftp_append(const char *fileName, char* data, int connMode = 0, tout_t aTimeout = TOUT_100MS);
      _TEST(ftp_append,"AT#FTPAPP",TOUT_100MS)

      return_t ftp_append_extended(int bytesToSend, char* data, int eof = 0, tout_t aTimeout = TOUT_100MS);
      _TEST(ftp_append_extended,"AT#FTPAPPEXT",TOUT_100MS)

      return_t ftp_close(tout_t aTimeout = TOUT_1SEC);
      _TEST(ftp_close,"AT#FTPCLOSE",TOUT_100MS)

      return_t ftp_change_working_directory(const char *dirname, tout_t aTimeout = TOUT_100MS);
      return_t ftp_change_working_directory(tout_t aTimeout = TOUT_100MS);
      _TEST(ftp_change_working_directory,"AT#FTPCWD",TOUT_100MS)

      return_t ftp_delete(const char *filename, tout_t aTimeout = TOUT_100MS);
      _TEST(ftp_delete,"AT#FTPDELE",TOUT_100MS)

      return_t ftp_get_file_size(const char *filename, tout_t aTimeout = TOUT_100MS);
      _TEST(ftp_get_file_size,"AT#FTPFSIZE",TOUT_100MS)

      return_t ftp_get(const char *filename, tout_t aTimeout = TOUT_100MS);
      _TEST(ftp_get,"AT#FTPGET",TOUT_100MS)

      return_t ftp_get_command_mode(const char *filename, int viewMode = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(ftp_get_command_mode,"AT#FTPGETPKT",TOUT_100MS)

      return_t ftp_list(const char *name, tout_t aTimeout = TOUT_100MS);
      return_t ftp_list(tout_t aTimeout = TOUT_100MS);
      _TEST(ftp_list,"AT#FTPLIST",TOUT_100MS)

      return_t ftp_read_message(tout_t aTimeout = TOUT_100MS);
      _TEST(ftp_read_message,"AT#FTPMSG",TOUT_100MS)

      return_t ftp_open(const char *server_port,const char *username, const char *password,int viewMode, int cid, tout_t aTimeout = TOUT_100MS);
      _TEST(ftp_open,"AT#FTPOPEN",TOUT_100MS)

      return_t ftp_put(const char *filename, int connMode = 0, tout_t aTimeout = TOUT_100MS);
      _TEST(ftp_put,"AT#FTPPUT",TOUT_100MS)

      return_t ftp_print_working_directory(tout_t aTimeout = TOUT_100MS);
      _TEST(ftp_print_working_directory,"AT#FTPPWD",TOUT_100MS)

      return_t ftp_receive_data_command_mode(int block_size, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(ftp_receive_data_command_mode,"AT#FTPRECV",TOUT_100MS)

      return_t ftp_restart_posizion_get(int restartPosition, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(ftp_restart_posizion_get,"AT#FTPREST",TOUT_100MS)

      return_t ftp_time_out(int tout = 100, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(ftp_time_out,"AT#FTPTO",TOUT_100MS)

      return_t ftp_type(int type = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(ftp_type,"AT#FTPTYPE",TOUT_100MS)

      return_t ftp_configuration(int tout = 100, int ipPIgnoring=0, int ftpSen = 0, int ftpext = 1, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(ftp_configuration,"AT#FTPCFG",TOUT_100MS)

   /*
      return_t cache_dns(int mode, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(cache_dns,"AT#CACHEDNS",TOUT_100MS)

      return_t manual_dns(int cid, const char *primary, const char *secondary, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(manual_dns,"AT#DNS",TOUT_100MS)
   */
   // SMTP ------------------------------------------------------------------------

      return_t smtp_mail_server(const char *smtp, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(smtp_mail_server,"AT#ESMTP",TOUT_100MS)

      return_t smtp_read_message(tout_t aTimeout = TOUT_100MS);
      _TEST(smtp_read_message,"AT#EMAILMSG",TOUT_100MS)

      return_t smtp_configure(int ssl_enabled = 0, int port = 25, int mode = 0, int un1 = 0, int un2 = 0, int cid = 1, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(smtp_configure,"AT#SMTPCFG",TOUT_100MS)

      return_t smtp_reset_parameters(tout_t aTimeout = TOUT_100MS);
      _TEST(smtp_reset_parameters,"AT#ERST",TOUT_100MS)

      return_t smtp_user(const char *eUser, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(smtp_user,"AT#EUSER",TOUT_100MS)

      return_t smtp_save_parameters(tout_t aTimeout = TOUT_100MS);
      _TEST(smtp_save_parameters,"AT#ESAV",TOUT_100MS)

      return_t smtp_password(const char *ePwd, tout_t aTimeout = TOUT_100MS);
      _TEST(smtp_password,"AT#EPASSW",TOUT_100MS)

      return_t smtp_mail_send(const char *da, const char *subj, char* data, tout_t aTimeout = TOUT_100MS);
      _TEST(smtp_mail_send,"AT#EMAILD",TOUT_100MS)

      return_t smtp_sender_address(const char *eAddr, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(smtp_sender_address,"AT#EADDR",TOUT_100MS)

   // HTTP ------------------------------------------------------------------------

      return_t configure_http_parameters(int prof_id, const char *server_address, int server_port, int auth_type, const char *username = "", const char *password = "", int ssl_enabled = 0, int timeout=120, int cid=1, int pkt_size = 0,tout_t aTimeout = TOUT_100MS);
      return_t configure_http_parameters(int prof_id, const char *server_address, int server_port, int auth_type, int ssl_enabled = 0, int timeout=120, int cid=1,tout_t aTimeout = TOUT_100MS);
      _READ_TEST(configure_http_parameters,"AT#HTTPCFG",TOUT_100MS)

      return_t send_http_query(int prof_id, int command, const char *resource, const char *extra_header_line,tout_t aTimeout = TOUT_100MS);
      return_t send_http_query(int prof_id, int command, const char *resource, tout_t aTimeout = TOUT_100MS);
      _TEST(send_http_query,"AT#HTTPQRY",TOUT_100MS)

      return_t send_http_send(int prof_id, int command, const char *resource, int data_len, char *data, const char *post_param ="", const char *extra_header_line = "",tout_t aTimeout = TOUT_100MS);
      _TEST(send_http_send,"AT#HTTPSND",TOUT_100MS)

      void receive_http_data_start(int prof_id, int max_byte = 0);
      return_t receive_http_data(int prof_id, int max_byte = 0,tout_t aTimeout = TOUT_100MS);
      _TEST(receive_http_data,"AT#HTTPRCV",TOUT_100MS)

   // SSL -------------------------------------------------------------------------

      return_t ssl_configure_general_param(int ssid , int cid, int pktSx =0, int maxTo = 90, int defTo = 100, int txTo = 50, int SSLSRingMode = 0, int noCarrierMode = 0, int skipHostMismatch = 1, int equalizeTx = 0, int unused1 = 0, int unused2 = 0,tout_t aTimeout = TOUT_100MS);
      _READ_TEST(ssl_configure_general_param,"AT#SSLCFG",TOUT_100MS)

      return_t ssl_configure_security_param(int ssid, int cipherSuite = 0, int authMode = 0,tout_t aTimeout = TOUT_100MS);
      _READ_TEST(ssl_configure_security_param,"AT#SSLSECCFG",TOUT_100MS)

      return_t ssl_enable(int ssid, int enable = 0,tout_t aTimeout = TOUT_100MS);
      _READ_TEST(ssl_enable,"AT#SSLEN",TOUT_100MS)

      return_t ssl_socket_open(int ssid, int rPort, const char *IPAddress, int closureType = 0, int connMode = 1, int timeout = 100, tout_t aTimeout = TOUT_100MS);
      _TEST(ssl_socket_open,"AT#SSLD",TOUT_100MS)

      return_t ssl_socket_restore(int ssid,tout_t aTimeout = TOUT_100MS);
      _TEST(ssl_socket_restore,"AT#SSLO",TOUT_100MS)

      return_t ssl_socket_close(int ssid, int closureType = 0, tout_t aTimeout = TOUT_100MS);
      _TEST(ssl_socket_close,"AT#SSLH",TOUT_100MS)

      return_t ssl_socket_send_data(int ssid, char* data, int timeout = 100, tout_t aTimeout = TOUT_100MS);
      _TEST(ssl_socket_send_data,"AT#SSLSEND",TOUT_100MS)

      return_t ssl_socket_receive_data(int ssid, int maxNumByte, int timeout = 100, tout_t aTimeout = TOUT_100MS);
      _TEST(ssl_socket_receive_data,"AT#SSLRECV",TOUT_100MS)

      return_t ssl_socket_status(int ssid,tout_t aTimeout = TOUT_100MS);
      _TEST(ssl_socket_status,"AT#SSLS",TOUT_100MS)

      return_t ssl_socket_info(int ssid,tout_t aTimeout = TOUT_100MS);
      return_t ssl_socket_info(tout_t aTimeout = TOUT_100MS);
      _TEST(ssl_socket_info,"AT#SSLI",TOUT_100MS)

      return_t ssl_socket_send_data_command_mode(int ssid, int bytestosend, char* data, int timeout = 100, tout_t aTimeout = TOUT_100MS);
      _TEST(ssl_socket_send_data_command_mode,"AT#SSLSENDEXT",TOUT_100MS)

      return_t ssl_security_data(int ssid, int action, int dataType, int size = 0, int md5WhenReading = 0, char* data= "", tout_t aTimeout = TOUT_100MS);
      _READ_TEST(ssl_security_data,"AT#SSLSECDATA",TOUT_100MS)

      return_t ssl_additional_parameters(int ssid, int version = 4, int SNI = 0, int preloadedCA = 1, int customCA = 1, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(ssl_additional_parameters,"AT#SSLSECCFG2",TOUT_100MS)

   // CIoT Optimization -----------------------------------------------------------
      return_t ciot_optimization_configuration(int n = 0, int supportedUEopt = 3, int preferredUEopt = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(ciot_optimization_configuration, "AT+CCIOTOPT", TOUT_100MS)

      return_t ciot_optimization_configuration2(char *bitmask, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(ciot_optimization_configuration2, "AT#CCIOTOPT", TOUT_100MS)

   // IoT Portal-------------------------------------------------------------------
      return_t odis_command_saving_retrieving_parameters(char* hostUniqueDevId = "HUID0", char *hostManufacturer = "HMAN0", char* hostModel = "HMOD0", char *hostSwVersion = "HSW0", tout_t aTimeout = TOUT_100MS);
      _READ_TEST(odis_command_saving_retrieving_parameters, "AT+ODIS", TOUT_100MS)

      return_t enableLWM2M(int enable, int ctxID,tout_t aTimeout=TOUT_1SEC);
      _READ_TEST(enableLWM2M,"AT#LWM2MENA",TOUT_100MS)

      return_t writeResource(int agent,int objID,int instanceID,int resourceID, int resourceInstance,int value, tout_t aTimeout=TOUT_100MS);
      _READ_TEST(writeResource,"AT#LWM2MW",TOUT_100MS);

      return_t  writeResourcefloat(int agent,int objID,int instanceID,int resourceID, int resourceInstance,float value, tout_t aTimeout=TOUT_100MS);
      _READ_TEST(writeResourcefloat,"AT#LWM2MW", TOUT_100MS);

      return_t disableLWM2M(int disable, tout_t aTimeout=TOUT_1SEC);
      _READ_TEST(disableLWM2M,"AT#LWM2MENA",TOUT_100MS);

      return_t setResourcefloat(int type,int objID,int instanceID,int resourceID, int resourceInstance,float value, tout_t aTimeout=TOUT_100MS);
      _READ_TEST(setResorcefloat,"AT#LWM2MSET",TOUT_100MS);

      return_t  setResourceBool(int type,int objID,int instanceID,int resourceID, int resourceInstance,int value, tout_t aTimeout=TOUT_100MS);
      _READ_TEST(setResourceBool,"AT#LWM2MSET",TOUT_100MS);

      return_t readResourcefloat(int agent,int objID,int instanceID,int resourceID, int resourceInstance, tout_t aTimeout=TOUT_100MS);
      _READ_TEST(readResourcefloat,"AT#LWM2MR",TOUT_100MS);

      return_t FOTA_set_extended_URC(int enable = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(FOTA_set_extended_URC, "AT#FOTAURC", TOUT_100MS)

      return_t OTA_delta_write(int size,  tout_t aTimeout = TOUT_100MS);
      _TEST(OTA_delta_write, "AT#OTAUPW", TOUT_100MS)

      return_t odis_parameters_management(int param, int action, char* value = "", int instance = 0, tout_t aTimeout = TOUT_100MS);
      _TEST(odis_parameters_management, "AT#HOSTODIS", TOUT_100MS)

   // M2M -------------------------------------------------------------------------

      return_t m2m_chdir(const char *path,tout_t aTimeout = TOUT_100MS);
      _READ_TEST(m2m_chdir,"AT#M2MCHDIR",TOUT_100MS)

      return_t m2m_mkdir(const char *directory_name,tout_t aTimeout = TOUT_100MS);
      _TEST(m2m_mkdir,"AT#M2MMKDIR",TOUT_100MS)

      return_t m2m_set_backup(int enable,tout_t aTimeout = TOUT_100MS);
      _READ_TEST(m2m_set_backup,"AT#M2MBACKUP",TOUT_100MS)

      return_t m2m_rmdir(const char *directory_name,tout_t aTimeout = TOUT_100MS);
      _TEST(m2m_rmdir,"AT#M2MRMDIR",TOUT_100MS)

      return_t m2m_application_execution(int mode = 0, int delay = 10,tout_t aTimeout = TOUT_100MS);
      _READ_TEST(m2m_application_execution,"AT+M2M",TOUT_100MS)

      return_t m2m_set_run_file_permission(int mode, const char * file_bin, int delay = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(m2m_set_run_file_permission,"AT#M2MRUN",TOUT_100MS)

      return_t m2m_delete(const char *file_name,tout_t aTimeout = TOUT_100MS);
      _TEST(m2m_delete,"AT#M2MDEL",TOUT_100MS)

      return_t m2m_write_file(const char *file_name, int size, int binToMod, char* data, tout_t aTimeout = TOUT_100MS);
      _TEST(m2m_write_file,"AT#M2MWRITE",TOUT_100MS)

      return_t m2m_list(const char *path,tout_t aTimeout = TOUT_100MS);
      return_t m2m_list(tout_t aTimeout = TOUT_100MS);
      _TEST(m2m_list,"AT#M2MLIST",TOUT_100MS)

      return_t m2m_read(const char *file_name,tout_t aTimeout = TOUT_100MS);
      _TEST(m2m_read,"AT#M2MREAD",TOUT_100MS)

      return_t m2m_ram_info(tout_t aTimeout = TOUT_100MS);
      _TEST(m2m_ram_info,"AT#M2MRAM",TOUT_100MS)

      return_t m2m_set_arguments(const char* file_bin, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(m2m_set_arguments, "AT#M2MARG", TOUT_100MS)

   // MQTT ------------------------------------------------------------------------

      return_t mqtt_enable(int instanceNumber, int enable = 0,tout_t aTimeout = TOUT_100MS);
      _READ_TEST(mqtt_enable,"AT#MQEN",TOUT_100MS)

      return_t mqtt_configure(int instanceNumber, const char *hostname, int port = 0, int cid = 0, int sslEn = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(mqtt_configure,"AT#MQCFG",TOUT_100MS)

      return_t mqtt_configure_2(int instanceNumber, int keepalive = 20, int cleanSession = 1, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(mqtt_configure_2,"AT#MQCFG2",TOUT_100MS)

      return_t mqtt_configure_lastwill_testament(int instanceNumber, int willFlag = 0, int willRetain = 0, int willQos = 0, const char *willTopic = "", const char *willMessage = "", tout_t aTimeout = TOUT_100MS);
      _READ_TEST(mqtt_configure_lastwill_testament,"AT#MQWCFG",TOUT_100MS)

      return_t mqtt_configure_timeout(int instanceNumber, int pktTimeout = 10, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(mqtt_configure_timeout,"AT#MQTCFG",TOUT_100MS)

      return_t mqtt_connect(int instanceNumber, const char *clientId, const char *username, const char *password, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(mqtt_connect,"AT#MQCONN",TOUT_100MS)

      return_t mqtt_disconnect(int instanceNumber, tout_t aTimeout = TOUT_100MS);
      _TEST(mqtt_disconnect,"AT#MQDISC",TOUT_100MS)

      return_t mqtt_topic_subscribe(int instanceNumber, const char *topic, tout_t aTimeout = TOUT_100MS);
      _TEST(mqtt_topic_subscribe,"AT#MQSUB",TOUT_100MS)

      return_t mqtt_topic_unsubscribe(int instanceNumber, const char *topic, tout_t aTimeout = TOUT_100MS);
      _TEST(mqtt_topic_unsubscribe,"AT#MQUNS",TOUT_100MS)

      return_t mqtt_publish(int instanceNumber, const char *topic, int retain, int qos, const char *message, tout_t aTimeout = TOUT_100MS);
      _TEST(mqtt_publish,"AT#MQPUBS",TOUT_100MS)

      return_t mqtt_read(int instanceNumber, int mId, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(mqtt_read,"AT#MQREAD",TOUT_100MS)

   // GNSS ------------------------------------------------------------------------

      return_t gnss_configuration(int parameter, int value, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(gnss_configuration,"AT$GPSCFG",TOUT_100MS)

      return_t gnss_restore_default_GPS_parameters(tout_t aTimeout = TOUT_100MS);
      _TEST(gnss_restore_default_GPS_parameters, "AT$GPSRST", TOUT_100MS)

      return_t gnss_save_parameters_configuration(tout_t aTimeout = TOUT_100MS);
      _TEST(gnss_save_parameters_configuration, "AT$GPSSAV", TOUT_100MS)

      return_t gnss_controller_power_management(int status = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(gnss_controller_power_management,"AT$GPSP",TOUT_100MS)

      return_t gnss_software_version(tout_t aTimeout = TOUT_100MS);
      _READ_TEST(gnss_software_version,"AT$GPSSW",TOUT_100MS)

      return_t gnss_reset_GPS_controller(int resetType, tout_t aTimeout = TOUT_100MS);
      _TEST(gnss_reset_GPS_controller, "AT$GPSR", TOUT_100MS)

      return_t gnss_nmea_data_configuration(int enable = 0, int gga = 0, int gll = 0, int gsa = 0, int gsv = 0, int rmc = 0, int vtg = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(gnss_nmea_data_configuration,"AT$GPSNMUN",TOUT_100MS)

      return_t gnss_nmea_extended_data_configuration(int gngns = 0, int gngsa = 0, int glgsv = 0, int gpgrs = 0, int gagsv = 0, int gagsa = 0, int gavtg = 0, int gpgga = 0, int pqgsa = 0, int pqgsv = 0, int gnvtg = 0, int gnrmc = 0, int gngga = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(gnss_nmea_extended_data_configuration,"AT$GPSNMUNEX",TOUT_100MS)

      return_t get_position_GTP_WWAN_service(tout_t aTimeout = TOUT_100MS);
      _TEST(get_position_GTP_WWAN_service, "AT#GTP", TOUT_100MS)

      return_t enable_GTP_WWAN_service(int enable = 0, tout_t aTimeout = TOUT_100MS);
      _TEST(enable_GTP_WWAN_service, "AT#GTPENA", TOUT_100MS)

      return_t gps_get_acquired_position(tout_t aTimeout = TOUT_100MS);
      _READ_TEST(gps_get_acquired_position, "AT$GPSACP", TOUT_100MS)

      return_t gnss_set_agnss_enable(int provider, int status, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(gnss_set_agnss_enable, "AT$AGNSS", TOUT_100MS)

   // Mobile Broadband ------------------------------------------------------------

      return_t ecm_setup(int cid, int did = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(ecm_setup,"AT#ECM",TOUT_100MS)

      return_t ecm_shutdown(int did, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(ecm_shutdown,"AT#ECMD",TOUT_100MS)

   // PSM -------------------------------------------------------------------------

      return_t psm_setting(int mode, const char * reqPeriodicRau, const char * reqGPRSreadyTimer, const char * reqPeriodicTau, const char * reqActiveTime, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(psm_setting,"AT+CPSMS",TOUT_100MS)

      return_t psm_setting2(int mode, int reqPeriodicRau, int reqGPRSreadyTimer, int reqPeriodicTau, int reqActiveTime, int psmVersion, int psmThreshold, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(psm_setting2,"AT#CPSMS",TOUT_100MS)

      return_t psm_URC(int en = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(psm_URC, "AT#PSMURC", TOUT_100MS)
   //IMS --------------------------------------------------------------------------

      return_t ims_registration_status(int mode = 0, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(ims_registration_status, "AT+CIREG", TOUT_100MS)

   // Debugging -------------------------------------------------------------------

      return_t set_trace(int mode, char* configuration_string, tout_t aTimeout = TOUT_100MS);
      _READ_TEST(set_trace, "AT#TRACE", TOUT_100MS)
   // -----------------------------------------------------------------------------
      const uint8_t *buffer(void) { return mBuffer;} //!< Returns pointer to local buffer
      size_t length(void) { return mBuffLen;}        //!< Returns length of local buffer
      const char * buffer_cstr(int aIndex = 0);
      const char * buffer_cstr_raw();
      void ConvertBufferToIRA(uint8_t* recv_buf, uint8_t* out_buf, int size);

      virtual void on_command(const char *aCommand) {/*Serial.println(aCommand);*/}      //!< Callback function on command issued
      virtual void on_receive() {}                          //!< Callback function on string received
      virtual void on_error(const char *aMessage) {}        //!< Callback function on error string received
      virtual void on_valid(const char *aMessage) {}        //!< Callback function on valid string received
      virtual void on_timeout(void) {}                      //!< Callback function on receive timeout event
      virtual return_t  on_message(const char *aMessage)    //!< Callback function on message received
      {return RETURN_CONTINUE;}
      virtual const char* on_pending_receive(const char *aMessage) //!< Callback function on string received
      {return aMessage;}

      return_t read_line(const char *aAnswer, tout_t aTimeout = TOUT_1SEC);
      virtual return_t wait_for(const char *aAnswer = OK_STRING, tout_t aTimeout = TOUT_200MS);
      virtual return_t wait_for(const char* aCommand, int flag = 0, const char *aAnswer = OK_STRING, tout_t aTimeout = TOUT_200MS);
      virtual return_t wait_for_unsolicited(tout_t aTimeout = TOUT_200MS);

      static const char *str_start(const char *buffer, const char *string);
      static const char *str_equal(const char *buffer, const char *string);
      static const char *return_string(return_t rc);

      Uart* getSerial(){return &mSerial;}

      protected:

      void send(const char *aCommand, const char *aTerm = "\r");
      void send(const uint8_t* data, int len);
      return_t read_send_wait(const char *aCommand, const char *aAnswer = OK_STRING, tout_t aTimeout = TOUT_200MS);
      return_t test_send_wait(const char *aCommand, const char *aAnswer = OK_STRING, tout_t aTimeout = TOUT_200MS);
      return_t send_wait(const char *aCommand, const char *aAnswer = OK_STRING, tout_t aTimeout = TOUT_200MS);
      return_t send_wait(const char *aCommand, int flag, const char *aAnswer = OK_STRING, tout_t aTimeout = TOUT_200MS );
      return_t send_wait(const char *aCommand, const char *aAnswer = OK_STRING, const char* term = TERMINATION_STRING, tout_t aTimeout = TOUT_200MS);
      return_t send_wait(const char *aCommand, int flag, const char *aAnswer = OK_STRING, const char* term = TERMINATION_STRING, tout_t aTimeout = TOUT_200MS);

      void CheckIRAOption(char* str);


      Uart &mSerial;                    //!< Reference to Uart used for communication
      uint32_t mBaudrate;                //!
      uint8_t mBuffer[ME310_BUFFSIZE];  //!< Transmission buffer
      uint8_t *mpBuffer = 0;            //!< Pointer to free position in buffer
      size_t  mBuffLen = 0;             //!< Buffer length
      uint8_t *_payloadData = 0;        //!< Pointer to free position in buffer for payload data

      uint32_t _option = 0;
      bool _isIRARx, _isIRATx;

      static const char CTRZ[1];

      static const char *OK_STRING;
      static const char *ERROR_STRING;
      static const char *CONNECT_STRING;
      static const char *CME_ERROR_STRING;
      static const char *SEQUENCE_STRING;
      static const char *WAIT_DATA_STRING;
      static const char *TERMINATION_STRING;
      static const char *NO_CARRIER_STRING;
   };

} // end namespace


#endif // __ME310__H
