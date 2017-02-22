/************************************************************************************
 * Copyright (C) 2012     
 *
 * TETCOS, Bangalore. India                                                         *

 * Tetcos owns the intellectual property rights in the Product and its content.     *
 * The copying, redistribution, reselling or publication of any or all of the       *
 * Product or its content without express prior written consent of Tetcos is        *
 * prohibited. Ownership and / or any other right relating to the software and all  *
 * intellectual property rights therein shall remain at all times with Tetcos.      *

 * Author:    Thamothara Kannan                                                      *
 * ---------------------------------------------------------------------------------*/

#ifndef _MAIN_FE_H_
#define  _MAIN_FE_H_
/* Typedef declaration of structure */
typedef struct stru_Device_SwitchTable SWITCH_TABLE;
typedef struct stru_Device_Variable DEVICE_VARIABLES;
typedef struct stru_Device_PhysicalLayer DEVICE_PHYSICALLAYER;
typedef struct stru_Frame_MacLayer FRAME_MAC;
typedef struct stru_Bridge_Protocol_Data_Unit CONFIG_BPDU;
typedef struct stru_BridgeData BRIDGE_DATA;
typedef struct stru_PortData PORT_DATA;

/*Typedef Declaration of enumeration*/
typedef enum enum_Subevent_Type SUB_EVENT;
typedef enum enum_SwitchingTechnique SWITCHING_TECHNIQUE_TYPES;
typedef enum enum_Packet_Type MAC_PACKET_TYPE;
typedef enum enum_Transmission_Type FRAME_TX_FLAG;
typedef enum enum_BPDU_Type BPDU_TYPE;
typedef enum enum_PortState PORT_STATE;
typedef enum enum_Link_State LINK_STATE;
#endif	/* _MAIN_FE_H_*/

#include "Fast_Ethernet.h"
