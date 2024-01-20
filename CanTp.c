#include "CanTp.h"
#include "CanTp_Cfg.h"

#ifndef CANTP_CBK_H
#include "CanTp_Cbk.h"
#endif /* #ifndef CANTP_CBK_H */

#ifndef COMSTACK_TYPES_H
#include "ComStack_Types.h"
#endif /* #ifndef COMSTACK_TYPES_H */

#ifndef PDUR_H
#include "PduR.h"
#endif /* #ifndef PDUR_H */
#include "Std_Types.h"

#define CANTP_N_PCI_TYPE_SF (0x00u) // Single Frame
#define CANTP_N_PCI_TYPE_FF (0x01u) // First Frame
#define CANTP_N_PCI_TYPE_CF (0x02u) // Consecutive Frame
#define CANTP_N_PCI_TYPE_FC (0x03u) // Flow Control
#define CANTP_CAN_FRAME_SIZE (0x08u)

typedef uint8 CanTp_NPciType;

typedef enum
{
    CANTP_WAIT = 0x00u,
    CANTP_PROCESSING
} CanTp_TaskStateType;


typedef struct
{
    uint8 can[0x08u];
    PduLengthType size;
    PduLengthType rmng;
} CanTp_NSduBufferType;


typedef enum
{
    CANTP_FRAME_STATE_INVALID = 0x00u,
    CANTP_RX_FRAME_STATE_FC_TX_REQUEST,
    CANTP_RX_FRAME_STATE_FC_TX_CONFIRMATION,
    CANTP_RX_FRAME_STATE_FC_OVFLW_TX_CONFIRMATION,
    CANTP_RX_FRAME_STATE_CF_RX_INDICATION,
    CANTP_TX_FRAME_STATE_SF_TX_REQUEST,
    CANTP_TX_FRAME_STATE_SF_TX_CONFIRMATION,
    CANTP_TX_FRAME_STATE_FF_TX_REQUEST,
    CANTP_TX_FRAME_STATE_FF_TX_CONFIRMATION,
    CANTP_TX_FRAME_STATE_CF_TX_REQUEST,
    CANTP_TX_FRAME_STATE_CF_TX_CONFIRMATION,
    CANTP_TX_FRAME_STATE_FC_RX_INDICATION,
    CANTP_FRAME_STATE_OK,
    CANTP_FRAME_STATE_ABORT
} CanTp_FrameStateType;


typedef struct
{
    const CanTpRxNSdu *cfg;
    CanTp_NSduBufferType buf;
    uint8 meta_data_lower[0x04u];
    uint8 meta_data_upper[0x04u];
    CanTpNSa saved_n_sa;
    CanTpNTa saved_n_ta;
    CanTpNAe saved_n_ae;
    boolean has_meta_data;
    uint8  fs;
    uint32 st_min;
    uint8 bs;
    uint8 sn;
    uint16 wft_max;
    PduInfoType can_if_pdu_info;
    PduInfoType pdu_r_pdu_info;
    struct
    {
        CanTp_TaskStateType taskState;
        CanTp_FrameStateType state;
        struct
        {
            uint32 st_min;
            uint8 bs;
        } m_param;
    } shared;
} CanTpRxConnection;


typedef struct
{
    const CanTpTxNSdu *cfg;
    CanTp_NSduBufferType buf;
    uint8 meta_data[0x04u];
    CanTpNSa saved_n_sa;
    CanTpNTa saved_n_ta;
    CanTpNAe saved_n_ae;
    boolean has_meta_data;
    uint8  fs;
    uint32 target_st_min;
    uint32 st_min;
    uint16 bs;
    uint8 sn;
    PduInfoType can_if_pdu_info;
    CanTp_TaskStateType taskState;
    struct
    {
        CanTp_FrameStateType state;
        uint32 flag;
    } shared;
} CanTpTxConnection;


typedef struct
{
	CanTpRxConnection rx;
	CanTpTxConnection tx;
    uint32 n[0x06u];
    uint8_least dir;
    uint32 t_flag;
} CanTp_NSduType;


typedef struct
{
	CanTp_NSduType sdu[CANTP_MAX_NUM_OF_N_SDU];
} CanTp_ChannelRtType;


typedef struct
{
	CanTpRxConnection rx;
	CanTpTxConnection tx;
    uint32 n[0x06u];
    uint8_least dir;
    uint32 t_flag;
} CanTp_NSduType;


CanTp_StateType CanTpState = CANTP_OFF;
static const CanTpConfig *CanTpConfigPtr = NULL_PTR;
static CanTp_ChannelRtType CanTpRt[CANTP_MAX_NUM_OF_CHANNEL];

static CanTp_FrameStateType CanTp_LDataConTSF(CanTp_NSduType *pNSdu);
static CanTp_FrameStateType CanTp_LDataConTFF(CanTp_NSduType *pNSdu);
static CanTp_FrameStateType CanTp_LDataConTFC(CanTp_NSduType *pNSdu);


void CanTp_Init(const CanTpConfig *CfgPtr)
{
	uint32_least counter;
    uint32_least channel_counter;
    uint32_least rt_sdu_counter;
    uint32_least cfg_sdu_counter;
    const CanTpChannel *p_cfg_cha;
    const CanTpRxNSdu *p_cfg_rx_sdu;
    const CanTpTxNSdu *p_cfg_tx_sdu;
    CanTp_ChannelRtType *p_rt_cha;
    CanTp_NSduType *p_rt_sdu;
    uint8 *p_cleared_data = (uint8 *)&CanTpRt[0x00u];
    uint32_least temp = sizeof(CanTpRt);

    if (CfgPtr != NULL_PTR) {
        CanTpConfigPtr = CfgPtr;

        for (counter = 0x00u; counter < temp; counter++) {
            p_cleared_data[counter] = 0x00u;
        }
        for ( channel_counter = 0x00u; channel_counter < CfgPtr->maxChannelCnt; channel_counter++) {
            p_rt_cha = &CanTpRt[channel_counter];
            p_cfg_cha = &CfgPtr->pChannel[channel_counter];

            for (rt_sdu_counter = 0x00u; rt_sdu_counter < CANTP_MAX_NUM_OF_N_SDU; rt_sdu_counter++) {
                for (cfg_sdu_counter = 0x00u; cfg_sdu_counter < p_cfg_cha->nSdu.txNSduCnt; cfg_sdu_counter++) {
                    p_cfg_tx_sdu = &p_cfg_cha->nSdu.tx[cfg_sdu_counter];
                    if (p_cfg_tx_sdu->nSduId == rt_sdu_counter) {
                        p_rt_sdu->tx.taskState = CANTP_WAIT;
                        p_rt_sdu->dir |= CANTP_TX;
                        p_rt_sdu = &p_rt_cha->sdu[p_cfg_tx_sdu->nSduId];
                        p_rt_sdu->tx.cfg = p_cfg_tx_sdu;
                    }
                }
                for (cfg_sdu_counter = 0x00u; cfg_sdu_counter < p_cfg_cha->nSdu.rxNSduCnt; cfg_sdu_counter++) {
                    p_cfg_rx_sdu = &p_cfg_cha->nSdu.rx[cfg_sdu_counter];
                    if (p_cfg_rx_sdu->nSduId == rt_sdu_counter) {
                        p_rt_sdu = &p_rt_cha->sdu[p_cfg_rx_sdu->nSduId];
                        p_rt_sdu->rx.shared.taskState = CANTP_WAIT;
                        p_rt_sdu->dir |= CANTP_RX;
                        p_rt_sdu->rx.cfg = p_cfg_rx_sdu;
                        p_rt_sdu->rx.shared.m_param.st_min = p_cfg_rx_sdu->sTMin;
                        p_rt_sdu->rx.shared.m_param.bs = p_cfg_rx_sdu->bs;
                    }
                }
            }
        }

        CanTpState = CANTP_ON;
    }
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
    if ((CanTp_StateType)CanTpState != (CanTp_StateType)CANTP_OFF) {
        CanTpState = CANTP_OFF;
    }
    else {
        CanTp_ReportError(0x00u, CANTP_SHUTDOWN, CANTP_E_UNINIT);
    }
}

void CanTp_RxIndication(PduIdType rxPduId, const PduInfoType *pPduInfo)
{
    CanTp_FrameStateType next_state;
    CanTp_NPciType pci;
    PduLengthType n_ae_field_size;
    CanTp_NSduType *p_n_sdu;

    if (pPduInfo != NULL_PTR)
    {
        if (CanTp_GetNSduFromPduId(rxPduId, &p_n_sdu) == E_OK)
        {
            if ((p_n_sdu->dir & CANTP_RX) != 0x00u)
            {
                n_ae_field_size = CanTp_GetAddrInfoSizeInPayload(p_n_sdu->rx.cfg->af);

                if (CanTp_DecodePCIValue(&pci, &pPduInfo->SduDataPtr[n_ae_field_size]) == E_OK)
                {
                    /* SWS_CanTp_00345: If frames with a payload <= 8 (either CAN 2.0 frames or
                     * small CAN FD frames) are used for a Rx N-SDU and CanTpRxPaddingActivation is
                     * equal to CANTP_ON, then CanTp receives by means of CanTp_RxIndication() call
                     * an SF Rx N-PDU belonging to that N-SDU, with a length smaller than eight
                     * bytes (i.e. PduInfoPtr.SduLength < 8), CanTp shall reject the reception. The
                     * runtime error code CANTP_E_PADDING shall be reported to the Default Error
                     * Tracer. */
                    if (((CanTp_StateType)p_n_sdu->rx.cfg->padding == (CanTp_StateType)CANTP_ON) &&
                        (pPduInfo->SduLength < CANTP_CAN_FRAME_SIZE))
                    {
                        PduR_CanTpRxIndication(p_n_sdu->rx.cfg->nSduId, E_NOT_OK);

                        CanTp_ReportRuntimeError(0x00u,
                                                 CANTP_RX,
                                                 CANTP_E_PADDING);

                        next_state = CANTP_FRAME_STATE_OK;
                    }
                    /* SWS_CanTp_00093: If a multiple segmented session occurs (on both receiver and
                     * sender side) with a handle whose communication type is functional, the CanTp
                     * module shall reject the request and report the runtime error code
                     * CANTP_E_INVALID_TATYPE to the Default Error Tracer. */
                    else if ((p_n_sdu->rx.cfg->taType == CANTP_FUNCTIONAL) &&
                             (pci == CANTP_N_PCI_TYPE_FF))
                    {
                        CanTp_ReportRuntimeError(0x00u,
                                                 CANTP_RX_INDICATION,
                                                 CANTP_E_INVALID_TATYPE);

                        next_state = CANTP_FRAME_STATE_OK;
                    }
                    else if (pci == CANTP_N_PCI_TYPE_SF)
                    {
                        next_state = CanTp_LDataIndRSF(p_n_sdu, pPduInfo, n_ae_field_size);
                    }
                    else if (pci == CANTP_N_PCI_TYPE_FF)
                    {
                        next_state = CanTp_LDataIndRFF(p_n_sdu, pPduInfo, n_ae_field_size);
                    }
                    else if ((pci == CANTP_N_PCI_TYPE_CF) &&
                             (p_n_sdu->rx.shared.state == CANTP_RX_FRAME_STATE_CF_RX_INDICATION))
                    {
                        next_state = CanTp_LDataIndRCF(p_n_sdu, pPduInfo, n_ae_field_size);
                    }
                    else
                    {
                        next_state = CANTP_FRAME_STATE_INVALID;
                    }

                    if (next_state != CANTP_FRAME_STATE_INVALID)
                    {
                        p_n_sdu->rx.shared.state = next_state;
                    }
                }
            }

            if ((p_n_sdu->dir & CANTP_TX) != 0x00u)
            {
                n_ae_field_size = CanTp_GetAddrInfoSizeInPayload(p_n_sdu->tx.cfg->af);

                if (CanTp_DecodePCIValue(&pci, &pPduInfo->SduDataPtr[n_ae_field_size]) == E_OK)
                {
                    if (p_n_sdu->tx.shared.state == CANTP_TX_FRAME_STATE_FC_RX_INDICATION)
                    {
                        next_state = CanTp_LDataIndTFC(p_n_sdu, pPduInfo, n_ae_field_size);
                    }
                    else
                    {
                        next_state = CANTP_FRAME_STATE_INVALID;
                    }

                    if (next_state != CANTP_FRAME_STATE_INVALID)
                    {
                        p_n_sdu->tx.shared.state = next_state;
                    }
                }
            }
        }
        else
        {
            CanTp_ReportError(0x00u, CANTP_RX_INDICATION, CANTP_E_INVALID_RX_ID);
        }
    }
    else
    {
        CanTp_ReportError(0x00u, CANTP_RX_INDICATION, CANTP_E_PARAM_POINTER);
    }
}

void CanTp_TxConfirmation(PduIdType txPduId, Std_ReturnType result)
{
    CanTp_FrameStateType next_state;
    CanTp_NSduType *p_n_sdu;

    if (CanTp_GetNSduFromPduId(txPduId, &p_n_sdu) == E_OK)
    {
        if (result == E_OK)
        {
            if ((p_n_sdu->dir & CANTP_RX) != 0x00u)
            {
                next_state = CANTP_FRAME_STATE_INVALID;

                if (p_n_sdu->rx.shared.state == CANTP_RX_FRAME_STATE_FC_TX_CONFIRMATION)
                {
                    next_state = CanTp_LDataConRFC(p_n_sdu);
                }

                if (next_state != CANTP_FRAME_STATE_INVALID)
                {
                    p_n_sdu->rx.shared.state = next_state;
                }
            }

            if ((p_n_sdu->dir & CANTP_RX) != 0x00u)
            {
                next_state = CANTP_FRAME_STATE_INVALID;

                if (p_n_sdu->tx.shared.state == CANTP_TX_FRAME_STATE_SF_TX_CONFIRMATION)
                {
                    next_state = CanTp_LDataConTSF(p_n_sdu);
                }
                else if (p_n_sdu->tx.shared.state == CANTP_TX_FRAME_STATE_FF_TX_CONFIRMATION)
                {
                    next_state = CanTp_LDataConTFF(p_n_sdu);
                }
                else if (p_n_sdu->tx.shared.state == CANTP_TX_FRAME_STATE_CF_TX_CONFIRMATION)
                {
                    next_state = CanTp_LDataConTCF(p_n_sdu);
                }
                else
                {
                    /* MISRA C, do nothing. */
                }

                if (next_state != CANTP_FRAME_STATE_INVALID)
                {
                    p_n_sdu->tx.shared.state = next_state;
                }
            }
        }
        else
        {
            /* SWS_CanTp_00355: CanTp shall abort the corresponding session, when
             * CanTp_TxConfirmation() is called with the result E_NOT_OK. */
            CanTp_AbortTxSession(p_n_sdu, CANTP_I_NONE, FALSE);
        }
    }
}


Std_ReturnType CanTp_Transmit(PduIdType CanTpTxSduId, const PduInfoType *CanTpTxInfoPtr){
    CanTp_NSduType *N_Sdu = NULL_PTR;
    Std_ReturnType r = E_NOT_OK;

    if ((CanTp_StateType)CanTpState == (CanTp_StateType)CANTP_ON) {
        if (CanTpTxInfoPtr != NULL_PTR) {
            if (CanTp_GetNSduFromPduId(CanTpTxSduId, &N_Sdu) == E_OK) {
                if (CanTpTxInfoPtr->MetaDataPtr != NULL_PTR) {
                	N_Sdu->tx.has_meta_data = TRUE;
                    if (N_Sdu->tx.cfg->af == CANTP_MIXED29BIT) {
                    	N_Sdu->tx.saved_n_sa.nSa = CanTpTxInfoPtr->MetaDataPtr[0x00u];
                    	N_Sdu->tx.saved_n_ta.nTa = CanTpTxInfoPtr->MetaDataPtr[0x01u];
                    	N_Sdu->tx.saved_n_ae.nAe = CanTpTxInfoPtr->MetaDataPtr[0x02u];
                    }
                    else if (N_Sdu->tx.cfg->af == CANTP_NORMALFIXED) {
                    	N_Sdu->tx.saved_n_sa.nSa = CanTpTxInfoPtr->MetaDataPtr[0x00u];
                    	N_Sdu->tx.saved_n_ta.nTa = CanTpTxInfoPtr->MetaDataPtr[0x01u];
                    }
                    else if (N_Sdu->tx.cfg->af == CANTP_MIXED) {
                    	N_Sdu->tx.saved_n_ae.nAe = CanTpTxInfoPtr->MetaDataPtr[0x00u];
                    }
                    else if (N_Sdu->tx.cfg->af == CANTP_EXTENDED) {
                    	N_Sdu->tx.saved_n_ta.nTa = CanTpTxInfoPtr->MetaDataPtr[0x00u];
                    }
                }
                else {
                	N_Sdu->tx.has_meta_data = FALSE;
                }
                if ((N_Sdu->tx.taskState != CANTP_PROCESSING) && (CanTpTxInfoPtr->SduLength > 0x0000u) && (CanTpTxInfoPtr->SduLength <= 0x0FFFu)) {
                	N_Sdu->tx.buf.size = CanTpTxInfoPtr->SduLength;

                    if ((((N_Sdu->tx.cfg->af == CANTP_STANDARD) || (N_Sdu->tx.cfg->af == CANTP_NORMALFIXED)) && (CanTpTxInfoPtr->SduLength <= 0x07u)) || (((N_Sdu->tx.cfg->af == CANTP_EXTENDED) ||  (N_Sdu->tx.cfg->af == CANTP_MIXED) ||  (N_Sdu->tx.cfg->af == CANTP_MIXED29BIT)) && (CanTpTxInfoPtr->SduLength <= 0x06u))) {
                    	N_Sdu->tx.shared.state = CANTP_TX_FRAME_STATE_SF_TX_REQUEST;
                        r = E_OK;
                    }
                    else
                    {
                        if (N_Sdu->tx.cfg->taType == CANTP_PHYSICAL) {
                        	N_Sdu->tx.shared.state = CANTP_TX_FRAME_STATE_FF_TX_REQUEST;
                            r = E_OK;
                        }
                    }
                    if (r == E_OK) {
                    	N_Sdu->tx.taskState = CANTP_PROCESSING;
                    }
                }
            }
            else {
                CanTp_ReportError(0x00u, CANTP_TRANSMIT, CANTP_E_INVALID_TX_ID);
            }
        }
        else {
            CanTp_ReportError(0x00u, CANTP_TRANSMIT, CANTP_E_PARAM_POINTER);
        }
    }
    else {
        CanTp_ReportError(0x00u, CANTP_TRANSMIT, CANTP_E_UNINIT);
    }

    return r;
}


Std_ReturnType CanTp_CancelTransmit(PduIdType CanTpTxSduId) {
    CanTp_NSduType *N_Sdu;
    Std_ReturnType r = E_NOT_OK;
    if ((CanTp_StateType)CanTpState == (CanTp_StateType)CANTP_ON) {
        if ((CanTp_GetNSduFromPduId(CanTpTxSduId, &N_Sdu) == E_OK) && ((N_Sdu->dir & CANTP_TX) != 0x00u)) {
            if (N_Sdu->tx.taskState == CANTP_PROCESSING) {
            	N_Sdu->tx.taskState = CANTP_WAIT;
                PduR_CanTpTxConfirmation(N_Sdu->tx.cfg->nSduId, E_NOT_OK);
                r = E_OK;
            }
            else {
                CanTp_ReportRuntimeError(0x00u, CANTP_CANCEL_TRANSMIT, CANTP_E_OPER_NOT_SUPPORTED);
            }
        }
        else {
            CanTp_ReportError(0x00u, CANTP_CANCEL_TRANSMIT, CANTP_E_PARAM_ID);
        }
    }
    else {
        CanTp_ReportError(0x00u, CANTP_CANCEL_TRANSMIT, CANTP_E_UNINIT);
    }

    return r;
}


Std_ReturnType CanTp_CancelReceive(PduIdType CanTpRxSduId)
{
    CanTp_NSduType *N_Sdu;
    CanTp_TaskStateType task_state;
    PduLengthType n_ae_field_size;
    Std_ReturnType r = E_NOT_OK;

    if ((CanTp_StateType)CanTpState == (CanTp_StateType)CANTP_ON) {
        if ((CanTp_GetNSduFromPduId(CanTpRxSduId, &N_Sdu) == E_OK) && ((N_Sdu->dir & CANTP_RX) != 0x00u)) {
            n_ae_field_size = CanTp_GetAddrInfoSizeInPayload(N_Sdu->rx.cfg->af);
            task_state = N_Sdu->rx.shared.taskState;
            if (task_state == CANTP_PROCESSING) { {
                	N_Sdu->rx.shared.taskState = CANTP_WAIT;
                    PduR_CanTpRxIndication(N_Sdu->rx.cfg->nSduId, E_NOT_OK);
                    r = E_OK;
                }
            }
        }
    }

    return r;
}



#if (CANTP_E_PARAM_ID  == STD_ON)

Std_ReturnType CanTp_ChangeParameter(PduIdType id, TPParameterType parameter, uint16 value)
{
    CanTp_NSduType *N_Sdu;
    CanTp_TaskStateType task_state;
    Std_ReturnType r = E_NOT_OK;

    if ((CanTp_StateType)CanTpState == (CanTp_StateType)CANTP_ON){
        if (CanTp_GetNSduFromPduId(id, &N_Sdu) == E_OK) {
            task_state = N_Sdu->rx.shared.taskState;

            if (task_state != CANTP_PROCESSING) {
                switch (parameter)  {
                    case TP_STMIN:
                    {
                        if ((value <= 0xFFu) && ((N_Sdu->dir & CANTP_RX) != 0x00u)) {
                        	N_Sdu->rx.shared.m_param.st_min = value;
                            r = E_OK;
                        }

                        break;
                    }
                    case TP_BS:
                    {
                        if ((value <= 0xFFu) && ((N_Sdu->dir & CANTP_RX) != 0x00u)) {
                        	N_Sdu->rx.shared.m_param.bs = (uint8)value;

                            r = E_OK;
                        }

                        break;
                    }
                    case TP_BC:
                    default:
                        break;
                }

            }
        }
    }

    return r;
}




#endif /* #if (CANTP_E_PARAM_ID  == STD_ON) */

#if (CANTP_READ_PARAMETER_API == STD_ON)

Std_ReturnType CanTp_ReadParameter(PduIdType id, TPParameterType parameter, uint16 *pValue)
{
    CanTp_NSduType *N_Sdu;
    uint16 value;
    Std_ReturnType r = E_NOT_OK;

    if ((CanTp_StateType)CanTpState == (CanTp_StateType)CANTP_ON) {
        if (pValue != NULL_PTR) { {
                if ((N_Sdu->dir & CANTP_RX) != 0x00u) {
                    switch (parameter)
                    {
                        case TP_STMIN:
                        {
                            value = (uint16)N_Sdu->rx.shared.m_param.st_min;

                            *pValue = value;
                            r = E_OK;

                            break;
                        }
                        case TP_BS:
                        {
                            value = (uint16)N_Sdu->rx.shared.m_param.bs;

                            *pValue = value;
                            r = E_OK;

                            break;
                        }
                        case TP_BC:
                        default:
                        {
                            break;
                        }
						
                    }
                }
            }
        }

    }


    return r;
}
#endif /* #if (CANTP_READ_PARAMETER_API == STD_ON) */
