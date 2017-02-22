//LDP Request message Forwarding

 


 

#include<stdio.h>

#include<Stdlib.h>

#include<string.h>

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

 

/* In this function output port of the current router is found

 * The output port is driven form Routing table of current router

 */

_declspec(dllexport)int fnNPMPLSLDPRequsetForwarding(OSPFTABLE *pstru_NN_OSPF_RoutingTable,char lnFEC[MAXIP])

{

            int nOutPort=0;

            //this loop will execute till the routing table becomes null

            while(pstru_NN_OSPF_RoutingTable!=NULL)

            {

                        // if the ip address of destination is equal to IP address in the table

                        if(strcmp(pstru_NN_OSPF_RoutingTable->stru_NP_Router_OSPF_DestinationAddress.ln_NP_Router_OSPF_IPAddress,lnFEC) == 0)

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

            // return the output port value

            return nOutPort;

 

}

 
