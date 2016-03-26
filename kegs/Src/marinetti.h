/*
 ActiveGS, Copyright 2004-2016 Olivier Goguel, https://github.com/ogoguel/ActiveGS
 Based on Kegs, Copyright 2004 Kent Dickey, https://kegs.sourceforge.net
 This code is covered by the GNU GPL licence
*/

#pragma once

#if defined(ACTIVEGS_ANDROID)
    // not tester yet!
    #define NO_MARINETTI_SUPPORT
#endif

#ifndef NO_MARINETTI_SUPPORT
    #ifdef WIN32
        #include <winsock2.h>
        #include <ws2tcpip.h>
        #include <stdio.h>
        #pragma comment(lib, "Ws2_32.lib")
    #else
        #include <sys/types.h>
        #include <sys/socket.h>
        #include <netinet/in.h>
        #include <netinet/in_systm.h>
        #include <netinet/ip.h>
        #include <netinet/tcp.h>
        #include <arpa/inet.h>
    #endif 
#endif // NO MARINETTI SUPPORT



#define MAX_TCP_QUEUE   32
#define MAX_ETH_QUEUE	64

#define MAX_TCP_SIZE 2048 
#define MAX_UDP_SIZE 2048  
#define MTU	1500


#define TCPIP_ERR_NO_DEVICE		1	// No network device found
#define TCPIP_ERR_NO_CONNECTION	2	// Unable to connect to network device
#define TCPIP_ERR_STARTUP_FAILED	3	// Unable to initiate network access
#define TCPIP_ERR_UNSUPPORTED		4	// An unsupported operation was attempted
#define TCPIP_ERR_MEMORY			5	// Unable to allocate buffer memory
#define TCPIP_ERR_SEND			6	// Error sending packet

extern void marinetti_init();
extern void marinetti_shutdown();


#ifndef NO_MARINETTI_SUPPORT

// helpers

typedef unsigned short u16;
typedef unsigned long u32;


#pragma pack(push)
#pragma pack(1)

#ifdef WIN32
#define SockLastError() WSAGetLastError()
#define SockClose(X) if (X>=0) { closesocket(X); printf("\n sock %d closed\n",X); X=-1; }
#else
#define SockLastError() -1
#define SockClose(X) if (X>=0) { close(X); X=-1; }
#define SOCKET_ERROR -1
#endif


#define _LOG(...) do {} while(0)
#ifndef _DEBUG
    #undef printf
    #define printf(...) do {} while(0)
#endif

enum enum_response_type
{
	RT_NA=0,
	RT_UDP=100,
	RT_ARP=101,
	RT_DHCP=102,
	RT_DNS=103,
	RT_TCP=104
};

// ETH/IP/UDP/ARP/TCP/DHCP HEADER

#define dhcpdiscover  1
#define dhcpoffer 2
#define dhcprequest 3
#define dhcpdecline 4
#define dhcppack  5
#define dhcpnack  6
#define dhcprelease 7
#define dhcpinform  8

// DNS STRUCT
typedef in_addr IP_STRUCT;

#ifdef WIN32
#define GET_INT_FROM_IP(X) X.S_un.S_addr
#else
#define GET_INT_FROM_IP(X) X.s_addr
#endif

struct MAC_STRUCT
{

	byte addr[6];	MAC_STRUCT& operator=(const byte* _ptr) 
	{
		memcpy(addr,_ptr,6);
		return *this;
	};

	void SetFromIP(IP_STRUCT _ip)
	{
		memset(addr,0x88,6);
		*(int*)&addr[0]=GET_INT_FROM_IP(_ip);
	};
} ;


#define IP_SET(i,d,c,b,a)  GET_INT_FROM_IP(i) = ((a << 24) + (b << 16) + (c << 8) + d)
#define MAC_SET(m,a,b,c,d,e,f)  m.addr[0]=a;m.addr[1]=b;m.addr[2]=c;m.addr[3]=d;m.addr[4]=e;m.addr[5]=f;


typedef struct 
{
	MAC_STRUCT dest_mac;
	MAC_STRUCT source_mac;
	unsigned short int type;
} ETH_HEADER;


#define FLAG_MF 0x2000
#define GET_OFFSET(X) ((X & 0x1FFF)*8)

typedef struct 
{
	 unsigned char ihl:4;
    unsigned char ver:4;
    unsigned char tos;
    unsigned short int length;
    unsigned short int identification;
	unsigned short int flag_offset;
//	unsigned short int offset:13;
    unsigned char ttl;
    unsigned char protocol;
    unsigned short int checksum;
    IP_STRUCT ucSource;
    IP_STRUCT ucDestination;
}  IP_HEADER;

#define DHCP_SNAME_LEN      64
#define DHCP_FILE_LEN       128
#define DHCP_FIXED_NON_UDP  236
#define DHCP_OPTION_LEN     1500
 
typedef struct 
{
    byte  op;       /* 0: Message opcode/type */
    byte  htype;    /* 1: Hardware addr type (net/if_types.h) */
    byte  hlen;     /* 2: Hardware addr length */
    byte  hops;     /* 3: Number of relay agent hops from client */
    u32 xid;      /* 4: Transaction ID */
    u16 secs;     /* 8: Seconds since client started looking */
    u16 flags;    /* 10: Flag bits */
    IP_STRUCT ciaddr;  /* 12: Client IP address (if already in use) */
    IP_STRUCT yiaddr;  /* 16: Client IP address */
    IP_STRUCT siaddr;  /* 18: IP address of next server to talk to */
    IP_STRUCT giaddr;  /* 20: DHCP relay agent IP address */
    unsigned char chaddr [16];  /* 24: Client hardware address */
    char sname [DHCP_SNAME_LEN];    /* 40: Server name */
    char file [DHCP_FILE_LEN];  /* 104: Boot filename */
    unsigned char options [DHCP_OPTION_LEN];
                /* 212: Optional parameters
                   (actual length dependent on MTU). */
} DHCP_HEADER;

typedef struct
{
	unsigned short src_port; 
	unsigned short dest_port; 
	unsigned int seq_number;
	unsigned int ack_number;
	unsigned char padding1:4;
	unsigned char data_offset:4;
	unsigned char flags;

#define TCP_FIN	(1 << 0)
#define TCP_SYN	(1 << 1)
#define TCP_RST	(1 << 2)
#define TCP_PSH	(1 << 3)
#define TCP_ACK	(1 << 4)
#define TCP_URG	(1 << 5)

	unsigned short window;
	unsigned short checksum;
	unsigned short urgent_pointer;
	unsigned int options;

} TCP_HEADER;

typedef struct
{
	u16 udp_src;
	u16 udp_dst;
	u16 udp_len;
	u16 udp_chk;
} UDP_HEADER;

typedef struct
{
unsigned short id;       // identification number
unsigned char rd :1;     // recursion desired
unsigned char tc :1;     // truncated message
unsigned char aa :1;     // authoritive answer
unsigned char opcode :4; // purpose of message
unsigned char qr :1;     // query/response flag
unsigned char rcode :4;  // response code
unsigned char cd :1;     // checking disabled
unsigned char ad :1;     // authenticated data
unsigned char z :1;      // its z! reserved
unsigned char ra :1;     // recursion available
unsigned short q_count;  // number of question entries
unsigned short ans_count; // number of answer entries
unsigned short auth_count; // number of authority entries
unsigned short add_count; // number of resource entries
} DNS_HEADER;

typedef struct
{
unsigned short qtype;
unsigned short qclass;
} QUESTION;


typedef struct
{
unsigned short type;
unsigned short _class;
unsigned int ttl;
unsigned short data_len;
} R_DATA;


typedef struct
{
 char name[256];
R_DATA *resource;
 char rdata[256];
} RES_RECORD;

typedef struct
{
 char name[256];
QUESTION *ques;
} QUERY;


#define TCPIP_CMD_STARTUP			1	// Initializes the network module
#define TCPIP_CMD_GET_DATAGRAM	2	// Called by GS to get a datagram from the network
 										// XXXXYYYY = pointer to buffer to receive datagram
#define TCPIP_CMD_SEND_DATAGRAM	3	// Called by GS to send a datagram
 										// XXXXYYYY = pointer to parameters:
 										// LONG	pointer to data
 										// WORD	datagram length in bytes
#define TCPIP_CMD_CONNECT			4	// Connects to the network
#define TCPIP_CMD_DISCONNECT		5	// Disconnects from the network
#define TCPIP_CMD_SHUTDOWN		6	// Shuts down the network support
#define TCPIP_SET_IP_ADDRESS 		7 // Pass in XXXXYYYY a pointerto the
										// six byte MAC address of the LL
#define TCPIP_GETIPADDRESS        9   // returns the Macs IP Address in X and Y
                                       // if IP Address could not be obtained will return 0

u16 ip_sum_calc(u16 len_ip_header,const byte* _buf);
u16 udp_sum_calc(u16 proto,u16 len_udp, const byte* src_addr,const byte* dest_addr, const byte* buff);

int dns_readname(const  byte* reader,const  byte* buffer,char outname[] );

extern IP_STRUCT client_ip_addr;
extern	MAC_STRUCT client_mac_addr;
extern	MAC_STRUCT gateway_mac_addr;
extern	IP_STRUCT gateway_dns_addr;
extern const MAC_STRUCT broadcast_mac_addr;
extern  IP_STRUCT local_ip_addr;
extern  IP_STRUCT host_ip_addr;

struct tcp_queue;

enum  PConnectionState
{
	PCS_INITIALIZED=300,
    PCS_SYNSENT,
	PCS_SYNCHING,
	PCS_BEINGESTABLISHED,
	PCS_ESTABLISHED,
	PCS_CLOSING,
	PCS_CLEARINGQUEUE,
    PCS_LASTACK,
	PCS_CLOSED
};

enum PConnectionType
{
    PCT_SERVER,
    PCT_CLIENT,
    PCT_HOST
};

struct PConnection
{
	int		type;	// UDP, TCP, //ARP
	int		dest_port;
	int		source_port;
	IP_STRUCT	dest_ip;	
	MAC_STRUCT	dest_mac;
	int		window;
	int		already_sent;
	int send_seq;
	int	last_ack;
    int state_seq;
	int		tcpsocket;
    
	enum PConnectionState state;
    enum PConnectionType socktype;
	PConnection();
	~PConnection();
	void Reset();
	
	tcp_queue*	sendtcppacket(int _flags, int len, byte* data);
	int sendudppacket(int len,const byte* data);

};

struct ARP_HEADER
{
	u16 hwaddr_format; // Ethernet, Token Ring, etc 
	u16 protoaddr_format; // Same as Ether Type field 
	byte hwaddr_length; // Lenght of hardware address
	byte protoaddr_length; // Length of protocol address
	u16			opcode; // Request or Response 
	MAC_STRUCT	src_hwaddr; 
	IP_STRUCT	src_protoaddr; 
	MAC_STRUCT	dst_hwaddr; 
	IP_STRUCT dst_protoaddr;
};


#define MAX_TCP_CONNECTION	 100
extern PConnection PConnections[MAX_TCP_CONNECTION];
PConnection* init_connection( enum PConnectionType _socktype,int _type,int _srcport,IP_STRUCT _destip, int _destport, MAC_STRUCT _destmac, int _window);
void reset_connection( enum PConnectionType _socktype,int _type, int _srcport,IP_STRUCT _destip, int _destport);
PConnection*  find_connection( enum PConnectionType _socktype,int _type, int _srcport,IP_STRUCT _destip, int _destport);


enum QUEUE_STATE
{
	QS_TOSEND=1000,
	QS_TOSENDANDRETRY,
	QS_SENT,
	QS_WAITACK,
	QS_IDLE,
	QS_FRAGMENT
};

struct eth_queue
{
	// ne rien avoir ici!
	struct 
	{
		ETH_HEADER eth;
		union
		{
			ARP_HEADER arp;
			IP_HEADER ip;
			byte padding[MTU];

		};
	};

	int		queuelen;

	void print();
};

#define ETH_MAX_SIZE ((MTU - sizeof(ETH_HEADER) - sizeof(IP_HEADER))&~7)

struct tcp_queue
{
	int		queuelen;
	int		queueseq;
	double	timeout;
	int		tcplen;		// taille des donnée tcp
	int		iplen;		// taille du packet ip défragmenté
	enum QUEUE_STATE state;
	PConnection* connection;
	struct
	{
		ETH_HEADER eth;
		IP_HEADER ip;
		union
		{
			UDP_HEADER udp;
			TCP_HEADER tcp;
			byte padding[16384];
		};
	};
	void print();
	void Reset() { state=QS_IDLE; };
	int isDNS() 
	{
		return (GET_INT_FROM_IP(gateway_dns_addr)== GET_INT_FROM_IP(this->ip.ucDestination)) 
			|| (GET_INT_FROM_IP(gateway_dns_addr)== GET_INT_FROM_IP(this->ip.ucSource)) ;
	};

	int isDHCP()
	{
		if (
			ip.protocol == 17 // UDP
			&&	ntohs(udp.udp_dst) == 67
			&& !memcmp(eth.dest_mac.addr, broadcast_mac_addr.addr,6)
			)
			return 1;
		else
			return 0;
	};
	int check_tcp();	
};

eth_queue* findETHQueue();
extern eth_queue	ethqueues[MAX_ETH_QUEUE];
tcp_queue* findTCPQueue();
tcp_queue* findTCPQueueByFragment(eth_queue* q);
extern tcp_queue	tcpqueues[MAX_TCP_QUEUE];



void print_ip(const IP_STRUCT p);
void print_mac(const MAC_STRUCT mac);
void print_dns(const byte* _ptr);
void print_udp(const UDP_HEADER *p);
void print_arp(const ARP_HEADER * p);
void	print_tcp(TCP_HEADER* p);
void print_dhcp(const byte *);
void print_ip_packet( const IP_HEADER* _header);

int init_server(void);
void server_loop(void);


int add_fragment(eth_queue* p);
int process_unfragmented(tcp_queue* p);
void reset_fragments();

#pragma pack(pop)
#endif
