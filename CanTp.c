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

/*====================================================================================================================*\
    Kod funkcji
\*====================================================================================================================*/

/*====================================================================================================================*/
/**
  @brief CanTp_Init

 Funckja inicjalizuje moduł CanTp [SWS_CANTP_00208]
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

 Funckja zwracajaca informacje o wersji [SWS_CANTP_00210]
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