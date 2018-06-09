#include "Engine\Net\Socket.h"

#include <iostream>


#include "ace/Reactor.h"
#include "ace/Select_Reactor.h"
#include "ace/TP_Reactor.h"
#include "ace/SOCK_Stream.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/SOCK_Connector.h"
#include "ace/INET_Addr.h"
#include "ace/ACE.h"
#include "ace/Get_Opt.h"
#include "ace/High_Res_Timer.h"
#include "ace/Thread_Manager.h"
#include "ace/Sched_Params.h"
#include "ace/Stats.h"
#include "ace/Throughput_Stats.h"
#include "ace/Sample_History.h"
#include "ace/OS_main.h"
#include "ace/OS_NS_arpa_inet.h"
#include "ace/OS_NS_ctype.h"
#include "ace/OS_NS_errno.h"
#include "ace/OS_NS_string.h"
#include "ace/OS_NS_unistd.h"

#include "ace/OS_NS_string.h"
#include "ace/OS_NS_sys_socket.h"
#include "ace/OS_NS_unistd.h"
#include "ace/OS_NS_stdlib.h"
#include "ace/OS_NS_stdio.h"
#include "ace/OS_NS_netdb.h"
#include "ace/Default_Constants.h"

#include <thread>

namespace NovaEngine {

	Socket::Socket(std::string host, int port) {
		_host = host;
		_port = port;

		ACE_OS::socket_init(ACE_WSOCK_VERSION);



		ACE_INET_Addr remote_addr(port, host.c_str());
		//remote_addr.set(ACE_HTONS(port), (ACE_UINT32)ACE_OS::inet_addr(ACE_TEXT_ALWAYS_CHAR(host.c_str())));

		if ((_handle = ACE_OS::socket(PF_INET, SOCK_STREAM, 0)) == ACE_INVALID_HANDLE)
			ACE_OS::perror(ACE_TEXT("socket")), ACE_OS::exit(1);

		int sockbufsize = 2048;
		if (sockbufsize > 0) {
			if (ACE_OS::setsockopt(_handle, SOL_SOCKET, SO_SNDBUF,
				(const char *)&sockbufsize,
				sizeof(sockbufsize)) != 0)
				ACE_OS::perror(ACE_TEXT("SO_SNDBUF")), ACE_OS::exit(1);
			if (ACE_OS::setsockopt(_handle, SOL_SOCKET, SO_RCVBUF,
				(const char *)&sockbufsize,
				sizeof(sockbufsize)) != 0)
				ACE_OS::perror(ACE_TEXT("SO_RCVBUF")), ACE_OS::exit(1);
		}
		
		struct hostent *hp;

		if ((hp = ACE_OS::gethostbyname(ACE_TEXT_ALWAYS_CHAR(host.c_str()))) == 0)
			ACE_OS::perror(ACE_TEXT("gethostbyname")), ACE_OS::exit(1);


		struct sockaddr_in saddr;

		ACE_OS::memset((void *)&saddr, 0, sizeof saddr);
		saddr.sin_family = AF_INET;
		saddr.sin_port = ACE_HTONS(port);
		ACE_OS::memcpy(&saddr.sin_addr, hp->h_addr, hp->h_length);

		if (ACE_OS::connect(_handle,
			reinterpret_cast<sockaddr *> (&saddr),
			sizeof saddr) == -1)
			ACE_OS::perror(ACE_TEXT("connect")), ACE_OS::exit(1);

		std::cout << "Connected" << std::endl;

		_running = true;

		std::thread readThread = std::thread([this]() {
			this->runReadThread();
		});
		readThread.detach();
		std::thread writeThread = std::thread([this]() {
			this->runWriteThread();
		});
		writeThread.detach();
	}
	Socket::~Socket() {
		ACE_OS::closesocket(_handle);
	}

	void Socket::sendPacket(int id, char* buf, size_t size) {
		Packet p;
		p.id = id;
		p.buf = buf;
		p.size = size;
		_packetsToSend.push(p);
	}

	void Socket::setPacketListener(std::function<void(Socket*, int)> listener) {
		_packetListener = listener;
	}

	bool Socket::tryRead(char* buf, size_t size) {
		if (ACE_OS::recv(_handle, buf, size) == -1) {
			_running = false;
			return false;
		}
		return true;
	}

	bool Socket::tryWrite(const char* buf, size_t size) {
		if (ACE_OS::send(_handle, buf, size) == -1) {
			_running = false;
			return false;
		}
		return true;
	}

	bool Socket::writeInt(int i) {
		return tryWrite((char*)&i, sizeof(i));
	}

	bool Socket::readInt(int* i) {
		if (!tryRead((char*)i, sizeof(int))) {
			return true;
		}
		return true;
	}

	void Socket::runReadThread() {
		while (_running) {
			int packedId;
			if (!readInt(&packedId)) {
				break;
			}
			if (_packetListener) {
				_packetListener(this, packedId);
			}
		}
	}

	void Socket::runWriteThread() {
		while (_running) {
			if (_packetsToSend.size() > 0) {
				Packet& packet = _packetsToSend.pop();
				int pid = packet.id;
				writeInt(pid);
				if (!tryWrite(packet.buf, packet.size)) {
					break;
				}
			}
		}
	}
}