#ifndef COMSTACK_TYPES_H
#define COMSTACK_TYPES_H

#include "Std_Types.h"
#include "Platform_Types.h"

typedef uint16 PduIdType;
typedef uint32 PduLengthType;

typedef enum {
    BUFREQ_OK,       
    BUFREQ_E_NOT_OK, 
    BUFREQ_BUSY,     
    BUFREQ_OVFL 
}BufReq_ReturnType;

typedef enum {
    TP_DATA_CONF, 
    TP_DATARETRY,   
    TP_CONFPENDING  
}TpDataStateType;

typedef enum{
    TP_STMIN,  
    TP_BS,      
    TP_BC       
}TPParameterType;

typedef struct{
    uint8*        SduDataPtr;   
    uint8*        MetaDataPtr;  
    PduLengthType SduLength;    
}PduInfoType;

typedef struct{
    TpDataStateType TpDataStateType; 
    PduLengthType   TxTpDataCnt;

}RetryInfoType;

#endif /* COMSTACK_TYPES_H */