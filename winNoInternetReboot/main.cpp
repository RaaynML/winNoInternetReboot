//Ping Cloudflare to check if internet is available using Winapi.
#include "Main.h"

#pragma comment(lib, "Ws2_32.lib") //needed for InetPton
#pragma comment(lib, "IPHLPAPI.lib") //needed for IcmpCreateFile


int main() {
	puts("Monitoring connectivity");
	static constexpr auto one_minute = DWORD(60000);
	static constexpr auto two_minutes = one_minute * 2;
	static constexpr auto ten_minutes = one_minute * 10;

	while ( rs::pingNetworkConnected() ) {
		Sleep(ten_minutes);
	}
	
	//the above while() will break if internet is down
	//then the below loop waits until internet is up again
	puts("Netork connection was lost, waiting for it to come back...");

	while ( !rs::pingNetworkConnected() ) {
		Sleep(two_minutes);
	}
	
	//Restart Nicehash on internet connection:
	if ( rs::pingNetworkConnected() ) {
		puts("Internet returned, restarting Nicehash...");
		system("taskkill /f /im NicehashMiner.exe");
		Sleep(2000); //give windoze time to catch up
		system("start /b NicehashMiner.exe");
	}

	return 0;
}
