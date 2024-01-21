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
    Deklaracje funkcji lokalnych
\*====================================================================================================================*/
static void CanTp_ResetRxState();
static void CanTp_ResetTxState(void);

/*====================================================================================================================*\
    Kod funkcji
\*====================================================================================================================*/

/**
  @brief CanTp_Init

 Funckja inicjalizuje moduł CanTp

*/
void CanTp_Init ( void ){
    CanTp_ResetRx();
    CanTp_ResetTx();
    CanTp_State = CAN_TP_ON;
}

void CanTp_GetVersionInfo(Std_VersionInfoType *versioninfo) {
    if (versioninfo != NULL_PTR) {
    	versioninfo->sw_major_version = CANTP_SW_MAJOR_V;
    	versioninfo->sw_minor_version = CANTP_SW_MINOR_V;
    	versioninfo->sw_patch_version = CANTP_SW_PATCH_V;
    	versioninfo->moduleID = (uint16)CANTP_MODULE_ID;
    	versioninfo->vendorID = 0x00u;
    }
}

void CanTp_Shutdown(void) {
    // if ((CanTp_StateType)CanTpState != (CanTp_StateType)CANTP_OFF) {
    CanTpState = CANTP_OFF;
    // }
    // else {
    //     CanTp_ReportError(0x00u, CANTP_SHUTDOWN, CANTP_E_UNINIT);
    // }
}
