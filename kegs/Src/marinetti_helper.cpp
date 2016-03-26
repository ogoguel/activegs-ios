/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#include "defc.h"
#include "sim65816.h"
#include "marinetti.h"

#ifndef NO_MARINETTI_SUPPORT

const MAC_STRUCT broadcast_mac_addr={ 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF };


void print_ip(IP_STRUCT _p)
{
#ifdef _DEBUG
	const byte*p = (const byte*)&_p;
	printf("%3d.%3d.%3d.%3d",p[0],p[1],p[2],p[3]);
#endif
}

void print_mac(MAC_STRUCT _p)
{
#ifdef _DEBUG
	const byte*mac = (const byte*)&_p;
    printf("%0X.%0X.%0X.%0X.%0X.%0X",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
#endif
}

int dns_readname(const  byte* _reader,const  byte* _buffer,char _outname[] )
{
	unsigned char *name = (unsigned char*)_outname;
	const unsigned char *reader = (const unsigned char*)_reader;
	const unsigned char *buffer = (const unsigned char*)_buffer;

	unsigned int p=0,jumped=0,offset;
	int i , j;

	int count = 1;

	name[0]='\0';

	//read the names in 3www6google3com format
	while(*reader!=0)
	{
		if(*reader>=192)
		{
			offset = (*reader)*256 + *(reader+1) - 49152; //49152 = 11000000 00000000 ;)
			reader = buffer + offset - 1;
			jumped = 1; //we have jumped to another location so counting wont go up!
		}
		else
		{
			name[p++]=*reader;
		}

		reader=reader+1;

		if(jumped==0) count = count + 1; //if we havent jumped to another location then we can count up
	}

	name[p]='\0'; //string complete
	if(jumped==1)
	{
		count = count + 1; //number of steps we actually moved forward in the packet
	}

	//now convert 3www6google3com0 to www.google.com
	for(i=0;i<(int)strlen((const char*)name);i++)
	{
		p=name[i];
		for(j=0;j<(int)p;j++)
		{
			name[i]=name[i+1];
			i=i+1;
		}
		name[i]='.';
	}

	name[i-1]='\0'; //remove the last dot
	return count;
}


u16 ip_sum_calc(u16 len_ip_header,const byte* _buf)
{
u16 word16;
u32 sum=0;
u16 i;

	const byte* buff = _buf;
	for (i=0;i<len_ip_header;i=i+2)
	{
		word16 =((buff[i]<<8)&0xFF00)+(buff[i+1]&0xFF);
		sum = sum + (u32) word16;
	}
	while (sum>>16)
		sum = (sum & 0xFFFF)+(sum >> 16);		

	sum = ~sum;
	return ((u16) sum);
}

u16 udp_sum_calc(u16 _proto,u16 len_udp, const byte* src_addr,const byte* dest_addr, const byte* buff)
{
u16 word16;
u32 sum=0;	

	int i;
	for (i=0;i<len_udp;i=i+2)
	{
		word16 =((buff[i]<<8)&0xFF00)+(buff[i+1]&0xFF);
		sum = sum + (u32)word16;
	}

    while (sum>>16)
		sum = (sum & 0xFFFF)+(sum >> 16);

	for (i=0;i<4;i=i+2)
	{
		word16 =((src_addr[i]<<8)&0xFF00)+(src_addr[i+1]&0xFF);
		sum=sum+ (u32)word16;	
		word16 =((dest_addr[i]<<8)&0xFF00)+(dest_addr[i+1]&0xFF);
		sum=sum+(u32)word16; 	
	}

	sum = sum + _proto + len_udp;
    while (sum>>16)
		sum = (sum & 0xFFFF)+(sum >> 16);
	sum = ~sum;

	

	return ((u16) sum);
}
 

void print_dns(const byte* _ptr)
{
	DNS_HEADER* dns = (DNS_HEADER*)_ptr;

	char namestr [256];
	const byte* name = (const byte*)(_ptr+sizeof(DNS_HEADER));
	dns_readname(name,_ptr, namestr);
	
#ifdef _DEBUG
	QUESTION* q = (QUESTION*)(_ptr+strlen((const char*)name)+1+sizeof(DNS_HEADER));

	printf("nb_question:%d nb_answer;%d\n", ntohs(dns->q_count), ntohs(dns->ans_count));
	printf("dns query : %s\n",namestr);
	printf("question type:%d class: %d\n", ntohs(q->qtype), ntohs(q->qclass));
#endif
	RES_RECORD answers[20];//s,auth[20],addit[20]; //the replies from the DNS server

	//reading answers
		int stop=0;
		int i;
		byte* reader = (byte*)(_ptr+strlen((const char*)name)+1+sizeof(DNS_HEADER)+sizeof(QUESTION));
		byte* buf = (byte*)_ptr;

		for(i=0;i<ntohs(dns->ans_count);i++)
		{
			stop = dns_readname(reader,buf,answers[i].name);
			reader+=stop;
			answers[i].resource=(R_DATA*)(reader);
			reader+=sizeof(R_DATA);
			if(ntohs(answers[i].resource->type)==1)
			{
				for(int j=0;j<ntohs(answers[i].resource->data_len);j++)
					answers[i].rdata[j]=reader[j];
				answers[i].rdata[ntohs(answers[i].resource->data_len)]=0;
				reader+=ntohs(answers[i].resource->data_len);
			}
			else
			{
				stop = dns_readname(reader,buf,answers[i].rdata);
				reader+=stop;
			}
		}

	for(i=0;i<ntohs(dns->ans_count);i++)
	{
		printf(" ### [DNS] Reply -- Name : %s ",answers[i].name);

		if(ntohs(answers[i].resource->type)==1) //IPv4 address
		{
			long *p;
			p=(long*)answers[i].rdata;
			sockaddr_in a;
			a.sin_addr.s_addr=(*p); //working without ntohl
			printf("has IPv4 address : %s",inet_ntoa(a.sin_addr));
		}
		if(ntohs(answers[i].resource->type)==5) //Canonical name for an alias
		{
			printf("has alias name : %s",answers[i].rdata);
		}

		printf("\n");
	}

	if (i==0)
		printf(" ### [DNS] Query -- %s",namestr);

}

void print_udp(const UDP_HEADER *p)
{
	printf(" ### [UDP] ");
	printf("len:%d, src: %d, dest %d ",ntohs(p->udp_len),ntohs(p->udp_src),ntohs(p->udp_dst));
}


void print_arp(const ARP_HEADER * p)
{
#ifdef DEBUG_ARP
	int arp_type = ntohs(p->opcode);
	printf("[ARP] -- type %d, ",arp_type);
	printf(", src:");
	print_ip(p->src_protoaddr);
	printf("/");
	print_mac(p->src_hwaddr);
	printf(", dest:");
	print_ip(p->dst_protoaddr);
	printf("/");
	print_mac(p->dst_hwaddr);
#endif
}

void print_tcp(TCP_HEADER* p)
{

#ifdef _DEBUG
	printf(" ### [TCP]");
    printf(" src_port:%d dst_port:%d",ntohs(p->src_port),ntohs(p->dest_port));
	printf(" s:%x",ntohl(p->seq_number));

	if (p->flags & TCP_SYN)
		printf(" SYN");
	if (p->flags & TCP_ACK)
	{
		printf(" ACK");
		printf("%x ",ntohl(p->ack_number));
	}
	if (p->flags & TCP_RST)
		printf(" RST");
	if (p->flags & TCP_FIN)
		printf(" FIN");
	if (p->flags & TCP_PSH)
		printf(" PSH");
	if (p->flags & TCP_URG)
		printf(" URG");
#endif
}


int tcp_queue::check_tcp()
{
	TCP_HEADER* p = &this->tcp;

	int len = this->iplen; 

	u16 chk = udp_sum_calc(6,len,(const byte*)&GET_INT_FROM_IP(this->ip.ucDestination),(const byte*)&GET_INT_FROM_IP(this->ip.ucSource),(const byte*)p);
	if (chk!=0 && p->checksum)
	{
		printf("tcp checksum errror!");
		return 0;
	}

	return 1;
}


void print_ip_packet(const IP_HEADER* p)
{
#ifdef DEBUG_IP
	int l =ntohs(p->length);
	printf("[IP] -- len:%3d", l);
    printf(" src_ip:");
	print_ip(p->ucSource);
	printf(" dest_ip:");
	print_ip(p->ucDestination);
	
	short unsigned int flag_offset = ntohs(p->flag_offset);
	if (flag_offset & FLAG_MG)
		printf(" MF");
	printf("\n");
#endif
}


PConnection::PConnection()
{
	memset(this,0,sizeof(PConnection));
	tcpsocket=-1;
}

void PConnection::Reset()
{
	// regarde si il y a encore des données à envoyer pour cette connection
	
	for(int q=0;q<MAX_TCP_QUEUE;q++)
	{
		if (tcpqueues[q].state != QS_IDLE && tcpqueues[q].connection == this )
		{
			printf("*** reset queue : %x  ***\n",tcpqueues[q].queueseq);
			tcpqueues[q].state = QS_IDLE;
		}
	}
	
	SockClose(tcpsocket);
	state = PCS_CLOSED;
}

PConnection::~PConnection()
{
	Reset();
	
}

PConnection* init_connection(enum PConnectionType _socktype,int _type, int _srcport,IP_STRUCT _destip, int _destport, MAC_STRUCT _destmac, int _window)
{
	if (_type != RT_TCP && _type != RT_UDP)
	{
		printf("type mismatch\n");
		return NULL;
	}

	PConnection* p = find_connection(_socktype,_type,_srcport,_destip,_destport);
	if (p)
	{
		if (p->tcpsocket>0)
            return p;
	}
	else
	for(int tcpindex=0;tcpindex<MAX_TCP_CONNECTION;tcpindex++)
	{
		if (PConnections[tcpindex].tcpsocket ==-1)
		{
			p = &PConnections[tcpindex];
			p->source_port = _srcport;
			p->dest_port = _destport;
			p->dest_ip = _destip;
			p->dest_mac = _destmac;
			p->window = _window;
			p->already_sent = 0;
			p->type = _type;
            p->socktype = _socktype;
			p->state=PCS_CLOSED;
			
			static int la = 0x300;

			p->last_ack = la;
			p->send_seq = p-> last_ack;
			la += 0x010000;

			printf("\nconnection created ");
			print_ip(_destip);
			printf("\n");
			break;
		}
	}
	if (p==NULL)
	{
		printf("\n *** too many open connections ****\n");
		return NULL;
	}

    
    if (_socktype == PCT_CLIENT)
    {
        p->tcpsocket = _window;
        p->window = 16384;
        p->state=PCS_SYNSENT;
		p->state_seq = p->send_seq;
        p->sendtcppacket(TCP_SYN,0,NULL);
        
        return p;
		
    }
    
	if (_type==RT_TCP) // TCP
	{
		p->tcpsocket =  socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (p->tcpsocket < 0)  
		{
			printf("cannot create socket\n");
			return NULL;
		}
		// envoie la commande
		struct sockaddr_in server;
		server.sin_family = AF_INET;
        if (GET_INT_FROM_IP(_destip)==GET_INT_FROM_IP(host_ip_addr))
            server.sin_addr = local_ip_addr; // host loop back
        else
            server.sin_addr = _destip;
		server.sin_port = htons( _destport );
		 memset(&(server.sin_zero),0,8); 

		int err = connect(p->tcpsocket,(struct sockaddr *) &server,sizeof(server) );
		if (err < 0)
		{
			printf("connect send failed with err:%d\n",SockLastError());
			p->Reset();
			return NULL;
		}
		p->state=PCS_INITIALIZED;

	}
	else
	{
		// udp
		p->tcpsocket =  socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (p->tcpsocket < 0)
		{
			printf("cannot open udp socket\n");
			p->Reset();
			return NULL;
		}
		
		p->state=PCS_INITIALIZED;
	}
	return p;
		
};

int PConnection::sendudppacket(int _len, const byte* _data)
{
	tcp_queue* packet = findTCPQueue();
	if (!packet) return 0;

	// UDP
	int udp_len = sizeof(UDP_HEADER)+_len;
	packet->udp.udp_len = htons(udp_len);
	packet->udp.udp_dst = htons(source_port);	// inverse
	packet->udp.udp_src = htons(dest_port);
	memcpy((byte*)&packet->udp+sizeof(UDP_HEADER),_data,_len);
	u16 chk = udp_sum_calc(17,udp_len,(const byte*)&dest_ip,(const byte*)&client_ip_addr,(const byte*)&packet->udp);
	packet->udp.udp_chk = htons(chk);


	// IP
	packet->ip.ucSource = dest_ip;
	packet->ip.ucDestination = client_ip_addr;
	packet->ip.protocol = 17; // udp
	packet->ip.ver = 4;
    packet->ip.ihl = 5;
	packet->ip.ttl = 64;


	int iplen = sizeof(IP_HEADER) + udp_len ;
	packet->ip.length = htons(iplen);
	u16 chkip=	ip_sum_calc(sizeof(IP_HEADER),(const byte*)&packet->ip);
	packet->ip.checksum = htons(chkip);

	// ETH
	packet->eth.dest_mac = client_mac_addr;
	packet->eth.source_mac = dest_mac;
	packet->eth.type = htons(0x0800); //IP
	
	int eth_len = iplen+sizeof(ETH_HEADER);
	packet->queuelen = eth_len ;
	packet->iplen = udp_len;
	packet->state = QS_TOSEND;
	return eth_len;

}

tcp_queue*	PConnection::sendtcppacket(int _flags,int _len, byte* _data)
{
	
	tcp_queue* packet = findTCPQueue();
	if (!packet) return 0;

	// TCP
	const int offset = 5;
	int tcp_header_len = offset*4;
	int tcp_len = tcp_header_len + _len;

	packet->tcp.flags = _flags ; 
	packet->tcp.ack_number = htonl(last_ack);

	packet->tcp.seq_number = htonl (send_seq );
	packet->tcp.window = htons(16384);
	packet->tcp.data_offset = offset;
	packet->tcp.dest_port = htons(source_port);	// inverse
	packet->tcp.src_port = htons(dest_port);
	memcpy((byte*)&packet->tcp+tcp_header_len,_data,_len);

	u16 chk = udp_sum_calc(6,tcp_len,(const byte*)&dest_ip,(const byte*)&client_ip_addr,(const byte*)&packet->tcp);
	packet->tcp.checksum = htons(chk);

	// IP
	packet->ip.ucSource = dest_ip;
	packet->ip.ucDestination = client_ip_addr;
	packet->ip.protocol = 6;
	packet->ip.ver = 4;
    packet->ip.ihl = 5;
	packet->ip.ttl = 64;


	int iplen = sizeof(IP_HEADER) + tcp_len ;
	packet->ip.length = htons(iplen);
	u16 chkip=	ip_sum_calc(sizeof(IP_HEADER),(const byte*)&packet->ip);
	packet->ip.checksum = htons(chkip);

	// ETH
	packet->eth.dest_mac = client_mac_addr;
	packet->eth.source_mac = dest_mac;
	packet->eth.type = htons(0x0800);

	int eth_len = iplen+sizeof(ETH_HEADER);
	packet->queuelen = eth_len ;
	packet->queueseq = send_seq+_len;
	packet->state = QS_TOSENDANDRETRY;
	packet->connection = this;
	
	packet->tcplen = _len;
	packet->iplen = tcp_len;
	send_seq+=_len;
	already_sent += _len;

	if (already_sent > window)
	{
		printf("****window error****\n");
	}
	return packet;	// nb d'octect envoyés
}

PConnection*  find_connection(enum PConnectionType _socktype,int _type,int _srcport,IP_STRUCT _destip, int _destport)
{
	for(int i=0;i<MAX_TCP_CONNECTION;i++)
	{
		PConnection* p  = &PConnections[i];
		if (
			p
			&&
			(p->type == _type)
			&&			
            (p->source_port == _srcport)
			&&
			(p->dest_port == _destport)
			&&
			(GET_INT_FROM_IP(p->dest_ip) == GET_INT_FROM_IP(_destip))
			&& 
			(p->state != PCS_CLOSED)
			)
			return p;
	}
	return NULL ;

}

void reset_connection(enum PConnectionType _socktype, int _type,int _srcport,IP_STRUCT _destip, int _destport)
{
	PConnection* p = find_connection(_socktype,_type,_srcport,_destip,_destport);
	if (p)
		p->Reset();
	else
		printf("ignoring reset on unknown connection\n");
}

void eth_queue::print()
{
	
#ifdef DEBUG_ETH
	if (this->eth.type==htons(0x0806))
		print_arp(&this->arp);
	else
	{
		print_ip_packet(&this->ip);
		int frag_offset = ntohs(this->ip.flag_offset);
		if (frag_offset & FLAG_MF)
				printf(" F");

		printf("id:%x offset:%d ",this->ip.identification,GET_OFFSET(frag_offset));
		switch(this->ip.protocol)
		{
			case 17: // udp
				printf("/UDP");
				break;
			case 6: // tcp
				printf("/TCP");
				break;
			default:
				break;
		}
	}
	printf("\n");
#endif
}


void tcp_queue::print()
{

	if (this->ip.protocol == 17 ) // UDP
	{
		print_udp(&this->udp);
			if (isDNS())				
				print_dns((const byte*)&this->udp+sizeof(UDP_HEADER));
	}
	else
	if (this->ip.protocol == 6 ) // TCP
	{


		print_tcp(&this->tcp);
		
		int offset =this->tcp.data_offset*4;
		int l = ntohs(this->ip.length);
		int tcplen = l - sizeof(IP_HEADER) - offset;
		if (tcplen)
			printf(" [tcplen:%d]",tcplen);
	}
	else
		printf("unknown protocol : %d",this->ip.protocol);
			printf("\n");
}



// SERVER

int server_sock;

int init_server(void)
{
  //  nb_client = 0;
    
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    
    if(server_sock == SOCKET_ERROR)
    {
        printf("server init failed\n");
        return 0;
    }
    
    struct sockaddr_in sin;
    
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(8080);
    sin.sin_family = AF_INET;
    
    if(bind(server_sock,(struct sockaddr *)&sin, sizeof sin) == SOCKET_ERROR)
    {
        printf("cannot bind\n");
        return 0;
    }
    
    if(listen(server_sock, 0) == SOCKET_ERROR)
    {
        printf("cannot listent\n");
        return 0;    
    }
    
    
    
    return 1;
}

void server_loop(void)
{
 	for(int i=0;i<MAX_TCP_CONNECTION;i++)
		{
			PConnection* p  = &PConnections[i];
			if (p->state != PCS_CLOSED) // *** ON ATTEND LA FIN DE LA CONNECTION => BCP PLUS RAPIDE??
			{
				// server not ready to accept new connection		
				return ;
			}
		}

    int max = server_sock;
    
    fd_set rdfs;
    
 
    FD_ZERO(&rdfs);
        
     // add the connection socket 
    FD_SET(server_sock, &rdfs);
      
    timeval timeout;
    timeout.tv_usec = 1;
    timeout.tv_sec = 0;
    
    if(select(max + 1, &rdfs, NULL, NULL, &timeout) == -1)
    {
 //       printf("failed select\n");
        return ;
    }
        
    if(FD_ISSET(server_sock, &rdfs))
    {
        // new client
        struct sockaddr csin = { 0 };
        socklen_t sinsize = sizeof(struct sockaddr);
        int csock = accept(server_sock, &csin, &sinsize);
        if(csock == SOCKET_ERROR)
        {
            printf("failed accept\n");;
            return;
        }
            
            
        max = csock > max ? csock : max;
            
        FD_SET(csock, &rdfs);
        IP_STRUCT *ip_client = (IP_STRUCT*)&csin.sa_data[0];
        MAC_STRUCT mac_client;
        mac_client.SetFromIP(*ip_client);
		

		for(int i=0;i<MAX_TCP_CONNECTION;i++)
		{
			PConnection* p  = &PConnections[i];
			if (p->state == PCS_SYNSENT)
			{
				printf("\n*** INGORING %d from ",csock);
				print_ip(*ip_client);
				printf("\n");
				return ;
			}
		}
	
		printf("\n*** connection %d from ",csock);
		print_ip(*ip_client);
		printf("\n");

		// génère un port random pour ne pallier le IPOPENUSER qui ne s base que sur les ports!
		static int free_port=100;
		init_connection(PCT_CLIENT,RT_TCP, 80, *ip_client, free_port++, mac_client,csock);
		
	}
  
}

#endif