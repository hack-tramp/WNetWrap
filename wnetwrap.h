#pragma once

#include <string>
#include <windows.h>
#include <WinInet.h>
#include <Winineti.h>

#include <map>
#include <regex>



namespace wrap {

	struct req {
		std::map<std::string, std::string> headers;
		bool set_header(std::string key, std::string value) {
			/* Header names are not case sensitive.
			From RFC 2616 - "Hypertext Transfer Protocol -- HTTP/1.1", Section 4.2, "Message Headers":
			Each header field consists of a name followed by a colon (":") and the field value. Field names are case-insensitive.
			The updating RFC 7230 does not list any changes from RFC 2616 at this part.
			so we always use fields / keys with lowercase to avoid duplication */
			std::transform(key.begin(), key.end(), key.begin(), ::tolower);
			try {
				if (headers.find(key) == headers.end()) { //if entry doesnt exist
					headers.insert(std::pair<std::string, std::string>(key, value)); //add it
				}
				else { //if entry exists, update value
					headers[key] = value;
				}
				return true;
			}
			catch (...) {
				return false;
			}
		};
		std::string method = "GET";
		LPCSTR AcceptedTypes[100] = { "*/*","*/*",NULL }; //must be null terminated std::string array
		std::string postdata;
		std::string ua = "Mozilla/5.0 (Windows NT 6.1; Win64; x64; rv:86.0) Gecko/20100101 Firefox/86.0";
	};

	struct resp {
		std::map <std::string, std::string> received_headers;
		std::map <std::string, std::string> sent_headers;
		std::string get_header(std::string key, std::string header_sel = "received") {

			try {
				std::string output = "";
				//if we are checking the sent headers then use lowercase as when we set header fields this is always in lowercase
				if (header_sel == "sent") {
					//lowercase the header fields
					std::transform(key.begin(), key.end(), key.begin(), ::tolower);
					if (sent_headers.find(key) == sent_headers.end()) { //if entry doesnt exist
					//dont make one, just exit  (i.e. not found)
						return "";
					}
					else { //if entry exists, return value
						output = sent_headers[key];
					}
				}
				else
				{
					if (received_headers.find(key) == received_headers.end()) { //if entry doesnt exist
						//dont make one, just exit  (i.e. not found)
						return "";
					}
					else { //if entry exists, return value
						output = received_headers[key];
					}
				}
				return output;
			}
			catch (...) {
				return "";
			}
		};
		std::string raw;
		std::string text;
		std::string certificate;
		std::string protocol;
		std::string cipher;
		std::string cipher_strength;
		std::string hash;
		std::string hash_strength;
		std::string exch;
		std::string ech_strength;
		std::string err;
	};

	resp HttpsRequest(std::string site, req request);
	std::string text_from_html(std::string html);


}

