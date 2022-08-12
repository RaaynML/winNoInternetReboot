//Ping the Cloudflare DNS IP to check if internet is available using Winapi.
#include <cstdio>
#include <ws2tcpip.h>	// InetPton
#include <iphlpapi.h>	// IPAddr
#include <icmpapi.h>	// IcmpCreateFile

#pragma comment(lib, "Ws2_32.lib") //needed for InetPton
#pragma comment(lib, "IPHLPAPI.lib") //needed for IcmpCreateFile

namespace rs {
	const auto ipAddressCloudflare() noexcept {
		IPAddr ip_inet {}; //make IPAddr from _in
		InetPtonW(AF_INET, L"1.1.1.1", &ip_inet);
		return ip_inet;
	}

	
	struct safeIcmpCreateFile {
		safeIcmpCreateFile() {
			icmp_handle = IcmpCreateFile();
		}
		~safeIcmpCreateFile() {
			if (icmp_handle != INVALID_HANDLE_VALUE)
				CloseHandle(icmp_handle);
		}
		operator HANDLE() {
			return icmp_handle;
		}
	private:
		HANDLE icmp_handle;
	};
	
	
	struct safeReplyBuffer {
		safeReplyBuffer() {
			reply_buffer = new byte[sizeof(ICMP_ECHO_REPLY) + sizeof(ICMP_ECHO_REPLY)];
		}
		~safeReplyBuffer() {
			delete[] reply_buffer;
		}
		operator LPVOID() { //isn't const because obviously
			return reply_buffer;
		}
		constexpr auto size() const {
			return DWORD{ sizeof(ICMP_ECHO_REPLY) + sizeof(ICMP_ECHO_REPLY) };
		}
	private:
		LPVOID reply_buffer;
	};

	
	bool pingNetworkConnected() noexcept {
		auto h_icmp_file = safeIcmpCreateFile{};
		auto reply_buffer = safeReplyBuffer{};
		
		//send ICMP_REQUEST and check for ICMP_DEST_UNREACH or ICMP_ECHOREPLY
		const auto dw_ping_reply = IcmpSendEcho( h_icmp_file,
			ipAddressCloudflare(), nullptr,
			0, nullptr,
			reply_buffer, reply_buffer.size(),
			2000 //2sec timeout
		);

		return ( dw_ping_reply == IP_SUCCESS ? true : false );
		//safe handle closes on any destruct
	}
};//namespace rs



int main() {
	static constexpr auto one_minute = DWORD(60000);
	static constexpr auto two_minutes = one_minute * 2;
	static constexpr auto ten_minutes = one_minute * 10;
	

	while ( rs::pingNetworkConnected() ) {
		Sleep(ten_minutes);
	}
	
	//the above while() will break if internet is down
	//then the below loop waits until internet is up again
	
	while ( !rs::pingNetworkConnected() ) {
		Sleep(two_minutes);
	}
	
	
	//Restart Nicehash on internet connection:
	if ( rs::pingNetworkConnected() ) {
		system("taskkill /f /im NicehashMiner.exe");
		Sleep(2000); //give windoze time to catch up
		system("start /b NicehashMiner.exe");
	}

	return 0;
}