/** ==================================================================================================================*\
  @file UT_Can_TP.c

  @brief Testy jednostkowe do CanTp
\*====================================================================================================================*/

#include "acutest.h"
#include "Std_Types.h"

#include "CanTp.c"   

#include <stdio.h>
#include <string.h>

#include "fff.h"

DEFINE_FFF_GLOBALS; 

FAKE_VALUE_FUNC(BufReq_ReturnType, PduR_CanTpCopyTxData, PduIdType, const PduInfoType*, const RetryInfoType*, PduLengthType*);
FAKE_VOID_FUNC(PduR_CanTpTxConfirmation, PduIdType, Std_ReturnType);
FAKE_VOID_FUNC(PduR_CanTpRxIndication, PduIdType, Std_ReturnType);
FAKE_VALUE_FUNC(BufReq_ReturnType, PduR_CanTpCopyRxData, PduIdType, const PduInfoType*, PduLengthType*);
FAKE_VALUE_FUNC(BufReq_ReturnType, PduR_CanTpStartOfReception, PduIdType, const PduInfoType*, PduLengthType, PduLengthType*);

FAKE_VALUE_FUNC(Std_ReturnType, CanIf_Transmit, PduIdType, const PduInfoType*);

uint8 PduR_CanTpCopyTxData_sdu_data[20][7];
PduLengthType *PduR_CanTpCopyTxData_availableDataPtr; 
PduLengthType* PduR_CanTpCopyRxData_buffSize_array;
PduLengthType *PduR_CanTpStartOfReception_buffSize_array;

BufReq_ReturnType PduR_CanTpCopyTxData_FF(PduIdType id, const PduInfoType* info, const RetryInfoType* retry, PduLengthType* availableDataPtr){
    static int i = 0;
    int iCtr;
    i = PduR_CanTpCopyTxData_fake.call_count - 1;
    for(iCtr = 0; iCtr < info->SduLength; iCtr++ ){
      info->SduDataPtr[iCtr] = PduR_CanTpCopyTxData_sdu_data[i][iCtr];
    }
    *availableDataPtr = PduR_CanTpCopyTxData_availableDataPtr[i];
    return PduR_CanTpCopyTxData_fake.return_val_seq[i];
}

BufReq_ReturnType PduR_CanTpStartOfReception_FF(PduIdType id, const PduInfoType* info, PduLengthType TpSduLength, PduLengthType* bufferSizePtr){
    static int i = 0;
    i = PduR_CanTpStartOfReception_fake.call_count - 1;
    *bufferSizePtr = PduR_CanTpStartOfReception_buffSize_array[i];
   return PduR_CanTpStartOfReception_fake.return_val_seq[i];
}

BufReq_ReturnType PduR_CanTpCopyRxData_FF(PduIdType id, const PduInfoType* info, PduLengthType* bufferSizePtr){
    static int i = 0;
    i = PduR_CanTpCopyRxData_fake.call_count - 1;
    *bufferSizePtr = PduR_CanTpCopyRxData_buffSize_array[i];
    return PduR_CanTpCopyRxData_fake.return_val_seq[i];
}


/** ==================================================================================================================*\
                                TESTY JEDNOSTKOWE
\*====================================================================================================================*/

/**
  @brief Test inicjalizacji

  Funkcja testująca inicjalizacji CanTp.
*/

void Test_Of_CanTp_Init(void){
    CanTp_VariablesRX.eCanTp_StateRX == CANTP_RX_PROCESSING_SUSPEND;
    CanTp_VariablesRX.uiBlocksNxtCts = 2;
    CanTp_VariablesRX.CanTp_Current_RxId = 1;
    CanTp_VariablesRX.uiExpected_CF_SN = 2;
    CanTp_VariablesRX.uiTransmittedBytes = 4;

    CanTp_VariablesTX.eCanTp_StateTX = CANTP_TX_PROCESSING;
    CanTp_VariablesTX.uiFrameNrFC = 1;
    CanTp_VariablesTX.CanTp_Current_TxId = 1;
    CanTp_VariablesTX.uiMsgLen = 87;
    CanTp_VariablesTX.uiTransmittedBytes = 44;
    CanTp_VariablesTX.uiNxtSN = 1;
    CanTp_VariablesTX.CanTp_Current_TxId = 2;

    CanTp_State = CANTP_OFF;

    CanTp_Init();
    
    TEST_CHECK(CanTp_VariablesTX.eCanTp_StateTX == CANTP_TX_WAIT);
    TEST_CHECK(CanTp_VariablesTX.CanTp_Current_TxId == 0); 
    TEST_CHECK(CanTp_VariablesTX.uiFrameNrFC == 0);
    TEST_CHECK(CanTp_VariablesTX.uiMsgLen == 0);
    TEST_CHECK(CanTp_VariablesTX.uiTransmittedBytes == 0);

    TEST_CHECK(CanTp_VariablesRX.eCanTp_StateRX == CANTP_RX_WAIT);
    TEST_CHECK(CanTp_VariablesRX.uiBlocksNxtCts == 0);
    TEST_CHECK(CanTp_VariablesRX.CanTp_Current_RxId == 0);
    TEST_CHECK(CanTp_VariablesRX.uiExpected_CF_SN == 0);
    TEST_CHECK(CanTp_VariablesRX.uiTransmittedBytes == 0);

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
    CanTp_VariablesTX.eCanTp_StateTX = CANTP_TX_PROCESSING;
    CanTp_VariablesTX.uiFrameNrFC = 2;
    CanTp_VariablesTX.CanTp_Current_TxId = 1;
    CanTp_VariablesTX.uiMsgLen = 87;
    CanTp_VariablesTX.uiTransmittedBytes = 42;
    CanTp_VariablesTX.uiNxtSN = 2;
    CanTp_VariablesTX.CanTp_Current_TxId = 1;

    CanTp_VariablesRX.eCanTp_StateRX == CANTP_RX_WAIT;
    CanTp_VariablesRX.uiBlocksNxtCts = 2;
    CanTp_VariablesRX.CanTp_Current_RxId = 1;
    CanTp_VariablesRX.uiExpected_CF_SN = 1;
    CanTp_VariablesRX.uiTransmittedBytes = 2;

    CanTp_State = CANTP_ON;

    CanTp_Shutdown();
    
    TEST_CHECK(CanTp_VariablesTX.eCanTp_StateTX == CANTP_TX_WAIT);
    TEST_CHECK(CanTp_VariablesTX.CanTp_Current_TxId == 0); 
    TEST_CHECK(CanTp_VariablesTX.uiFrameNrFC == 0);
    TEST_CHECK(CanTp_VariablesTX.uiMsgLen == 0);
    TEST_CHECK(CanTp_VariablesTX.uiTransmittedBytes == 0);

    TEST_CHECK(CanTp_VariablesRX.eCanTp_StateRX == CANTP_RX_WAIT);
    TEST_CHECK(CanTp_VariablesRX.uiBlocksNxtCts == 0);
    TEST_CHECK(CanTp_VariablesRX.CanTp_Current_RxId == 0);
    TEST_CHECK(CanTp_VariablesRX.uiExpected_CF_SN == 0);
    TEST_CHECK(CanTp_VariablesRX.uiTransmittedBytes == 0);

    TEST_CHECK(CanTp_State == CANTP_OFF);
    TEST_CHECK(CanTp_State != CANTP_ON);
}

/**
  @brief Test żądania transmisji

  Funkcja testująca żądanie transmisji PDU.
*/
void TestOf_CanTp_Transmit(void){

  PduIdType PduId = 0x01;
  PduInfoType PduInfo;
  uint8 puiSduData[8];
  uint8 *puiMetaData;
  PduInfo.MetaDataPtr = puiMetaData;
  PduInfo.SduDataPtr = puiSduData;
  Std_ReturnType ret; 
 
  PduLengthType availableDataPtr_arr[10] = {1,2,3,4,5,6,7,8,9,0};
  uint8 puiSduDataArr[3][5] = { "dupa ", "dupa ", "test2"};


  PduLengthType availableData;
  int i;

  for( i = 0; i < 3; i++){
      memcpy(PduR_CanTpCopyTxData_sdu_data[i], puiSduDataArr[i], sizeof(uint8)*7);
  }

  PduR_CanTpCopyTxData_availableDataPtr = availableDataPtr_arr;

  RESET_FAKE(PduR_CanTpCopyTxData);
  RESET_FAKE(CanIf_Transmit);
  RESET_FAKE(PduR_CanTpTxConfirmation);

  PduR_CanTpCopyTxData_fake.custom_fake = PduR_CanTpCopyTxData_FF;

  Std_ReturnType CanIf_Transmit_retv[] = {E_OK, E_OK, E_NOT_OK, E_OK};
  SET_RETURN_SEQ(CanIf_Transmit, CanIf_Transmit_retv, 4);

  BufReq_ReturnType PduR_CanTpCopyTxData_retv[] = {BUFREQ_OK, BUFREQ_E_NOT_OK, BUFREQ_BUSY, BUFREQ_OK};
  SET_RETURN_SEQ(PduR_CanTpCopyTxData, PduR_CanTpCopyTxData_retv, 4);

  
  /*                  TEST 1
       CanTp_State = CANTP_OFF ---> Nic nie rób
  */ 
  CanTp_State = CANTP_OFF;
  CanTp_VariablesTX.eCanTp_StateTX = CANTP_TX_WAIT;
  CanTp_VariablesTX.CanTp_Current_TxId = 2;
  PduInfo.SduLength = 7;
  
  ret = CanTp_Transmit(PduId, &PduInfo);
  
  TEST_CHECK( ret == E_NOT_OK );
  TEST_CHECK(CanTp_VariablesTX.eCanTp_StateTX == CANTP_TX_WAIT);
  TEST_CHECK(CanTp_VariablesTX.CanTp_Current_TxId == 2);
 
 /*
                                    TEST 2 
  CANTP_TX_WAIT - SduLenght < 8 - wysyłane Single Frame - zwracane E_OK oraz BUFREQ_OK
*/
  CanTp_State = CANTP_ON;
  CanTp_VariablesTX.eCanTp_StateTX = CANTP_TX_WAIT;
  CanTp_VariablesTX.CanTp_Current_TxId = 2;
  PduInfo.SduLength = 7;

  ret = CanTp_Transmit(PduId, &PduInfo);

  TEST_CHECK(ret == E_OK);

  TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 1);
  TEST_CHECK(PduR_CanTpCopyTxData_fake.arg0_val == PduId);
  TEST_CHECK(PduR_CanTpCopyTxData_fake.arg1_val->SduLength == 7);
  TEST_CHECK(PduR_CanTpCopyTxData_fake.arg2_val == NULL);
 
  TEST_CHECK(CanIf_Transmit_fake.call_count == 1 );
  TEST_CHECK(CanIf_Transmit_fake.arg0_val == 1 );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[0] == 0x7 );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[1] == 'd' );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[2] == 'u' );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[3] == 'p' );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[4] == 'a' );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[5] == ' ' );

  TEST_CHECK(CanTp_VariablesTX.eCanTp_StateTX == CANTP_TX_WAIT);
  TEST_CHECK(CanTp_VariablesTX.CanTp_Current_TxId == 2);
  
  CanTp_ResetRX();

/*
                               TEST3 
  CANTP_TX_WAIT - SduLenght < 8 -  zwracane E_NOT_OK oraz BUFREQ_E_NOT_OK
*/
  CanTp_VariablesTX.eCanTp_StateTX = CANTP_TX_WAIT;
  CanTp_VariablesTX.CanTp_Current_TxId = 2;
  PduInfo.SduLength = 7;

  ret = CanTp_Transmit(PduId, &PduInfo);
  TEST_CHECK(ret == E_NOT_OK);
  TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 2);
  TEST_CHECK(CanTp_VariablesTX.eCanTp_StateTX == CANTP_TX_WAIT);
  TEST_CHECK(CanTp_VariablesTX.CanTp_Current_TxId == 0);
  TEST_CHECK(PduR_CanTpTxConfirmation_fake.call_count == 1);
  TEST_CHECK(PduR_CanTpTxConfirmation_fake.arg1_val == E_NOT_OK);
}

/**
  @brief Test przerwania transmisji

  Funkcja testująca przerwanie transmisji PDU.
*/
void Test_Of_CanTp_CancelTransmit(void){

  /*
                  TEST 1 
              Zwracane - E_OK (Zgodne ID)
  */
    Std_ReturnType ret;
    
    CanTp_VariablesTX.eCanTp_StateTX = CANTP_TX_WAIT;
    CanTp_VariablesTX.uiFrameNrFC = 1;
    CanTp_VariablesTX.CanTp_Current_TxId = 2;
    CanTp_VariablesTX.uiMsgLen = 87;
    CanTp_VariablesTX.uiTransmittedBytes = 46;
    CanTp_VariablesTX.uiNxtSN = 0;

    ret = CanTp_CancelTransmit(2);

    TEST_CHECK(CanTp_VariablesTX.eCanTp_StateTX == CANTP_TX_WAIT);
    TEST_CHECK(CanTp_VariablesTX.CanTp_Current_TxId == 0); 
    TEST_CHECK(CanTp_VariablesTX.uiFrameNrFC == 0);
    TEST_CHECK(CanTp_VariablesTX.uiMsgLen == 0);
    TEST_CHECK(CanTp_VariablesTX.uiTransmittedBytes == 0);
    TEST_CHECK(ret == E_OK);

  /*
                    TEST 2
              Zwracane E_NOT_OK (Błędne ID)
  */
    CanTp_VariablesTX.eCanTp_StateTX = CANTP_TX_WAIT;
    CanTp_VariablesTX.uiFrameNrFC = 3;
    CanTp_VariablesTX.CanTp_Current_TxId = 3;
    CanTp_VariablesTX.uiMsgLen = 87;
    CanTp_VariablesTX.uiTransmittedBytes = 45;
    CanTp_VariablesTX.uiNxtSN = 0;

    ret = CanTp_CancelTransmit(1);

    TEST_CHECK(CanTp_VariablesTX.eCanTp_StateTX == CANTP_TX_WAIT);
    TEST_CHECK(CanTp_VariablesTX.CanTp_Current_TxId == 3); 
    TEST_CHECK(CanTp_VariablesTX.uiFrameNrFC == 3);
    TEST_CHECK(CanTp_VariablesTX.uiMsgLen == 87);
    TEST_CHECK(CanTp_VariablesTX.uiTransmittedBytes == 45);
    TEST_CHECK(ret == E_NOT_OK);

}

/**
  @brief Test przerwania odbioru

  Funkcja testująca przerwanie odbioru PDU.
*/
void Test_Of_CanTp_CancelReceive(void){
    Std_ReturnType ret;
  /*
                     TEST 1 
              Zwracane - E_OK (Zgodne ID)
  */
    CanTp_VariablesRX.eCanTp_StateRX == CANTP_RX_PROCESSING;
    CanTp_VariablesRX.uiBlocksNxtCts = 2;
    CanTp_VariablesRX.CanTp_Current_RxId = 2;
    CanTp_VariablesRX.uiExpected_CF_SN = 2;
    CanTp_VariablesRX.uiTransmittedBytes = 4;

    ret = CanTp_CancelReceive(2);

    TEST_CHECK(CanTp_VariablesRX.eCanTp_StateRX == CANTP_RX_WAIT);
    TEST_CHECK(CanTp_VariablesRX.uiBlocksNxtCts == 0);
    TEST_CHECK(CanTp_VariablesRX.CanTp_Current_RxId == 0);
    TEST_CHECK(CanTp_VariablesRX.uiExpected_CF_SN == 0);
    TEST_CHECK(CanTp_VariablesRX.uiTransmittedBytes == 0);
    TEST_CHECK(ret == E_OK);

   /*
                    TEST 2
              Zwracane E_NOT_OK (Błędne ID)
  */
    CanTp_VariablesRX.eCanTp_StateRX = CANTP_RX_PROCESSING;
    CanTp_VariablesRX.uiBlocksNxtCts = 1;
    CanTp_VariablesRX.CanTp_Current_RxId = 3;
    CanTp_VariablesRX.uiExpected_CF_SN = 1;
    CanTp_VariablesRX.uiTransmittedBytes = 10;

    ret = CanTp_CancelReceive(1);

    TEST_CHECK(CanTp_VariablesRX.eCanTp_StateRX == CANTP_RX_PROCESSING);
    TEST_CHECK(CanTp_VariablesRX.uiBlocksNxtCts == 1);
    TEST_CHECK(CanTp_VariablesRX.CanTp_Current_RxId == 3);
    TEST_CHECK(CanTp_VariablesRX.uiExpected_CF_SN == 1);
    TEST_CHECK(CanTp_VariablesRX.uiTransmittedBytes == 10);
    TEST_CHECK(ret == E_NOT_OK);
}


/**
  @brief Test zmiany wartosci parametru

  Funkcja testująca zmiane wartosci parametru.
*/

void Test_Of_CanTp_ChangeParameter(void){
  PduIdType PduId = 1;
  TPParameterType Parameter;
  Std_ReturnType ret; 

  /*
                    TEST 1
          Zmiana parametru TP_BS, TP_STMIN - zwracane E_OK
  */
  Parameter = TP_BS;
  CanTp_State = CANTP_ON;
  ret = CanTp_ChangeParameter(PduId, Parameter, 1);
  TEST_CHECK(ret == E_OK);
  Parameter = TP_STMIN;
  ret = CanTp_ChangeParameter(PduId, Parameter, 1);
  TEST_CHECK(ret == E_OK);
  /*
                    TEST 2
          Zmiana parametru TP_BC - zwracane E_NOT_OK
  */
  Parameter = TP_BC;
  ret = CanTp_ChangeParameter(PduId, Parameter, 1);
  TEST_CHECK(ret == E_NOT_OK);
  /*
                    TEST 3
          Zmiana parametru TP_BS - zwracane E_NOT_OK bo State CANTP_OFF
  */
  Parameter = TP_BS;
  CanTp_State = CANTP_OFF;
  ret = CanTp_ChangeParameter(PduId, Parameter, 1);
  TEST_CHECK(ret == E_NOT_OK);
}

/**
  @brief Test odczytu wartosci parametru

  Funkcja testująca odczyt wartosci parametru.
*/

void Test_Of_CanTp_ReadParameter(void){
  PduIdType PduId = 1;
  TPParameterType Parameter;
  Std_ReturnType ret; 
  uint16 puiReadValue;
  /*
                    TEST 1
          Odczyt parametru TP_BS i TP_STmin - zwracane E_OK
  */
  Parameter = TP_BS;
  CanTp_State = CANTP_ON;
  ret = CanTp_ChangeParameter(PduId, Parameter, 1);
  ret = CanTp_ReadParameter(PduId, Parameter, &puiReadValue);
  TEST_CHECK(ret == E_OK);
  TEST_CHECK(puiReadValue == 1);
  Parameter = TP_STMIN;
  ret = CanTp_ChangeParameter(PduId, Parameter, 1);
  ret = CanTp_ReadParameter(PduId, Parameter, &puiReadValue);
  TEST_CHECK(ret == E_OK);
  TEST_CHECK(puiReadValue == 1);
  /*
                    TEST 2
          Zmiana parametru TP_BC - zwracane E_NOT_OK
  */
  puiReadValue = 2;
  Parameter = TP_BC;
  ret = CanTp_ReadParameter(PduId, Parameter, &puiReadValue);
  TEST_CHECK(ret == E_NOT_OK);
  TEST_CHECK(puiReadValue == 2);
  /*
                    TEST 3
          Zmiana parametru TP_BS - zwracane E_NOT_OK bo State CANTP_OFF
  */
  puiReadValue = 3;
  Parameter = TP_BS;
  CanTp_State = CANTP_OFF;
  ret = CanTp_ReadParameter(PduId, Parameter, &puiReadValue);
  TEST_CHECK(ret == E_NOT_OK);
  TEST_CHECK(puiReadValue == 3);
}

/**
  @brief Test zarządzania modułem

  Funkcja testująca zarządzanie modułem CanTp.
*/

void Test_Of_CanTp_MainFunction(){
    RESET_FAKE(PduR_CanTpRxIndication);
    RESET_FAKE(CanIf_Transmit);
      
    Std_ReturnType retCanIf_Transmit[6] = {E_OK, E_OK, E_NOT_OK, E_NOT_OK, E_OK, E_NOT_OK};
    SET_RETURN_SEQ(CanIf_Transmit, retCanIf_Transmit, 6);
    PduLengthType BufferSizeArr[7] = {0,1,0,3,9,0,6};
    PduR_CanTpCopyRxData_buffSize_array = BufferSizeArr;

    BufReq_ReturnType BufferRetValues[7] = {BUFREQ_OK, BUFREQ_OVFL, BUFREQ_OK, BUFREQ_OK, BUFREQ_OK, BUFREQ_E_NOT_OK, BUFREQ_OK};
    SET_RETURN_SEQ(PduR_CanTpCopyRxData, BufferRetValues, 7);
    PduR_CanTpCopyRxData_fake.custom_fake = PduR_CanTpCopyRxData_FF;

    CanTp_TimerReset(&N_Ar_timer);
    CanTp_TimerReset(&N_Br_timer);
    CanTp_TimerReset(&N_Cr_timer);
    /*
                  TEST 1
      Test dla deaktywowanych timerów - brak zmian
    */
    CanTp_MainFunction();
    CanTp_MainFunction();
    CanTp_MainFunction();

    TEST_CHECK(N_Ar_timer.eState == TIMER_DISABLE);
    TEST_CHECK(N_Br_timer.eState == TIMER_DISABLE);
    TEST_CHECK(N_Cr_timer.eState == TIMER_DISABLE);
    TEST_CHECK(N_Ar_timer.uiCounter == 0);
    TEST_CHECK(N_Br_timer.uiCounter == 0);
    TEST_CHECK(N_Cr_timer.uiCounter == 0);
    TEST_CHECK(PduR_CanTpCopyRxData_fake.call_count == 0); 
    TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 0);

    /*
                TEST 2
    Standardowa praca układu dla Timerów RX
    */
    CanTp_VariablesRX.uiMsgLen = 10;
    CanTp_VariablesRX.uiTransmittedBytes = 0;

    CanTp_TimerStart(&N_Ar_timer);
    CanTp_TimerStart(&N_Br_timer);
    CanTp_TimerStart(&N_Cr_timer);
    
    CanTp_MainFunction();
    CanTp_MainFunction();
    CanTp_MainFunction();

    TEST_CHECK(N_Ar_timer.eState == TIMER_ENABLE);
    TEST_CHECK(N_Br_timer.eState == TIMER_ENABLE);
    TEST_CHECK(N_Cr_timer.eState == TIMER_ENABLE);
    TEST_CHECK(N_Ar_timer.uiCounter == 3);
    TEST_CHECK(N_Br_timer.uiCounter == 3);
    TEST_CHECK(N_Cr_timer.uiCounter == 3);
    TEST_CHECK(PduR_CanTpCopyRxData_fake.call_count == 3); 
    TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 0);

    CanTp_MainFunction();

    TEST_CHECK(N_Ar_timer.eState == TIMER_ENABLE);
    TEST_CHECK(N_Br_timer.eState == TIMER_ENABLE);
    TEST_CHECK(N_Cr_timer.eState == TIMER_ENABLE);
    TEST_CHECK(N_Ar_timer.uiCounter == 4);
    TEST_CHECK(N_Br_timer.uiCounter == 4);
    TEST_CHECK(N_Cr_timer.uiCounter == 4);
    TEST_CHECK(PduR_CanTpCopyRxData_fake.call_count == 4); 
    TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 0);

    /*
                TEST 3
    Standardowa praca układu dla Timerów TX
    */
    CanTp_ResetRX();
    CanTp_ResetTX();

    CanTp_TimerStart(&N_Bs_timer);
    CanTp_TimerStart(&N_Cs_timer);
    CanTp_TimerStart(&N_As_timer);

    CanTp_MainFunction();
    CanTp_MainFunction();
    CanTp_MainFunction();
    CanTp_MainFunction();

    TEST_CHECK(N_As_timer.uiCounter == 4);
    TEST_CHECK(N_As_timer.eState == TIMER_ENABLE);
    TEST_CHECK(N_Bs_timer.uiCounter == 4);
    TEST_CHECK(N_Bs_timer.eState == TIMER_ENABLE);
    TEST_CHECK(N_Cs_timer.uiCounter == 4);
    TEST_CHECK(N_Cs_timer.eState == TIMER_ENABLE);

    /*
                TEST 4
    Timeout dla Timera As TX
    */

    CanTp_ResetRX();
    CanTp_ResetTX();

    N_As_timer.uiCounter = 90;
    CanTp_TimerStart(&N_As_timer);
    CanTp_MainFunction();

    TEST_CHECK(N_As_timer.uiCounter == 91);
    TEST_CHECK(N_As_timer.eState == TIMER_ENABLE);
    TEST_CHECK(N_Bs_timer.uiCounter == 0);
    TEST_CHECK(N_Bs_timer.eState == TIMER_DISABLE);
    TEST_CHECK(N_Cs_timer.uiCounter == 0);
    TEST_CHECK(N_Cs_timer.eState == TIMER_DISABLE);

    TEST_CHECK(PduR_CanTpTxConfirmation_fake.call_count == 0);
    
    N_As_timer.uiCounter = 99;
    CanTp_MainFunction();

    TEST_CHECK(N_As_timer.uiCounter == 0);
    TEST_CHECK(N_As_timer.eState == TIMER_DISABLE);
    TEST_CHECK(N_Bs_timer.uiCounter == 0);
    TEST_CHECK(N_Bs_timer.eState == TIMER_DISABLE);
    TEST_CHECK(N_Cs_timer.uiCounter == 0);
    TEST_CHECK(N_Cs_timer.eState == TIMER_DISABLE);

    TEST_CHECK(PduR_CanTpTxConfirmation_fake.call_count == 1);
}


/**
  @brief Test Potwierdzenia transmisji

  Funkcja testującapotwierdzenie transmisji PDU.
*/
void Test_Of_CanTp_TxConfirmation(void){
  PduInfoType PduInfo;
  uint8 puiSduData[8];
  uint8 *puiMetaData;
  PduInfo.MetaDataPtr = puiMetaData;
  PduInfo.SduDataPtr = puiSduData;
  Std_ReturnType ret; 

  PduLengthType pAvailableDataArr[10] = {1,2,3,4,5,6,7,8,9,0};
  uint8 puiSduDataArr[4][4] = {"dupa", "dupa", "test", "dwa."};

  PduLengthType availableData;
  int i;
  for( i = 0; i < 4; i++){
      memcpy(PduR_CanTpCopyTxData_sdu_data[i], puiSduDataArr[i], sizeof(uint8)*7);
  }
  PduR_CanTpCopyTxData_availableDataPtr = pAvailableDataArr;

  RESET_FAKE(PduR_CanTpCopyTxData);
  RESET_FAKE(CanIf_Transmit);
  RESET_FAKE(PduR_CanTpTxConfirmation);

  PduR_CanTpCopyTxData_fake.custom_fake = PduR_CanTpCopyTxData_FF;

  Std_ReturnType retCanIf_Transmit[7] = {E_OK, E_OK, E_NOT_OK, E_OK, E_NOT_OK, E_NOT_OK, E_NOT_OK};
  SET_RETURN_SEQ(CanIf_Transmit, retCanIf_Transmit, 7);

  BufReq_ReturnType retPduR_CanTpCopyTxData[9] = {BUFREQ_OK, BUFREQ_OK, BUFREQ_OK, BUFREQ_E_NOT_OK , BUFREQ_BUSY, BUFREQ_BUSY, BUFREQ_E_NOT_OK, BUFREQ_BUSY, BUFREQ_OK};
  SET_RETURN_SEQ(PduR_CanTpCopyTxData, retPduR_CanTpCopyTxData, 9);

  CanTp_State = CANTP_ON;
/*
          TEST 1
  Transmisja poprawna
*/
  CanTp_VariablesTX.eCanTp_StateTX = CANTP_TX_PROCESSING;
  CanTp_VariablesTX.uiFrameNrFC = 1;
  CanTp_VariablesTX.CanTp_Current_TxId = 1;
  CanTp_VariablesTX.uiMsgLen = 80;
  CanTp_VariablesTX.uiTransmittedBytes = 75;
  CanTp_VariablesTX.uiNxtSN = 0;

  CanTp_TxConfirmation(1, E_OK);
 
  TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 1);
  TEST_CHECK(PduR_CanTpCopyTxData_fake.arg0_val == 1);
  TEST_CHECK(PduR_CanTpCopyTxData_fake.arg1_val->SduLength == 5);
  TEST_CHECK(PduR_CanTpCopyTxData_fake.arg2_val == NULL);

  TEST_CHECK(CanIf_Transmit_fake.call_count == 1 );

  TEST_CHECK(CanIf_Transmit_fake.arg0_val == 1 ); 
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[0] == 0x20 );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[1] == 'd' );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[2] == 'u' );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[3] == 'p' );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[4] == 'a' );

  TEST_CHECK(CanTp_VariablesTX.eCanTp_StateTX == CANTP_TX_PROCESSING);
  TEST_CHECK(CanTp_VariablesTX.CanTp_Current_TxId == 1); 
  TEST_CHECK(CanTp_VariablesTX.uiFrameNrFC == 0);
  TEST_CHECK(CanTp_VariablesTX.uiMsgLen == 80);
  TEST_CHECK(CanTp_VariablesTX.uiTransmittedBytes == 80);

  CanTp_ResetRX();
  CanTp_ResetTX();

  /*
            TEST 2
    RETURN E_NOT_OK
  */

  CanTp_VariablesTX.eCanTp_StateTX = CANTP_TX_PROCESSING;
  CanTp_VariablesTX.uiFrameNrFC = 3;
  CanTp_VariablesTX.CanTp_Current_TxId = 1;
  CanTp_VariablesTX.uiMsgLen = 80;
  CanTp_VariablesTX.uiTransmittedBytes = 75;
  CanTp_VariablesTX.uiNxtSN = 2;

  CanTp_TxConfirmation (1, E_NOT_OK );
 
  TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 1);
  
  TEST_CHECK( PduR_CanTpTxConfirmation_fake.call_count == 1 );
  TEST_CHECK( PduR_CanTpTxConfirmation_fake.arg0_val == 1 );
  TEST_CHECK( PduR_CanTpTxConfirmation_fake.arg1_val == E_NOT_OK );

  TEST_CHECK(CanTp_VariablesTX.eCanTp_StateTX == CANTP_TX_WAIT);
  TEST_CHECK(CanTp_VariablesTX.CanTp_Current_TxId == 0);
  TEST_CHECK(CanIf_Transmit_fake.call_count == 1 );
  TEST_CHECK(CanTp_VariablesTX.uiFrameNrFC == 0);
  TEST_CHECK(CanTp_VariablesTX.uiMsgLen == 0);
  TEST_CHECK(CanTp_VariablesTX.uiTransmittedBytes == 0);

  CanTp_ResetRX();
  CanTp_ResetTX();

  /*
      TEST 3 
    UNKNOWN ID
  */

  CanTp_VariablesTX.eCanTp_StateTX = CANTP_TX_PROCESSING;
  CanTp_VariablesTX.uiFrameNrFC = 2;
  CanTp_VariablesTX.CanTp_Current_TxId = 3;
  CanTp_VariablesTX.uiMsgLen = 80;
  CanTp_VariablesTX.uiTransmittedBytes = 75;
  CanTp_VariablesTX.uiNxtSN = 0;

  CanTp_TxConfirmation(2, E_NOT_OK);

  TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 1);
  TEST_CHECK( PduR_CanTpTxConfirmation_fake.call_count == 1 );

  TEST_CHECK(CanIf_Transmit_fake.call_count == 1 );

  TEST_CHECK(CanTp_VariablesTX.eCanTp_StateTX == CANTP_TX_PROCESSING);
  TEST_CHECK(CanTp_VariablesTX.CanTp_Current_TxId == 3);
  TEST_CHECK(CanTp_VariablesTX.uiFrameNrFC == 2);
  TEST_CHECK(CanTp_VariablesTX.uiMsgLen == 80);
  TEST_CHECK(CanTp_VariablesTX.uiTransmittedBytes == 75);

  CanTp_ResetRX();
  CanTp_ResetTX();
}

/** ==================================================================================================================*\
                                TESTY FUNKCJI POMOCNICZYCH
\*====================================================================================================================*/

/**
  @brief Test Resetu odbiornika

  Funkcja testująca reset odbiornika.
*/
void Test_Of_CanTp_ResetRX(){     
  CanTp_VariablesRX.eCanTp_StateRX == CANTP_RX_PROCESSING;
  CanTp_VariablesRX.uiExpected_CF_SN == 1;
  CanTp_VariablesRX.uiMsgLen == 5;
  CanTp_VariablesRX.uiTransmittedBytes == 4;
  CanTp_VariablesRX.uiBlocksNxtCts == 2;
  CanTp_VariablesRX.CanTp_Current_RxId == 2;
  CanTp_ResetRX();
  TEST_CHECK(CanTp_VariablesRX.eCanTp_StateRX == CANTP_RX_WAIT);
  TEST_CHECK(CanTp_VariablesRX.uiExpected_CF_SN == 0);
  TEST_CHECK(CanTp_VariablesRX.uiMsgLen == 0);
  TEST_CHECK(CanTp_VariablesRX.uiTransmittedBytes == 0);
  TEST_CHECK(CanTp_VariablesRX.uiBlocksNxtCts == 0);
  TEST_CHECK(CanTp_VariablesRX.CanTp_Current_RxId == 0);
}

/**
  @brief Test Resetu nadajnika

  Funkcja testująca reset nadajnika.
*/
void Test_Of_CanTp_ResetTX(void){
  CanTp_VariablesTX.eCanTp_StateTX = CANTP_TX_PROCESSING;
  CanTp_VariablesTX.uiFrameNrFC = 2;
  CanTp_VariablesTX.CanTp_Current_TxId = 3;
  CanTp_VariablesTX.uiMsgLen = 4;
  CanTp_VariablesTX.uiTransmittedBytes = 5;
  CanTp_ResetTX();
  TEST_CHECK(CanTp_VariablesTX.eCanTp_StateTX == CANTP_TX_WAIT);
  TEST_CHECK(CanTp_VariablesTX.uiFrameNrFC == 0);
  TEST_CHECK(CanTp_VariablesTX.CanTp_Current_TxId == 0);
  TEST_CHECK(CanTp_VariablesTX.uiMsgLen == 0);
  TEST_CHECK(CanTp_VariablesTX.uiTransmittedBytes == 0);
}

/**
  @brief Test Wyliczania rozmiarów Blocków

  Funkcja testująca obliczanie rozmiaru blocków.
*/
void Test_Of_CanTp_CalcBlocksSize(void){
  CanTp_VariablesRX.uiMsgLen = 8;
  CanTp_VariablesRX.uiTransmittedBytes = 0;
  TEST_CHECK(CanTp_CalcBlocksSize(10) == 2);
  TEST_CHECK(CanTp_CalcBlocksSize(6) == 0);
}

/**
  @brief Test sprawdzania typu ramki

  Funkcja testująca sprawdzanie typu ramki
*/
void Test_Of_CanTp_FrameCheckType(void){
    uint8 puiSduData[7];
    CanPCI_Type CanFrameInfo;
    PduInfoType CanData;
    CanData.SduDataPtr = puiSduData;
    CanData.SduLength = 7;

    // SF - Single Frame Type check
    CanData.SduDataPtr[0] = 0x0F; 
    CanData.SduDataPtr[1] = 0;
    CanData.SduDataPtr[2] = 0;

    TEST_CHECK(CanTp_GetPCI(&CanData, &CanFrameInfo) == E_OK);
    TEST_CHECK(CanFrameInfo.eFrameType == CAN_SF);
    TEST_CHECK(CanFrameInfo.uiFrameLenght == 0xF);
    TEST_CHECK(CanFrameInfo.uiBlockSize == 0);
    TEST_CHECK(CanFrameInfo.uiFlowStatus == 0);
    TEST_CHECK(CanFrameInfo.uiSeparationTime == 0);
    TEST_CHECK(CanFrameInfo.uiSeparationTime == 0);
}


TEST_LIST = {
    { "Test of CanTp_MainFunction", Test_Of_CanTp_MainFunction },
    { "Test of CanTp_ReadParameter", Test_Of_CanTp_ReadParameter },
    { "Test of CanTp_ChangeParameter", Test_Of_CanTp_ChangeParameter },
    { "Test of CanTp_FrameCheckType", Test_Of_CanTp_FrameCheckType },
    { "Test of CanTp_CalcBlocksSize", Test_Of_CanTp_CalcBlocksSize },
    { "Test of CanTp_ResetTX", Test_Of_CanTp_ResetTX },
    { "Test of CanTp_ResetRX", Test_Of_CanTp_ResetRX },
    { "Test of CanTp_TxConfirmation", Test_Of_CanTp_TxConfirmation },
    { "Test of CanTp_CancelReceive", Test_Of_CanTp_CancelReceive },
    { "Test of CanTp_CancelTransmit", Test_Of_CanTp_CancelTransmit },
    { "Test of CanTp_Transmit", TestOf_CanTp_Transmit },
    { "Test of CanTp_Init", Test_Of_CanTp_Init },
    { "Test of CanTp_Shutdown", Test_Of_CanTp_Shutdown },
	  { "Test of CanTp_GetVersionInfo", Test_Of_CanTp_GetVersionInfo },
    { NULL, NULL }                           
};