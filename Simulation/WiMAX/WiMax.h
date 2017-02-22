/************************************************************************************
 * Copyright (C) 2015                                                               *
 * TETCOS, Bangalore. India                                                         *
 *                                                                                  *
 * Tetcos owns the intellectual property rights in the Product and its content.     *
 * The copying, redistribution, reselling or publication of any or all of the       *
 * Product or its content without express prior written consent of Tetcos is        *
 * prohibited. Ownership and / or any other right relating to the software and all  *
 * intellectual property rights therein shall remain at all times with Tetcos.      *
 *                                                                                  *
 * Author: Surabhi Bothra	                                                        *
 *                                                                                  *
 * ---------------------------------------------------------------------------------*/
#ifndef _NETSIM_WIMAX_H_
#define _NETSIM_WIMAX_H_
//For MSVC commpiler. For GCC link via Linker command 
#pragma comment(lib,"WiMax.lib")
#pragma comment(lib,"Metrics.lib")
#pragma comment(lib,"NetworkStack.lib")
#pragma comment(lib,"Mobility.lib")

#include "List.h"

#ifdef  __cplusplus
extern "C" {
#endif

//Default config parameter
#define WiMax_TRANSMISSION_TYPE_DEFAULT			_strdup("OFDM")
#define WiMax_FREQUENCY_DEFAULT					3.5 //GHz
#define WiMax_TX_POWER_DEFAULT					10 //W
#define WiMax_CHANNEL_BANDWIDTH_DEFAULT			3.5 //MHz
#define WiMax_FFT_SIZE_DEFAULT					256
#define WiMax_FRAME_DURATION_DEFAULT			5 //ms
#define WiMax_USED_SUBCARRIER_DEFAULT			200
#define WiMax_SAMPLING_FACTOR_DEFAULT			_strdup("8/7")
#define WiMax_TTG_DEFAULT						105 //us
#define WiMax_RTG_DEFAULT						60 //us
#define WiMax_CP_FACTOR_DEFAULT					_strdup("1/8")
#define WiMax_DL_FRAME_DURATION_DEFAULT			2.5

	//BroadCast MAC
	#define BROADCAST_MAC "FFFFFFFFFFFF"
	#define MAC_HEADER_SIZE 6 
	
	//2 bits binary number
	#define B2_00	0
	#define B2_01	1
	#define B2_10	2
	#define B2_11	3
	//3 bits binary number
	#define B3_000	0
	#define B3_001	1
	#define B3_010	2
	#define B3_011	3
	#define B3_100	4
	#define B3_101	5
	#define B3_110	6
	#define B3_111	7

	/** Mac Subheader bit
	   Table 6, IEEE802.16-2004-page 37
	 */
	#define UPSTREAM_GRANT_MANAGEMENT_BIT	0
	#define PACKING_BIT						1
	#define FRAGMENT_BIT					2
	#define EXTENDED_BIT					3
	#define ARQ_FEEDBACK_BIT				4
	#define MESH_SUBHEADER_BIT				5

		//Typdef's of struct
	typedef struct stru_WiMax_SS_MAC		WIMAX_SS_MAC;
	typedef struct stru_WiMax_SS_Phy		WIMAX_SS_PHY;
	typedef struct stru_WiMax_BS_MAC		WIMAX_BS_MAC;
	typedef struct stru_WiMax_BS_Phy		WIMAX_BS_PHY;
	typedef struct stru_AssociatedSSInfo	WIMAX_SS_INFO;
	typedef struct stru_WiMax_MacHeader		WIMAX_MAC_HEADER;
	typedef struct stru_WiMax_Queue			WIMAX_QUEUE;
	typedef struct stru_802_16_DL_MAP		DL_MAP;
	typedef struct stru_802_16_UL_MAP		UL_MAP;
	typedef struct stru_802_16_DL_MAP_IE	DL_Map_IE;
	typedef struct stru_802_16_UL_MAP_IE	UL_Map_IE;
	typedef struct stru_802_16_SymbolParameter SYMBOL_PARAMETER;
	typedef struct stru_802_16_Frame_Control_Header	FCH_Header;
	typedef struct stru_802_16_FragmentSubHeader FRAGMENT_SUB_HEADER;
	typedef struct stru_802_16_BS_Metrics	BS_METRICS;	
	
	//Typedef for enum
	typedef enum enum_802_16_CodingRate		CODING_RATE;
	typedef enum enum_802_16_Modulation		MODULATION_TECHNIQUE;
	typedef enum enum_802_16_SubEvent		SUBEVENT;
	typedef enum enum_802_16_ChannelCharacteristics CHANNEL_MODEL;
	typedef enum enum_802_16_MAC_Management_Message MANAGEMENT_MESSAGE;

	# define FCH_SIZE		11	
	# define TypeofServices	6
	# define No_of_Bursts_DL 7

	/** Enumeration for Modulation technique*/
	enum enum_802_16_Modulation
	{
		Modulation_BPSK,
		Modulation_QPSK,
		Modulation_16_QAM,
		Modulation_64_QAM,
	};

	/** Enumeration for coding rate */
	enum enum_802_16_CodingRate
	{
		Coding_UNCODED,
		Coding_1_2_REP4,
		Coding_1_2_REP3,
		Coding_1_2_REP2,
		Coding_1_2,
		Coding_2_3,
		Coding_3_4,
	};


	enum enum_802_16_MAC_Management_Message
	{
		/**NOT IN STANDARD. Included here for commonality*/
		MMM_FCH=0,
		MMM_BW_REQUEST,
		//Standard
		MMM_UCD,		
		MMM_DCD,				
		MMM_DL_MAP,		
		MMM_UL_MAP,	
		MMM_DSD_REQ,
	};
	#define WiMax_CONTROL_PACKET(MMM) MAC_PROTOCOL_IEEE802_16*100+MMM

	enum enum_802_16_SubEvent
	{
		TRANSMIT_FCH = MAC_PROTOCOL_IEEE802_16*100+1,
		TRANSMIT_DL_MAP,
		TRANSMIT_UL_MAP,
		TRANSMIT_MAC_PDU,
	};

	/// Enueration for pathloss models
	enum enum_802_16_ChannelCharacteristics
	{
		NO_PATHLOSS = 0,
		LINE_OF_SIGHT,
		FADING,
		FADING_AND_SHADOWING,
	};
	//MAC Header
	/* IEEE802.16-2012 Table 6.1 page 56-57 */
	/** Table 5, IEEE802.16-2004 page 38
    Generic MAC Header fields
	 */
	struct stru_WiMax_MacHeader
	{
		unsigned int HT:1;
		unsigned int EC:1;
		char Type[6];
		unsigned int reserved1:1;
		unsigned int CI:1;
		unsigned int EKS:2;
		unsigned int reserved2:1;
		unsigned int LEN:11;
		unsigned int CID:16;
		unsigned int HCS:8;
		FRAGMENT_SUB_HEADER* Fragment_Header;	
	};

		/* Structure for symbol paramter
	8.3.2.1 Primitive parameter definitions,
	IEEE 802.16-2012 page 428
	 */
	struct stru_802_16_SymbolParameter
	{
		double BW;				//This is the nominal channel bandwidth.

		int Nused;				// Number of used subcarriers.

		double SamplingFactor;	/* Sampling factor. This parameter, in conjunction with BW 
								 * and Nused determines the subcarrier spacing, and the useful symbol time.
								 */

		double G;				// This is the ratio of CP time to “useful” time

		int Nfft;				// Smallest power of two greater than Nused.

		double Fs;				// Sampling Frequency Fs = floor(n ⋅ BW ⁄ 8000) × 8000

		double SubCarrierSpacing; // Δf = Fs/ NFFT

		double Usefulsymboltime; // Tb = 1 ⁄ Δf

		double CPTime;	// Tg = G ⋅ Tb

		double OFDMSymbolTime; // Ts = Tb + Tg

		double SamplingTime;	// Tb ⁄ NFFT

		//Data subcarriers = 192
		int DataSubCarrierCount; 
		unsigned int NoofULSymbols;
		unsigned int NoofDLSymbols;
		unsigned int UPlinkFrameStartSymbol;
	};

	struct stru_802_16_Phy_Parameters
	{
		int Index;				
		double SNR;
		MODULATION_TECHNIQUE Modulation;
		CODING_RATE CodingRate;
		int NBSC;	
		// Data subCarrier count*Bits based on modulation * coding rate
		int BitsCountInOneSymbol;	
	};
	#define MAX_PHY_PARAMETER 7
	struct stru_802_16_Phy_Parameters struPhyParameters[MAX_PHY_PARAMETER];

	struct stru_DL_Frame_Prefix_IE
	{
		unsigned int Rate_Id:4;
		unsigned int PreamblePresent:1;
		unsigned int length:11;
	};

	//Pg 452,Table 225,IEEE 802.16-2004
	struct stru_802_16_Frame_Control_Header
	{	
		unsigned int BS_ID:4;
		unsigned int Frame_Number:4;
		unsigned int configurationChangeCount:4;
		unsigned int reserved:4;
		struct stru_DL_Frame_Prefix_IE* pstruDL_FP_IE[4];
		unsigned int HCS:8;	//8 bits Header Check Sequence
	};

	//Pg 39,Table 8
	struct stru_802_16_FragmentSubHeader
	{
		unsigned int FC:2;//fragmentation state
		unsigned int FSN:3;//Sequence number of the current SDU fragment
		unsigned int reserved:3; 
	};

	//SS info
	struct stru_AssociatedSSInfo
	{
		NETSIM_ID nSSId;
		NETSIM_ID nSSInterfaceId;

		//Queue
		WIMAX_QUEUE* Queue[TypeofServices];
		_ele* ele;
	};
#define SSINFO_ALLOC() (WIMAX_SS_INFO*)list_alloc(sizeof(WIMAX_SS_INFO),offsetof(struct stru_AssociatedSSInfo,ele))
#define SSINFO_NEXT(info) info=LIST_NEXT(&(info))
	
	struct stru_802_16_BS_Metrics
	{
		NETSIM_ID BS_Id;
		NETSIM_ID SS_Id;
		double* distance;
		double* Datarate;
	};


	struct stru_WiMax_Queue
	{
		unsigned int nSize;
		NetSim_PACKET* head;
		NetSim_PACKET* tail;
	};

	struct stru_802_16_UL_MAP_IE
	{
		unsigned int CID:16;
		unsigned int Starttime:11;
		unsigned int SubchannelIndex:5;
		unsigned int UIUC:4;
		unsigned int duration:10;
		unsigned int MidambleRepetitionInterval:2;
		struct stru_802_16_UL_MAP_IE* pstruNext;
	};

	struct stru_802_16_UL_MAP
	{
		MANAGEMENT_MESSAGE ManagementMessageType;//8 bits
		unsigned int UCDCount:8;
		unsigned int UplinkChannelId:8;
		unsigned int AllocationStartTime:32;
		unsigned int IE_Count;
		double PacketSize;
		UL_Map_IE* pstruULMAPIE;
	};

	//Pg 461,Table 236,IEEE 802.16-2004
	struct stru_802_16_DL_MAP_IE
	{
		unsigned int CID:16;
		unsigned int DIUC:4;
		unsigned int PreamblePresent:1;
		unsigned int StartTime:11;
		struct stru_802_16_DL_MAP_IE* pstruNext;
	};

	//Table-16, DL_MAP Msg Format Pg 46
	//IEEE 802.16-2004
	struct stru_802_16_DL_MAP
	{
		MANAGEMENT_MESSAGE ManagementMessageType;//8 bits
		unsigned int DCDCount:8;
		NETSIM_ID BS_ID;//48 bits
		unsigned int IE_Count;
		double PacketSize;
		DL_Map_IE* pstruDLMAPIE;
	};

	//SS Mac
	struct stru_WiMax_SS_MAC
	{
		//Config parameter
		char* TransmissionType;
		NETSIM_ID BSId;
		NETSIM_ID BSInterface;
		//Queue
		WIMAX_QUEUE* Queue[TypeofServices];
		//variable to pack a fragmented packet
		NetSim_PACKET** Fragment_Packet;
	};

	//SS Phy
	struct stru_WiMax_SS_Phy
	{
		//Config parameter
		char* TransmissionType;
		double TotalReceivedPower;
		double SNR;
	};

	//BS Mac
	struct stru_WiMax_BS_MAC
	{
		unsigned int nAssociatedSSCount;
		WIMAX_SS_INFO* ssInfo;
		unsigned int Symbol_Count_DL;
		unsigned int Symbol_Count_UL;
		unsigned int Symbol_Available_DL;
		unsigned int Symbol_Available_UL;
		unsigned int Symbol_Reqd_DL_MAP;
		unsigned int Symbol_Reqd_UL_MAP;
		unsigned int ByteCountinoneSymbol;
		double DLFrameTime;
		double FrameStartTime;
		DL_MAP* struDL_MAP;
		UL_MAP* struUL_MAP;
		NetSim_PACKET* struFCH;
		BS_METRICS* struBS_Metrics;
		//variable to pack a fragmented packet
		NetSim_PACKET** Fragment_Packet;
	};

	//BS Phy
	struct stru_WiMax_BS_Phy
	{
		//Config parameter
		char* TransmissionType;
		double ChannelBandwidth;
		double Distance;	
		double Frequency;
		unsigned int FFTSize;
		unsigned int NoUsedSubCarrier;
		double SamplingFactor;
		MODULATION_TECHNIQUE Modulation;
		CHANNEL_MODEL ChannelModel;	
		double GuardInterval;
		CODING_RATE CodingRate;
		double FrameDuration;
		unsigned int FrameNumber;
		double DlFrameDuration;
		unsigned int Tx_Transition_Gap;
		unsigned int Rx_Transition_Gap;
		
		unsigned int BitsCountInOneSymbol;
		double Datarate;
		double Tx_power;
		double PathLossExponent;
		double StandardDeviation;		
		double FadingFigure;
		SYMBOL_PARAMETER* pstruSymbolParameter;
		NetSim_PACKET* struDLPacketList[No_of_Bursts_DL];
	};

	// variable to calculate the received power.
	double** CummulativeReceivedPower;

	//Functions prototype
	int fn_NetSim_WiMax_Init_F(struct stru_NetSim_Network *NETWORK_Formal,\
	NetSim_EVENTDETAILS *pstruEventDetails_Formal,char *pszAppPath_Formal,\
	char *pszWritePath_Formal,int nVersion_Type,void **fnPointer);
	int fn_NetSim_WiMax_Mobility(NETSIM_ID nDeviceId);
	int fn_NetSim_WiMax_AssociateSS(NETSIM_ID nDeviceId,NETSIM_ID nInterfaceId);
	int fn_NetSim_WiMax_OFDMInit(WIMAX_BS_PHY* bsPhy);
	int fn_NetSim_WiMax_BurstsInit(WIMAX_BS_MAC* BS_MAC,WIMAX_BS_PHY* BS_PHY);
	int fn_NetSim_WiMax_CalculateAndSetReceivedPower(NETSIM_ID BS_Id,NETSIM_ID IntId,NETSIM_ID SS_Id,NETSIM_ID InterfaceId);
	int fn_NetSim_WiMax_CalculateReceivedPower(NETSIM_ID nDeviceId, NETSIM_ID nInterfaceId,double Tx_power,double Distance,double Frequency,double PathLossExponent,double *ReceivedPower);
	int fn_NetSim_WiMax_CalculateShadowLoss(unsigned long* ulSeed1, unsigned long *ulSeed2, double* ShadowReceivedPower,double StandardDeviation);
	int fn_NetSim_WiMax_Calculate_SNR(NETSIM_ID BS_Id,NETSIM_ID IntId,NETSIM_ID SS_Id,NETSIM_ID InterfaceId);
	int fn_NetSim_WiMax_Mod(NETSIM_ID BS_Id,NETSIM_ID IntId,NETSIM_ID SS_Id,NETSIM_ID InterfaceId);

	double fn_NetSim_WiMax_FormFCH(double time);
	double fn_NetSim_WiMax_FormDL_MAP(double time);
	double fn_NetSim_WiMax_FormUL_MAP(double time);
	NetSim_PACKET* fn_NetSim_WiMax_GenerateBroadcastCtrlPacket(NETSIM_ID nDeviceId,NETSIM_ID nInterfaceId,MANAGEMENT_MESSAGE MessageType,double Time);
	double fn_NetSim_WiMax_CalculateTransmissionTime(NetSim_PACKET* Packet,WIMAX_BS_PHY* BS_Phy);
	int fn_NetSim_AddPacketToList(NetSim_PACKET** list,NetSim_PACKET* packet);

	int fn_NetSim_WiMax_Send_MAC_PDU();
	int fn_NetSim_WiMax_GetBurstNo(QUALITY_OF_SERVICE QOS);
	int fn_NetSim_WiMax_Update_DL_Map_IE();
	int fn_NetSim_WiMax_Receive_MAC_PDU();
	int fn_NetSim_Write_Phy_Out_SS(WIMAX_BS_MAC* BS_Mac, NetSim_PACKET* Packet, SYMBOL_PARAMETER* SymbolParameter,unsigned int SymbolRequired);
	int fn_NetSim_WiMax_Update_UL_Map_IE();
	int fn_NetSim_WiMax_Write_Phy_Out_BS(double fch_time,double dl_map_time,double ul_map_time);
	int fn_NetSim_Update_FCH(NetSim_PACKET* FCH_pkt,double Size,MANAGEMENT_MESSAGE Message);
	NetSim_PACKET* fn_NetSim_WiMax_GetPacketFromList(WIMAX_BS_PHY* BS_Phy,int index);
	
	char* fn_NetSim_WiMax_ConfigPacketTrace_F();
	int fn_NetSim_WiMax_Finish_F();
	int fn_NetSim_WiMax_Configure_F(void** var);
	int fn_NetSim_WiMax_Metrics_F(char* file);
	int fn_NetSim_WiMax_WriteBSMetrics(FILE *fp);
	int fn_NetSim_WiMax_FreePacket_F(NetSim_PACKET* packet);
	int fn_NetSim_WiMax_CopyPacket_F(const NetSim_PACKET* destPacket,const NetSim_PACKET* srcPacket);
	char* fn_NetSim_WiMax_Trace_F(int Subevent);

	int fn_NetSim_WiMax_BS_Mac_In();
	int fn_NetSim_WiMax_SS_Mac_In();
	int fn_NetSim_WiMax_BS_Physical_Out();
	int fn_NetSim_WiMax_SS_Physical_Out();
	int fn_NetSim_WiMax_Physical_In();
	
	int fn_NetSim_WiMax_BroadCastPacket(NetSim_PACKET* pstruPacket,NETSIM_ID nDevId,NETSIM_ID nInterface);
	int fn_NetSim_WiMax_TransmitPacket(NetSim_PACKET* pstruPacket,NETSIM_ID nDevId,NETSIM_ID nConDevId,NETSIM_ID nConInterface);
	int fn_NetSim_WiMax_TransmitP2PPacket(NetSim_PACKET* pstruPacket,NETSIM_ID nDevId,NETSIM_ID nInterface);
	
	NetSim_PACKET* fn_NetSim_WiMax_Fragment_Packet(double Size,NETSIM_ID DeviceId,NETSIM_ID InterfaceId);
	int fn_NetSim_Pack_Fragmented_Packet(NETSIM_ID DeviceId,NETSIM_ID InterfaceId);

	
	#ifdef  __cplusplus
}
#endif
#endif /* _NETSIM_WIMAX_H_ */

