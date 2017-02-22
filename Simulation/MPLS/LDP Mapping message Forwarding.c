//LDP Mapping message Forwarding



 

#include<stdio.h>

#include<Stdlib.h>

#define MAXIP 50

 

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

 

/*This function is used to find the output port of the current router

 * Using the output port. the LDP label mapping message will be sent

 */

_declspec(dllexport)int fnNPMPLSLDPMappingForwarding(OSPFTABLE *pstru_NN_OSPF_RoutingTable,int nIngressRouterId)

{

 

            int nOutPort=0;

            //this loop will execute till the routing table becomes null

            while(pstru_NN_OSPF_RoutingTable!=NULL)

            {

                        // if the ip address of destination is equal to IP address in the table

                        if(pstru_NN_OSPF_RoutingTable->n_NP_Router_OSPF_Destination==nIngressRouterId)

                        {

                                                // set the output port

                                    nOutPort = pstru_NN_OSPF_RoutingTable->n_NP_Router_OSPF_OutputPort;

                                    break;//and terminate the loop

                        }

                        else

                        {

                                    // move to next entry

                                    pstru_NN_OSPF_RoutingTable=pstru_NN_OSPF_RoutingTable->pstru_NP_Router_NextEntry;

                        }

            }

            return nOutPort;

 

}

 
