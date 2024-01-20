#ifndef STD_TYPES_H
#define STD_TYPES_H

/**===================================================================================================================*\
 @file Definicja typów standardowych Autosar 4.x+

 Do użycia w ramach projektu z przedmiotu "Programowanie systemów elektoniki samochodowej".

 Andrzej Wetula (c) 2020
\*====================================================================================================================*/

#include "Platform_Types.h"
/* Normalnie byłoby też Compiler.h, ale dla uproszczenia nie jest wymagane w projekcie */

typedef struct
{
    uint16 vendorID;
    uint16 moduleID;
    uint8 sw_major_version;
    uint8 sw_minor_version;
    uint8 sw_patch_version;
} Std_VersionInfoType;

typedef uint8 Std_ReturnType;

#ifndef STATUSTYPEDEFINED
#define STATUSTYPEDEFINED
#define E_OK        0x00U
#endif
#define E_NOT_OK    0x01U

#define STD_HIGH     0x01U   /* Physical state 5V or 3.3V */
#define STD_LOW      0x00U   /* Physical state 0V */

#define STD_ACTIVE  0x01U    /* Logical state active */
#define STD_IDLE    0x00U    /* Logical state idle */

#define STD_ON      0x01U
#define STD_OFF     0x00U
#define NULL_PTR ((void *)0x00u)


#endif /* STD_TYPES_H */
