/** ==================================================================================================================*\
  @file UT_Can_TP.c

  @brief Testy jednostkowe do CanTp
\*====================================================================================================================*/

#include "acutest.h"
#include "Std_Types.h"

#include "CanTp.c"   

/**
  @brief Test inicjalizacji

  Funkcja testująca inicjalizacji CanTp.
*/

void Test_Of_CanTp_Init(void){
    CanTp_VariablesTX.CanTp_StateTX = CANTP_TX_WAIT;
    CanTp_VariablesTX.frame_nr_FC = 1;
    CanTp_VariablesTX.CanTp_Current_TxId = 0x1;
    CanTp_VariablesTX.message_legth = 100;
    CanTp_VariablesTX.sent_bytes = 95;
    CanTp_VariablesTX.next_SN = 0;
    CanTp_VariablesTX.CanTp_Current_TxId = 2;

    CanTp_VariablesRX.CanTp_StateRX == CANTP_RX_PROCESSING;
    CanTp_VariablesRX.blocks_to_next_cts = 1;
    CanTp_VariablesRX.CanTp_Current_RxId = 1;
    CanTp_VariablesRX.expected_CF_SN = 1;
    CanTp_VariablesRX.sended_bytes = 1;

    CanTp_State = CANTP_OFF;

    CanTp_Init();
    
    TEST_CHECK(CanTp_VariablesTX.CanTp_StateTX == CANTP_TX_WAIT);
    TEST_CHECK(CanTp_VariablesTX.CanTp_Current_TxId == 0); 
    TEST_CHECK(CanTp_VariablesTX.frame_nr_FC == 0);
    TEST_CHECK(CanTp_VariablesTX.message_legth == 0);
    TEST_CHECK(CanTp_VariablesTX.sent_bytes == 0);

    TEST_CHECK(CanTp_VariablesRX.CanTp_StateRX == CANTP_RX_WAIT);
    TEST_CHECK(CanTp_VariablesRX.blocks_to_next_cts == 0);
    TEST_CHECK(CanTp_VariablesRX.CanTp_Current_RxId == 0);
    TEST_CHECK(CanTp_VariablesRX.expected_CF_SN == 0);
    TEST_CHECK(CanTp_VariablesRX.sended_bytes == 0);

    TEST_CHECK(CanTp_State == CANTP_ON);
}

/**
  @brief Test wersji

  Funkcja testująca zapisywana wersję CanTp.
*/
void Test_Of_CanTp_GetVersionInfo(void)
{
	Std_VersionInfoType versioninfo;
	CanTp_GetVersionInfo(&versioninfo);
	TEST_CHECK(versioninfo.sw_major_version == 0x00u);
	TEST_CHECK(versioninfo.sw_minor_version == 0x01u);
	TEST_CHECK(versioninfo.sw_patch_version == 0x00u);
	TEST_CHECK(versioninfo.vendorID == 0x00u);
  TEST_CHECK(versioninfo.moduleID == 0x0Eu);

	TEST_CHECK(versioninfo.sw_major_version != 0x01u);
	TEST_CHECK(versioninfo.sw_minor_version != 0x00u);
	TEST_CHECK(versioninfo.sw_patch_version != 0x01u);
	TEST_CHECK(versioninfo.vendorID != 0x01u);
  TEST_CHECK(versioninfo.moduleID != 0x00u);
}

/**
  @brief Test Wyłączenia modułu

  Funkcja testująca wyłączania modułu CanTp.
*/
void Test_Of_CanTp_Shutdown(void){
    CanTp_VariablesTX.CanTp_StateTX = CANTP_TX_WAIT;
    CanTp_VariablesTX.frame_nr_FC = 1;
    CanTp_VariablesTX.CanTp_Current_TxId = 0x1;
    CanTp_VariablesTX.message_legth = 100;
    CanTp_VariablesTX.sent_bytes = 95;
    CanTp_VariablesTX.next_SN = 0;
    CanTp_VariablesTX.CanTp_Current_TxId = 2;

    CanTp_VariablesRX.CanTp_StateRX == CANTP_RX_PROCESSING;
    CanTp_VariablesRX.blocks_to_next_cts = 1;
    CanTp_VariablesRX.CanTp_Current_RxId = 1;
    CanTp_VariablesRX.expected_CF_SN = 1;
    CanTp_VariablesRX.sended_bytes = 1;

    CanTp_State = CANTP_ON;

    CanTp_Shutdown();
    
    TEST_CHECK(CanTp_VariablesTX.CanTp_StateTX == CANTP_TX_WAIT);
    TEST_CHECK(CanTp_VariablesTX.CanTp_Current_TxId == 0); 
    TEST_CHECK(CanTp_VariablesTX.frame_nr_FC == 0);
    TEST_CHECK(CanTp_VariablesTX.message_legth == 0);
    TEST_CHECK(CanTp_VariablesTX.sent_bytes == 0);

    TEST_CHECK(CanTp_VariablesRX.CanTp_StateRX == CANTP_RX_WAIT);
    TEST_CHECK(CanTp_VariablesRX.blocks_to_next_cts == 0);
    TEST_CHECK(CanTp_VariablesRX.CanTp_Current_RxId == 0);
    TEST_CHECK(CanTp_VariablesRX.expected_CF_SN == 0);
    TEST_CHECK(CanTp_VariablesRX.sended_bytes == 0);

    TEST_CHECK(CanTp_State == CANTP_OFF);
    TEST_CHECK(CanTp_State != CANTP_ON);
}

TEST_LIST = {
    { "Test of CanTp_Init", Test_Of_CanTp_Init },
    { "Test of CanTp_Shutdown", Test_Of_CanTp_Shutdown },
	  { "Test of CanTp_GetVersionInfo", Test_Of_CanTp_GetVersionInfo },
    { NULL, NULL }                           
};