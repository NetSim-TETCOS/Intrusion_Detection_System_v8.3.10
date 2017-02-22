//LFIB (Label Forwarding Information Base) Creation

 

//Code:

 

#include<stdio.h>

#include<stdlib.h>

#include<String.h>

#define MAXIP 50

 

/* This structure is for LFIB of router

 * It has FEC, next hop and Incoming label, Outgoing label, Incoming Interface, Outgoing Interface, types of operation elements

 * In our simulation, we have considered Destination IP Address as FEC and Source IP Address as QoS

 */

typedef struct LabelForwardingInformationBase

{

            char lnForwardingEquivalenceClass[MAXIP];

            int nIncomingLabel;

            int nIncomingInterface;

            char lnNextHop[MAXIP];

            int nOutgoingLabel;

            int nOutgoingInterface;

            char szOperation[MAXIP];

            char lnQOS[MAXIP];

            struct LabelForwardingInformationBase *pstruNextentry;

}LFIB;

 

/* This function is used add the entries in LFIB*/

_declspec(dllexport)int fnNPMPLSLFIBFormation(int nOperation,char lnFEC[MAXIP],int nIncomingPort,int nOugoingPort,char lnNextHop[MAXIP],int nIncomingLabel,int nOutgoingLabel,char lnQOS[MAXIP],LFIB **pstruMPLSLFIB,LFIB **pstruMPLSReferenceLFIB)

{

            // Create temporary object for LFIB structure

            LFIB *pstruTempLFIB;

            // Allocate memory for that sobject

            pstruTempLFIB = (LFIB*)malloc(sizeof(LFIB));

            // Allocate memory for operation

 

            if(nOperation == 1) // If 1-> operation is PUSH

                        strcpy(pstruTempLFIB->szOperation,"PUSH");

            else if(nOperation == 2) //If 2-> operation is SWAP

                        strcpy(pstruTempLFIB->szOperation,"SWAP");

            else if(nOperation == 3) // If 1-> operation is POP

                        strcpy(pstruTempLFIB->szOperation,"POP");

            else

                        strcpy(pstruTempLFIB->szOperation,"0");

            // Assign next hop

            strcpy(pstruTempLFIB->lnNextHop,lnNextHop);

            // Assign Incoming port

            pstruTempLFIB->nIncomingInterface = nIncomingPort;

            // Assign outgoing port

            pstruTempLFIB->nOutgoingInterface = nOugoingPort;

            // Assign incoming label

            pstruTempLFIB->nIncomingLabel = nIncomingLabel;

            // Assign outgoing label

            pstruTempLFIB->nOutgoingLabel = nOutgoingLabel;

            // Assign FEC

            strcpy(pstruTempLFIB->lnForwardingEquivalenceClass,lnFEC);

            // Assign Qos

            strcpy(pstruTempLFIB->lnQOS,lnQOS);

            pstruTempLFIB->pstruNextentry = NULL;

 

            // Assign the temporary object in the LFIB of current router

            if(*pstruMPLSLFIB == NULL)

            {

                        *pstruMPLSLFIB = pstruTempLFIB;

                        *pstruMPLSReferenceLFIB= pstruTempLFIB;

            }

            else

            {

                        (*pstruMPLSReferenceLFIB)->pstruNextentry = pstruTempLFIB;

                        *pstruMPLSReferenceLFIB = pstruTempLFIB;

            }

            return 0;

}

 
