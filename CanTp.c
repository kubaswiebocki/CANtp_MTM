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
    Makra lokalne
\*====================================================================================================================*/


/*====================================================================================================================*\
    Typy lokalne
\*====================================================================================================================*/
typedef enum {
    CANTP_ON, 
    CANTP_OFF
}CanTpState_type;

typedef enum {
    CANTP_RX_WAIT,
    CANTP_RX_PROCESSING,
    CANTP_RX_PROCESSING_SUSPEND
} CanTpStateRX_type;

typedef enum {
    CANTP_TX_WAIT,                     
    CANTP_TX_PROCESSING,                
    CANTP_TX_PROCESSING_SUSPENDED
} CanTpStateTX_type;

typedef struct{
    uint16 message_length; 
    uint16 expected_CF_SN;              
    uint16 sended_bytes;                
    CanTpStateRX_type CanTp_StateRX;   
    PduIdType CanTp_Current_RxId;      
    uint8 blocks_to_next_cts; 
} CanTp_VariablesRX_type;

typedef struct{
    CanTpStateTX_type CanTp_StateTX;
    PduIdType CanTp_Current_TxId;
    uint16 sent_bytes;
    uint16 message_legth;
    uint16 frame_nr_FC;  
    uint8_t next_SN;
} CanTp_VariablesTX_type;

typedef enum {
    SF = 0, // CANTP_N_PCI_TYPE_SF
    FF = 1, // CANTP_N_PCI_TYPE_FF
    CF = 2, // CANTP_N_PCI_TYPE_CF
    FC = 3, // CANTP_N_PCI_TYPE_FC
    DEFAULT = 4
} frame_type_t;

typedef struct{
    frame_type_t frame_type;
    uint32 frame_lenght; 
    uint8 SN; // Sequence Nubmer
    uint8 BS; // Block Size
    uint8 FS; // Flow Status
    uint8 ST; // Separation Time
} CanPCI_Type;
/*====================================================================================================================*\
    Zmienne globalne
\*====================================================================================================================*/
CanTpState_type CanTp_State; 

CanTp_VariablesTX_type CanTp_VariablesTX;

CanTp_VariablesRX_type CanTp_VariablesRX;



/*====================================================================================================================*\
    Deklaracje funkcji lokalnych
\*====================================================================================================================*/
static void CanTp_ResetRX(void);
static void CanTp_ResetTX(void);

static Std_ReturnType CanTp_PrepareSegmenetedFrame(CanPCI_Type *CanPCI, PduInfoType *CanPdu_Info, uint8_t *Can_payload);

static Std_ReturnType CanTp_SendSingleFrame(PduIdType id, uint8* payload, uint32 size);
static Std_ReturnType CanTp_SendFirstFrame(PduIdType id, uint32 message_lenght);

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

/**
  @brief CanTp_Shutdown

  Funkcja wywoływana w celu wyłączenia modułu CanTp. [SWS_CANTP_00211]

  Wypełnia:
   [SWS_CANTP_00202]
   [SWS_CANTP_00200]
*/
void CanTp_Shutdown(void){
    /* Wypełnia [SWS_CANTP_00202]*/
    CanTp_ResetRX();
    CanTp_ResetTX();
    /* Wypełnia [SWS_CANTP_00200]*/
    CanTp_State = CANTP_OFF;
}

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

Std_ReturnType CanTp_Transmit(PduIdType TxPduId, const PduInfoType* PduInfoPtr){

    BufReq_ReturnType BufReq_State;
    PduLengthType PDU_Len;
    Std_ReturnType ret = E_OK;
    
    PduInfoType Temp_PDU;
    uint8_t payload[8];
    Temp_PDU.SduDataPtr = payload;
    Temp_PDU.MetaDataPtr = NULL;
    
    if( CanTp_State == CANTP_ON ){
        if( CanTp_VariablesTX.CanTp_StateTX == CANTP_TX_WAIT){
            if(PduInfoPtr->SduLength < 8){
                Temp_PDU.SduLength = PduInfoPtr->SduLength;
                BufReq_State = PduR_CanTpCopyTxData(TxPduId, &Temp_PDU, NULL, &PDU_Len);
                if(BufReq_State == BUFREQ_OK){
                     /* Wypełnia [SWS_CANTP_00231]*/
                     /* Wypełnia [SWS_CANTP_00354]*/
                    ret = CanTp_SendSingleFrame(TxPduId, Temp_PDU.SduDataPtr, PduInfoPtr->SduLength );
                }
                /* Wypełnia [SWS_CANTP_00298]*/
                else if(BufReq_State == BUFREQ_E_NOT_OK){
                    CanTp_ResetTX();  
                    /* Wypełnia [SWS_CANTP_00205]*/
                    PduR_CanTpTxConfirmation(TxPduId, E_NOT_OK);
                    ret = E_NOT_OK;
                }
                else {
                    /* Wypełnia [SWS_CANTP_00204]*/
                    // PduR_CanTpTxConfirmation(TxPduId, E_OK);
                    ret = E_OK;
                }
            }
            else{
                if(CanTp_SendFirstFrame(TxPduId, PduInfoPtr->SduLength) == E_OK){
                    CanTp_VariablesTX.CanTp_StateTX = TxPduId;
                    CanTp_VariablesTX.CanTp_StateTX = CANTP_TX_PROCESSING_SUSPENDED;
                    CanTp_VariablesTX.message_legth = PduInfoPtr->SduLength;
                    CanTp_VariablesTX.sent_bytes = 0;
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



/*====================================================================================================================*\
    Definicja funkcji lokalnych
\*====================================================================================================================*/

/*====================================================================================================================*/
static void CanTp_ResetRX(void){
    CanTp_VariablesRX.CanTp_StateRX = CANTP_RX_WAIT;
    CanTp_VariablesRX.expected_CF_SN = 0;
    CanTp_VariablesRX.message_length = 0;
    CanTp_VariablesRX.sended_bytes = 0;
    CanTp_VariablesRX.blocks_to_next_cts = 0;
    CanTp_VariablesRX.CanTp_Current_RxId = 0;
}

static void CanTp_ResetTX(void){
    CanTp_VariablesTX.sent_bytes = 0;
    CanTp_VariablesTX.message_legth = 0;
    CanTp_VariablesTX.CanTp_StateTX = CANTP_TX_WAIT;
    CanTp_VariablesTX.frame_nr_FC = 0;
    CanTp_VariablesTX.CanTp_Current_TxId = 0;
}

static Std_ReturnType CanTp_PrepareSegmenetedFrame(CanPCI_Type *CanPCI, PduInfoType *CanPdu_Info, uint8_t *Can_payload){

    Std_ReturnType ret = E_OK;

    if(NE_NULL_PTR(CanPCI) && NE_NULL_PTR(CanPdu_Info) && NE_NULL_PTR(Can_payload)){  
        
        switch(CanPCI->frame_type){
            case SF:
                *(CanPdu_Info->SduDataPtr) = 0;
                *(CanPdu_Info->SduDataPtr) = CANTP_N_PCI_TYPE_SF << 4;
        
                if(CanPCI->frame_lenght <= 7){
                    *(CanPdu_Info->SduDataPtr) = 0x0F & CanPCI->frame_lenght; 
                    for(uint8_t i = 0; i < CanPCI->frame_lenght; i++){
                        *(CanPdu_Info->SduDataPtr + (i + 1)) = *(Can_payload + i);
                    }  
                }
                else{
                    ret = E_NOT_OK;
                }
            break;
            case CF: 
                *(CanPdu_Info->SduDataPtr) = 0;
                *(CanPdu_Info->SduDataPtr) = CANTP_N_PCI_TYPE_CF << 4;
                if(CanPCI->SN < 7){
                    *(CanPdu_Info->SduDataPtr) |= (0x0F & CanPCI->SN);
                    *(CanPdu_Info->SduDataPtr + 1) = *(Can_payload);
                    *(CanPdu_Info->SduDataPtr + 2) = *(Can_payload + 1);
                    *(CanPdu_Info->SduDataPtr + 3) = *(Can_payload + 2);
                    *(CanPdu_Info->SduDataPtr + 4) = *(Can_payload + 3);
                    *(CanPdu_Info->SduDataPtr + 5) = *(Can_payload + 4);
                    *(CanPdu_Info->SduDataPtr + 6) = *(Can_payload + 5);
                    *(CanPdu_Info->SduDataPtr + 7) = *(Can_payload + 6);
                }
                else{
                    ret = E_NOT_OK; 
                }

            break;
            case FF:    
                *(CanPdu_Info->SduDataPtr) = 0;
                *(CanPdu_Info->SduDataPtr) = CANTP_N_PCI_TYPE_FF << 4;

                if(CanPCI->frame_lenght <= 4095){
                    *(CanPdu_Info->SduDataPtr) |= (0x0F & (CanPCI->frame_lenght >> 8));
                    *(CanPdu_Info->SduDataPtr + 1) = (0xFF & (CanPCI->frame_lenght));

                    *(CanPdu_Info->SduDataPtr + 2) = *(Can_payload);
                    *(CanPdu_Info->SduDataPtr + 3) = *(Can_payload + 1);
                    *(CanPdu_Info->SduDataPtr + 4) = *(Can_payload + 2);
                    *(CanPdu_Info->SduDataPtr + 5) = *(Can_payload + 3);
                    *(CanPdu_Info->SduDataPtr + 6) = *(Can_payload + 4);
                    *(CanPdu_Info->SduDataPtr + 7) = *(Can_payload + 5);
                }
                else{
                    *(CanPdu_Info->SduDataPtr + 1) = 0;

                    *(CanPdu_Info->SduDataPtr + 2) = (CanPCI->frame_lenght >> 24) & 0xFF;
                    *(CanPdu_Info->SduDataPtr + 3) = (CanPCI->frame_lenght >> 16) & 0xFF;
                    *(CanPdu_Info->SduDataPtr + 4) = (CanPCI->frame_lenght >> 8) & 0xFF;
                    *(CanPdu_Info->SduDataPtr + 5) = (CanPCI->frame_lenght >> 0) & 0xFF;

                    *(CanPdu_Info->SduDataPtr + 6) = *(Can_payload);
                    *(CanPdu_Info->SduDataPtr + 7) = *(Can_payload + 1);
                }
            break;
            case FC:
                *(CanPdu_Info->SduDataPtr) = 0;
                *(CanPdu_Info->SduDataPtr) = CANTP_N_PCI_TYPE_FC << 4;

                if(CanPCI->FS < 7){
                    *(CanPdu_Info->SduDataPtr) |= (0x0F & CanPCI->FS);
                    *(CanPdu_Info->SduDataPtr + 1) = CanPCI->BS;
                    *(CanPdu_Info->SduDataPtr + 2) = CanPCI->ST;
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

static Std_ReturnType CanTp_SendSingleFrame(PduIdType id, uint8* payload, uint32 size){
    PduInfoType PduInfo;
    uint8 SduDataPtr[8];
    uint8 *MetaDataPtr;
    PduInfo.MetaDataPtr = MetaDataPtr;
    PduInfo.SduDataPtr = SduDataPtr;

    CanPCI_Type CanPCI = {CANTP_N_PCI_TYPE_SF, size, 0, 0, 0, 0};
    Std_ReturnType ret = E_OK;
    ret = E_OK;
    CanTp_PrepareSegmenetedFrame(&CanPCI, &PduInfo, payload);
    
    if(CanIf_Transmit(id , &PduInfo) == E_OK ){
    }
    else{
        PduR_CanTpTxConfirmation(id, E_NOT_OK);
        ret = E_NOT_OK;
    }
    return ret;
}

static Std_ReturnType CanTp_SendFirstFrame(PduIdType id, uint32 message_lenght){
    PduInfoType PduInfo;
    uint8 SduDataPtr[8];
    uint8 *MetaDataPtr;
    PduInfo.MetaDataPtr = MetaDataPtr;
    PduInfo.SduDataPtr = SduDataPtr;

    CanPCI_Type CanPCI = {CANTP_N_PCI_TYPE_FF, message_lenght, 0, 0, 0, 0}; 
    uint8 payload[8] = {0,0,0,0,0,0,0,0};
    Std_ReturnType ret = E_OK;

    CanTp_PrepareSegmenetedFrame(&CanPCI, &PduInfo, payload);

    if(CanIf_Transmit(id, &PduInfo) == E_OK ){}
    else{
        PduR_CanTpTxConfirmation(id, E_NOT_OK);
        ret = E_NOT_OK;
    }
    return ret;
}

