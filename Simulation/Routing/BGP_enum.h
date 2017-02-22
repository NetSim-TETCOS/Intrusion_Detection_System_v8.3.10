/************************************************************************************
* Copyright (C) 2014
* TETCOS, Bangalore. India															*

* Tetcos owns the intellectual property rights in the Product and its content.     *
* The copying, redistribution, reselling or publication of any or all of the       *
* Product or its content without express prior written consent of Tetcos is        *
* prohibited. Ownership and / or any other right relating to the software and all  *
* intellectual property rights therein shall remain at all times with Tetcos.      *
* Author:	Thangarasu														*
* ---------------------------------------------------------------------------------*/

#include "EnumString.h"

BEGIN_ENUM(BGP_Subevent)
{
	//Administrative Events
	DECL_ENUM_ELEMENT_WITH_VAL(BGP_ManualStart,APP_PROTOCOL_BGP*100+1),
		DECL_ENUM_ELEMENT(BGP_ManualStop),
		DECL_ENUM_ELEMENT(BGP_AutomaticStart),
		DECL_ENUM_ELEMENT(BGP_ManualStart_with_PassiveTcpEstablishment),
		DECL_ENUM_ELEMENT(BGP_AutomaticStart_with_PassiveTcpEstablishment),
		DECL_ENUM_ELEMENT(BGP_AutomaticStart_with_DampPeerOscillations),
		DECL_ENUM_ELEMENT(BGP_AutomaticStart_with_DampPeerOscillations_and_PassiveTcpEstablishment),
		DECL_ENUM_ELEMENT(BGP_AutomaticStop),
		//Timer events
		DECL_ENUM_ELEMENT(BGP_ConnectRetryTimer_Expires),
		DECL_ENUM_ELEMENT(BGP_HoldTimer_Expires),
		DECL_ENUM_ELEMENT(BGP_KeepaliveTimer_Expires),
		DECL_ENUM_ELEMENT(BGP_DelayOpenTimer_Expires),
		DECL_ENUM_ELEMENT(BGP_IdleHoldTimer_Expires),
		//TCP Connection-Based Events
		DECL_ENUM_ELEMENT(BGP_TcpConnection_Valid),
		DECL_ENUM_ELEMENT(BGP_Tcp_CR_Invalid),
		DECL_ENUM_ELEMENT(BGP_Tcp_CR_Acked),
		DECL_ENUM_ELEMENT(BGP_TcpConnectionConfirmed),
		DECL_ENUM_ELEMENT(BGP_TcpConnectionFails),
		//BGP Message-Based Events
		DECL_ENUM_ELEMENT(BGP_BGPOpen),
		DECL_ENUM_ELEMENT(BGP_BGPOpen_with_DelayOpenTimer_running),
		DECL_ENUM_ELEMENT(BGP_BGPHeaderErr),
		DECL_ENUM_ELEMENT(BGP_BGPOpenMsgErr),
		DECL_ENUM_ELEMENT(BGP_OpenCollisionDump),
		DECL_ENUM_ELEMENT(BGP_NotifMsgVerErr),
		DECL_ENUM_ELEMENT(BGP_NotifMsg),
		DECL_ENUM_ELEMENT(BGP_KeepAliveMsg),
		DECL_ENUM_ELEMENT(BGP_UpdateMsg),
		DECL_ENUM_ELEMENT(BGP_UpdateMsgErr),
}
END_ENUM(BGP_Subevent);
