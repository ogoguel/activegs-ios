/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#include "defc.h"
#include "sim65816.h"
#include "marinetti.h"


#ifdef NO_MARINETTI_SUPPORT
int process_marinetti_command(int _command,word32* param, word32& _regx, word32& _regy)
{	
	return TCPIP_ERR_NO_DEVICE;
}

void marinetti_init()
{
	printf("no marinetti support\n");
}

void marinetti_shutdown()
{

}

   
#else

/*
 05/6187: SEGMENT ARRIVE
 05/6905: ACK_FINWAIT2
 05/6945: ACK_CLOSING
 05/6cd5: PROCESSTHIS
 05/505f: TCPIPMangleDomainName
 05/4c70: TCPIPValidateI
 05/3f4d: PASC2HEXIP
 05/6DE0: BASICACK
 */



// Constantes

MAC_STRUCT server_mac_addr;
IP_STRUCT server_ip_addr;
IP_STRUCT server_identifier;
IP_STRUCT gateway_ip_addr;
MAC_STRUCT gateway_mac_addr;
IP_STRUCT gateway_dns_addr; 
IP_STRUCT gateway_mask_addr;
IP_STRUCT client_ip_addr;
IP_STRUCT local_ip_addr;
IP_STRUCT host_ip_addr;

// Globales

MAC_STRUCT client_mac_addr;
int eth_first,eth_last;
eth_queue	ethqueues[MAX_ETH_QUEUE];
tcp_queue	tcpqueues[MAX_TCP_QUEUE];
PConnection PConnections[MAX_TCP_CONNECTION];

double start_tcp_time;

void marinetti_init()
{
    
    MAC_SET(server_mac_addr,0xAB,0xAB,0xAB,0xAB,0xAB, 0xAB );
    IP_SET(server_ip_addr, 100,100,0,1 );
    IP_SET(server_identifier, 9,9,9,9 );
    IP_SET(gateway_ip_addr,128,0,0,254 );
    MAC_SET(gateway_mac_addr,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA );
    IP_SET(gateway_dns_addr, 208,67,222,222); // OPENDNS  
    IP_SET(gateway_mask_addr, 255,255,255,0 );
    IP_SET(client_ip_addr,128,0,0,100 );
    IP_SET(local_ip_addr,127,0,0,1 );
    IP_SET(host_ip_addr,128,0,0,1 );
    
    
}

void marinetti_shutdown()
{
    for(int i=0;i<MAX_TCP_CONNECTION;i++)
        PConnections[i].Reset();	
}


int  send_dhcp_response(tcp_queue* p)
{
	printf("building dhcp response\n");
	tcp_queue* packet = findTCPQueue();
	if (!packet)
		return 0;
    
	DHCP_HEADER* _dhcp = (DHCP_HEADER*)((const byte*)&p->udp+sizeof(UDP_HEADER));
	int dhcplen = ntohs(p->udp.udp_len)-sizeof(UDP_HEADER);
    
	packet->udp.udp_dst = p->udp.udp_src; // inverse
	packet->udp.udp_src = p->udp.udp_dst; // inverse
    
	DHCP_HEADER* target = (DHCP_HEADER*)((byte*)&packet->udp + sizeof(UDP_HEADER));
	// construit la réponse en UDP
	byte* response_str = (byte*)target;
    
	unsigned int pos= sizeof(DHCP_HEADER)-DHCP_OPTION_LEN;
	unsigned int parse= 0;
	const byte* dhcp_request = _dhcp->options;
    
	// vérifie le magic cookie
	if (
		dhcp_request[0]!=99
        ||	dhcp_request[1]!=130
        ||	dhcp_request[2]!=83
        ||	dhcp_request[3]!=99)
	{
		printf("invalid magic cookie\n");
		return 0;
	}
	memcpy(&response_str[pos],dhcp_request,4);
	parse+=4;
	pos+=4;
    
    while( parse < ( dhcplen - ( sizeof(DHCP_HEADER) - DHCP_OPTION_LEN) ) )
	{
		int cmd = dhcp_request[parse++];
		if (cmd==255) //-1
			break;
		int len = dhcp_request[parse++];
        
		switch(cmd)
		{
			case 53:	// messagetype
            {
                int type = dhcp_request[parse];					
                response_str[pos++]=53; 
                response_str[pos++]=1; 
                
                switch(type)
                {
                    case dhcpdiscover: 
                        printf(" ### [DHCP] dhcpdiscover");
                        response_str[pos++]=dhcpoffer;
                        break;
                    case dhcprequest: // request
                        printf(" ### [DHCP] dhcprequest");
                        response_str[pos++]=dhcppack;
                        break;
                        
                }
            }
				break;
                
			case 55:	// options
            {
                for(int l=0;l<len;l++)
                {
                    int c = dhcp_request[parse+l];
                    switch(c)
                    {
                        case 1: // subnetmask
                            response_str[pos++]=c;					
                            response_str[pos++]=4;
                            memcpy(&response_str[pos],&GET_INT_FROM_IP(gateway_mask_addr),4);
                            printf(" subnetmask:");
                            print_ip(gateway_mask_addr);
                            pos+=4;
                            break;
                        case 3: // routers
                            response_str[pos++]=c;					
                            response_str[pos++]=4;
                            memcpy(&response_str[pos],&GET_INT_FROM_IP(gateway_ip_addr),4);
                            printf(" routers:");
                            print_ip(gateway_ip_addr);
                            
                            pos+=4;
                            break;
                        case 6: // dns
                            response_str[pos++]=c;					
                            response_str[pos++]=4;
                            memcpy(&response_str[pos],&GET_INT_FROM_IP(gateway_dns_addr),4);
                            printf(" dns:");
                            print_ip(gateway_dns_addr);
                            pos+=4;
                            break;
                        case 26: // mtu
                            response_str[pos++]=c;					
                            response_str[pos++]=2;
                            response_str[pos++]=MTU/256;
                            response_str[pos++]=MTU%256;
                            break;
                    }
                }
            }
				break;
            case 54:	// server
                response_str[pos++]=54;					
                response_str[pos++]=4;
                printf(" server:");
                print_ip(server_identifier);
                memcpy(&response_str[pos],&GET_INT_FROM_IP(server_identifier),4);
                pos+=4;
				break;
            case 51:	// lease
                response_str[pos++]=54;					
                response_str[pos++]=4;
                response_str[pos++]=0;
                response_str[pos++]=0;
                response_str[pos++]=0;
                response_str[pos++]=0;
				break;
                
                
			default:
				break;
		}
		parse+=len;
	}
	response_str[pos++]=0xFF;	// end
    
	packet->udp.udp_len = htons(sizeof(UDP_HEADER) + pos);
    
	packet->ip.protocol = 17; // udp
	packet->ip.ver = 4;
    packet->ip.ihl = 5;
	packet->ip.ttl = 64;
	packet->ip.ucSource = server_ip_addr;
	packet->ip.ucDestination = client_ip_addr;
	packet->ip.length = htons(sizeof(IP_HEADER)+sizeof(UDP_HEADER) + pos);
	
	packet->eth.type = htons(0x0800); // IP
	packet->eth.dest_mac = client_mac_addr ;
	packet->eth.source_mac = server_mac_addr ;
	
	target->yiaddr = client_ip_addr;
	*(MAC_STRUCT*)&target->chaddr = client_mac_addr;
	target->xid = _dhcp->xid;
	packet->queuelen = sizeof(ETH_HEADER)+sizeof(IP_HEADER)+sizeof(UDP_HEADER) + pos;
	packet->iplen = sizeof(UDP_HEADER) + pos;
	packet->state = QS_TOSEND;
	return 1;
}


int process_marinetti_command(int _command,word32* _param, word32& _regx, word32& _regy)
{	
	
	switch(_command)
	{
        case TCPIP_CMD_STARTUP:
        {
			printf("TCPIP_CMD_STARTUP\n");
#ifdef _WIN32   
            int err;
            
            WORD wVersionRequested;
            WSADATA wsaData;
            
            wVersionRequested = MAKEWORD(2, 2);
            err = WSAStartup(wVersionRequested, &wsaData);
            if (err != 0)
            {
                printf("WSAStartup failed with error: %d\n", err);
                return TCPIP_ERR_STARTUP_FAILED;
            }
#endif
            
            return 0; 
        }
            break;
            
        case TCPIP_CMD_GET_DATAGRAM:
        {
            _param[1]=0;
            
            server_loop();
            
            // if (eth_first == eth_last)
            for(int i=0;i<MAX_TCP_CONNECTION;i++)
            {
                PConnection* p  = &PConnections[i];
                if (p && p->tcpsocket>=0)
                {
                    if (p->socktype == PCT_HOST && p->type==RT_UDP )// && p->udp_query)
                    {
                        
                        // regarde si il y a des données de dispo 
                        fd_set input_set;
                        timeval timeout;
                        timeout.tv_usec = 1;
                        timeout.tv_sec = 0;
                        FD_ZERO(&input_set);
                        FD_SET(p->tcpsocket,&input_set);
                        int s = select(p->tcpsocket+1,&input_set,NULL,NULL,&timeout);
                        if (s>0)
                        {
                            
                            byte recvbuf[MAX_UDP_SIZE];
                            int iResult = recv(p->tcpsocket, (char*)recvbuf, MAX_UDP_SIZE, 0);
                            if ( iResult > 0 )
                            {			
                                _LOG("Bytes received: %d\n", iResult);
                                //	p->udp_query--;
                                p->sendudppacket(iResult,recvbuf);
                            }
                            else if ( iResult == 0 )
                                printf("Connection closed\n");
                            else
                                printf("recv failed: %d\n",  SockLastError());
                        }
                        else
                            if (s==SOCKET_ERROR)
                                printf("select failed: %d\n", SockLastError());
                    }
                    else
                        if ( p->type==RT_TCP  && p->state == PCS_CLEARINGQUEUE)
                        {
                            // regarde si il y a encore des données à envoyer pour cette connection
                            int empty=1;
                            for(int q=0;q<MAX_TCP_QUEUE;q++)
                            {
                                if (tcpqueues[q].state != QS_IDLE && tcpqueues[q].connection == p )
                                {
                                    printf("*** tcp data remaing ***");
                                    empty=0;
                                    break;
                                }
                            }
                            if (eth_first != eth_last)
                            {
                                printf("*** eth data remaininng ***");
                                empty=0;
                                break;
                                
                            }
                            if (empty)
                            {
                                // on peut envoyer le dernier ack et fermer la connection
                                p->state = PCS_LASTACK;
                                p->sendtcppacket(TCP_ACK|TCP_FIN,0,NULL);
                                p->state_seq =  p->send_seq;
                                printf("\nLAST ACK SENT\n");
                            }
                            
                        }
                        else
                            if ( p->type==RT_TCP&& p->state == PCS_ESTABLISHED && (p->already_sent<(p->window-MAX_TCP_SIZE)))
                            {
                                // regarde si il y a des données TCP en entrée
                                // regarde si il y a des données de dispo 
                                fd_set input_set;
                                timeval timeout;
                                timeout.tv_usec = 1;
                                timeout.tv_sec = 0;
                                FD_ZERO(&input_set);
                                FD_SET(p->tcpsocket,&input_set);
                                int s = select(p->tcpsocket+1,&input_set,NULL,NULL,&timeout);
                                if (s>0)
                                {
                                    byte recv_data[MAX_TCP_SIZE];	// window max size
                                    int lr =recv(p->tcpsocket,(char*)recv_data,MAX_TCP_SIZE,0);
                                    if (lr>0)
                                    {
                                        //	p->udp_query--;
                                        if (lr>p->window)
                                            printf("*** overflow ***\n");
                                        
                                        p->sendtcppacket(TCP_ACK,lr,recv_data);
                                        
                                    }
                                }
                            }
                }
            }
            
            // ajout les TCP à la liste des datagrammes à envoyer
            
            for(int q=0;q<MAX_TCP_QUEUE;q++)
            {
                tcp_queue* tcp = &tcpqueues[q];
                if (tcp->state == QS_TOSEND||tcp->state == QS_TOSENDANDRETRY )
                {
                    printf("\n<<TCP -- ");
                    tcp->print();
                    
                    static int identification=1;
                    
                    int sent = 0;
                    identification++;
                    while(sent < tcp->iplen)
                    {
                        int tosend = tcp->iplen - sent;
                        int frag = 0;
                        
                        eth_queue* qdest = findETHQueue();
                        if (!qdest)
                        {
                            printf("\n *** no more queue ***\n");
                            // not sure how it will recover???
                            break;
                        }
                        
                        qdest->eth = tcp->eth;
                        qdest->ip = tcp->ip;
                        
                        if (tosend>ETH_MAX_SIZE)
                        {
                            frag = FLAG_MF;
                            tosend = ETH_MAX_SIZE;
                        }
                        
                        qdest->ip.flag_offset  = htons(frag + sent/8); 
                        qdest->ip.identification = identification;
                        qdest->ip.length = htons(tosend+sizeof(IP_HEADER));
                        qdest->ip.checksum = 0;
                        qdest->queuelen = tosend+sizeof(IP_HEADER)+sizeof(ETH_HEADER);
                        
                        memcpy(((byte*)&qdest->ip) + sizeof(IP_HEADER),((const byte*)&tcp->padding) + sent,tosend);
                        u16 chkip=	ip_sum_calc(sizeof(IP_HEADER),(const byte*)&qdest->ip);
                        qdest->ip.checksum = htons(chkip);
                        
                        sent += tosend;
                    }
                    
                    if (tcp->state == QS_TOSENDANDRETRY)
                    {
                        tcp->state = QS_SENT;
                        tcp->timeout =get_dtime();
                    }
                    else
                    {
                        tcp->state = QS_IDLE; // pas d'ack
                    }
                    
                }
                else
                    if (tcp->state == QS_SENT)
                    {
                        double tick = get_dtime();
                        if (tick - tcp->timeout>30.0)	// 5 000ms
                        {
                            if (tcp->connection->state != PCS_ESTABLISHED)
                            {
                                tcp->state = QS_IDLE;
                                printf("syn did not succeed...\n");
                                tcp->connection->Reset();
                            }
                            else
                            {
                                printf("\nretrying...");
                                tcp->state = QS_TOSENDANDRETRY;
                            }
                        }
                    }
            }
            
            // envoie les datagrammes (dans l'ordre)
            if (eth_first != eth_last)
            {
                eth_queue& q = ethqueues[eth_first];
                eth_first = (eth_first+1)% MAX_ETH_QUEUE;
                q.print();
                
                word32 p = _param[0];
                byte* pdata = (byte*)&g_memory_ptr[p];
                _param[1] = q.queuelen;
                memcpy((void*)pdata,&q.eth,_param[1]);
                *((byte*)pdata+q.queuelen) = 0;
                
            }
            
            return 0;
        }			
            break;
            
        case TCPIP_CMD_SEND_DATAGRAM:
		{
			
			word32 len = _param[1] & 0xFFFF;
			byte* pdata = ( byte*)&g_memory_ptr[_param[0]];
			*(pdata+len) = 0; // odd-even
            
			eth_queue* p = (eth_queue*)pdata;
            
#ifdef DEBUG_ETH
			int d = (int)((get_dtime()-start_tcp_time)*1000);
			printf("\n>>>[%07d]\n",d);
#endif
            
			int t = ntohs(p->eth.type);
			switch(t)
			{
				case 0x0806: // arp
				{
                    
					print_arp(&p->arp);
					eth_queue* packet = findETHQueue();
					if (!packet)
						return TCPIP_ERR_UNSUPPORTED;
                    
                    
					MAC_STRUCT target_mac;
					IP_STRUCT target_ip = p->arp.dst_protoaddr;
					if (GET_INT_FROM_IP(p->arp.dst_protoaddr) == GET_INT_FROM_IP(gateway_ip_addr))
						target_mac = gateway_mac_addr;
					else
						target_mac.SetFromIP(target_ip);
					
					packet->eth.type = ntohs(0x0806);
					packet->eth.dest_mac = client_mac_addr;
					packet->eth.source_mac = target_mac;	//gateway_mac_addr;
                    
					// copie la source en dest					
					packet->arp.dst_hwaddr = p->arp.src_hwaddr;
					packet->arp.dst_protoaddr = p->arp.src_protoaddr;
                    
					// remplis le destinataire
					packet->arp.src_hwaddr =target_mac; //gateway_mac_addr;
					packet->arp.src_protoaddr = target_ip; //gateway_ip_addr;
					packet->arp.hwaddr_format = p->arp.hwaddr_format;
					packet->arp.hwaddr_length = p->arp.hwaddr_length;
					packet->arp.opcode = htons(2); // reply
					packet->queuelen = sizeof(ARP_HEADER)+sizeof(ETH_HEADER);
                    
				}
                    break;
				case 0x0800: // IP
				{
					print_ip_packet(&p->ip);
					// ajoute les données dans unfragmented
					add_fragment(p);
				}
                    break;
				default:
					printf("unsupported protocol");
					return TCPIP_ERR_UNSUPPORTED;
			}
            
		}
            return 0;
            
        case TCPIP_CMD_CONNECT:
            for(int i=0;i<MAX_TCP_CONNECTION;i++)
                PConnections[i].Reset();
            for(int i=0;i<MAX_TCP_QUEUE;i++)
                tcpqueues[i].Reset();
            eth_first=eth_last = 0;
            start_tcp_time = get_dtime();
            init_server();
            return 0;
            
            // Disconnects from the network
        case TCPIP_CMD_DISCONNECT:
            for(int i=0;i<MAX_TCP_CONNECTION;i++)
                PConnections[i].Reset();	
            return 0;
            
        case TCPIP_CMD_SHUTDOWN:
#ifdef WIN32
            WSACleanup();
#endif
            return 0;
            
        case TCPIP_SET_IP_ADDRESS: 	
            client_mac_addr = (const byte*)_param;
            return 0;
            
            // returns the Macs IP Address in X and Y if IP Address could not be obtained will return 0
        case TCPIP_GETIPADDRESS:
            _regx=0;
            _regy=0;
            return 0;
            
        default:	
            break;
	}
	return TCPIP_ERR_UNSUPPORTED;
}


eth_queue* findETHQueue()
{
	int l = eth_last;
	l =  (l+1)%MAX_ETH_QUEUE;
	if (l==eth_first)
	{
		printf("\n*** no more ETH queues****\n");
		return NULL;
	}
	eth_queue* q = &ethqueues[eth_last];
	memset(q,0,sizeof(eth_queue));
	eth_last = l;
	return q;
}


tcp_queue* findTCPQueueByFragment(eth_queue* f)
{
	int free_frag = -1;
	tcp_queue* tcp= NULL;
	for(int q=0;q<MAX_TCP_QUEUE;q++)
	{
		tcp = &tcpqueues[q]; 
		if (tcp->state == QS_IDLE)
			free_frag = q;
        
		if (tcp->state == QS_FRAGMENT && tcp->ip.identification == f->ip.identification)
			return tcp;
	}
    
	if (free_frag == -1)
	{
		printf("\n*** no more TCP queues****\n");
		return NULL;
	}
	tcp = &tcpqueues[free_frag];
	memset(tcp,0,sizeof(tcp_queue));
	tcp->state = QS_FRAGMENT;
	tcp->eth = f->eth;
	tcp->ip = f->ip;
	return tcp;
	
}


tcp_queue* findTCPQueue()
{
    tcp_queue* q = NULL;
    for(int i=0;i<MAX_TCP_QUEUE;i++)
	{
		if (tcpqueues[i].state == QS_IDLE)
		{
            tcp_queue* q = &tcpqueues[i];
            memset(q,0,sizeof(tcp_queue));
            q->state = QS_IDLE;
            return q;
		}
	}
    printf("\n*** no more TCP queues****\n");
	return q;
}



int add_fragment(eth_queue* p)
{
	tcp_queue* f = findTCPQueueByFragment(p);	
	if (!f)
		return 0;
    
	// ajoute les données
	f->ip = p->ip;
	f->eth = p->eth;
    
	int ipdatalen = ntohs(p->ip.length) - sizeof(IP_HEADER);
    
	memcpy(f->padding+f->iplen,(const byte*)&p->ip+sizeof(IP_HEADER),ipdatalen);
	f->iplen += ipdatalen;
    
	int fo = ntohs(p->ip.flag_offset);
	if (!(fo & FLAG_MF))
	{
		process_unfragmented(f);
		f->state=QS_IDLE;
	}
    
	return 1;
}

int process_unfragmented(tcp_queue* p)
{
    
    int ip_protocol = p->ip.protocol;
    p->print();
	
    if (ip_protocol==17) // udp
    {
		
        int dest_port = ntohs(p->udp.udp_dst);
        int src_port = ntohs(p->udp.udp_src);
        
        if (p->isDHCP()) 
        {
            send_dhcp_response(p);
        }
        else
        {
            print_udp(&p->udp);
            
            // crée une nouvelle connection udp
            PConnection* pco = init_connection(PCT_HOST,RT_UDP, src_port,p->ip.ucDestination, dest_port, p->eth.dest_mac,0);
            if (pco==NULL)
            {
                printf("\ncannot init connection\n");
                return TCPIP_ERR_SEND;
            }
            
            // envoie la commande
            struct sockaddr_in server;
            server.sin_family = AF_INET;
            server.sin_addr = p->ip.ucDestination;
            server.sin_port = htons( dest_port );
            
            int len = ntohs(p->udp.udp_len)-sizeof(UDP_HEADER);
            const char* data = (const char*)&p->udp+sizeof(UDP_HEADER);
            // prépare la réponse
            int ret = sendto(pco->tcpsocket, data, len, 0, (struct sockaddr *) &server,sizeof(server) );
            if (ret< 0) 
            {
#ifdef _DEBUG
                int err = SockLastError();
                printf("socket send failed with err:%d\n",err);
#endif
                return TCPIP_ERR_SEND;
            }
        }
    }
    else
		if (ip_protocol==6) // tcp
		{
            
#ifdef _DEBUG
			int chk = p->check_tcp();
			if (!chk)
			{
				printf("ignoring packet\n");
				return 0;
			}
#endif
            
			int offset =p->tcp.data_offset*4;
            int tcplen  = p->iplen - offset;
            
			int dest_port = ntohs(p->tcp.dest_port);
			int	source_port = ntohs(p->tcp.src_port);
			struct in_addr dest_ip;
			dest_ip= p->ip.ucDestination;
            
			const byte *tcpdata = (const byte*)&p->tcp +offset;
            
			if (p->tcp.flags & TCP_RST)
			{
				printf("\n*** CONNECTION RESET ***\n");
				const char* reset_str = "HTTP/1.0 500"; 
				::send(p->connection->tcpsocket, (const char*)reset_str, strlen(reset_str), 0);
				reset_connection(PCT_HOST,RT_TCP,source_port,dest_ip,dest_port);
                
			}
			
			if (p->tcp.flags & TCP_ACK)
			{
				PConnection* ptcp = find_connection(PCT_HOST,RT_TCP,source_port,dest_ip, dest_port);
				if (ptcp)
				{
					// libère les query correspondantes
					int ack_seq = ntohl(p->tcp.ack_number);
                    
					for(int q=0;q<MAX_TCP_QUEUE;q++)
					{
						if (	tcpqueues[q].state == QS_SENT 
							&&	tcpqueues[q].queueseq <= ack_seq 
							&&	tcpqueues[q].connection == ptcp)
						{
							printf("- clearing %x (%d)",tcpqueues[q].queueseq,tcpqueues[q].tcplen);
							ptcp->already_sent -= tcpqueues[q].tcplen;
							if (ptcp->already_sent<0)
							{
								printf("too many acked!***\n");
								ptcp->already_sent=0; // pour essayer de se récupérer
							}
                            
							tcpqueues[q].state = QS_IDLE;
						}
					}
					if (ptcp->state == PCS_BEINGESTABLISHED && (ptcp->state_seq+1) == ack_seq)
					{
						ptcp->state = PCS_ESTABLISHED;
						ptcp->send_seq++;
						printf("\nCONNECTION ESTABLISHED (SERVER)\n");
					}
					else
                        if (ptcp->state == PCS_SYNSENT && (ptcp->state_seq+1) == ack_seq)
                        {
                            ptcp->sendtcppacket(TCP_ACK , 0, NULL);
                            ptcp->state = PCS_ESTABLISHED;
							ptcp->send_seq++;
                            printf("\nCONNECTION ESTABLISHED2 (CLIENT)\n");
                            
                        }
                        else
                            if (ptcp->state == PCS_SYNCHING && (ptcp->state_seq+1) == ack_seq)
                            {
                                ptcp->state = PCS_ESTABLISHED;
                                printf("\nCONNECTION ESTABLISHED (CLIENT)\n");
                                ptcp->send_seq++;
                            }
                            else
                                if (ptcp->state == PCS_CLOSING && ptcp->state_seq == ack_seq)
                                {
                                    // envoie notre data de fin
                                    ptcp->state = PCS_CLEARINGQUEUE;
                                    
                                }
                                else
                                    if (ptcp->state == PCS_LASTACK && ( ptcp->state_seq + 1) == ack_seq)
                                    {
                                        // envoie notre data de fin, mais n'attend pas de réponse
                                        ptcp->last_ack = ntohl(p->tcp.seq_number)+1;
                                        ptcp->sendtcppacket(TCP_ACK,0,NULL);
                                        ptcp->Reset();
                                        printf("\nCONNECTION CLOSED\n");
                                    }
                    
                    
				}
                
			}
			
			if (p->tcp.flags & TCP_FIN)	
			{
				PConnection* ptcp = find_connection(PCT_HOST,RT_TCP,source_port,dest_ip, dest_port);
				if (ptcp)
				{	
					printf("\nclose_wait\n");
                    
					ptcp->last_ack = ntohl(p->tcp.seq_number)+2;
					ptcp->sendtcppacket(TCP_ACK , 0, NULL);
                    ptcp->state_seq = ptcp->send_seq;
					ptcp->state = PCS_CLOSING;
				}
			}
            
			
			if (p->tcp.flags & TCP_SYN && !(p->tcp.flags & TCP_ACK))
			{
				PConnection* ptcp = init_connection(PCT_HOST,RT_TCP,source_port,dest_ip, dest_port,p->eth.dest_mac, ntohs(p->tcp.window));
				if (ptcp==NULL)
					return TCPIP_ERR_NO_CONNECTION;
                
				ptcp->last_ack = ntohl(p->tcp.seq_number)+1;
				ptcp->state = PCS_SYNCHING;
                ptcp->state_seq = ptcp->send_seq;
				ptcp->sendtcppacket(TCP_ACK|TCP_SYN, 0, NULL);
				
			}
            
			if (tcplen)
			{
				PConnection* ptcp = find_connection(PCT_HOST,RT_TCP,source_port,dest_ip, dest_port);
				if (ptcp==NULL)
					return TCPIP_ERR_NO_CONNECTION;
                
				int ret = send(ptcp->tcpsocket, (const char*)tcpdata, tcplen, 0);
				if (ret< 0) 
				{
#ifdef _DEBUG
					int err = SockLastError();
					printf("socket send failed with err:%d\n",err);
#endif
					return TCPIP_ERR_SEND;
				}
				
				// envoie l'ack
				ptcp->last_ack = ntohl( p->tcp.seq_number) +tcplen ;
				ptcp->sendtcppacket(TCP_ACK , 0, NULL);
                
			}
            
		}				
		else
		{
			printf("unsupported ip protocol\n");
			return TCPIP_ERR_UNSUPPORTED;
		}
    
	return 1;
}

#endif
