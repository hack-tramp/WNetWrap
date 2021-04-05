#include "wnetwrap.h"

#pragma comment(lib, "Wininet.lib")

wrap::resp wrap::HttpsRequest(std::string site, wrap::req request, std::string dload = "") {
	wrap::resp output;
	HINTERNET hInternet = InternetOpenA(request.ua.c_str(), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);

	if (hInternet == NULL)
	{
		output.err = "InternetOpen failed: " + GetLastError();
		return output;
	}
	else
	{
		//do some very basic URI parsing to separate host (for InternetConnect) from path (used in HttpOpenRequest)
		//also to see what protocol is specified
		std::string host, path, protocol, urlfile = "";

		//protocol and host
		host = site;
		if (host.find("://") != std::string::npos) { //get protocol if available
			protocol = host.substr(0, host.find(":"));
			host = host.substr(host.find("://") + 3);
			//cout << "clipped host: " + host << endl;
		}
		else { //otherwise assume it's https 
			protocol = "https";
		}
		//default is https
		DWORD service = INTERNET_SERVICE_HTTP;
		INTERNET_PORT port = INTERNET_DEFAULT_HTTPS_PORT;

		if (protocol == "https") {
			port = INTERNET_DEFAULT_HTTPS_PORT;
		}
		else if (protocol == "http") {
			port = INTERNET_DEFAULT_HTTP_PORT;
		}
		else if (protocol == "ftp") {
			port = INTERNET_DEFAULT_FTP_PORT;
			service = INTERNET_SERVICE_FTP;
		}

		//path (includes fragments, params etc)
		//assume there is a path if url has a further ? or /
		if ((host.find("/") != std::string::npos) || (host.find("?") != std::string::npos)) {
			//and something actually comes after it - i.e. the ? or / isnt the last char
			if ((host.back() != '?') && (host.back() != '/')) {
				if (host.find("/") != std::string::npos) {
					path = host.substr(host.find("/"));
					host = host.substr(0, host.find("/"));
				}
				else {
					path = host.substr(host.find("?"));
					host = host.substr(0, host.find("?"));
				}
				//if theres a file extension, store filename
				if (path.find_last_of("/")!= std::string::npos) {
					if (path.substr(path.find_last_of("/") + 1).find(".") != std::string::npos) {
						urlfile = path.substr(path.find_last_of("/") + 1);
					}
				}
				else {
					if (path.find_last_of("?") != std::string::npos) {
						if (path.substr(path.find_last_of("?") + 1).find(".") != std::string::npos) {
							urlfile = path.substr(path.find_last_of("?") + 1);
						}
					}
				}
			}
			else { //if its the last char, trim because wininet doesnt like a trailing / or ?
				host = host.substr(0, host.size() - 1);
				//std::cout << "trimmed last char of host" << std::endl;
			}
		}
		//entering dl means the file is saved as its original filename
		if (dload == "dl") {
			dload = urlfile;
		}
		std::cout << port << std::endl;
		std::cout << service << std::endl;
		std::cout << "host: "+host << std::endl << "path: "+path << std::endl << "protocol: "+protocol << std::endl << "file: " + urlfile << std::endl << "input url: " + site << std::endl;
		HINTERNET hConnect = InternetConnectA(hInternet, host.c_str(), port, NULL, NULL, service, 0, NULL);


		if (hConnect == NULL)
		{
			output.err = "InternetConnect failed: " + GetLastError();
			return output;
		}
		else
		{
			//eg path "Dir1/Dir2/Login.php?page=1" - might need leading /
			HINTERNET hRequest = HttpOpenRequestA(hConnect, request.method.c_str(), path.c_str(), "HTTP/1.1", NULL, request.AcceptedTypes, INTERNET_FLAG_SECURE, 0);

			if (hRequest == NULL)
			{
				output.err = "HttpOpenRequest failed: " + GetLastError();
				return output;
			}
			else
			{
				//headers will not work with non ASCII chars 
				//see remarks here: https://docs.microsoft.com/en-us/windows/win32/api/wininet/nf-wininet-httpsendrequestw

				//assemble headers from map
				std::string final_headers = "";
				for (auto elem : request.headers)
				{
					final_headers += elem.first + ":" + elem.second + "\r\n";
				}
				if (final_headers != "") {
					final_headers += "\r\n\r\n"; //null terminated string
					
				}


				std::string pdata = "";
				DWORD pdlength = 0;
				//if this is a post request, prepare the post data
				if (request.method == "POST") {
					pdata = request.postdata;
					if (pdata.size() < 4294967295) { //to get rid of size_t to dword c4267
						pdlength = (DWORD)pdata.size();
					}

				}
				//std::cout << "final headers c string:" << std::endl;
				//std::cout << final_headers.c_str() << std::endl;
				//std::cout << final_headers.size() << std::endl;
				//BOOL addhdr = HttpAddRequestHeadersA(hRequest, final_headers.c_str(), -1L, HTTP_ADDREQ_FLAG_ADD);
				BOOL sendr = HttpSendRequestA(hRequest, final_headers.c_str(), -1L, (LPVOID) pdata.c_str(), pdlength);
				if (!sendr)
				{
					output.err = "HttpSendRequest failed with error code " + GetLastError();
					return output;
				}
				else
				{
					std::string strResponse;
					const int nBuffSize = 1024;
					char buff[nBuffSize];
					FILE* pfile = nullptr;
					if (dload != "") {
						pfile = fopen(dload.c_str(), "wb");
					}
					
					BOOL bKeepReading = true;
					DWORD dwBytesRead = -1;

					while (bKeepReading && dwBytesRead != 0)
					{
						bKeepReading = InternetReadFile(hRequest, buff, nBuffSize, &dwBytesRead);
						
						if (pfile != nullptr) {
							fwrite(buff, sizeof(char), dwBytesRead, pfile);
						}
						else {
							strResponse.append(buff, dwBytesRead);
						}
						
					}
					if (pfile!=nullptr) {
						fflush(pfile);
						fclose(pfile);
					}



					//get headers recd
					char* recd_headers = (char*)calloc(1, sizeof(char));
					char* temp; //for C6308

					//get the size headers
					DWORD d = 1, d2 = 0;
					HttpQueryInfoA(hRequest, HTTP_QUERY_RAW_HEADERS_CRLF, recd_headers, &d, &d2);
					//alloc some space for the headers
					temp = (char*)realloc(recd_headers, d * sizeof(char)); //done to avoid C6308

					if (temp != NULL) {
						recd_headers = temp;
					}

					if (!HttpQueryInfoA(hRequest, HTTP_QUERY_RAW_HEADERS_CRLF, recd_headers, &d, &d2)) {
						//error handling
					}
					else {
						if (recd_headers != NULL) {
							std::string s(recd_headers, d);
							//break headers std::string into map
							std::string delimiter = "\n";
							size_t pos = 0;
							std::string token, k, v;
							while ((pos = s.find(delimiter)) != std::string::npos) {
								token = s.substr(0, pos);
								if (token.find(":") != std::string::npos) { //filters out lines without :, e.g. 200 OK
									output.received_headers.insert(std::pair<std::string, std::string>(token.substr(0, token.find(":")), token.substr(token.find(":") + 1)));
								}
								s.erase(0, pos + delimiter.length());
							}
						}
					}

					free(recd_headers);

					//get the headers we sent
					char* sent_headers = (char*)calloc(1, sizeof(char));;

					//get the size headers
					d = 1;
					d2 = 0;
					HttpQueryInfoA(hRequest, HTTP_QUERY_RAW_HEADERS_CRLF | HTTP_QUERY_FLAG_REQUEST_HEADERS, sent_headers, &d, &d2);

					temp = (char*)realloc(sent_headers, d * sizeof(char)); //done to avoid C6308
					if (temp != NULL) {
						sent_headers = temp;
					}


					if (!HttpQueryInfoA(hRequest, HTTP_QUERY_RAW_HEADERS_CRLF | HTTP_QUERY_FLAG_REQUEST_HEADERS, sent_headers, &d, &d2)) {
						//error handling
					}
					else {

						if (sent_headers != NULL) {
							std::cout << std::endl << sent_headers << std::endl;
							std::string s(sent_headers, d);
							//break headers std::string into map
							std::string delimiter = "\n";
							size_t pos = 0;
							std::string token;
							while ((pos = s.find(delimiter)) != std::string::npos) {

								token = s.substr(0, pos);
								//cout << "processing:\n" + token << endl;
								if (token.find(":") != std::string::npos) {
									std::string first = token.substr(0, token.find(":"));
									std::string second = token.substr(token.find(":") + 1);
									//cout << "adding: " + first +" " + second << endl;
									//NOTE: SENT HEADER KEYS ARE RETURNED WITH A TRAILING SPACE BY WININET - RECD HEADER KEYS ARENT
									//FOR THIS REASON FOR SENT HEADERS WE DO SUBSTR 0, token.find(":") - 1 
									output.sent_headers.insert(std::pair<std::string, std::string>(token.substr(0, token.find(":") ), token.substr(token.find(":") + 1)));
								}
								s.erase(0, pos + delimiter.length());
							}
						}
					}
					free(sent_headers);
					//free(temp); crashes

					//get security info - see here : https://stackoverflow.com/questions/41187935/can-not-programmatically-determine-which-tls-version-my-app-uses
					INTERNET_SECURITY_CONNECTION_INFO connInfo = { 0 };
					DWORD certInfoLength = sizeof(INTERNET_SECURITY_CONNECTION_INFO);
					InternetQueryOption(hRequest, INTERNET_OPTION_SECURITY_CONNECTION_INFO, &connInfo, &certInfoLength);
					//cout << connInfo.connectionInfo.dwProtocol << endl;
					switch (connInfo.connectionInfo.dwProtocol) {
					case(SP_PROT_TLS1_2_CLIENT): output.protocol = "Transport Layer Security 1.2 client-side"; break;
					case(SP_PROT_TLS1_1_CLIENT): output.protocol = "Transport Layer Security 1.1 client-side"; break;
					case(SP_PROT_TLS1_CLIENT): output.protocol = "Transport Layer Security 1.0 client-side"; break;
					case(SP_PROT_TLS1_SERVER): output.protocol = "Transport Layer Security 1.0 server-side"; break;
					case(SP_PROT_SSL3_CLIENT): output.protocol = "Secure Sockets Layer 3.0 client-side."; break;
					case(SP_PROT_SSL3_SERVER): output.protocol = "Secure Sockets Layer 3.0 server-side."; break;
					case(SP_PROT_TLS1_1_SERVER): output.protocol = "Transport Layer Security 1.1 server-side."; break;
					case(SP_PROT_TLS1_2_SERVER): output.protocol = "Transport Layer Security 1.2 server-side."; break;
					case(SP_PROT_SSL2_CLIENT): output.protocol = "Secure Sockets Layer 2.0 client-side. Superseded by SP_PROT_TLS1_CLIENT."; break;
					case(SP_PROT_SSL2_SERVER): output.protocol = "Secure Sockets Layer 2.0 server-side. Superseded by SP_PROT_TLS1_SERVER. "; break;
					case(SP_PROT_PCT1_CLIENT): output.protocol = "Private Communications Technology 1.0 client-side. Obsolete."; break;
					case(SP_PROT_PCT1_SERVER): output.protocol = "Private Communications Technology 1.0 server-side. Obsolete."; break;
					}

					char* cert_info_string = new char[2048];
					cert_info_string[0] = '\0';
					DWORD cert_info_length = 2048;

					if (!InternetQueryOption(hRequest, INTERNET_OPTION_SECURITY_CERTIFICATE, cert_info_string, &cert_info_length))
					{
						output.err = "InternetQueryOption failed " + GetLastError();
						return output;
					}
					output.certificate = cert_info_string;
					delete[] cert_info_string;

					//cout << output.certificate << endl;
					//cout << output.protocol << endl;

					output.raw = strResponse;
					output.text = wrap::text_from_html(strResponse);

					return output;
				}
				InternetCloseHandle(hRequest);
			}
			InternetCloseHandle(hConnect);
		}
		InternetCloseHandle(hInternet);
	}



}

// not a proper parser
std::string wrap::text_from_html(std::string html) {
	std::string output = "";
	bool over_tag = false; //whether we are currently going over a tag
	bool store = true; //whether we're storing what we're going over
	bool js = false;
	bool css = false;
	for (std::string::size_type i = 0; i < html.size(); ++i) {
		if (html[i] == '<') {
			if (html[i + 1] == '/') {
				//cout << "close tag" << endl;
				if (html.substr(i + 2, 5) == "style") {
					//cout << "close style tag" << endl;
					css = false;
				}
				if (html.substr(i + 2, 6) == "script") {
					//cout << "close js tag" << endl;
					js = false;
				}
			}
			else {
				//cout << "open tag" << endl;
				if (html.substr(i + 1, 5) == "style") {
					//cout << "open style tag" << endl;
					css = true;
				}
				if (html.substr(i + 1, 6) == "script") {
					//cout << "open js tag" << endl;
					js = true;
				}
			}
			over_tag = true;
		}
		if ((!js) && (!css) && (!over_tag)) {
			output = output + (html[i]);
		}
		if (html[i] == '>') {
			over_tag = false;
		}

	}

	return output;
}

