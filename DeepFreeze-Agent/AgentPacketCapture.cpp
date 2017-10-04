#include "stdafx.h"
#include <iostream>
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <time.h>
//#include "AgentPacketCapture.h"

#define MAX_HOSTNAME_LAN 255
#define MAX_ADDR_LEN 9
#define SIO_RCVALL _WSAIOW(IOC_VENDOR,1) //set the card into promiscious mode


// Use std namespace for this project
using namespace std;

AgentPacketCapture::AgentPacketCapture()
{
}



// http://stackoverflow.com/questions/7309773/c-writing-structs-to-a-file-pcap
typedef struct pcap_hdr_s {
	uint32_t magic_number;   /* magic number */
	uint16_t version_major;  /* major version number */
	uint16_t version_minor;  /* minor version number */
	int32_t  thiszone;       /* GMT to local correction */
	uint32_t sigfigs;        /* accuracy of timestamps */
	uint32_t snaplen;        /* max length of captured packets, in octets */
	uint32_t network;        /* data link type */
} pcap_hdr_t;

typedef struct pcaprec_hdr_s {
	uint32_t ts_sec;         /* timestamp seconds */
	uint32_t ts_usec;        /* timestamp microseconds */
	uint32_t incl_len;       /* number of octets of packet saved in file */
	uint32_t orig_len;       /* actual length of packet */
} pcaprec_hdr_t;

typedef struct ethernet_hdr_s {
	uint8_t dst[6];    /* destination host address */
	uint8_t src[6];    /* source host address */
	uint16_t type;     /* IP? ARP? RARP? etc */
} ethernet_hdr_t;

typedef struct ipheader {
	unsigned char ihl : 4, version : 4;  // <- 4 bits wide only
										 //unsigned char h_lenver; //IP Version
	unsigned char tos; // Type of service
	unsigned short total_len; // IP header length
	unsigned short ident;
	unsigned short frag_and_flags;
	unsigned char ttl; // Packet Time-to-Live
	unsigned char proto; // Next protocol
	unsigned short checksum;
	unsigned int sourceIP; // Source IP addr
	unsigned int destIP; // Dest IP addr
}IP_HDR;

// TCP header
typedef struct tcp_header {
	unsigned short source_port; // source port
	unsigned short dest_port; // destination port
	unsigned int sequence; // sequence number - 32 bits
	unsigned int acknowledge; // acknowledgement number - 32 bits

	unsigned char ns : 1; //Nonce Sum Flag Added in RFC 3540.
	unsigned char reserved_part1 : 3; //according to rfc
	unsigned char data_offset : 4; /*The number of 32-bit words in the TCP header.
								   This indicates where the data begins.
								   The length of the TCP header is always a multiple
								   of 32 bits.*/

	unsigned char fin : 1; //Finish Flag
	unsigned char syn : 1; //Synchronise Flag
	unsigned char rst : 1; //Reset Flag
	unsigned char psh : 1; //Push Flag
	unsigned char ack : 1; //Acknowledgement Flag
	unsigned char urg : 1; //Urgent Flag

	unsigned char ecn : 1; //ECN-Echo Flag
	unsigned char cwr : 1; //Congestion Window Reduced Flag

						   ////////////////////////////////

	unsigned short window; // window
	unsigned short checksum; // checksum
	unsigned short urgent_pointer; // urgent pointer
} TCP_HDR;

// UDP Header
typedef struct udp_hdr
{
	unsigned short source_port; // Source port no.
	unsigned short dest_port; // Dest. port no.
	unsigned short udp_length; // Udp packet length
	unsigned short udp_checksum; // Udp checksum (optional)
} UDP_HDR;

typedef struct icmp_hdr
{
	BYTE type; // ICMP Error type
	BYTE code; // Type sub code
	USHORT checksum;
	USHORT id;
	USHORT seq;
} ICMP_HDR;

// Adapted from wireshark text2pcsap CRC compute
typedef struct crc32_s {         /* pseudo header for checksum calculation */
	uint32_t src_addr;
	uint32_t dest_addr;
	uint8_t  zero;
	uint8_t  protocol;
	uint16_t length;
} crc32_t;

int parse(char *buf, int packetSize);
int writeRawPacketCapture(char *buf, int packetSize);
int checkPacketSize(char *buf);
int writePcapHeader(std::string fileName);
int writePcapPacket(char *buf, string fileName, int packetSize, int totalLength);
int filterPort(char *buf, int port);
int printPacket(char *buf, int packetSize);
int printPacketData(char *buf, int packetSize, int outSize);
int parseTCP(char *buf, int iHeaderLen, int packetSize);
int parseUDP(char *buf, int iHeaderLen, int packetSize);
int currentTime();
int currentTimeHighRes();
int buildFCS(char *buf, int packetSize);
unsigned int reverse(unsigned x); // CRC32A compute
unsigned int crc32a(unsigned char *message); // CRC32A compute

int main()
{
	DWORD dwBytesRet;
	unsigned int optval = 1;
	char RecvBuf[65535] = { 0 };
	std::string fileName = "test.pcap";
	WSAData version;        //We need to check the version.
	WORD mkword = MAKEWORD(2, 2);
	int what = WSAStartup(mkword, &version);
	if (what != 0) {
		std::cout << "This version is not supported! - \n" << WSAGetLastError() << std::endl;
	}
	else {
		std::cout << "[*] WSAstartup is good!" << std::endl;
	}

	SOCKET u_sock = socket(AF_INET, SOCK_RAW, IPPROTO_IP);
	if (u_sock == INVALID_SOCKET)
		std::cout << "[!] Creating socket fail\n";

	else
		std::cout << "[*] Created sniffer socket!\n";

	//Setup recvive function for packets
	char FAR name[MAX_HOSTNAME_LAN];
	gethostname(name, MAX_HOSTNAME_LAN);

	struct hostent FAR * pHostent;
	pHostent = (struct hostent *)malloc(sizeof(struct hostent));
	pHostent = gethostbyname(name);
	SOCKADDR_IN sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(6000);

	memcpy(&sa.sin_addr.S_un.S_addr, pHostent->h_addr_list[0], pHostent->h_length);
	bind(u_sock, (SOCKADDR *)&sa, sizeof(sa));
	WSAIoctl(u_sock, SIO_RCVALL, &optval, sizeof(optval), NULL, 0, &dwBytesRet, NULL, NULL);
	// Setup pcap file for writing
	if (writePcapHeader(fileName)) {
		// Value for packet header location
		int filterPortNumber = 0;
		int filterIpAddr = 0;
		int dataSize = 0;
		int packetCount = 0;
		int maxPacketCount = 1000000;
		int fileSize = 0;
		int maxFileSize = 1 * 1024 * 1024; //In MB format
		std::string fileName = "Debug.pcap";
		int packetOut = 0;
		while (packetCount < maxPacketCount) {
			int get = recv(u_sock, RecvBuf, sizeof(RecvBuf), 0);
			if (get == SOCKET_ERROR) {
				std::cout << "Error in Receiving: " << WSAGetLastError() << std::endl;
			}
			if (get > 0) {
				if (filterPortNumber) {
					if (!filterPort(RecvBuf, filterPortNumber)) {
						continue;
					}
				}
				dataSize = get + 13; // Ethernet header is 14, stack starts at 0
									 // add in the fake Ethernet header size
									 //writeRawPacketCapture(RecvBuf, get);
				writePcapPacket(RecvBuf, fileName, dataSize, get);
				if (packetOut) {
					printPacketData(RecvBuf, get, dataSize);
					parse(RecvBuf, get);
					printPacket(RecvBuf, get);
				}
			}
			// Add in global and packet headers
			fileSize += dataSize + 40;
			if (fileSize > maxFileSize) {
				wcout << "[!] Max file size reached: " << maxFileSize << endl;
				break;
			}
			packetCount = packetCount + 1;
		}
	}
	return 0;
}

// Check if packet size is to large
int checkPacketSize(char *buf) {
	int iTotalLength;
	IP_HDR *pIpheader;
	pIpheader = (IP_HDR *)buf;
	iTotalLength = pIpheader->total_len;
	if (iTotalLength > 65535) {
		// If over packet size drop packet, return False
		return 0;
	}
	else {
		// Under limit return True
		return 1;
	}
}

// Write Packet to disk in .pcap format
int writePcapHeader(string fileName) {
	ofstream fileout("test2.pcap", ios::binary);
	// build pcap format global header
	// https://wiki.wireshark.org/Development/LibpcapFileFormat
	pcap_hdr_s fileHeader;
	// Set file format itself and the byte ordering
	fileHeader.magic_number = 0xa1b2c3d4;
	fileHeader.version_major = 2;
	fileHeader.version_minor = 4;
	// UTC Timezone
	fileHeader.thiszone = 0;
	// Accuracy of the capture
	fileHeader.sigfigs = 0;
	fileHeader.snaplen = 0x040000;
	// http://www.tcpdump.org/linktypes.html
	// 1 = Ethernet LL
	fileHeader.network = 1;
	// write to stream
	fileout.write(reinterpret_cast<const char*>(&fileHeader),
		sizeof fileHeader);
	fileout.close();
	cout << "[*] PCAP global file header created: " << fileName << endl;
	return 1;
}

int writeRawPacketCapture(char *buf, int packetSize) {
	ofstream fileout("raw-cap.txt", ios::binary | ios::app);
	// Build fake temp Ethernet header
	ethernet_hdr_t ethernetHeader;
	for (int a = 0; a<7; a = a + 1) {
		ethernetHeader.dst[a] = 8;
		ethernetHeader.src[a] = 8;
	}
	ethernetHeader.type = 0x0000;
	fileout.write(reinterpret_cast<const char*>(&ethernetHeader),
		sizeof ethernetHeader);
	for (int i = 0; i < packetSize; i++) {
		fileout.write(&buf[i], sizeof(buf));
	}
	fileout.close();
}

// Write the packet header and packet to .pcap
int writePcapPacket(char *buf, string fileName, int packetSize, int totalLength) {
	ofstream fileout("test2.pcap", ios::binary | ios::app);
	time_t ltime = currentTime();
	pcaprec_hdr_t packetHeader;
	packetHeader.ts_sec = ltime;
	packetHeader.ts_usec = currentTimeHighRes();
	packetHeader.incl_len = packetSize + 14;
	packetHeader.orig_len = packetSize + 14;
	// Build fake temp Ethernet header
	ethernet_hdr_t ethernetHeader;
	for (int a = 0; a<7; a = a + 1) {
		ethernetHeader.dst[a] = 8;
		ethernetHeader.src[a] = 8;
	}
	ethernetHeader.type = 0x0008; // Ethernet -> IP
	unsigned char *ucBuffer = (unsigned char*)&buf[0];
	/*
	char * val = reinterpret_cast<char*>(&ethernetHeader);
	unsigned char *ucBuffer2 = (unsigned char*)&val[0];
	int value = crc32a(ucBuffer2);
	wcout << "CRC: " << hex << value << endl;
	*/
	fileout.write(reinterpret_cast<const char*>(&packetHeader),
		sizeof packetHeader);
	fileout.write(reinterpret_cast<const char*>(&ethernetHeader),
		sizeof ethernetHeader);
	for (int i = 0; i < packetSize; i++) {
		fileout.write(&buf[i], 1);
	}
	fileout.close();
}

// Filter on port, returns true or false
int filterPort(char *buf, int port) {
	int iProtocol, iHeaderLen;
	IP_HDR *pIpheader;
	pIpheader = (IP_HDR *)buf;
	iProtocol = pIpheader->proto;
	iHeaderLen = pIpheader->ihl;
	if (iProtocol == IPPROTO_TCP) {
		unsigned short iphdrlen;
		iphdrlen = iHeaderLen * 4;
		TCP_HDR *pTcpHeader;
		// Advance the pointer to correct location
		pTcpHeader = (TCP_HDR*)(buf + iphdrlen);
		int tSourcePort = ntohs(pTcpHeader->source_port);
		int tDestPort = ntohs(pTcpHeader->dest_port);
		if (tDestPort == port || tSourcePort == port) {
			return true;
		}
		else {
			return false;
		}
	}
	if (iProtocol == IPPROTO_UDP) {
		unsigned short iphdrlen;
		iphdrlen = iHeaderLen * 4;
		UDP_HDR *pUdpHeader;
		// Advance the pointer to correct location
		pUdpHeader = (UDP_HDR*)(buf + iphdrlen);
		int uSourcePort = ntohs(pUdpHeader->source_port);
		int uDestPort = ntohs(pUdpHeader->dest_port);
		if (uDestPort == port || uSourcePort == port) {
			return true;
		}
		else {
			return false;
		}
	}
	else {
		// Catch all return false if not UDP/TCP
		return false;
	}
}

// Parse packet aray
int parse(char *buf, int packetSize) {
	int iProtocol, iVersion, iHeaderLen, iService, iTotalLength, iSourceip, iDestip, iTtl;
	int iTTL;
	char *szSourceIP, *szDestIP;
	SOCKADDR_IN saSource, saDest;
	IP_HDR *pIpheader;
	pIpheader = (IP_HDR *)buf;
	//Check Proto
	iProtocol = pIpheader->proto;
	iVersion = pIpheader->version;
	iHeaderLen = pIpheader->ihl;
	iService = pIpheader->tos;
	iTotalLength = pIpheader->total_len;
	iSourceip = pIpheader->sourceIP;
	iDestip = pIpheader->destIP;
	iTtl = pIpheader->ttl;
	if (iSourceip) {
		saSource.sin_addr.s_addr = pIpheader->sourceIP;
		szSourceIP = inet_ntoa(saSource.sin_addr);
		std::cout << szSourceIP << " -> ";
	}
	if (iDestip) {
		saDest.sin_addr.s_addr = pIpheader->sourceIP;
		szDestIP = inet_ntoa(saDest.sin_addr);
		std::cout << szDestIP << std::endl;
	}
	std::cout << "*-------------IP HEADER-------------*" << endl;
	std::cout << "| -- IP Version is: " << iVersion << std::endl;
	std::cout << "| -- IP Service is: " << iService << std::endl;
	std::cout << "| -- IP Header size is: " << ntohs(iTotalLength) << std::endl;
	std::wcout << "| -- IP Packet TTL : " << int(iTtl) << std::endl;
	std::wcout << "| -- IP Source IP : " << szSourceIP << std::endl;
	std::wcout << "| -- IP Dest IP : " << szDestIP << std::endl;
	std::wcout << "| -- IP Next Protocol: " << iProtocol << std::endl;

	if (iProtocol == IPPROTO_TCP) {
		parseTCP(buf, iHeaderLen, packetSize);
	}
	if (iProtocol == IPPROTO_UDP) {
		parseUDP(buf, iHeaderLen, packetSize);
	}
}

// Parse a TCP packet from know start point
int parseTCP(char *buf, int iHeaderLen, int packetSize) {
	std::cout << "*-------------TCP HEADER-------------*" << endl;
	// IP Header is IHL * 4 for byte count
	unsigned short iphdrlen;
	iphdrlen = iHeaderLen * 4;
	TCP_HDR *pTcpHeader;
	// break array location
	pTcpHeader = (TCP_HDR*)(buf + iphdrlen);
	int tSourcePort = pTcpHeader->source_port;
	int tDestPort = pTcpHeader->dest_port;
	wcout << "| -- Source Port: " << ntohs(tSourcePort) << endl;
	wcout << "| -- Dest Port: " << ntohs(tDestPort) << endl;
	//wcout << "| -- TCP Packet Length: " << ntohs(uPacketLength) << endl;
}

int parseUDP(char *buf, int iHeaderLen, int packetSize) {
	std::cout << "*-------------UDP HEADER-------------*" << endl;
	// IP Header is IHL * 4 for byte count
	unsigned short iphdrlen;
	iphdrlen = iHeaderLen * 4;
	UDP_HDR *pUdpHeader;
	// break array location
	pUdpHeader = (UDP_HDR*)(buf + iphdrlen);
	int uSourcePort = pUdpHeader->source_port;
	int uDestPort = pUdpHeader->dest_port;
	int uPacketLength = pUdpHeader->udp_length;
	int uCheckSum = pUdpHeader->udp_checksum;
	wcout << "| -- Source Port: " << ntohs(uSourcePort) << endl;
	wcout << "| -- Dest Port: " << ntohs(uDestPort) << endl;
	wcout << "| -- UDP Packet Length: " << ntohs(uPacketLength) << endl;
	wcout << "| -- UDP Checksum: " << ntohs(uCheckSum) << endl;
}

int printPacket(char *buf, int packetSize) {
	std::cout << "*-------------ASCII DUMP------------*" << endl;
	for (int i = 0; i < packetSize; i++) {
		std::cout << buf[i];
	}
	std::cout << std::endl << std::endl;
	buf[packetSize] = 0; // Null-terminate the buffer
	std::cout << "*------------RAW HEX DUMP-----------*" << endl;
	buf[packetSize] = 0; // Null-terminate the buffer
	for (int i = 0; i < packetSize; i++) {
		std::cout << std::hex << (int)buf[i];
	}
	std::cout << std::endl << std::endl;
}

int printPacketData(char *buf, int packetSize, int outSize) {
	std::cout << "*------------PACKET DATA-------------*" << endl;
	std::wcout << "| -- Bytes wrote: " << outSize << std::endl;
	std::wcout << "| -- Bytes Recv: " << packetSize << std::endl;
}

int currentTime() {
	time_t ltime;
	time(&ltime);
	struct tm* timeinfo = gmtime(&ltime);
	ltime = mktime(timeinfo);
	return ltime;
}

int currentTimeHighRes() {
	FILETIME time;
	SYSTEMTIME sysTime;
	GetSystemTimeAsFileTime(&time);
	FileTimeToSystemTime(&time, &sysTime);
	return sysTime.wMilliseconds;

}

/*
Bellow is the crc32a functions
http://www.hackersdelight.org/hdcodetxt/crc.c.txt
*/
// Reverses (reflects) bits in a 32-bit word.
unsigned reverse(unsigned x) {
	x = ((x & 0x55555555) << 1) | ((x >> 1) & 0x55555555);
	x = ((x & 0x33333333) << 2) | ((x >> 2) & 0x33333333);
	x = ((x & 0x0F0F0F0F) << 4) | ((x >> 4) & 0x0F0F0F0F);
	x = (x << 24) | ((x & 0xFF00) << 8) |
		((x >> 8) & 0xFF00) | (x >> 24);
	return x;
}

unsigned int crc32a(unsigned char *message) {
	int i, j;
	unsigned int byte, crc;

	i = 0;
	crc = 0xFFFFFFFF;
	while (message[i] != 0) {
		byte = message[i];            // Get next byte.
		byte = reverse(byte);         // 32-bit reversal.
		for (j = 0; j <= 7; j++) {    // Do eight times.
			if ((int)(crc ^ byte) < 0)
				crc = (crc << 1) ^ 0x04C11DB7;
			else crc = crc << 1;
			byte = byte << 1;          // Ready next msg bit.
		}
		i = i + 1;
	}
	return reverse(~crc);
}
