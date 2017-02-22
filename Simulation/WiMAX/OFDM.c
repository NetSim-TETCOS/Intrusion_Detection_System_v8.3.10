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

#include "main.h"
#include "WiMax.h"
int fn_NetSim_WiMax_OFDMInit(WIMAX_BS_PHY* bsPhy)
{
	double DLTime;
	double ULTime;
	double FrameDuration;
	SYMBOL_PARAMETER* pstruSymbol;
	bsPhy->pstruSymbolParameter = fnpAllocateMemory(1,sizeof *bsPhy->pstruSymbolParameter);
	pstruSymbol = bsPhy->pstruSymbolParameter;
	pstruSymbol->BW = bsPhy->ChannelBandwidth;
	FrameDuration = bsPhy->FrameDuration;
	if(fmod(bsPhy->ChannelBandwidth, 1.75) == 0.0)
	{
		pstruSymbol->SamplingFactor = 8/7.0;
	}
	else if (fmod(bsPhy->ChannelBandwidth, 2.5) == 0.0)
	{
		pstruSymbol->SamplingFactor = 7/6.0;
	}
	else
	{
		fnNetSimError("Unsupported Channel bandwidth %d for WiMax Protocol",bsPhy->ChannelBandwidth);
	}
	
	pstruSymbol->Fs = floor(pstruSymbol->SamplingFactor * (pstruSymbol->BW/8)*1000)*8000;
	
	/* Ref 8.3.2.4 Table 213 page 429 IEEE802.16-2004 */

	//Check phy layer stamdard and assign accordingly
	pstruSymbol->Nfft = 256;
	pstruSymbol->Nused = 200;
	pstruSymbol->DataSubCarrierCount = 192;
	pstruSymbol->G = bsPhy->GuardInterval;

	pstruSymbol->SubCarrierSpacing = pstruSymbol->Fs/pstruSymbol->Nfft;
	pstruSymbol->Usefulsymboltime = 1000000/pstruSymbol->SubCarrierSpacing; //(in microseconds)

	pstruSymbol->CPTime = (pstruSymbol->G * pstruSymbol->Usefulsymboltime);

	pstruSymbol->OFDMSymbolTime = pstruSymbol->Usefulsymboltime + pstruSymbol->CPTime;

	pstruSymbol->SamplingTime = pstruSymbol->Usefulsymboltime/pstruSymbol->Nfft;
	
	DLTime = bsPhy->DlFrameDuration*1000;
	if(DLTime + bsPhy->Tx_Transition_Gap < FrameDuration*1000)
		{
			ULTime = (FrameDuration* 1000- bsPhy->Tx_Transition_Gap) - DLTime;
		}
	else
		{
			ULTime =0;
			bsPhy->Tx_Transition_Gap =0;
		}

	pstruSymbol->NoofDLSymbols = (int) (DLTime/pstruSymbol->OFDMSymbolTime);
	pstruSymbol->NoofULSymbols = (int) (ULTime/pstruSymbol->OFDMSymbolTime);

	pstruSymbol->UPlinkFrameStartSymbol = pstruSymbol->NoofDLSymbols + (int)ceil(bsPhy->Tx_Transition_Gap/pstruSymbol->OFDMSymbolTime);
	return 1;
}
