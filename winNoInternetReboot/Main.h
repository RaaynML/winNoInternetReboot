#pragma once
#include <cstdio>
#include <ws2tcpip.h>	// InetPton
#include <iphlpapi.h>	// IPAddr
#include <icmpapi.h>	// IcmpCreateFile

namespace rs {
	
	const auto ipAddressCloudflare() noexcept {
		IPAddr ip_inet{}; //make IPAddr from _in
		InetPtonW(AF_INET, L"1.1.1.1", &ip_inet);
		return ip_inet;
	}


	struct safeIcmpCreateFile {
		safeIcmpCreateFile() {
			icmp_handle = IcmpCreateFile();
		}
		~safeIcmpCreateFile() {
			if (icmp_handle != INVALID_HANDLE_VALUE)
				IcmpCloseHandle(icmp_handle);
		}
		operator HANDLE() {
			return icmp_handle;
		}
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
		LPVOID reply_buffer;
	};


	bool pingNetworkConnected() noexcept {
		static safeIcmpCreateFile h_icmp_file{};
		static safeReplyBuffer reply_buffer{};

		//send ICMP_REQUEST and check for ICMP_DEST_UNREACH or ICMP_ECHOREPLY
		const auto dw_ping_reply = IcmpSendEcho(h_icmp_file,
			ipAddressCloudflare(), nullptr, 0, nullptr,
			reply_buffer, reply_buffer.size(),
			2000 //2sec timeout
		);

		return (dw_ping_reply == IP_SUCCESS ? true : false);
		//safe handle closes on any destruct
	}
	
};//namespace rs
