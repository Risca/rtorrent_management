#define _XOPEN_SOURCE 500

#include <xmlrpc-c/client_simple.hpp>

#include <ftw.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <time.h>

#define MINUTE (60)
#define HOUR (60*MINUTE)
#define DAY (24*HOUR)
#define WEEK (7*DAY)

static const std::string g_ServerUrl = "http://192.168.77.79/RPC2";
static const xmlrpc_c::value_i8 g_MinimumRatio = 1500;
static const xmlrpc_c::value_i8 g_MinimumAge = 3 * WEEK;

xmlrpc_c::value_array GetTorrentList(const char* view)
{
	xmlrpc_c::clientSimple rpc;
	xmlrpc_c::value result;
	rpc.call(g_ServerUrl, "download_list", "ss", &result, "", view);
	return result;
}

xmlrpc_c::value_array GetStartedTorrents()
{
	return GetTorrentList("started");
}

xmlrpc_c::value_array GetDoneTorrents()
{
	return GetTorrentList("seeding");
}

xmlrpc_c::value GetTorrentInfo(const char* method, const xmlrpc_c::value_string& hash)
{
	xmlrpc_c::clientSimple rpc;
	xmlrpc_c::value result;
	rpc.call(g_ServerUrl, method, "s", &result, hash.cvalue().c_str());
	return result;
}

xmlrpc_c::value_i8 GetTorrentRatio(const xmlrpc_c::value_string& hash)
{
	return GetTorrentInfo("d.get_ratio", hash);
}

xmlrpc_c::value_i8 GetTorrentAge(const xmlrpc_c::value_string& hash)
{
	const xmlrpc_c::value_i8 now = time(NULL);
	const xmlrpc_c::value_i8 ts = GetTorrentInfo("d.get_state_changed", hash);
	return now - ts;
}

xmlrpc_c::value_string GetBasePath(const xmlrpc_c::value_string& hash)
{
	return GetTorrentInfo("d.get_base_path", hash);
}

xmlrpc_c::value_int Erase(const xmlrpc_c::value_string& hash)
{
	return GetTorrentInfo("d.erase", hash);
}

int unlink_cb(const char* path, const struct stat*, int, struct FTW*)
{
	int rv = remove(path);
	if (rv) {
		perror(path);
	}
	return rv;
}

int main(int argc, char* argv[])
{
	try {
		xmlrpc_c::carray torrents = GetDoneTorrents().cvalue();
		std::cout << "Found " << torrents.size() << " seeding torrents" << std::endl;
		for (xmlrpc_c::value_string t : torrents) {
			xmlrpc_c::value_i8 ratio = GetTorrentRatio(t);
			xmlrpc_c::value_i8 age = GetTorrentAge(t);
			if (ratio > g_MinimumRatio || age > g_MinimumAge) {
				std::string path = GetBasePath(t);
				if (0 == Erase(t)) {
					if (0 == nftw(path.c_str(), unlink_cb, 64, FTW_DEPTH | FTW_PHYS)) {
						std::cout << "Removed \'" << path << "\'" << std::endl;
					}
					else {
						std::cerr << "Failed to erase \'" << path << "\'" << std::endl;
					}
				}
				else {
					std::cerr << "Failed to remove torrent: " << t.cvalue() << std::endl;
				}
			}
		}
	}
	catch (const std::exception& e) {
		std::cerr << "Client threw error: " << e.what() << std::endl;
	}
	catch (...) {
		std::cerr << "Client threw unexpected error." << std::endl;
	}

	return 0;
}
