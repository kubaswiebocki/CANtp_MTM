
#ifndef CAN_TP_H
#define CAN_TP_H

/**===================================================================================================================*\
  @file CanTp.h

  @brief Can Transport Layer
  
  Implementation of Can Transport Layer

  @see Can_Tp.pdf
\*====================================================================================================================*/

/*====================================================================================================================*\
    Załączenie nagłówków
\*====================================================================================================================*/
#include "ComStack_Types.h"
/*====================================================================================================================*\
    Makra globalne
\*====================================================================================================================*/
#define NE_NULL_PTR( ptr ) (ptr != NULL)
#define DEFAULT_ST 0

/* [SWS_CANTP_00319] */
#define NULL_PTR ((void *)0x00u)

#define CANTP_MODULE_ID (0x0Eu)
#define CANTP_SW_MAJOR_V (0x00u)
#define CANTP_SW_MINOR_V (0x01u)
#define CANTP_SW_PATCH_V (0x00u)

#define CANTP_N_PCI_TYPE_SF (0x00u)
#define CANTP_N_PCI_TYPE_FF (0x01u)
#define CANTP_N_PCI_TYPE_CF (0x02u)
#define CANTP_N_PCI_TYPE_FC (0x03u)
#define CANTP_CAN_FRAME_SIZE (0x08u)

#define N_AR_TIMEOUT_VAL 100
#define N_BR_TIMEOUT_VAL 100
#define N_CR_TIMEOUT_VAL 100

#define N_AS_TIMEOUT_VAL 100
#define N_BS_TIMEOUT_VAL 100
#define N_CS_TIMEOUT_VAL 100

#define STMmin_TIMEOUT_VAL 100

#define FC_WAIT_FRAME_CTR_MAX 10
/*====================================================================================================================*\
    Typy globalne
\*====================================================================================================================*/
typedef enum{
    TIMER_ACTIVE,
    TIMER_NOT_ACTIVE
} TimerState;

typedef struct{
    TimerState    eState;
    uint32        counter; 
    const uint32  timeout; 
} CanTp_Timer_type;


/*====================================================================================================================*\
    Deklaracje funkcji globalnych
\*====================================================================================================================*/
void CanTp_Init ( void );
void CanTp_GetVersionInfo ( Std_VersionInfoType* versioninfo );
void CanTp_Shutdown ( void );

Std_ReturnType CanTp_Transmit ( PduIdType TxPduId, const PduInfoType* PduInfoPtr );
Std_ReturnType CanTp_CancelTransmit ( PduIdType TxPduId );
Std_ReturnType CanTp_CancelReceive ( PduIdType RxPduId );
Std_ReturnType CanTp_ChangeParameter ( PduIdType id, TPParameterType parameter, uint16 value );
Std_ReturnType CanTp_ReadParameter ( PduIdType id, TPParameterType parameter, uint16* value );

void CanTp_MainFunction ( void ); 
void CanTp_RxIndication ( PduIdType RxPduId, const PduInfoType* PduInfoPtr );
void CanTp_TxConfirmation ( PduIdType TxPduId, Std_ReturnType result );

void CanTp_TimerStart(CanTp_Timer_type *timer);
void CanTp_TimerReset(CanTp_Timer_type *timer);
Std_ReturnType CanTp_TimerTick(CanTp_Timer_type *timer);
Std_ReturnType CanTp_TimerTimeout(const CanTp_Timer_type *timer);
#endif