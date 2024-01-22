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
FAKE_VALUE_FUNC(BufReq_ReturnType, PduR_CanTpStartOfReception, PduIdType, const PduInfoType*, PduLengthType, PduLengthType* );

FAKE_VALUE_FUNC(Std_ReturnType, CanIf_Transmit, PduIdType, const PduInfoType*);

uint8 PduR_CanTpCopyTxData_sdu_data[20][7];
PduLengthType *PduR_CanTpCopyTxData_availableDataPtr; 
PduLengthType* PduR_CanTpCopyRxData_buffSize_array;
PduLengthType *PduR_CanTpStartOfReception_buffSize_array;

BufReq_ReturnType PduR_CanTpCopyTxData_FF(PduIdType id, const PduInfoType* info, const RetryInfoType* retry, PduLengthType* availableDataPtr ){
    static int i = 0;
    int loop_ctr;
    i = PduR_CanTpCopyTxData_fake.call_count - 1;
    for( loop_ctr = 0; loop_ctr < info->SduLength; loop_ctr++ ){
      info->SduDataPtr[loop_ctr] = PduR_CanTpCopyTxData_sdu_data[i][loop_ctr];
    }
    *availableDataPtr = PduR_CanTpCopyTxData_availableDataPtr[i];
    return PduR_CanTpCopyTxData_fake.return_val_seq[i];
}

BufReq_ReturnType PduR_CanTpStartOfReception_FF(PduIdType id, const PduInfoType* info, PduLengthType TpSduLength, PduLengthType* bufferSizePtr ){
    static int i = 0;
    i = PduR_CanTpStartOfReception_fake.call_count - 1;
    *bufferSizePtr = PduR_CanTpStartOfReception_buffSize_array[i];
   return PduR_CanTpStartOfReception_fake.return_val_seq[i];
}

BufReq_ReturnType PduR_CanTpCopyRxData_FF(PduIdType id, const PduInfoType* info, PduLengthType* bufferSizePtr ){
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

/**
  @brief Test żądania transmisji

  Funkcja testująca żądanie transmisji PDU.
*/
void TestOf_CanTp_Transmit(void){

  PduIdType PduId = 0x01;
  PduInfoType PduInfo;
  uint8 SduDataPtr[8];
  uint8 *MetaDataPtr;
  PduInfo.MetaDataPtr = MetaDataPtr;
  PduInfo.SduDataPtr = SduDataPtr;
  Std_ReturnType ret; 
 
  PduLengthType availableDataPtr_array_local[10] = {1,2,3,4,5,6,7,8,9,0};
  uint8 sdu_data_ptr_array[3][7] = { "dupa...", "dupa...", "test..2"};


  PduLengthType availableData;
  int i;

  for( i = 0; i < 4; i++){
      memcpy(PduR_CanTpCopyTxData_sdu_data[i], sdu_data_ptr_array[i], sizeof(uint8)*7);
  }

  PduR_CanTpCopyTxData_availableDataPtr = availableDataPtr_array_local;

  RESET_FAKE( PduR_CanTpCopyTxData);
  RESET_FAKE( CanIf_Transmit );
  RESET_FAKE( PduR_CanTpTxConfirmation);

  PduR_CanTpCopyTxData_fake.custom_fake = PduR_CanTpCopyTxData_FF;

  Std_ReturnType CanIf_Transmit_retv[] = {E_OK, E_OK, E_NOT_OK, E_OK};
  SET_RETURN_SEQ(CanIf_Transmit, CanIf_Transmit_retv, 4);

  BufReq_ReturnType PduR_CanTpCopyTxData_retv[] = {BUFREQ_OK, BUFREQ_E_NOT_OK, BUFREQ_BUSY, BUFREQ_OK};
  SET_RETURN_SEQ(PduR_CanTpCopyTxData, PduR_CanTpCopyTxData_retv, 4);

  
  /*                  TEST 1
       CanTp_State = CANTP_OFF ---> Nic nie rób
  */ 
  CanTp_State = CANTP_OFF;
  CanTp_VariablesTX.CanTp_StateTX = CANTP_TX_WAIT;
  CanTp_VariablesTX.CanTp_Current_TxId = 2;
  PduInfo.SduLength = 7;
  
  ret = CanTp_Transmit(PduId, &PduInfo);
  
  TEST_CHECK( ret == E_NOT_OK );
  TEST_CHECK(CanTp_VariablesTX.CanTp_StateTX == CANTP_TX_WAIT);
  TEST_CHECK(CanTp_VariablesTX.CanTp_Current_TxId == 2);
 
 /*
                                    TEST 2 
  CANTP_TX_WAIT - SduLenght < 8 - wysyłane Single Frame - zwracane E_OK oraz BUFREQ_OK
*/
  CanTp_State = CANTP_ON;
  CanTp_VariablesTX.CanTp_StateTX = CANTP_TX_WAIT;
  CanTp_VariablesTX.CanTp_Current_TxId = 1;
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
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[5] == '.' );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[6] == '.' );
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[7] == '.' );

  TEST_CHECK(CanTp_VariablesTX.CanTp_StateTX == CANTP_TX_WAIT);
  TEST_CHECK(CanTp_VariablesTX.CanTp_Current_TxId == 1);
  
  CanTp_ResetRX();

/*
                               TEST3 
  CANTP_TX_WAIT - SduLenght < 8 -  zwracane E_NOT_OK oraz BUFREQ_E_NOT_OK
*/
  CanTp_VariablesTX.CanTp_StateTX = CANTP_TX_WAIT;
  CanTp_VariablesTX.CanTp_Current_TxId = 1;
  PduInfo.SduLength = 7;

  ret = CanTp_Transmit(PduId, &PduInfo);
  TEST_CHECK(ret == E_NOT_OK);
  TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 2);
  TEST_CHECK(CanTp_VariablesTX.CanTp_StateTX == CANTP_TX_WAIT);
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
    
    CanTp_VariablesTX.CanTp_StateTX = CANTP_TX_WAIT;
    CanTp_VariablesTX.frame_nr_FC = 1;
    CanTp_VariablesTX.CanTp_Current_TxId = 0x2;
    CanTp_VariablesTX.message_legth = 100;
    CanTp_VariablesTX.sent_bytes = 95;
    CanTp_VariablesTX.next_SN = 0;

    ret = CanTp_CancelTransmit(0x2);

    TEST_CHECK(CanTp_VariablesTX.CanTp_StateTX == CANTP_TX_WAIT);
    TEST_CHECK(CanTp_VariablesTX.CanTp_Current_TxId == 0); 
    TEST_CHECK(CanTp_VariablesTX.frame_nr_FC == 0);
    TEST_CHECK(CanTp_VariablesTX.message_legth == 0);
    TEST_CHECK(CanTp_VariablesTX.sent_bytes == 0);
    TEST_CHECK(ret == E_OK);

  /*
                    TEST 2
              Zwracane E_NOT_OK (Błędne ID)
  */
    CanTp_VariablesTX.CanTp_StateTX = CANTP_TX_WAIT;
    CanTp_VariablesTX.frame_nr_FC = 1;
    CanTp_VariablesTX.CanTp_Current_TxId = 0x3;
    CanTp_VariablesTX.message_legth = 100;
    CanTp_VariablesTX.sent_bytes = 95;
    CanTp_VariablesTX.next_SN = 0;

    ret = CanTp_CancelTransmit(0x1);

    TEST_CHECK(CanTp_VariablesTX.CanTp_StateTX == CANTP_TX_WAIT);
    TEST_CHECK(CanTp_VariablesTX.CanTp_Current_TxId == 0x3); 
    TEST_CHECK(CanTp_VariablesTX.frame_nr_FC == 1);
    TEST_CHECK(CanTp_VariablesTX.message_legth == 100);
    TEST_CHECK(CanTp_VariablesTX.sent_bytes == 95);
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
    CanTp_VariablesRX.CanTp_StateRX == CANTP_RX_PROCESSING;
    CanTp_VariablesRX.blocks_to_next_cts = 1;
    CanTp_VariablesRX.CanTp_Current_RxId = 0x1;
    CanTp_VariablesRX.expected_CF_SN = 1;
    CanTp_VariablesRX.sended_bytes = 1;

    ret = CanTp_CancelReceive(0x1);

    TEST_CHECK(CanTp_VariablesRX.CanTp_StateRX == CANTP_RX_WAIT);
    TEST_CHECK(CanTp_VariablesRX.blocks_to_next_cts == 0);
    TEST_CHECK(CanTp_VariablesRX.CanTp_Current_RxId == 0);
    TEST_CHECK(CanTp_VariablesRX.expected_CF_SN == 0);
    TEST_CHECK(CanTp_VariablesRX.sended_bytes == 0);
    TEST_CHECK(ret == E_OK);

   /*
                    TEST 2
              Zwracane E_NOT_OK (Błędne ID)
  */
    CanTp_VariablesRX.CanTp_StateRX = CANTP_RX_PROCESSING;
    CanTp_VariablesRX.blocks_to_next_cts = 1;
    CanTp_VariablesRX.CanTp_Current_RxId = 0x3;
    CanTp_VariablesRX.expected_CF_SN = 1;
    CanTp_VariablesRX.sended_bytes = 1;

    ret = CanTp_CancelReceive(0x1);

    TEST_CHECK(CanTp_VariablesRX.CanTp_StateRX == CANTP_RX_PROCESSING);
    TEST_CHECK(CanTp_VariablesRX.blocks_to_next_cts == 1);
    TEST_CHECK(CanTp_VariablesRX.CanTp_Current_RxId == 0x3);
    TEST_CHECK(CanTp_VariablesRX.expected_CF_SN == 1);
    TEST_CHECK(CanTp_VariablesRX.sended_bytes == 1);
    TEST_CHECK(ret == E_NOT_OK);

}

/**
  @brief Test zarządzania modułem

  Funkcja testująca zarządzanie modułem CanTp.
*/
void Test_Of_CanTp_MainFunction(){
    RESET_FAKE(PduR_CanTpRxIndication);
    RESET_FAKE(CanIf_Transmit);
      
    Std_ReturnType RetVal[10] = {E_OK, E_OK, E_OK, E_OK, E_OK, E_OK};
    Std_ReturnType RetVal_transmit[10] = {E_OK, E_OK, E_NOT_OK, E_NOT_OK, E_OK, E_OK};
    SET_RETURN_SEQ( CanIf_Transmit, RetVal_transmit, 10);
    PduLengthType buffSize_array_local[11] = {0,0,0,0,10,0,0,0,9,0, 10};
    PduR_CanTpCopyRxData_buffSize_array = buffSize_array_local;

    BufReq_ReturnType BufferReturnVals[11] = { BUFREQ_OK, BUFREQ_OK, BUFREQ_OK, BUFREQ_OK, BUFREQ_OK, BUFREQ_OK, BUFREQ_OK, BUFREQ_OVFL, BUFREQ_E_NOT_OK, BUFREQ_OK, BUFREQ_OK};
    SET_RETURN_SEQ(PduR_CanTpCopyRxData, BufferReturnVals, 11);
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
    CanTp_MainFunction();

    TEST_CHECK(N_Ar_timer.eState == TIMER_NOT_ACTIVE);
    TEST_CHECK(N_Br_timer.eState == TIMER_NOT_ACTIVE);
    TEST_CHECK(N_Cr_timer.eState == TIMER_NOT_ACTIVE);
    TEST_CHECK(N_Ar_timer.counter == 0);
    TEST_CHECK(N_Br_timer.counter == 0);
    TEST_CHECK(N_Cr_timer.counter == 0);
    TEST_CHECK(PduR_CanTpCopyRxData_fake.call_count == 0); 
    TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 0);

    /*
                TEST 2
    Standardowa praca układu dla Timerów RX
    */
    CanTp_VariablesRX.message_length = 10;
    CanTp_VariablesRX.sended_bytes = 0;

    CanTp_TimerStart(&N_Ar_timer);
    CanTp_TimerStart(&N_Br_timer);
    CanTp_TimerStart(&N_Cr_timer);
    
    CanTp_MainFunction();
    CanTp_MainFunction();
    CanTp_MainFunction();
    CanTp_MainFunction();

    TEST_CHECK(N_Ar_timer.eState == TIMER_ACTIVE);
    TEST_CHECK(N_Br_timer.eState == TIMER_ACTIVE);
    TEST_CHECK(N_Cr_timer.eState == TIMER_ACTIVE);
    TEST_CHECK(N_Ar_timer.counter == 4);
    TEST_CHECK(N_Br_timer.counter == 4);
    TEST_CHECK(N_Cr_timer.counter == 4);
      
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

    TEST_CHECK(N_As_timer.counter == 3);
    TEST_CHECK(N_As_timer.eState == TIMER_ACTIVE);
    TEST_CHECK(N_Bs_timer.counter == 3);
    TEST_CHECK(N_Bs_timer.eState == TIMER_ACTIVE);
    TEST_CHECK(N_Cs_timer.counter == 3);
    TEST_CHECK(N_Cs_timer.eState == TIMER_ACTIVE);

    /*
                TEST 4
    Timeout dla Timera As TX
    */

    CanTp_ResetRX();
    CanTp_ResetTX();

    N_As_timer.counter = 99;
    CanTp_TimerStart(&N_As_timer);
    CanTp_MainFunction();

    TEST_CHECK(N_As_timer.counter == 0);
    TEST_CHECK(N_As_timer.eState == TIMER_NOT_ACTIVE);

    TEST_CHECK(N_Bs_timer.counter == 0);
    TEST_CHECK(N_Bs_timer.eState == TIMER_NOT_ACTIVE);

    TEST_CHECK(N_Cs_timer.counter == 0);
    TEST_CHECK(N_Cs_timer.eState == TIMER_NOT_ACTIVE);

    TEST_CHECK(PduR_CanTpTxConfirmation_fake.call_count == 1);
}


/**
  @brief Test Potwierdzenia transmisji

  Funkcja testującapotwierdzenie transmisji PDU.
*/
void Test_Of_CanTp_TxConfirmation(void){
   PduIdType PduId = 0x01;
  PduInfoType PduInfo;
  uint8 SduDataPtr[8];
  uint8 *MetaDataPtr;
  PduInfo.MetaDataPtr = MetaDataPtr;
  PduInfo.SduDataPtr = SduDataPtr;
  Std_ReturnType ret; 

  PduLengthType availableDataPtr_array_local[10] = {1,2,3,4,5,6,7,8,9,0};
  uint8 sdu_data_ptr_array[5][7] = { "dupa...", "dupa...", "test...","dwa....", "trzy..." };

  PduLengthType availableData;
  int i;
  for( i = 0; i < 5; i++){
      memcpy(PduR_CanTpCopyTxData_sdu_data[i], sdu_data_ptr_array[i], sizeof(uint8)*7);
  }
  PduR_CanTpCopyTxData_availableDataPtr = availableDataPtr_array_local;

  RESET_FAKE(PduR_CanTpCopyTxData);
  RESET_FAKE(CanIf_Transmit);
  RESET_FAKE(PduR_CanTpTxConfirmation);

  PduR_CanTpCopyTxData_fake.custom_fake = PduR_CanTpCopyTxData_FF;

  Std_ReturnType CanIf_Transmit_retv[] = {E_OK, E_OK, E_NOT_OK, E_OK};
  SET_RETURN_SEQ(CanIf_Transmit, CanIf_Transmit_retv, 4);

  BufReq_ReturnType PduR_CanTpCopyTxData_retv[] = {BUFREQ_OK, BUFREQ_OK, BUFREQ_OK, BUFREQ_E_NOT_OK , BUFREQ_BUSY};
  SET_RETURN_SEQ(PduR_CanTpCopyTxData, PduR_CanTpCopyTxData_retv, 5);

  CanTp_State = CANTP_ON;
/*
          TEST 1
  Transmisja poprawna
*/
  CanTp_VariablesTX.CanTp_StateTX = CANTP_TX_PROCESSING;
  CanTp_VariablesTX.frame_nr_FC = 1;
  CanTp_VariablesTX.CanTp_Current_TxId = 0x1;
  CanTp_VariablesTX.message_legth = 100;
  CanTp_VariablesTX.sent_bytes = 95;
  CanTp_VariablesTX.next_SN = 0;

  CanTp_VariablesTX.CanTp_Current_TxId = 1;
  CanTp_TxConfirmation (1, E_OK);
 
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
  TEST_CHECK(CanIf_Transmit_fake.arg1_val->SduDataPtr[5] == '.' );

  TEST_CHECK(CanTp_VariablesTX.CanTp_StateTX == CANTP_TX_PROCESSING);
  TEST_CHECK(CanTp_VariablesTX.CanTp_Current_TxId == 1); 
  TEST_CHECK(CanTp_VariablesTX.frame_nr_FC == 0);
  TEST_CHECK(CanTp_VariablesTX.message_legth == 100);
  TEST_CHECK(CanTp_VariablesTX.sent_bytes == 100);

  CanTp_ResetRX();
  CanTp_ResetTX();

  /*
            TEST 2
    RETURN E_NOT_OK
  */

  CanTp_VariablesTX.CanTp_StateTX = CANTP_TX_PROCESSING;
  CanTp_VariablesTX.frame_nr_FC = 1;
  CanTp_VariablesTX.CanTp_Current_TxId = 0x1;
  CanTp_VariablesTX.message_legth = 100;
  CanTp_VariablesTX.sent_bytes = 95;
  CanTp_VariablesTX.next_SN = 0;

  CanTp_VariablesTX.CanTp_Current_TxId = 1;

  CanTp_TxConfirmation (1, E_NOT_OK );
 
  TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 1);
  
  TEST_CHECK( PduR_CanTpTxConfirmation_fake.call_count == 1 );
  TEST_CHECK( PduR_CanTpTxConfirmation_fake.arg0_val == 1 );
  TEST_CHECK( PduR_CanTpTxConfirmation_fake.arg1_val == E_NOT_OK );

  TEST_CHECK(CanIf_Transmit_fake.call_count == 1 );
  TEST_CHECK(CanTp_VariablesTX.CanTp_StateTX == CANTP_TX_WAIT);
  TEST_CHECK(CanTp_VariablesTX.CanTp_Current_TxId == 0);
  TEST_CHECK(CanTp_VariablesTX.frame_nr_FC == 0);
  TEST_CHECK(CanTp_VariablesTX.message_legth == 0);
  TEST_CHECK(CanTp_VariablesTX.sent_bytes == 0);

  CanTp_ResetRX();
  CanTp_ResetTX();

  /*
      TEST 3 
    UNKNOWN ID
  */

  CanTp_VariablesTX.CanTp_StateTX = CANTP_TX_PROCESSING;
  CanTp_VariablesTX.frame_nr_FC = 1;
  CanTp_VariablesTX.CanTp_Current_TxId = 0x1;
  CanTp_VariablesTX.message_legth = 100;
  CanTp_VariablesTX.sent_bytes = 95;
  CanTp_VariablesTX.next_SN = 0;

  CanTp_VariablesTX.CanTp_Current_TxId = 1;

  CanTp_TxConfirmation(2, E_NOT_OK);

  TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 1);
  TEST_CHECK( PduR_CanTpTxConfirmation_fake.call_count == 1 );

  TEST_CHECK(CanIf_Transmit_fake.call_count == 1 );

  TEST_CHECK(CanTp_VariablesTX.CanTp_StateTX == CANTP_TX_PROCESSING);
  TEST_CHECK(CanTp_VariablesTX.CanTp_Current_TxId == 1);
  TEST_CHECK(CanTp_VariablesTX.frame_nr_FC == 1);
  TEST_CHECK(CanTp_VariablesTX.message_legth == 100);
  TEST_CHECK(CanTp_VariablesTX.sent_bytes == 95);

  CanTp_ResetRX();
  CanTp_ResetTX();
}

/** ==================================================================================================================*\
                                TESTY FUNKCJI POMOCNICZYCH
\*====================================================================================================================*/

void Test_Of_CanTp_ResetRX(){     
    CanTp_VariablesRX.CanTp_StateRX == CANTP_RX_PROCESSING;
    CanTp_VariablesRX.expected_CF_SN == 1;
    CanTp_VariablesRX.message_length == 5;
    CanTp_VariablesRX.sended_bytes == 4;
    CanTp_VariablesRX.blocks_to_next_cts == 2;
    CanTp_VariablesRX.CanTp_Current_RxId == 2;
    CanTp_ResetRX();
    TEST_CHECK(CanTp_VariablesRX.CanTp_StateRX == CANTP_RX_WAIT);
    TEST_CHECK(CanTp_VariablesRX.expected_CF_SN == 0);
    TEST_CHECK(CanTp_VariablesRX.message_length == 0);
    TEST_CHECK(CanTp_VariablesRX.sended_bytes == 0);
    TEST_CHECK(CanTp_VariablesRX.blocks_to_next_cts == 0);
    TEST_CHECK(CanTp_VariablesRX.CanTp_Current_RxId == 0);
}

void Test_Of_CanTp_ResetTX(void){
    CanTp_VariablesTX.CanTp_StateTX = CANTP_TX_PROCESSING;
    CanTp_VariablesTX.frame_nr_FC = 2;
    CanTp_VariablesTX.CanTp_Current_TxId = 3;
    CanTp_VariablesTX.message_legth = 4;
    CanTp_VariablesTX.sent_bytes = 5;
    CanTp_ResetTX();
    TEST_CHECK(CanTp_VariablesTX.CanTp_StateTX == CANTP_TX_WAIT);
    TEST_CHECK(CanTp_VariablesTX.frame_nr_FC == 0);
    TEST_CHECK(CanTp_VariablesTX.CanTp_Current_TxId == 0);
    TEST_CHECK(CanTp_VariablesTX.message_legth == 0);
    TEST_CHECK(CanTp_VariablesTX.sent_bytes == 0);
}

void Test_Of_CanTp_Calc_Available_Blocks(void){
  uint16 ret;
  CanTp_VariablesRX.message_length = 8;
  CanTp_VariablesRX.sended_bytes = 0;
  TEST_CHECK(CanTp_Calc_Available_Blocks(10) == 2);
  TEST_CHECK(CanTp_Calc_Available_Blocks(6) == 0);
}

void Test_Of_CanTp_FrameCheckType(void)
{
    sint32 result;
    Std_ReturnType ret;
    uint8 sdu_data[8];
    CanPCI_Type CanFrameInfo;
    PduInfoType can_data;
    can_data.SduDataPtr = sdu_data;
    can_data.SduLength = 8;

    // SF - Single Frame Type check
    can_data.SduDataPtr[0] = 0x0F; 
    can_data.SduDataPtr[1] = 0;
    can_data.SduDataPtr[2] = 0;

    ret = CanTp_GetPCI(&can_data, &CanFrameInfo);
    TEST_CHECK(ret == E_OK);
    TEST_CHECK(CanFrameInfo.frame_type == SF);
    TEST_CHECK(CanFrameInfo.frame_lenght == 0xF);
    TEST_CHECK(CanFrameInfo.BS == 0);
    TEST_CHECK(CanFrameInfo.FS == 0);
    TEST_CHECK(CanFrameInfo.SN == 0);
    TEST_CHECK(CanFrameInfo.ST == 0);
}


TEST_LIST = {
    { "Test of CanTp_MainFunction", Test_Of_CanTp_MainFunction },
    // { "Test of CanTp_FrameCheckType", Test_Of_CanTp_FrameCheckType },
    // { "Test of CanTp_Calc_Available_Blocks", Test_Of_CanTp_Calc_Available_Blocks },
    // { "Test of CanTp_ResetTX", Test_Of_CanTp_ResetTX },
    // { "Test of CanTp_ResetRX", Test_Of_CanTp_ResetRX },
    // { "Test of CanTp_TxConfirmation", Test_Of_CanTp_TxConfirmation },
    // { "Test of CanTp_CancelReceive", Test_Of_CanTp_CancelReceive },
    // { "Test of CanTp_CancelTransmit", Test_Of_CanTp_CancelTransmit },
    // { "Test of CanTp_Transmit", TestOf_CanTp_Transmit },
    // { "Test of CanTp_Init", Test_Of_CanTp_Init },
    // { "Test of CanTp_Shutdown", Test_Of_CanTp_Shutdown },
	  // { "Test of CanTp_GetVersionInfo", Test_Of_CanTp_GetVersionInfo },
    { NULL, NULL }                           
};