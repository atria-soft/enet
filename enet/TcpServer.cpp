/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <enet/debug.hpp>
#include <enet/Tcp.hpp>
#include <enet/TcpServer.hpp>
#include <enet/enet.hpp>
extern "C" {
	#include <sys/types.h>
	#include <errno.h>
	#include <unistd.h>
	#include <string.h>
}
#include <etk/stdTools.hpp>

#ifdef __TARGET_OS__Windows
	#include <winsock2.h>
	#include <ws2tcpip.h>
	//https://msdn.microsoft.com/fr-fr/library/windows/desktop/ms737889(v=vs.85).aspx
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netinet/tcp.h>
	#include <netdb.h>
#endif

enet::TcpServer::TcpServer() :
  m_socketId(-1),
  m_host("127.0.0.1"),
  m_port(23191) {
	
}

enet::TcpServer::~TcpServer() {
	unlink();
}

void enet::TcpServer::setIpV4(uint8_t _fist, uint8_t _second, uint8_t _third, uint8_t _quatro) {
	etk::String tmpname;
	tmpname  = etk::toString(_fist);
	tmpname += ".";
	tmpname += etk::toString(_second);
	tmpname += ".";
	tmpname += etk::toString(_third);
	tmpname += ".";
	tmpname += etk::toString(_quatro);
	setHostNane(tmpname);
}

void enet::TcpServer::setHostNane(const etk::String& _name) {
	if (_name == m_host) {
		return;
	}
	m_host = _name;
}

void enet::TcpServer::setPort(uint16_t _port) {
	if (_port == m_port) {
		return;
	}
	m_port = _port;
}

#ifdef __TARGET_OS__Windows
	bool enet::TcpServer::link() {
		if (enet::isInit() == false) {
			ENET_ERROR("Need call enet::init(...) before accessing to the socket");
			return false;
		}
		ENET_INFO("Start connection on " << m_host << ":" << m_port);
		
		struct addrinfo *result = null;
		struct addrinfo hints;
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;
		
		// Resolve the server address and port
		etk::String portValue = etk::toString(m_port);
		int iResult = getaddrinfo(null, portValue.c_str(), &hints, &result);
		if (iResult != 0) {
			ENET_ERROR("getaddrinfo failed with error: " << iResult);
			return 1;
		}
		
		// open in Socket normal mode
		m_socketId = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (m_socketId == INVALID_SOCKET) {
			ENET_ERROR("ERROR while opening socket : errno=" << errno << "," << strerror(errno));
			freeaddrinfo(result);
			return false;
		}
		// set the reuse of the socket if previously opened :
		int sockOpt = 1;
		if(setsockopt(m_socketId, SOL_SOCKET, SO_REUSEADDR, (const char*)&sockOpt, sizeof(int)) != 0) {
			ENET_ERROR("ERROR while configuring socket re-use : errno=" << errno << "," << strerror(errno));
			return false;
		}
		ENET_INFO("Start binding Socket ... (can take some time ...)");
		if (bind(m_socketId, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
			ENET_ERROR("ERROR on binding errno=" << WSAGetLastError());
			freeaddrinfo(result);
			closesocket(m_socketId);
			m_socketId = INVALID_SOCKET;
			return false;
		}
		return true;
	}
#else
	bool enet::TcpServer::link() {
		if (enet::isInit() == false) {
			ENET_ERROR("Need call enet::init(...) before accessing to the socket");
			return false;
		}
		ENET_INFO("Start connection on " << m_host << ":" << m_port);
		// open in Socket normal mode
		m_socketId = socket(AF_INET, SOCK_STREAM, 0);
		if (m_socketId < 0) {
			ENET_ERROR("ERROR while opening socket : errno=" << errno << "," << strerror(errno));
			return false;
		}
		// set the reuse of the socket if previously opened :
		int sockOpt = 1;
		if(setsockopt(m_socketId, SOL_SOCKET, SO_REUSEADDR, (const char*)&sockOpt, sizeof(int)) != 0) {
			ENET_ERROR("ERROR while configuring socket re-use : errno=" << errno << "," << strerror(errno));
			return false;
		}
		// clear all
		struct sockaddr_in servAddr;
		bzero((char *) &servAddr, sizeof(servAddr));
		servAddr.sin_family = AF_INET;
		servAddr.sin_addr.s_addr = INADDR_ANY;
		servAddr.sin_port = htons(m_port);
		ENET_INFO("Start binding Socket ... (can take some time ...)");
		if (bind(m_socketId, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
			ENET_ERROR("ERROR on binding errno=" << errno << "," << strerror(errno));
			#ifdef __TARGET_OS__Windows
				closesocket(m_socketId);
				m_socketId = INVALID_SOCKET;
			#else
				close(m_socketId);
				m_socketId = -1;
			#endif
			return false;
		}
		return true;
	}
#endif

enet::Tcp enet::TcpServer::waitNext() {
	if (enet::isInit() == false) {
		ENET_ERROR("Need call enet::init(...) before accessing to the socket");
		return etk::move(enet::Tcp());
	}
	ENET_INFO("End binding Socket ... (start listen)");
	#ifdef __TARGET_OS__Windows
		int ret = listen(m_socketId, SOMAXCONN);
		if (ret == SOCKET_ERROR) {
			ENET_ERROR("listen failed with error: " << WSAGetLastError());
			return enet::Tcp();;
		}
	#else
		listen(m_socketId, 1); // 1 is for the number of connection at the same time ...
	#endif
	ENET_INFO("End listen Socket ... (start accept)");
	struct sockaddr_in clientAddr;
	socklen_t clilen = sizeof(clientAddr);
	int32_t socketIdClient = accept(m_socketId, (struct sockaddr *) &clientAddr, &clilen);
	if (socketIdClient < 0) {
		ENET_ERROR("ERROR on accept errno=" << errno << "," << strerror(errno));
		#ifdef __TARGET_OS__Windows
			closesocket(m_socketId);
			m_socketId = INVALID_SOCKET;
		#else
			close(m_socketId);
			m_socketId = -1;
		#endif
		
		return enet::Tcp();
	}
	etk::String remoteAddress;
	{
		struct sockaddr_storage addr;
		char ipstr[INET6_ADDRSTRLEN];
		socklen_t len = sizeof(addr);
		getpeername(socketIdClient, (struct sockaddr*)&addr, &len);
		// deal with both IPv4 and IPv6:
		if (addr.ss_family == AF_INET) {
			struct sockaddr_in *s = (struct sockaddr_in *)&addr;
			int port = ntohs(s->sin_port);
			remoteAddress = etk::toString(s->sin_addr.s_addr&0xFF);
			remoteAddress += ".";
			remoteAddress += etk::toString((s->sin_addr.s_addr>>8)&0xFF);
			remoteAddress += ".";
			remoteAddress += etk::toString((s->sin_addr.s_addr>>16)&0xFF);
			remoteAddress += ".";
			remoteAddress += etk::toString((s->sin_addr.s_addr>>24)&0xFF);
			remoteAddress += ":";
			remoteAddress += etk::toString(port);
		} else { // AF_INET6
			struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
			int port = ntohs(s->sin6_port);
			remoteAddress = etk::toHex(s->sin6_addr.s6_addr[0], 2);
			remoteAddress += etk::toHex(s->sin6_addr.s6_addr[1], 2);
			remoteAddress += ".";
			remoteAddress += etk::toHex(s->sin6_addr.s6_addr[2], 2);
			remoteAddress += etk::toHex(s->sin6_addr.s6_addr[3], 2);
			remoteAddress += ".";
			remoteAddress += etk::toHex(s->sin6_addr.s6_addr[4], 2);
			remoteAddress += etk::toHex(s->sin6_addr.s6_addr[5], 2);
			remoteAddress += ".";
			remoteAddress += etk::toHex(s->sin6_addr.s6_addr[6], 2);
			remoteAddress += etk::toHex(s->sin6_addr.s6_addr[7], 2);
			remoteAddress += ".";
			remoteAddress += etk::toHex(s->sin6_addr.s6_addr[8], 2);
			remoteAddress += etk::toHex(s->sin6_addr.s6_addr[9], 2);
			remoteAddress += ".";
			remoteAddress += etk::toHex(s->sin6_addr.s6_addr[10], 2);
			remoteAddress += etk::toHex(s->sin6_addr.s6_addr[11], 2);
			remoteAddress += ".";
			remoteAddress += etk::toHex(s->sin6_addr.s6_addr[12], 2);
			remoteAddress += etk::toHex(s->sin6_addr.s6_addr[13], 2);
			remoteAddress += ".";
			remoteAddress += etk::toHex(s->sin6_addr.s6_addr[14], 2);
			remoteAddress += etk::toHex(s->sin6_addr.s6_addr[15], 2);
			remoteAddress += ":";
			remoteAddress += etk::toString(port);
		}
	}
	ENET_ERROR("End configuring Socket ... Find New one FROM " << remoteAddress);
	return enet::Tcp(socketIdClient, m_host + ":" + etk::toString(m_port), remoteAddress);
}


bool enet::TcpServer::unlink() {
	#ifdef __TARGET_OS__Windows
		if (m_socketId != INVALID_SOCKET) {
			ENET_INFO(" close server socket");
			closesocket(m_socketId);
			m_socketId = INVALID_SOCKET;
		}
	#else
		if (m_socketId >= 0) {
			ENET_INFO(" close server socket");
			close(m_socketId);
			m_socketId = -1;
		}
	#endif
	return true;
}
