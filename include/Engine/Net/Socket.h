#ifndef SOCKET_H
#define SOCKET_H

#include <string>
#include <functional>
#include <vector>

#include "ace/ACE.h"
#include "Engine/Utils/Queue.h"

namespace NovaEngine {

	typedef struct PACKET {
		int id;
		char* buf;
		size_t size;
	} Packet;

	class Socket {
	private:
		std::string _host;
		int _port;
		ACE_HANDLE _handle;
		std::function<void(Socket*, int)> _packetListener;
		Queue<Packet> _packetsToSend;
		volatile bool _running;
	public:
		Socket(std::string host, int port);
		~Socket();

		void sendPacket(int id, char* buf, size_t size);
		template<typename T> inline void sendPacket(int id, T packet) {
			size_t size = sizeof(packet);
			char* data = new char[size];
			memcpy(data, &packet, size);
			sendPacket(id, data, size);
		}
		void setPacketListener(std::function<void(Socket*, int)> listener);

		void runReadThread();
		void runWriteThread();

		bool tryRead(char* buf, size_t size);
		bool tryWrite(const char* buf, size_t size);

		bool writeInt(int i);
		bool readInt(int* i);
	};
}

#endif // !SOCKET_H