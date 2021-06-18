/*
fix: accept gzip data... see this async wrapper that does it: https://www.codeproject.com/articles/43860/an-asynchronous-http-download-class-for-mfc-atl-an
fix: HttpOpenRequestA and HttpSendRequestA seem to work passing utf-8 params even though the docs recommend always using the W commands

notes
- in url only query params, not host / path is url encoded
- due to MS bug timeout is done via worker thread, this means it cant be increased beyond MS default
also timeouts might cause memory leaks, are all threads, pointers, structs, vars etc deleted/freed? 
according to this SO thread no need to delete or free unless new or malloc called https://stackoverflow.com/questions/5243360/how-to-free-memory-for-structure-variable
should smart pointers be used in the workers? the docs mainly talk about cases where new is used https://docs.microsoft.com/en-us/cpp/cpp/smart-pointers-modern-cpp?view=msvc-160

fixed: post data size limit - fixed by converting postdata string to char array before sending
*/

#pragma once

// needed for utf-8 url encoding -  see comments at https://alfps.wordpress.com/2011/12/08/unicode-part-2-utf-8-stream-mode/
// win10 build 17134 (April 2018 Update) and later users can use setlocale(LC_ALL, ".UTF8") see https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/setlocale-wsetlocale?view=msvc-160
#pragma execution_character_set( "utf-8" ) 

#include <string>
#include <windows.h>
#include <WinInet.h>
#include <Winineti.h>
#include <stdio.h>
#include <cstdint>
#include <map>
#include <regex>
#include <initializer_list>
#include <locale>
#include <codecvt>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm> 
#include <urlmon.h>



namespace wrap {



	struct Body { //used for postdata
		std::string body;
	};

	struct File { //used for uploading file multipart post
		std::string file;
	};

	struct Download {
		std::string dl = "dl"; //default is to download file keeping original filename
	};

	struct Method {
		std::string method;
	};

	struct Url {
		std::string adr;
	};

	struct Header {
		std::map<std::string, std::string> hdr;

		Header(std::initializer_list<std::pair<const std::string, std::string> > hdr) :
			hdr(hdr) {
		}
	};

	struct Parameters {
		std::map<std::string, std::string> p;

		Parameters(std::initializer_list<std::pair<const std::string, std::string> > p) :
			p(p) {
		}
	};

	struct Payload {
		std::map<std::string, std::string> pd;

		Payload(std::initializer_list<std::pair<const std::string, std::string> > pd) :
			pd(pd) {
		}
	};

	struct Multipart {
		std::map<std::string, std::string> mp;

		Multipart(std::initializer_list<std::pair<const std::string, std::string> > mp) :
			mp(mp) {
		}
	}; 

	struct Authentication {
		std::string usr;
		std::string pwd;
	};

	struct Bearer {
		std::string token;
	};

	struct Timeout {
		DWORD timeout;
		std::string type = "connection"; // connection or request
	};

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
		bool clear_headers(std::string key = "") {
			if (key == "") { //leave empty to clear all 
				try {
					headers.clear();
					return true;
				}
				catch (...) {
					return false;
				}

			}
			else {
				try {
					std::transform(key.begin(), key.end(), key.begin(), ::tolower);
					if (headers.find(key) != headers.end()) { // if element exists
						headers.erase(key);
						return true;
					}
					else {
						return false;
					}
				}
				catch (...) {
					return false;
				}
			}
		};
		//std::string postdata;
		std::string ua = "Mozilla/5.0 (Windows NT 6.1; Win64; x64; rv:89.0) Gecko/20100101 Firefox/89.0";
		std::string Params;
		std::string PostData; // key is type of data (body,payload or file) and value is the data
		std::string Cookies;
		std::string Dl;
		std::string Proxies;
		std::string Method = "GET";
		std::string Auth;
		std::string Url = "www.example.com";
		Header Header = { {"",""} };
		DWORD TimeoutConnect = 0;
		DWORD TimeoutRequest = 0;
		void reset() {
			ua = "Mozilla/5.0 (Windows NT 6.1; Win64; x64; rv:89.0) Gecko/20100101 Firefox/89.0";
			Params = "";
			PostData = "";
			Cookies = "";
			Dl = "";
			Proxies = "";
			Method = "GET";
			Auth = "";
			Url = "www.example.com";
			Header = { {"",""} };
			TimeoutConnect = 0;
			TimeoutRequest = 0;
		};
	};

	struct Response {
		std::map <std::string, std::string> header;
		std::map <std::string, std::string> sent_headers;
		std::map <std::string, std::string> secinfo;
		std::string url;
		std::string raw;
		std::string text;
		std::string status_code;
		std::string err;
	};



	std::string text_from_html(std::string html);

	//initial req object that will be used to pass params to cpp source function
	extern req toSource;

	void Params(wrap::Parameters s);

	void Params(wrap::Url u);

	void Params(wrap::Header h);

	void Params(wrap::Payload pd);

	void Params(wrap::Multipart mp);

	void Params(wrap::Authentication auth);

	void Params(wrap::Bearer token);

	void Params(wrap::Method m);

	void Params(wrap::Download dl);

	void Params(wrap::Body body);

	void Params(wrap::File file);

	void Params(wrap::Timeout timeout);

	// url, headers, params, postdata, cookies, dl flag, proxies, timeout, method, auth 
	Response httpsreq(req Request);

	//this is to achieve the effect of allowing random params of different types
	//https://stackoverflow.com/questions/67089840
	//not even URL is needed - it's set to example.com by default
	template <typename ...Ts>
	Response HttpsRequest(Ts&& ...args)
	{

		std::initializer_list<int> ignore = { (Params(args), 0)... };
		(void)ignore; // Hack that prevents the initializer_list from being optimized and the "ignore variable is unused" warning
		return httpsreq(toSource);
	};


}