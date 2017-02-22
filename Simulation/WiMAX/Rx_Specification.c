#include "main.h"
#include "WiMax.h"

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
********************* 802.16d ************************
Index,SNR,Modulation,CodingRate,NBPSC,BitsCountInOneSymbol
NBPSC - Coded bits per subcarrier
page-492 Table 266—Receiver SNR
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/


struct stru_802_16_Phy_Parameters struPhyParameters[MAX_PHY_PARAMETER]=
{
	{0,	6.4,	Modulation_BPSK,	Coding_1_2,		1,	96},
	{1,	9.4,	Modulation_QPSK,	Coding_1_2,		2,	192},
	{2,	11.2,	Modulation_QPSK,	Coding_3_4,		2,	288},
	{3,	16.4,	Modulation_16_QAM,	Coding_1_2,		4,	384},
	{4,	18.2,	Modulation_16_QAM,	Coding_3_4,		4,	576},
	{5,	22.4,	Modulation_64_QAM,	Coding_2_3,		6,	768},
	{6,	24.4,	Modulation_64_QAM,	Coding_3_4,		6,	864},
};	