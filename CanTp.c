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

typedef enum{
    FC_OVFLW = 0,
    FC_WAIT = 1,
    FC_CTS = 2
} FlowControlStatus_type;

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

static Std_ReturnType CanTp_GetPCI(const PduInfoType* can_data, CanPCI_Type* CanFrameInfo);
static Std_ReturnType CanTp_PrepareSegmenetedFrame(CanPCI_Type *CanPCI, PduInfoType *CanPdu_Info, uint8_t *Can_payload);

static Std_ReturnType CanTp_SendSingleFrame(PduIdType id, uint8* payload, uint32 size);
static Std_ReturnType CanTp_SendFirstFrame(PduIdType id, uint32 message_lenght);

static uint16 CanTp_Calculate_Available_Blocks(uint16 buffer_size);

static void CanTp_FirstFrameReception(PduIdType RxPduId, const PduInfoType *PduInfoPtr, CanPCI_Type *Can_PCI);
static void CanTp_SingleFrameReception(PduIdType RxPduId, CanPCI_Type *Can_PCI, const PduInfoType* PduInfoPtr);
static void CanTp_ConsecutiveFrameReception(PduIdType RxPduId, CanPCI_Type *Can_PCI, const PduInfoType* PduInfoPtr);
static void CanTp_FlowControlReception(PduIdType RxPduId, CanPCI_Type *Can_PCI);

static Std_ReturnType CanTp_SendConsecutiveFrame(PduIdType id, uint8 SN, uint8* payload, uint32 size);
static Std_ReturnType CanTp_SendFlowControl(PduIdType ID, uint8 BlockSize, FlowControlStatus_type FC_Status, uint8 SeparationTime );
static void CanTp_SendNextCF(void);
static void CanTp_set_blocks_to_next_cts(uint8 blocks);
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
  @brief CanTp_RxIndication

  Wskazanie odebranej jednostki PDU [SWS_CANTP_00214]
  Wypełnia:
   [SWS_CANTP_00235]
   [SWS_CANTP_00322]
*/

void CanTp_RxIndication(PduIdType RxPduId, const PduInfoType* PduInfoPtr){

    CanPCI_Type Can_PCI;
    PduInfoType Extracted_Data;   
    uint8 temp_data[8];           

    if(CanTp_State == CANTP_ON){
        if( CanTp_VariablesRX.CanTp_StateRX == CANTP_RX_WAIT){
        
            CanTp_GetPCI(PduInfoPtr, &Can_PCI);

            if( Can_PCI.frame_type == FF ){
                CanTp_FirstFrameReception(RxPduId, PduInfoPtr, &Can_PCI);
            }
            else if( Can_PCI.frame_type == SF ){
                CanTp_SingleFrameReception(RxPduId, &Can_PCI, PduInfoPtr);           
            } 
            else if( Can_PCI.frame_type == FC ){
                CanTp_FlowControlReception(RxPduId, &Can_PCI);
            }
            else
            {
                CanTp_VariablesRX.CanTp_StateRX = CANTP_RX_WAIT; 
            } 
        }
        else if(CanTp_VariablesRX.CanTp_StateRX == CANTP_RX_PROCESSING){
             CanTp_GetPCI(PduInfoPtr, &Can_PCI);
             if(Can_PCI.frame_type == CF){
                CanTp_ConsecutiveFrameReception(RxPduId, &Can_PCI, PduInfoPtr);
            }
            else if(Can_PCI.frame_type == FC){
                CanTp_FlowControlReception(RxPduId, &Can_PCI);
            }
            else if(Can_PCI.frame_type == FF){            
                PduR_CanTpRxIndication (CanTp_VariablesRX.CanTp_Current_RxId, E_NOT_OK);
                CanTp_ResetRX();
                CanTp_FirstFrameReception(RxPduId, PduInfoPtr, &Can_PCI);
            }
            else if(Can_PCI.frame_type == SF){            
                PduR_CanTpRxIndication (CanTp_VariablesRX.CanTp_Current_RxId, E_NOT_OK);
                CanTp_ResetRX();
                CanTp_SingleFrameReception(RxPduId, &Can_PCI, PduInfoPtr);
            }
            else{
            }
        }
        else {    
            CanTp_GetPCI(PduInfoPtr, &Can_PCI);
            if(Can_PCI.frame_type == FC){
                CanTp_FlowControlReception(RxPduId, &Can_PCI);
            }
            else if(Can_PCI.frame_type == FF) {            
                PduR_CanTpRxIndication (CanTp_VariablesRX.CanTp_Current_RxId, E_NOT_OK);
                CanTp_ResetRX();
                CanTp_FirstFrameReception(RxPduId, PduInfoPtr, &Can_PCI);
            }
            else if(Can_PCI.frame_type == SF) {            
                PduR_CanTpRxIndication (CanTp_VariablesRX.CanTp_Current_RxId, E_NOT_OK);
                CanTp_ResetRX();
                CanTp_SingleFrameReception(RxPduId, &Can_PCI, PduInfoPtr);
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
void CanTp_TxConfirmation (PduIdType TxPduId, Std_ReturnType result){
if( CanTp_State == CANTP_ON ){  
    if(CanTp_VariablesRX.CanTp_Current_RxId == TxPduId){
        if( (CanTp_VariablesRX.CanTp_StateRX == CANTP_RX_PROCESSING ) || (CanTp_VariablesRX.CanTp_StateRX == CANTP_RX_PROCESSING_SUSPEND)){
            if(result == E_OK){}    
            else{
                PduR_CanTpRxIndication(CanTp_VariablesRX.CanTp_Current_RxId, E_NOT_OK);
                CanTp_ResetRX();
            }
        }
        else{} 
    }
    if(CanTp_VariablesTX.CanTp_Current_TxId == TxPduId ){
        if(result == E_OK){
            if(CanTp_VariablesTX.CanTp_StateTX == CANTP_TX_PROCESSING)
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
                *(CanPdu_Info->SduDataPtr) = SF << 4;
        
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
                *(CanPdu_Info->SduDataPtr) = CF << 4;
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
                *(CanPdu_Info->SduDataPtr) = FF << 4;

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
                *(CanPdu_Info->SduDataPtr) = FC << 4;

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

static Std_ReturnType CanTp_GetPCI(const PduInfoType* can_data, CanPCI_Type* CanFrameInfo){
    Std_ReturnType ret = E_OK;

    if(NE_NULL_PTR(can_data) && NE_NULL_PTR(CanFrameInfo) && (NE_NULL_PTR(can_data->SduDataPtr))){

        CanFrameInfo->frame_type = DEFAULT;
        CanFrameInfo->frame_lenght = 0;
        CanFrameInfo->BS = 0;
        CanFrameInfo->FS = 0;
        CanFrameInfo->ST = 0;
        CanFrameInfo->SN = 0;

        switch((can_data->SduDataPtr[0]) >> 4){
            case CANTP_N_PCI_TYPE_SF:
                CanFrameInfo->frame_type = SF;
                CanFrameInfo->frame_lenght = can_data->SduDataPtr[0];
            break;

            case CANTP_N_PCI_TYPE_FF:
                CanFrameInfo->frame_type = FF;
                if( (can_data->SduDataPtr[0] & 0x0F) | can_data->SduDataPtr[1] ) {
                    CanFrameInfo->frame_lenght =  can_data->SduDataPtr[0] & 0x0F;
                    CanFrameInfo->frame_lenght =  (CanFrameInfo->frame_lenght << 8) | can_data->SduDataPtr[1]; 
                }
                else{
                    CanFrameInfo->frame_lenght =  can_data->SduDataPtr[2];
                    CanFrameInfo->frame_lenght =  (CanFrameInfo->frame_lenght << 8) | can_data->SduDataPtr[3]; 
                    CanFrameInfo->frame_lenght =  (CanFrameInfo->frame_lenght << 8) | can_data->SduDataPtr[4];
                    CanFrameInfo->frame_lenght =  (CanFrameInfo->frame_lenght << 8) | can_data->SduDataPtr[5];
                }
            break;

            case CANTP_N_PCI_TYPE_CF:
                CanFrameInfo->frame_type = CF;
                CanFrameInfo->SN= (can_data->SduDataPtr[0] & 0x0F );
            break;

            case CANTP_N_PCI_TYPE_FC:
                CanFrameInfo->frame_type = FC;
                CanFrameInfo->FS = can_data->SduDataPtr[0] & 0x0F; 
                CanFrameInfo->BS = can_data->SduDataPtr[1]; 
                CanFrameInfo->ST = can_data->SduDataPtr[2]; 
            break;

            default:
                CanFrameInfo->frame_type = DEFAULT;
                ret = E_NOT_OK;
            break;
        }
    }
    else{
        ret = E_NOT_OK;
    }
    return ret;
}

static void CanTp_FirstFrameReception(PduIdType RxPduId, const PduInfoType *PduInfoPtr, CanPCI_Type *Can_PCI){
         
    PduLengthType buffer_size; 
    BufReq_ReturnType Buf_State; 
    uint16 current_block_size;
    Buf_State = PduR_CanTpStartOfReception( RxPduId, PduInfoPtr, Can_PCI->frame_lenght, &buffer_size);
    if(Buf_State == BUFREQ_OK){
        CanTp_VariablesRX.message_length = Can_PCI->frame_lenght;
        CanTp_VariablesRX.CanTp_Current_RxId = RxPduId;
        current_block_size = CanTp_Calculate_Available_Blocks( buffer_size ); 
        if( current_block_size > 0){    
            CanTp_SendFlowControl(CanTp_VariablesRX.CanTp_Current_RxId, current_block_size, FC_CTS, DEFAULT_ST);   
            CanTp_set_blocks_to_next_cts( current_block_size );
            CanTp_VariablesRX.CanTp_StateRX = CANTP_RX_PROCESSING;
        }
        else{
            CanTp_set_blocks_to_next_cts( current_block_size );
            CanTp_SendFlowControl(CanTp_VariablesRX.CanTp_Current_RxId, current_block_size, FC_WAIT, DEFAULT_ST );   
            CanTp_VariablesRX.CanTp_StateRX = CANTP_RX_PROCESSING_SUSPEND;
           
        }
        CanTp_VariablesRX.expected_CF_SN = 1; 
    } 
    else if ( Buf_State == BUFREQ_OVFL ){
        CanTp_SendFlowControl(CanTp_VariablesRX.CanTp_Current_RxId, current_block_size, FC_OVFLW, DEFAULT_ST );
        CanTp_ResetRX();
        CanTp_VariablesRX.CanTp_StateRX = CANTP_RX_WAIT;
    }
    else {
        CanTp_ResetRX();
    }
}

static void CanTp_SingleFrameReception(PduIdType RxPduId, CanPCI_Type *Can_PCI, const PduInfoType* PduInfoPtr){
    PduLengthType buffer_size;     
    BufReq_ReturnType Buf_State;  
    PduInfoType Extracted_Data;

    CanTp_VariablesRX.CanTp_StateRX = CANTP_RX_WAIT;
    Buf_State = PduR_CanTpStartOfReception( RxPduId, PduInfoPtr, Can_PCI->frame_lenght, &buffer_size);

    if((Buf_State == BUFREQ_OK)){             
        if(buffer_size >= Can_PCI->frame_lenght){
            Extracted_Data.SduLength = Can_PCI->frame_lenght;
            Extracted_Data.SduDataPtr = (PduInfoPtr->SduDataPtr+1);
            Buf_State = PduR_CanTpCopyRxData(RxPduId,  &Extracted_Data, &buffer_size);
            PduR_CanTpRxIndication(RxPduId, Buf_State);        
        }
        else{
            PduR_CanTpRxIndication(RxPduId, E_NOT_OK);
        }
    }
    else{}
}

static void CanTp_ConsecutiveFrameReception(PduIdType RxPduId, CanPCI_Type *Can_PCI, const PduInfoType* PduInfoPtr){
    PduLengthType buffer_size;      
    BufReq_ReturnType Buf_State;   
    PduInfoType Extracted_Data;
    uint16 current_block_size;

    if(CanTp_VariablesRX.CanTp_Current_RxId ==  RxPduId){
        if(CanTp_VariablesRX.expected_CF_SN == Can_PCI->SN){
            Extracted_Data.SduLength = Can_PCI->frame_lenght;
            Extracted_Data.SduDataPtr = (PduInfoPtr->SduDataPtr+1);
            Buf_State = PduR_CanTpCopyRxData(RxPduId,  &Extracted_Data, &buffer_size);
            if(Buf_State == BUFREQ_OK){
                CanTp_VariablesRX.sended_bytes += PduInfoPtr->SduLength;
                CanTp_VariablesRX.blocks_to_next_cts--;
                if( CanTp_VariablesRX.sended_bytes == CanTp_VariablesRX.message_length){
                    PduR_CanTpRxIndication(CanTp_VariablesRX.CanTp_Current_RxId, E_OK);
                    CanTp_ResetRX();
                }     
                else{ 
                    CanTp_VariablesRX.expected_CF_SN++;
                    CanTp_VariablesRX.expected_CF_SN%8;
                    current_block_size = CanTp_Calculate_Available_Blocks( buffer_size);
                    if(current_block_size > 0){
                        if(CanTp_VariablesRX.blocks_to_next_cts == 0 ){
                            CanTp_SendFlowControl(CanTp_VariablesRX.CanTp_Current_RxId, current_block_size, FC_CTS, DEFAULT_ST );
                            CanTp_set_blocks_to_next_cts(current_block_size);
                        }
                        CanTp_VariablesRX.CanTp_StateRX = CANTP_RX_PROCESSING;         
                    }
                    else{
                        CanTp_VariablesRX.CanTp_StateRX = CANTP_RX_PROCESSING_SUSPEND;
                        CanTp_SendFlowControl(CanTp_VariablesRX.CanTp_Current_RxId, current_block_size, FC_WAIT, DEFAULT_ST ); 
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

static void CanTp_FlowControlReception(PduIdType RxPduId, CanPCI_Type *Can_PCI){
    if( CanTp_VariablesTX.CanTp_StateTX == CANTP_TX_PROCESSING_SUSPENDED ){
        if(CanTp_VariablesTX.CanTp_Current_TxId == RxPduId ){
            if(Can_PCI->FS == FC_CTS){
                CanTp_VariablesTX.frame_nr_FC = Can_PCI->BS; 
                CanTp_SendNextCF();
            }   
            else if( Can_PCI->FS == FC_WAIT ){
            }
            else if( Can_PCI->FS == FC_OVFLW){
                PduR_CanTpTxConfirmation(CanTp_VariablesTX.CanTp_Current_TxId, E_NOT_OK);
                CanTp_ResetTX();
            }
            else{
                PduR_CanTpTxConfirmation(CanTp_VariablesTX.CanTp_Current_TxId, E_NOT_OK);
                CanTp_ResetTX();
            }
        }
        else{
        }
    }
    else{   
    }
}

static uint16 CanTp_Calculate_Available_Blocks(uint16 buffer_size){
    uint16 retval; 
    uint16 remaining_bytes = CanTp_VariablesRX.message_length - CanTp_VariablesRX.sended_bytes;
    if(buffer_size >= remaining_bytes){
        retval = remaining_bytes / 7;
        if(CanTp_VariablesRX.message_length%7 > 0) retval++; 
    }
    else{
        retval = buffer_size / 7;
    }
    return retval;
} 

static void CanTp_SendNextCF(void){
    BufReq_ReturnType BufReq_State;
    PduInfoType PduInfoPtr;
    PduLengthType Pdu_Len;
    Std_ReturnType ret;
    uint8 bytes_to_send;
    uint8_t payload[8];

    PduInfoPtr.SduDataPtr = payload;
    PduInfoPtr.MetaDataPtr = NULL;
    
    if(CanTp_VariablesTX.sent_bytes == CanTp_VariablesTX.message_legth){
        PduR_CanTpTxConfirmation(CanTp_VariablesTX.CanTp_Current_TxId, E_OK);
        CanTp_ResetTX();
    }
    else{
        if(CanTp_VariablesTX.message_legth - CanTp_VariablesTX.sent_bytes < 7) bytes_to_send = CanTp_VariablesTX.message_legth - CanTp_VariablesTX.sent_bytes;
        else bytes_to_send = 7;
        PduInfoPtr.SduLength = bytes_to_send;
        BufReq_State = PduR_CanTpCopyTxData(CanTp_VariablesTX.CanTp_Current_TxId, &PduInfoPtr, NULL, &Pdu_Len);
        if(BufReq_State == BUFREQ_OK){
            ret = CanTp_SendConsecutiveFrame(CanTp_VariablesTX.CanTp_Current_TxId, CanTp_VariablesTX.next_SN, PduInfoPtr.SduDataPtr, bytes_to_send);
            if( ret == E_OK ){
                CanTp_VariablesTX.sent_bytes = CanTp_VariablesTX.sent_bytes + bytes_to_send;
                CanTp_VariablesTX.frame_nr_FC--;
                CanTp_VariablesTX.next_SN = (CanTp_VariablesTX.next_SN + 1)%8;
                if((CanTp_VariablesTX.frame_nr_FC == 0) && (CanTp_VariablesTX.sent_bytes != CanTp_VariablesTX.message_legth))CanTp_VariablesTX.CanTp_StateTX = CANTP_TX_PROCESSING_SUSPENDED;
                else CanTp_VariablesTX.CanTp_StateTX = CANTP_TX_PROCESSING;        
            }
            else{
                CanTp_ResetTX();
            }
        }
        else if(BufReq_State == BUFREQ_E_NOT_OK){
            PduR_CanTpTxConfirmation(CanTp_VariablesTX.CanTp_Current_TxId, E_NOT_OK);
            CanTp_ResetTX();         
        }
        else {
            CanTp_VariablesTX.CanTp_StateTX = CANTP_TX_PROCESSING_SUSPENDED;
        }
    }
}


static Std_ReturnType CanTp_SendConsecutiveFrame(PduIdType id, uint8 SN, uint8* payload, uint32 size){
    PduInfoType PduInfo;
    uint8 SduDataPtr[8];
    uint8 *MetaDataPtr;
    PduInfo.MetaDataPtr = MetaDataPtr;
    PduInfo.SduDataPtr = SduDataPtr;
    PduInfo.SduLength = size;
    CanPCI_Type CanPCI;

    CanPCI.frame_type = CF;
    CanPCI.SN = SN;

    Std_ReturnType ret = E_OK;
    CanTp_PrepareSegmenetedFrame(&CanPCI, &PduInfo, payload);
    if(CanIf_Transmit(id , &PduInfo) == E_OK){}
    else{
        PduR_CanTpTxConfirmation(id, E_NOT_OK);
        ret = E_NOT_OK;
    }
    return ret;
}

static void CanTp_set_blocks_to_next_cts(uint8 blocks){
    CanTp_VariablesRX.blocks_to_next_cts = blocks;
}

static Std_ReturnType CanTp_SendFlowControl(PduIdType ID, uint8 BlockSize, FlowControlStatus_type FC_Status, uint8 SeparationTime ){
    Std_ReturnType ret = E_OK;
    PduInfoType PduInfoPtr;
    CanPCI_Type CanPCI;
    uint8 payloadx[8];
    uint8 *MetaDataPtr;
    uint8 SduDataPtr[8];
    CanPCI.frame_type = FC;
    CanPCI.FS = FC_Status;
    CanPCI.BS = BlockSize;
    CanPCI.ST = SeparationTime;
    PduInfoPtr.SduLength = 0;
    PduInfoPtr.MetaDataPtr = MetaDataPtr;
    PduInfoPtr.SduDataPtr = SduDataPtr;

   if(( FC_Status == FC_OVFLW )||
      ( FC_Status == FC_WAIT  )||
      ( FC_Status == FC_CTS   ))
    {
       CanTp_PrepareSegmenetedFrame(&CanPCI, &PduInfoPtr, payloadx );
        ret = CanIf_Transmit(ID, &PduInfoPtr);
        if( ret == E_NOT_OK ){
            CanTp_ResetRX();
            PduR_CanTpRxIndication(ID, E_NOT_OK);
        }
        else{
            if(FC_Status == FC_CTS){}
            else if(FC_Status == FC_WAIT){}
        } 
    }
    else{
       ret = E_NOT_OK; 
    }
    return ret;
}