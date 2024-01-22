/**===================================================================================================================*\
  @file CanTp.c

  @brief Can Transport Layer
  
    Implementacja Can Transport Layer
\*====================================================================================================================*/

/*====================================================================================================================*\
    Załączenie nagłówków
\*====================================================================================================================*/
#include "CanTp.h"
#include "CanIf.h"
#include "PduR.h"

/*====================================================================================================================*\
    Zmienne globalne
\*====================================================================================================================*/
CanTpState_type CanTp_State; 

CanTp_VariablesTX_type CanTp_VariablesTX;

CanTp_VariablesRX_type CanTp_VariablesRX;

CanTp_Timer_type N_Ar_timer =   {TIMER_DISABLE, 0, N_AR_TIMEOUT_VAL};
CanTp_Timer_type N_Br_timer =   {TIMER_DISABLE, 0, N_BR_TIMEOUT_VAL};
CanTp_Timer_type N_Cr_timer =   {TIMER_DISABLE, 0, N_CR_TIMEOUT_VAL};
CanTp_Timer_type N_As_timer =   {TIMER_DISABLE, 0, N_AS_TIMEOUT_VAL};
CanTp_Timer_type N_Bs_timer =   {TIMER_DISABLE, 0, N_BS_TIMEOUT_VAL};
CanTp_Timer_type N_Cs_timer =   {TIMER_DISABLE, 0, N_CS_TIMEOUT_VAL};
CanTp_Timer_type STMmin_timer = {TIMER_DISABLE, 0, STMmin_TIMEOUT_VAL};
uint32 FC_Wait_frame_ctr;
/*====================================================================================================================*\
    Deklaracje funkcji lokalnych
\*====================================================================================================================*/
static void CanTp_ResetRX(void);
static void CanTp_ResetTX(void);
static void CanTp_SendNextCF(void);
static void CanTp_FlowControlReception(PduIdType RxPduId, CanPCI_Type *CanPCI);
static void CanTp_FirstFrameReception(PduIdType RxPduId, const PduInfoType *PduInfoPtr, CanPCI_Type *CanPCI);
static void CanTp_SingleFrameReception(PduIdType RxPduId, CanPCI_Type *CanPCI, const PduInfoType* PduInfoPtr);
static void CanTp_ConsecutiveFrameReception(PduIdType RxPduId, CanPCI_Type *CanPCI, const PduInfoType* PduInfoPtr);

static uint16 CanTp_CalcBlocksSize(uint16 uiBufferSize);

static Std_ReturnType CanTp_GetPCI(const PduInfoType* CanData, CanPCI_Type* CanFrameInfo);
static Std_ReturnType CanTp_SendFlowControl(PduIdType ID, uint8 uiBlockSize, FlowControlStatus_type FC_Status, uint8 uiSeparationTime);
static Std_ReturnType CanTp_SendConsecutiveFrame(PduIdType id, uint8 uiSequenceNumber, uint8* puiPayload, uint32 uiSize);
static Std_ReturnType CanTp_SendSingleFrame(PduIdType id, uint8* puiPayload, uint32 uiSize);
static Std_ReturnType CanTp_SendFirstFrame(PduIdType id, uint32 uiMsgLen);
static Std_ReturnType CanTp_PrepareSegmenetedFrame(CanPCI_Type *CanPCI, PduInfoType *CanPdu_Info, uint8_t *puiCanPayload);
/*====================================================================================================================*\
    Kod funkcji
\*====================================================================================================================*/

/*====================================================================================================================*/
/**
  @brief CanTp_Init

 Funkcja inicjalizuje moduł CanTp [SWS_CANTP_00208]
*/
/*====================================================================================================================*/
void CanTp_Init (void){
    CanTp_ResetRX();
    CanTp_ResetTX();
    CanTp_State = CANTP_ON;
}
/*====================================================================================================================*/
/**
  @brief CanTp_GetVersionInfo

 Funkcja zwracajaca informacje o wersji [SWS_CANTP_00210]
*/
/*====================================================================================================================*/
void CanTp_GetVersionInfo(Std_VersionInfoType *versioninfo) {
    if (versioninfo != NULL_PTR) {
    	versioninfo->sw_major_version = CANTP_SW_MAJOR_V;
    	versioninfo->sw_minor_version = CANTP_SW_MINOR_V;
    	versioninfo->sw_patch_version = CANTP_SW_PATCH_V;
    	versioninfo->moduleID = (uint16)CANTP_MODULE_ID;
    	versioninfo->vendorID = 0x00u;
    }
}
/*====================================================================================================================*/
/**
  @brief CanTp_Shutdown

  Funkcja wywoływana w celu wyłączenia modułu CanTp. [SWS_CANTP_00211]

  Wypełnia:
   [SWS_CANTP_00202]
   [SWS_CANTP_00200]
*/
/*====================================================================================================================*/
void CanTp_Shutdown(void){
    /* Wypełnia [SWS_CANTP_00202]*/
    CanTp_ResetRX();
    CanTp_ResetTX();
    /* Wypełnia [SWS_CANTP_00200]*/
    CanTp_State = CANTP_OFF;
}
/*====================================================================================================================*/
/**
  @brief CanTp_Transmit
  Funkcja żąda przesłania PDU (Protocal Data Unit)  [SWS_CANTP_00212]
  Wypełnia:
   [SWS_CANTP_00231]
   [SWS_CANTP_00232]
   [SWS_CANTP_00204]
   [SWS_CANTP_00205]
   [SWS_CANTP_00206]
   [SWS_CANTP_00298]
   [SWS_CANTP_00299]
   [SWS_CANTP_00321]
   [SWS_CANTP_00354]
*/
/*====================================================================================================================*/
Std_ReturnType CanTp_Transmit(PduIdType TxPduId, const PduInfoType* PduInfoPtr){
    BufReq_ReturnType BufReqState;
    PduLengthType PduLen;
    Std_ReturnType ret = E_OK;
    PduInfoType PduTmp;
    uint8_t uiPayload[8];
    PduTmp.SduDataPtr = uiPayload;
    PduTmp.MetaDataPtr = NULL;  
    if(CanTp_State == CANTP_ON){
        if( CanTp_VariablesTX.eCanTp_StateTX == CANTP_TX_WAIT){
            if(PduInfoPtr->SduLength < 8){
                PduTmp.SduLength = PduInfoPtr->SduLength;
                BufReqState = PduR_CanTpCopyTxData(TxPduId, &PduTmp, NULL, &PduLen);
                if(BufReqState == BUFREQ_OK){
                     /* Wypełnia [SWS_CANTP_00231]*/
                     /* Wypełnia [SWS_CANTP_00354]*/
                    ret = CanTp_SendSingleFrame(TxPduId, PduTmp.SduDataPtr, PduInfoPtr->SduLength );
                }
                /* Wypełnia [SWS_CANTP_00298]*/
                else if(BufReqState == BUFREQ_E_NOT_OK){
                    CanTp_ResetTX();  
                    /* Wypełnia [SWS_CANTP_00205]*/
                    PduR_CanTpTxConfirmation(TxPduId, E_NOT_OK);
                    ret = E_NOT_OK;
                }
                else {
                    /* Wypełnia [SWS_CANTP_00204]*/
                    // PduR_CanTpTxConfirmation(TxPduId, E_OK);
                    CanTp_TimerStart(&N_Cs_timer);
                    ret = E_OK;
                }
            }
            else{
                if(CanTp_SendFirstFrame(TxPduId, PduInfoPtr->SduLength) == E_OK){
                    CanTp_VariablesTX.eCanTp_StateTX = TxPduId;
                    CanTp_VariablesTX.eCanTp_StateTX = CANTP_TX_PROCESSING_SUSPENDED;
                    CanTp_VariablesTX.uiMsgLen = PduInfoPtr->SduLength;
                    CanTp_VariablesTX.uiTransmittedBytes = 0;
                    ret = E_OK;
                }
                else{
                    ret = E_NOT_OK;
                }
            }
        }
        else{
            ret = E_NOT_OK;
        }
    }   
    else{ 
        ret = E_NOT_OK;
    }
    return ret;
}

/**
  @brief CanTp_CancelTransmit

  Przerwanie trwającej transmisji PDU [SWS_CANTP_00246]
  Wypełnia:
   [SWS_CANTP_00254]
   [SWS_CANTP_00255]
   [SWS_CANTP_00256]
*/
Std_ReturnType CanTp_CancelTransmit(PduIdType TxPduId){
    Std_ReturnType ret;             
    if(CanTp_VariablesTX.CanTp_Current_TxId == TxPduId ){
        /* Wypełnia [SWS_CANTP_00255]*/
        PduR_CanTpTxConfirmation(CanTp_VariablesTX.CanTp_Current_TxId, E_NOT_OK);
        CanTp_ResetTX();
        /* Wypełnia [SWS_CANTP_00256]*/
        ret = E_OK;
    }
    else{
        /* Wypełnia [SWS_CANTP_00254]*/
        ret = E_NOT_OK;
    }
    return ret;
}

/**
  @brief CanTp_CancelReceive

  Przerwanie trwającego odbioru PDU [SWS_CANTP_00257]
  Wypełnia:
   [SWS_CANTP_00260]
   [SWS_CANTP_00261]
   [SWS_CANTP_00262]
   [SWS_CANTP_00263]
*/

Std_ReturnType CanTp_CancelReceive(PduIdType RxPduId){
    Std_ReturnType ret;             
    if( CanTp_VariablesRX.CanTp_Current_RxId == RxPduId ){
        /* Wypełnia [SWS_CANTP_00263]*/
        PduR_CanTpRxIndication(CanTp_VariablesRX.CanTp_Current_RxId, E_NOT_OK);
        CanTp_ResetRX();
        /* Wypełnia [SWS_CANTP_00261]*/
        ret = E_OK;
    }
    else{
        /* Wypełnia [SWS_CANTP_00260]*/
        /* Wypełnia [SWS_CANTP_00262]*/
        ret = E_NOT_OK;
    }
    return ret;
}

/**
  @brief CanTp_ChangeParameter

  Zadanie zmiany konkretnego parametru.[SWS_CANTP_00302]
  Wypełnia:
   [SWS_CANTP_00303]
   [SWS_CANTP_00304]
   [SWS_CANTP_00305]
   [SWS_CANTP_00338]
*/
Std_ReturnType CanTp_ChangeParameter(PduIdType id, TPParameterType parameter, uint16 value){
    Std_ReturnType ret = E_NOT_OK;
    if (CanTp_State == CANTP_ON){
        switch(parameter){
            case TP_STMIN:
                ret = E_OK;
                break;
            case TP_BS:
                ret = E_OK;
                break;
            case TP_BC:
                break;
            default:
                break;
        }
    }
    return ret;
}
/**
  @brief CanTp_ReadParameter

  Służy do odczytu aktualnych wartośći BS i STmin.[SWS_CANTP_00323]
  Wypełnia:
   [SWS_CANTP_00324]
*/
Std_ReturnType CanTp_ReadParameter(PduIdType id, TPParameterType parameter, uint16* value){
    Std_ReturnType ret = E_NOT_OK;
    if (CanTp_State == CANTP_ON){
        uint16 readVal;
        switch (parameter) {
            case TP_STMIN:
                readVal = 1;
                ret = E_OK;
                break;
            case TP_BS:
                readVal = 1;
                ret = E_OK;
                break;
            case TP_BC:
            default:
                break;
        }
        if ((ret == E_OK) && (readVal <= 0xff)) {
            *value = readVal;
        } else {
            ret = E_NOT_OK;
        }
    }
    return ret;
}

/**
  @brief CanTp_MainFunction
  Funkcja odpowiadająca za zarządzaniem modułem CanTp [SWS_CANTP_00213]
  Wypełnia:
   [SWS_CANTP_00164]
   [SWS_CANTP_00300]
*/
void CanTp_MainFunction(void){
    /* Wypełnia [SWS_CANTP_00164]*/
    static boolean N_Ar_timeout, N_Br_timeout, N_Cr_timeout, N_As_timeout, N_Bs_timeout, N_Cs_timeout, STMmin_timeout;
    static PduLengthType PduLen;
    static const PduInfoType PduInfoConst = {NULL, NULL, 0};
    uint16 uiBlockSize;
    uint8 uiSeparationTime;
    BufReq_ReturnType BufReqState; 

    CanTp_TimerTick(&N_Ar_timer);
    CanTp_TimerTick(&N_Br_timer);
    CanTp_TimerTick(&N_Cr_timer);

    CanTp_TimerTick(&N_As_timer);
    CanTp_TimerTick(&N_Bs_timer);
    CanTp_TimerTick(&N_Cs_timer);

   if(N_Br_timer.eState == TIMER_ENABLE){
       BufReqState = PduR_CanTpCopyRxData(CanTp_VariablesRX.CanTp_Current_RxId, &PduInfoConst, &PduLen);
       if(BufReqState == BUFREQ_E_NOT_OK){
           PduR_CanTpRxIndication(CanTp_VariablesRX.CanTp_Current_RxId, E_NOT_OK);
       }
       else{
            uiBlockSize = CanTp_CalcBlocksSize(PduLen);
            if(uiBlockSize > 0){
                CanTp_VariablesRX.uiBlocksNxtCts = uiBlockSize;
                CanTp_VariablesRX.eCanTp_StateRX = CANTP_RX_PROCESSING;
                if(CanTp_SendFlowControl(CanTp_VariablesRX.CanTp_Current_RxId, uiBlockSize, FC_CTS, uiSeparationTime) == E_NOT_OK){            
                    CanTp_ResetRX();
                }
                else{
                    CanTp_TimerReset(&N_Br_timer); 
                }  
            }
            if(CanTp_TimerTimeout(&N_Br_timer)){
                FC_Wait_frame_ctr++;
                N_Br_timer.uiCounter = 0;
                if(FC_Wait_frame_ctr >= FC_WAIT_FRAME_CTR_MAX){
                    PduR_CanTpRxIndication (CanTp_VariablesRX.CanTp_Current_RxId, E_NOT_OK);
                    CanTp_ResetRX();         
                    FC_Wait_frame_ctr = 0;
                }
                else{
                    if(CanTp_SendFlowControl(CanTp_VariablesRX.CanTp_Current_RxId, uiBlockSize, FC_WAIT, uiSeparationTime) == E_NOT_OK){
                        CanTp_ResetRX();
                    }
                }
            }
        }
   }
   if(N_Cr_timer.eState == TIMER_ENABLE){
       if(CanTp_TimerTimeout(&N_Cr_timer) == E_NOT_OK){
            PduR_CanTpRxIndication(CanTp_VariablesRX.CanTp_Current_RxId, E_NOT_OK);
            CanTp_ResetRX();
       }
   }
   if(N_Ar_timer.eState == TIMER_ENABLE){
       if(CanTp_TimerTimeout(&N_Ar_timer) == E_NOT_OK){
            PduR_CanTpRxIndication(CanTp_VariablesRX.CanTp_Current_RxId, E_NOT_OK);
            CanTp_ResetRX();
       }
   }
    if(N_Cs_timer.eState == TIMER_ENABLE){
       if(CanTp_TimerTimeout(&N_Cs_timer) == E_NOT_OK){
            PduR_CanTpTxConfirmation(CanTp_VariablesTX.CanTp_Current_TxId, E_NOT_OK);
            CanTp_ResetTX();
       }
   }
    if(N_As_timer.eState == TIMER_ENABLE){
       if(CanTp_TimerTimeout(&N_As_timer) == E_NOT_OK){
            PduR_CanTpTxConfirmation(CanTp_VariablesTX.CanTp_Current_TxId, E_NOT_OK);
            CanTp_ResetTX();
       }
   }
    if(N_Bs_timer.eState == TIMER_ENABLE){
       if(CanTp_TimerTimeout(&N_Bs_timer) == E_NOT_OK){
            PduR_CanTpTxConfirmation(CanTp_VariablesTX.CanTp_Current_TxId, E_NOT_OK);
            CanTp_ResetTX();
       }
   }
} 

/**
  @brief CanTp_RxIndication

  Wskazanie odebranej jednostki PDU [SWS_CANTP_00214]
  Wypełnia:
   [SWS_CANTP_00235]
   [SWS_CANTP_00322]
*/

void CanTp_RxIndication(PduIdType RxPduId, const PduInfoType* PduInfoPtr){
    
    CanPCI_Type CanPCI;    

    if(CanTp_State == CANTP_ON){
        if(CanTp_VariablesRX.eCanTp_StateRX == CANTP_RX_WAIT){
        
            CanTp_GetPCI(PduInfoPtr, &CanPCI);

            if(CanPCI.eFrameType == CAN_FF){
                CanTp_FirstFrameReception(RxPduId, PduInfoPtr, &CanPCI);
            }
            else if(CanPCI.eFrameType == CAN_SF){
                CanTp_SingleFrameReception(RxPduId, &CanPCI, PduInfoPtr);           
            } 
            else if(CanPCI.eFrameType == CAN_FC){
                CanTp_FlowControlReception(RxPduId, &CanPCI);
            }
            else
            {
                CanTp_VariablesRX.eCanTp_StateRX = CANTP_RX_WAIT; 
            } 
        }
        else if(CanTp_VariablesRX.eCanTp_StateRX == CANTP_RX_PROCESSING){
             CanTp_GetPCI(PduInfoPtr, &CanPCI);
             if(CanPCI.eFrameType == CAN_CF){
                CanTp_ConsecutiveFrameReception(RxPduId, &CanPCI, PduInfoPtr);
            }
            else if(CanPCI.eFrameType == CAN_FC){
                CanTp_FlowControlReception(RxPduId, &CanPCI);
            }
            else if(CanPCI.eFrameType == CAN_FF){            
                PduR_CanTpRxIndication (CanTp_VariablesRX.CanTp_Current_RxId, E_NOT_OK);
                CanTp_ResetRX();
                CanTp_FirstFrameReception(RxPduId, PduInfoPtr, &CanPCI);
            }
            else if(CanPCI.eFrameType == CAN_SF){            
                PduR_CanTpRxIndication (CanTp_VariablesRX.CanTp_Current_RxId, E_NOT_OK);
                CanTp_ResetRX();
                CanTp_SingleFrameReception(RxPduId, &CanPCI, PduInfoPtr);
            }
            else{
            }
        }
        else {    
            CanTp_GetPCI(PduInfoPtr, &CanPCI);
            if(CanPCI.eFrameType == CAN_FC){
                CanTp_FlowControlReception(RxPduId, &CanPCI);
            }
            else if(CanPCI.eFrameType == CAN_FF) {            
                PduR_CanTpRxIndication (CanTp_VariablesRX.CanTp_Current_RxId, E_NOT_OK);
                CanTp_ResetRX();
                CanTp_FirstFrameReception(RxPduId, PduInfoPtr, &CanPCI);
            }
            else if(CanPCI.eFrameType == CAN_SF) {            
                PduR_CanTpRxIndication (CanTp_VariablesRX.CanTp_Current_RxId, E_NOT_OK);
                CanTp_ResetRX();
                CanTp_SingleFrameReception(RxPduId, &CanPCI, PduInfoPtr);
            }
            else {
                PduR_CanTpRxIndication (CanTp_VariablesRX.CanTp_Current_RxId, E_NOT_OK);
                CanTp_ResetRX();
            }
        }
    }
}

/**
  @brief CanTp_TxConfirmation

  Informacja o statusie transmisji PDU.[SWS_CANTP_00215]
  Wypełnia:
   [SWS_CANTP_00236]
*/
void CanTp_TxConfirmation(PduIdType TxPduId, Std_ReturnType result){
if( CanTp_State == CANTP_ON ){  
    if(CanTp_VariablesRX.CanTp_Current_RxId == TxPduId){
        if( (CanTp_VariablesRX.eCanTp_StateRX == CANTP_RX_PROCESSING ) || (CanTp_VariablesRX.eCanTp_StateRX == CANTP_RX_PROCESSING_SUSPEND)){
            if(result == E_OK){
                CanTp_TimerReset(&N_Ar_timer);   
            }    
            else{
                PduR_CanTpRxIndication(CanTp_VariablesRX.CanTp_Current_RxId, E_NOT_OK);
                CanTp_ResetRX();
            }
        }
        else{} 
    }
    if(CanTp_VariablesTX.CanTp_Current_TxId == TxPduId ){
        if(result == E_OK){
            if(CanTp_VariablesTX.eCanTp_StateTX == CANTP_TX_PROCESSING)
            {
               CanTp_SendNextCF();               
            }
            else{}
        }
        else{
            PduR_CanTpTxConfirmation(CanTp_VariablesTX.CanTp_Current_TxId, E_NOT_OK);
            CanTp_ResetTX();
        }
    }
    else{}
}
}

/*====================================================================================================================*\
    Definicja funkcji lokalnych
\*====================================================================================================================*/

/*====================================================================================================================*/
/**
  @brief CanTp_ResetRX
*/
static void CanTp_ResetRX(void){
    CanTp_VariablesRX.eCanTp_StateRX = CANTP_RX_WAIT;
    CanTp_VariablesRX.uiMsgLen = 0;
    CanTp_VariablesRX.uiTransmittedBytes = 0;
    CanTp_VariablesRX.uiExpected_CF_SN = 0;
    CanTp_VariablesRX.uiBlocksNxtCts = 0;
    CanTp_VariablesRX.CanTp_Current_RxId = 0;
    CanTp_TimerReset(&N_Ar_timer);
    CanTp_TimerReset(&N_Br_timer);
    CanTp_TimerReset(&N_Cr_timer);
}
/**
  @brief CanTp_ResetTX
*/
static void CanTp_ResetTX(void){
    CanTp_VariablesTX.eCanTp_StateTX = CANTP_TX_WAIT;
    CanTp_VariablesTX.uiMsgLen = 0;
    CanTp_VariablesTX.uiTransmittedBytes = 0;
    CanTp_VariablesTX.uiFrameNrFC = 0;
    CanTp_VariablesTX.CanTp_Current_TxId = 0;
    CanTp_TimerReset(&N_As_timer);
    CanTp_TimerReset(&N_Bs_timer);
    CanTp_TimerReset(&N_Cs_timer);
}

/**
  @brief CanTp_PrepareSegmenetedFrame
*/
static Std_ReturnType CanTp_PrepareSegmenetedFrame(CanPCI_Type *CanPCI, PduInfoType *CanPdu_Info, uint8_t *puiCanPayload){

    Std_ReturnType ret = E_OK;

    if(NE_NULL_PTR(CanPCI) && NE_NULL_PTR(CanPdu_Info) && NE_NULL_PTR(puiCanPayload)){  
        
        switch(CanPCI->eFrameType){
            case CAN_SF:
                *(CanPdu_Info->SduDataPtr) = 0;
                *(CanPdu_Info->SduDataPtr) = CAN_SF << 4;
        
                if(CanPCI->uiFrameLenght <= 7){
                    *(CanPdu_Info->SduDataPtr) = 0x0F & CanPCI->uiFrameLenght; 
                    for(uint8_t i = 0; i < CanPCI->uiFrameLenght; i++){
                        *(CanPdu_Info->SduDataPtr + (i + 1)) = *(puiCanPayload + i);
                    }  
                }
                else{
                    ret = E_NOT_OK;
                }
            break;
            case CAN_CF: 
                *(CanPdu_Info->SduDataPtr) = 0;
                *(CanPdu_Info->SduDataPtr) = CAN_CF << 4;
                if(CanPCI->uiSequenceNumber < 7){
                    *(CanPdu_Info->SduDataPtr) |= (0x0F & CanPCI->uiSequenceNumber);
                    *(CanPdu_Info->SduDataPtr + 1) = *(puiCanPayload);
                    *(CanPdu_Info->SduDataPtr + 2) = *(puiCanPayload + 1);
                    *(CanPdu_Info->SduDataPtr + 3) = *(puiCanPayload + 2);
                    *(CanPdu_Info->SduDataPtr + 4) = *(puiCanPayload + 3);
                    *(CanPdu_Info->SduDataPtr + 5) = *(puiCanPayload + 4);
                    *(CanPdu_Info->SduDataPtr + 6) = *(puiCanPayload + 5);
                    *(CanPdu_Info->SduDataPtr + 7) = *(puiCanPayload + 6);
                }
                else{
                    ret = E_NOT_OK; 
                }
            break;
            case CAN_FF:    
                *(CanPdu_Info->SduDataPtr) = 0;
                *(CanPdu_Info->SduDataPtr) = CAN_FF << 4;
                if(CanPCI->uiFrameLenght <= 4095){
                    *(CanPdu_Info->SduDataPtr) |= (0x0F & (CanPCI->uiFrameLenght >> 8));
                    *(CanPdu_Info->SduDataPtr + 1) = (0xFF & (CanPCI->uiFrameLenght));
                    *(CanPdu_Info->SduDataPtr + 2) = *(puiCanPayload);
                    *(CanPdu_Info->SduDataPtr + 3) = *(puiCanPayload + 1);
                    *(CanPdu_Info->SduDataPtr + 4) = *(puiCanPayload + 2);
                    *(CanPdu_Info->SduDataPtr + 5) = *(puiCanPayload + 3);
                    *(CanPdu_Info->SduDataPtr + 6) = *(puiCanPayload + 4);
                    *(CanPdu_Info->SduDataPtr + 7) = *(puiCanPayload + 5);
                }
                else{
                    *(CanPdu_Info->SduDataPtr + 1) = 0;
                    *(CanPdu_Info->SduDataPtr + 2) = (CanPCI->uiFrameLenght >> 24) & 0xFF;
                    *(CanPdu_Info->SduDataPtr + 3) = (CanPCI->uiFrameLenght >> 16) & 0xFF;
                    *(CanPdu_Info->SduDataPtr + 4) = (CanPCI->uiFrameLenght >> 8) & 0xFF;
                    *(CanPdu_Info->SduDataPtr + 5) = (CanPCI->uiFrameLenght >> 0) & 0xFF;
                    *(CanPdu_Info->SduDataPtr + 6) = *(puiCanPayload);
                    *(CanPdu_Info->SduDataPtr + 7) = *(puiCanPayload + 1);
                }
            break;
            case CAN_FC:
                *(CanPdu_Info->SduDataPtr) = 0;
                *(CanPdu_Info->SduDataPtr) = CAN_FC << 4;
                if(CanPCI->uiFlowStatus < 7){
                    *(CanPdu_Info->SduDataPtr) |= (0x0F & CanPCI->uiFlowStatus);
                    *(CanPdu_Info->SduDataPtr + 1) = CanPCI->uiBlockSize;
                    *(CanPdu_Info->SduDataPtr + 2) = CanPCI->uiSeparationTime;
                }
                else{
                    ret = E_NOT_OK;
                }
            break;
            default:
                ret = E_NOT_OK;
            break;
        }
    }
    else{
        ret = E_NOT_OK;
    }
    return ret;
}
/**
  @brief CanTp_SendSingleFrame
*/
static Std_ReturnType CanTp_SendSingleFrame(PduIdType id, uint8* puiPayload, uint32 uiSize){
    PduInfoType PduInfo;
    uint8 puiSduData[8];
    uint8 *puiMetaData;
    PduInfo.MetaDataPtr = puiMetaData;
    PduInfo.SduDataPtr = puiSduData;

    CanPCI_Type CanPCI = {CANTP_N_PCI_TYPE_SF, uiSize, 0, 0, 0, 0};
    Std_ReturnType ret = E_OK;
    ret = E_OK;
    CanTp_PrepareSegmenetedFrame(&CanPCI, &PduInfo, puiPayload);
    
    if(CanIf_Transmit(id, &PduInfo) == E_OK ){
        CanTp_TimerStart(&N_As_timer);
    }
    else{
        PduR_CanTpTxConfirmation(id, E_NOT_OK);
        ret = E_NOT_OK;
    }
    return ret;
}
/**
  @brief CanTp_SendFirstFrame
*/
static Std_ReturnType CanTp_SendFirstFrame(PduIdType id, uint32 uiMsgLen){
    PduInfoType PduInfo;
    uint8 puiSduData[8];
    uint8 *puiMetaData;
    PduInfo.MetaDataPtr = puiMetaData;
    PduInfo.SduDataPtr = puiSduData;

    CanPCI_Type CanPCI = {CANTP_N_PCI_TYPE_FF, uiMsgLen, 0, 0, 0, 0}; 
    uint8 uiPayload[8] = {0,0,0,0,0,0,0,0};
    Std_ReturnType ret = E_OK;

    CanTp_PrepareSegmenetedFrame(&CanPCI, &PduInfo, uiPayload);

    if(CanIf_Transmit(id, &PduInfo) == E_OK ){
        CanTp_TimerStart(&N_As_timer);
        CanTp_TimerStart(&N_Bs_timer);
    }
    else{
        PduR_CanTpTxConfirmation(id, E_NOT_OK);
        ret = E_NOT_OK;
    }
    return ret;
}
/**
  @brief CanTp_GetPCI
*/
static Std_ReturnType CanTp_GetPCI(const PduInfoType* CanData, CanPCI_Type* CanFrameInfo){
    Std_ReturnType ret = E_OK;

    if(NE_NULL_PTR(CanData) && NE_NULL_PTR(CanFrameInfo) && (NE_NULL_PTR(CanData->SduDataPtr))){
        CanFrameInfo->eFrameType = DEFAULT;
        CanFrameInfo->uiFrameLenght = 0;
        CanFrameInfo->uiBlockSize = 0;
        CanFrameInfo->uiFlowStatus = 0;
        CanFrameInfo->uiSeparationTime = 0;
        CanFrameInfo->uiSequenceNumber = 0;

        switch((CanData->SduDataPtr[0]) >> 4){
            case CANTP_N_PCI_TYPE_SF:
                CanFrameInfo->eFrameType = CAN_SF;
                CanFrameInfo->uiFrameLenght = CanData->SduDataPtr[0];
            break;
            case CANTP_N_PCI_TYPE_FF:
                CanFrameInfo->eFrameType = CAN_FF;
                if( (CanData->SduDataPtr[0] & 0x0F) | CanData->SduDataPtr[1] ) {
                    CanFrameInfo->uiFrameLenght =  CanData->SduDataPtr[0] & 0x0F;
                    CanFrameInfo->uiFrameLenght =  (CanFrameInfo->uiFrameLenght << 8) | CanData->SduDataPtr[1]; 
                }
                else{
                    CanFrameInfo->uiFrameLenght =  CanData->SduDataPtr[2];
                    CanFrameInfo->uiFrameLenght =  (CanFrameInfo->uiFrameLenght << 8) | CanData->SduDataPtr[3]; 
                    CanFrameInfo->uiFrameLenght =  (CanFrameInfo->uiFrameLenght << 8) | CanData->SduDataPtr[4];
                    CanFrameInfo->uiFrameLenght =  (CanFrameInfo->uiFrameLenght << 8) | CanData->SduDataPtr[5];
                }
            break;
            case CANTP_N_PCI_TYPE_CF:
                CanFrameInfo->eFrameType = CAN_CF;
                CanFrameInfo->uiSequenceNumber= (CanData->SduDataPtr[0] & 0x0F );
            break;
            case CANTP_N_PCI_TYPE_FC:
                CanFrameInfo->eFrameType = CAN_FC;
                CanFrameInfo->uiFlowStatus = CanData->SduDataPtr[0] & 0x0F; 
                CanFrameInfo->uiBlockSize = CanData->SduDataPtr[1]; 
                CanFrameInfo->uiSeparationTime = CanData->SduDataPtr[2]; 
            break;
            default:
                CanFrameInfo->eFrameType = DEFAULT;
                ret = E_NOT_OK;
            break;
        }
    }
    else{
        ret = E_NOT_OK;
    }
    return ret;
}
/**
  @brief CanTp_FirstFrameReception
*/
static void CanTp_FirstFrameReception(PduIdType RxPduId, const PduInfoType *PduInfoPtr, CanPCI_Type *CanPCI){  
    PduLengthType BufferSize; 
    BufReq_ReturnType BufferState; 
    uint16 uiCurrentBlockSize;
    BufferState = PduR_CanTpStartOfReception( RxPduId, PduInfoPtr, CanPCI->uiFrameLenght, &BufferSize);
    if(BufferState == BUFREQ_OK){
        CanTp_VariablesRX.uiMsgLen = CanPCI->uiFrameLenght;
        CanTp_VariablesRX.CanTp_Current_RxId = RxPduId;
        uiCurrentBlockSize = CanTp_CalcBlocksSize(BufferSize); 
        if( uiCurrentBlockSize > 0){    
            CanTp_SendFlowControl(CanTp_VariablesRX.CanTp_Current_RxId, uiCurrentBlockSize, FC_CTS, DEFAULT_ST);   
            CanTp_VariablesRX.uiBlocksNxtCts = uiCurrentBlockSize;
            CanTp_VariablesRX.eCanTp_StateRX = CANTP_RX_PROCESSING;
        }
        else{
            CanTp_VariablesRX.uiBlocksNxtCts = uiCurrentBlockSize;
            CanTp_SendFlowControl(CanTp_VariablesRX.CanTp_Current_RxId, uiCurrentBlockSize, FC_WAIT, DEFAULT_ST );   
            CanTp_VariablesRX.eCanTp_StateRX = CANTP_RX_PROCESSING_SUSPEND;
        }
        CanTp_VariablesRX.uiExpected_CF_SN = 1; 
    } 
    else if (BufferState == BUFREQ_OVFL){
        CanTp_SendFlowControl(CanTp_VariablesRX.CanTp_Current_RxId, uiCurrentBlockSize, FC_OVFLW, DEFAULT_ST );
        CanTp_ResetRX();
        CanTp_VariablesRX.eCanTp_StateRX = CANTP_RX_WAIT;
    }
    else {
        CanTp_ResetRX();
    }
}
/**
  @brief CanTp_SingleFrameReception
*/
static void CanTp_SingleFrameReception(PduIdType RxPduId, CanPCI_Type *CanPCI, const PduInfoType* PduInfoPtr){
    PduLengthType BufferSize; 
    BufReq_ReturnType BufferState;  
    PduInfoType ExtractedData;
    CanTp_VariablesRX.eCanTp_StateRX = CANTP_RX_WAIT;
    BufferState = PduR_CanTpStartOfReception( RxPduId, PduInfoPtr, CanPCI->uiFrameLenght, &BufferSize);

    if((BufferState == BUFREQ_OK)){             
        if(BufferSize >= CanPCI->uiFrameLenght){
            ExtractedData.SduLength = CanPCI->uiFrameLenght;
            ExtractedData.SduDataPtr = (PduInfoPtr->SduDataPtr+1);
            BufferState = PduR_CanTpCopyRxData(RxPduId,  &ExtractedData, &BufferSize);
            PduR_CanTpRxIndication(RxPduId, BufferState);        
        }
        else{
            PduR_CanTpRxIndication(RxPduId, E_NOT_OK);
        }
    }
    else{}
}
/**
  @brief CanTp_ConsecutiveFrameReception
*/
static void CanTp_ConsecutiveFrameReception(PduIdType RxPduId, CanPCI_Type *CanPCI, const PduInfoType* PduInfoPtr){
    PduLengthType BufferSize;      
    BufReq_ReturnType BufferState;   
    PduInfoType ExtractedData;
    uint16 uiCurrentBlockSize;

    CanTp_TimerReset(&N_Cr_timer);

    if(CanTp_VariablesRX.CanTp_Current_RxId ==  RxPduId){
        if(CanTp_VariablesRX.uiExpected_CF_SN == CanPCI->uiSequenceNumber){
            ExtractedData.SduLength = CanPCI->uiFrameLenght;
            ExtractedData.SduDataPtr = (PduInfoPtr->SduDataPtr+1);
            BufferState = PduR_CanTpCopyRxData(RxPduId,  &ExtractedData, &BufferSize);
            if(BufferState == BUFREQ_OK){
                CanTp_VariablesRX.uiTransmittedBytes += PduInfoPtr->SduLength;
                CanTp_VariablesRX.uiBlocksNxtCts--;
                if( CanTp_VariablesRX.uiTransmittedBytes == CanTp_VariablesRX.uiMsgLen){
                    PduR_CanTpRxIndication(CanTp_VariablesRX.CanTp_Current_RxId, E_OK);
                    CanTp_ResetRX();
                }     
                else{ 
                    CanTp_VariablesRX.uiExpected_CF_SN++;
                    CanTp_VariablesRX.uiExpected_CF_SN%8;
                    uiCurrentBlockSize = CanTp_CalcBlocksSize(BufferSize);
                    if(uiCurrentBlockSize > 0){
                        CanTp_TimerStart(&N_Cr_timer);
                        if(CanTp_VariablesRX.uiBlocksNxtCts == 0 ){
                            CanTp_SendFlowControl(CanTp_VariablesRX.CanTp_Current_RxId, uiCurrentBlockSize, FC_CTS, DEFAULT_ST );
                            CanTp_VariablesRX.uiBlocksNxtCts = uiCurrentBlockSize;
                        }
                        CanTp_VariablesRX.eCanTp_StateRX = CANTP_RX_PROCESSING;         
                    }
                    else{
                        CanTp_VariablesRX.eCanTp_StateRX = CANTP_RX_PROCESSING_SUSPEND;
                        CanTp_SendFlowControl(CanTp_VariablesRX.CanTp_Current_RxId, uiCurrentBlockSize, FC_WAIT, DEFAULT_ST ); 
                    }
                }
            }
            else{ 
                PduR_CanTpRxIndication(CanTp_VariablesRX.CanTp_Current_RxId, E_NOT_OK);
                CanTp_ResetRX();
            }
        }
        else{ 
            PduR_CanTpRxIndication(CanTp_VariablesRX.CanTp_Current_RxId, E_NOT_OK);
            CanTp_ResetRX();
        }
    }
    else {
        PduR_CanTpRxIndication(CanTp_VariablesRX.CanTp_Current_RxId, E_NOT_OK);
        CanTp_ResetRX();
    }
}
/**
  @brief CanTp_FlowControlReception
*/
static void CanTp_FlowControlReception(PduIdType RxPduId, CanPCI_Type *CanPCI){
    if( CanTp_VariablesTX.eCanTp_StateTX == CANTP_TX_PROCESSING_SUSPENDED ){
        if(CanTp_VariablesTX.CanTp_Current_TxId == RxPduId ){
            if(CanPCI->uiFlowStatus == FC_CTS){
                CanTp_VariablesTX.uiFrameNrFC = CanPCI->uiBlockSize; 
                CanTp_SendNextCF();
            }   
            else if(CanPCI->uiFlowStatus == FC_WAIT){
                CanTp_TimerReset(&N_Bs_timer);
                CanTp_TimerStart(&N_Bs_timer);
            }
            else if(CanPCI->uiFlowStatus == FC_OVFLW){
                PduR_CanTpTxConfirmation(CanTp_VariablesTX.CanTp_Current_TxId, E_NOT_OK);
                CanTp_ResetTX();
            }
            else{
                PduR_CanTpTxConfirmation(CanTp_VariablesTX.CanTp_Current_TxId, E_NOT_OK);
                CanTp_ResetTX();
            }
        }
        else{}
    }
    else{}
}
/**
  @brief CanTp_CalcBlocksSize
*/
static uint16 CanTp_CalcBlocksSize(uint16 uiBufferSize){
    uint16 ret; 
    uint16 uiBytesLeft = CanTp_VariablesRX.uiMsgLen - CanTp_VariablesRX.uiTransmittedBytes;
    if(uiBufferSize >= uiBytesLeft){
        ret = uiBytesLeft/7;
        if(CanTp_VariablesRX.uiMsgLen%7 > 0) ret++; 
    }
    else{
        ret = uiBufferSize/7;
    }
    return ret;
} 
/**
  @brief CanTp_SendNextCF
*/
static void CanTp_SendNextCF(void){
    BufReq_ReturnType BufReqState;
    PduInfoType PduInfoPtr;
    PduLengthType PduLen;
    Std_ReturnType ret;
    uint8 uiBytesToSend;
    uint8_t uiPayload[8];

    PduInfoPtr.SduDataPtr = uiPayload;
    PduInfoPtr.MetaDataPtr = NULL;
    
    if(CanTp_VariablesTX.uiTransmittedBytes == CanTp_VariablesTX.uiMsgLen){
        PduR_CanTpTxConfirmation(CanTp_VariablesTX.CanTp_Current_TxId, E_OK);
        CanTp_ResetTX();
    }
    else{
        if(CanTp_VariablesTX.uiMsgLen - CanTp_VariablesTX.uiTransmittedBytes < 7) uiBytesToSend = CanTp_VariablesTX.uiMsgLen - CanTp_VariablesTX.uiTransmittedBytes;
        else uiBytesToSend = 7;
        PduInfoPtr.SduLength = uiBytesToSend;
        BufReqState = PduR_CanTpCopyTxData(CanTp_VariablesTX.CanTp_Current_TxId, &PduInfoPtr, NULL, &PduLen);
        if(BufReqState == BUFREQ_OK){
            ret = CanTp_SendConsecutiveFrame(CanTp_VariablesTX.CanTp_Current_TxId, CanTp_VariablesTX.uiNxtSN, PduInfoPtr.SduDataPtr, uiBytesToSend);
            if( ret == E_OK ){
                CanTp_VariablesTX.uiTransmittedBytes = CanTp_VariablesTX.uiTransmittedBytes + uiBytesToSend;
                CanTp_VariablesTX.uiFrameNrFC--;
                CanTp_VariablesTX.uiNxtSN = (CanTp_VariablesTX.uiNxtSN + 1)%8;
                if((CanTp_VariablesTX.uiFrameNrFC == 0) && (CanTp_VariablesTX.uiTransmittedBytes != CanTp_VariablesTX.uiMsgLen))CanTp_VariablesTX.eCanTp_StateTX = CANTP_TX_PROCESSING_SUSPENDED;
                else CanTp_VariablesTX.eCanTp_StateTX = CANTP_TX_PROCESSING;        
            }
            else{
                CanTp_ResetTX();
            }
        }
        else if(BufReqState == BUFREQ_E_NOT_OK){
            PduR_CanTpTxConfirmation(CanTp_VariablesTX.CanTp_Current_TxId, E_NOT_OK);
            CanTp_ResetTX();         
        }
        else {
            CanTp_TimerStart(&N_Cs_timer);
            CanTp_VariablesTX.eCanTp_StateTX = CANTP_TX_PROCESSING_SUSPENDED;
        }
    }
}

/**
  @brief CanTp_SendConsecutiveFrame
*/
static Std_ReturnType CanTp_SendConsecutiveFrame(PduIdType id, uint8 uiSequenceNumber, uint8* uiPayload, uint32 uiSize){
    PduInfoType PduInfo;
    uint8 puiSduData[8];
    uint8 *puiMetaData;
    PduInfo.MetaDataPtr = puiMetaData;
    PduInfo.SduDataPtr = puiSduData;
    PduInfo.SduLength = uiSize;
    CanPCI_Type CanPCI;

    CanPCI.eFrameType = CAN_CF;
    CanPCI.uiSequenceNumber = uiSequenceNumber;

    Std_ReturnType ret = E_OK;
    CanTp_PrepareSegmenetedFrame(&CanPCI, &PduInfo, uiPayload);
    if(CanIf_Transmit(id , &PduInfo) == E_OK){
        CanTp_TimerStart(&N_As_timer);
        CanTp_TimerStart(&N_Bs_timer);
    }
    else{
        PduR_CanTpTxConfirmation(id, E_NOT_OK);
        ret = E_NOT_OK;
    }
    return ret;
}

/**
  @brief CanTp_SendFlowControl
*/
static Std_ReturnType CanTp_SendFlowControl(PduIdType ID, uint8 uiBlockSize, FlowControlStatus_type FC_Status, uint8 uiSeparationTime ){
    Std_ReturnType ret = E_OK;
    PduInfoType PduInfoPtr;
    CanPCI_Type CanPCI;
    uint8 uiPayload[8];
    uint8 *puiMetaData;
    uint8 puiSduData[8];
    CanPCI.eFrameType = CAN_FC;
    CanPCI.uiFlowStatus = FC_Status;
    CanPCI.uiBlockSize = uiBlockSize;
    CanPCI.uiSeparationTime = uiSeparationTime;
    PduInfoPtr.SduLength = 0;
    PduInfoPtr.MetaDataPtr = puiMetaData;
    PduInfoPtr.SduDataPtr = puiSduData;

   if(( FC_Status == FC_OVFLW )||
      ( FC_Status == FC_WAIT  )||
      ( FC_Status == FC_CTS   ))
    {
       CanTp_PrepareSegmenetedFrame(&CanPCI, &PduInfoPtr, uiPayload);
        ret = CanIf_Transmit(ID, &PduInfoPtr);
        if( ret == E_NOT_OK ){
            CanTp_ResetRX();
            PduR_CanTpRxIndication(ID, E_NOT_OK);
        }
        else{
            CanTp_TimerStart(&N_Ar_timer);
            if(FC_Status == FC_CTS){
                CanTp_TimerStart(&N_Cr_timer);
            }
            else if(FC_Status == FC_WAIT){
                CanTp_TimerStart(&N_Br_timer);
            }
        } 
    }
    else{
       ret = E_NOT_OK; 
    }
    return ret;
}


/*====================================================================================================================*\
    Definicje funkcji Timera
\*====================================================================================================================*/

/*====================================================================================================================*/

/**
  @brief CanTp_TimerStart
*/
void CanTp_TimerStart(CanTp_Timer_type *pTimer){
    pTimer->eState = TIMER_ENABLE;
}

/**
  @brief CanTp_TimerReset
*/
void CanTp_TimerReset(CanTp_Timer_type *pTimer){
    pTimer->eState = TIMER_DISABLE;
    pTimer->uiCounter = 0;
}

/**
  @brief CanTp_TimerTick
*/
Std_ReturnType CanTp_TimerTick(CanTp_Timer_type *pTimer){
    Std_ReturnType ret = E_OK;   
    if(pTimer->eState == TIMER_ENABLE){
        if(pTimer->uiCounter < UINT32_MAX){
            pTimer->uiCounter++;
        }
        else{
            ret = E_NOT_OK;
        }
    }
    return ret;
}

/**
  @brief CanTp_TimerTimeout
*/
Std_ReturnType CanTp_TimerTimeout(const CanTp_Timer_type *pTimer){
    if(pTimer->uiCounter >= pTimer->uiTimeout){
        return E_NOT_OK;
    }
    else{
        return E_OK;
    }
}
