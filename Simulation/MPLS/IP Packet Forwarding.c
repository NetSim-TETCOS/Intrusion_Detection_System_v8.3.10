//IP Packet Forwarding

 

 

#include<stdio.h>

#include<stdlib.h>

#include<string.h>

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

 

typedef struct stru_NP_Router_OSPF_LookupTable

{

            int n_NP_Router_OSPF_Cost;    //store the cost of the routing table

            int n_NP_Router_OSPF_Destination;     //store the destination of the routing table

            int n_NP_Router_OSPF_OutputPort;      //store the output port of the routing table

            int n_NP_Router_OSPF_NextHop;                //store the next hop of the routing table

            //int n_NP_Router_OSPF_Update;  //this field of finding the updation

            //IP address structure

            struct IPADDRESS1

            {

                         char ln_NP_Router_OSPF_IPAddress[MAXIP];

            }stru_NP_Router_OSPF_DestinationAddress,stru_NP_Router_OSPF_NextHopAddress;

            struct stru_NP_Router_OSPF_LookupTable *pstru_NP_Router_NextEntry;

}OSPFTABLE;

 

/* This function is used to forward the data from a device to another device by finding the output port

 * There are three types of operation

 * PUSH - It just insert the label into the IP Packet and transmit the packet to next device

 * SWAP - It just swap the label in the IP Packet and transmits the packet to next device

 * POP - It just remove the label from the IP Packet and transmits the packet to next device

 */

_declspec(dllexport)int fnNPMPLSDataForwarding(char lnFEC[MAXIP],char lnQoS[MAXIP],int *nLabelNumber,LFIB *pstruTempLFIB,OSPFTABLE *pstruOSPFTempTable )

{

            int nOutPort=0;

            // Do the following steps until the LFIB of current router becomes NULL

            while(pstruTempLFIB != NULL)

            {

                        if(strcmp(lnFEC,pstruTempLFIB->lnForwardingEquivalenceClass)==0)

                        { // if the given FEC is equal to the FEC of LFIB

                                    if( *nLabelNumber== 0 && strcmp(pstruTempLFIB->szOperation,"PUSH") ==0)

                                    { // if the label number is 0 and the operation is PUSH

                                                if(strcmp(pstruTempLFIB->lnQOS,lnQoS)==0)

                                                { // If the given QoS is equal to the QoS of LFIB

                                                            // Assign the label number of packet

                                                            *nLabelNumber = pstruTempLFIB->nOutgoingLabel;

                                                            // Assign the port number

                                                            nOutPort = pstruTempLFIB->nOutgoingInterface;

                                                            break; // Terminate the loop

                                                }

                                    }

                                    else if(*nLabelNumber == pstruTempLFIB->nIncomingLabel && strcmp(pstruTempLFIB->szOperation,"SWAP") ==0)

                                    { // If the label number is equal to incoming label and the operation is SWAP

                                                // Assign the label number

                                                *nLabelNumber = pstruTempLFIB->nOutgoingLabel;

                                                // Assign the port number

                                                nOutPort = pstruTempLFIB->nOutgoingInterface;

                                                break; // Terminate the loop

                                    }

                                    else if(strcmp(pstruTempLFIB->szOperation,"POP") ==0)

                                    { // If the operation is POP

                                                // Assign the label number as 0

                                                *nLabelNumber = 0;

                                                // Follow the steps until OSPF routing table becomes NULL

                                                while(pstruOSPFTempTable != NULL)

                                                {

                                                            if(strcmp(lnFEC,pstruOSPFTempTable->stru_NP_Router_OSPF_DestinationAddress.ln_NP_Router_OSPF_IPAddress)==0)

                                                            { // If the FEC is equal to the destination IP Address of OSPF routing table

                                                                        // Assign the port number

                                                                        nOutPort = pstruOSPFTempTable->n_NP_Router_OSPF_OutputPort;

                                                                        break; // Terminate the loop

                                                            }

                                                            // Move to next entry

                                                            pstruOSPFTempTable = pstruOSPFTempTable->pstru_NP_Router_NextEntry;

                                                }

                                                break; // Terminate the loop

                                    }

                        }

                        // Move to next entry

                        pstruTempLFIB = pstruTempLFIB->pstruNextentry;

            }

            // return the outport value

            return nOutPort;

}

 
