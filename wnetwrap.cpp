﻿#include "wnetwrap.h"
#pragma comment(lib, "Wininet.lib")
#pragma comment( lib, "urlmon" )

DWORD WINAPI WorkerInternetConnect(LPVOID);
DWORD WINAPI WorkerInternetRequest(LPVOID);
struct TPARAMS {
	HINTERNET hInternet = NULL;
	HINTERNET hInternetOut = NULL; //returned handle
	std::string host = "";
	INTERNET_PORT port = 0;
	DWORD service = 0;
};
struct TRPARAMS { 
	HINTERNET hRequest = NULL;
	BOOL res = NULL;
	std::string headers = "";
};

wrap::req wrap::toSource;

//adapted from https://stackoverflow.com/questions/20634666/get-a-mime-type-from-a-extension-in-c
std::string GetMimeType(const std::string& ext) {

	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	
	std::wstring str = converter.from_bytes(ext);

	LPWSTR pwzMimeOut = NULL;
	HRESULT hr = FindMimeFromData(NULL, str.c_str(), NULL, 0,
		NULL, FMFD_URLASFILENAME, &pwzMimeOut, 0x0);
	if (SUCCEEDED(hr)) {
		std::wstring strResult(pwzMimeOut);
		// Despite the documentation stating to call operator delete, the
		// returned string must be cleaned up using CoTaskMemFree
		CoTaskMemFree(pwzMimeOut);
		std::string narrow = converter.to_bytes(strResult);
		return narrow;
	}
	return "application/unknown";
}

static std::string base64_encode(const std::string& in) {

	std::string out;

	int val = 0, valb = -6;
	for (unsigned char c : in) {
		val = (val << 8) + c;
		valb += 8;
		while (valb >= 0) {
			out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(val >> valb) & 0x3F]);
			valb -= 6;
		}
	}
	if (valb > -6) out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[((val << 8) >> (valb + 8)) & 0x3F]);
	while (out.size() % 4) out.push_back('=');
	return out;
}

std::string hex_encode(char const c)
{
	char s[3];

	if (c & 0x80)
	{
		std::snprintf(&s[0], 3, "%02X",
			static_cast<unsigned int>(c & 0xff)
		);
	}
	else
	{
		std::snprintf(&s[0], 3, "%02X",
			static_cast<unsigned int>(c)
		);
	}

	return std::string(s);
}

std::string url_encode(std::string const& str)
{
	std::string res;
	res.reserve(str.size());
	bool form = true;
	for (auto const& e : str)
	{
		if (e == ' ' && form)
		{
			res += "+";
		}
		else if (std::isalnum(static_cast<unsigned char>(e)) ||
			e == '-' || e == '_' || e == '.' || e == '~')
		{
			res += e;
		}
		else
		{
			res += "%" + hex_encode(e);
		}
	}

	return res;
}

void wrap::Params(wrap::Parameters p) {
	
	//assemble parameters from map
	std::string final_params = "";
	int pcount = 0;
	for (auto elem : p.p)
	{
		pcount++;
		if (elem.first != "") {
			if (pcount > 1) { // add & if we're after 1st param
				final_params += "&";
			}
			final_params += url_encode(elem.first) + "=" + url_encode(elem.second);
		}
	}
	wrap::toSource.Params = "?" + final_params;
	//std::cout << "Parameters url encoded:" << std::endl;
	//std::cout << wrap::toSource.Params << std::endl;
}

void wrap::Params(wrap::Url u) {
	wrap::toSource.Url = u.adr;
	//std::cout << "got url: " + u.adr << std::endl;
}

void wrap::Params(wrap::Header h) {
	wrap::toSource.Header = h;
}

void wrap::Params(wrap::Method m) {
	wrap::toSource.Method = m.method;
}

void wrap::Params(wrap::Download dl) {
	wrap::toSource.Dl = dl.dl;
}

void wrap::Params(wrap::File f) {
	std::string fname = f.file;
	// read entire file into string
	if (std::ifstream inputstream{ fname, std::ios::binary | std::ios::ate }) {
		auto fsize = inputstream.tellg();
		std::string finput(fsize, '\0'); // construct string to stream size
		inputstream.seekg(0);
		if (inputstream.read(&finput[0], fsize))
			std::cout << finput << '\n';
			wrap::toSource.PostData = finput;
	}
}

void wrap::Params(wrap::Body b) {
	wrap::toSource.PostData = b.body;
}

void wrap::Params(wrap::Payload pd) {
	//assemble payload from map
	std::string final_postdata = "";
	int pcount = 0;
	for (auto elem : pd.pd)
	{
		pcount++;
		if (elem.first != "") {
			if (pcount > 1) { // add & if we're after 1st param
				final_postdata += "&";
			}
			final_postdata += url_encode(elem.first) + "=" + url_encode(elem.second);
		}
	}
	wrap::toSource.PostData = final_postdata;
	//std::cout << "Payload data url encoded:" << std::endl;
	//std::cout << toSource.PostData << std::endl;
}

void wrap::Params(wrap::Multipart mp) {
	//assemble multipart post payload from map
	std::string final_postdata = "";
	std::string boundary = "735323031399963166993862150";
	wrap::toSource.Header.hdr["content-type"] = "multipart/form-data; boundary=" + boundary;
	std::string fname, ctype;
	int pcount = 0;
	for (auto elem : mp.mp)
	{
		pcount++;

		if (elem.first.substr(0,5) != "file:") { // if theres no file: prefix, assume this is not a file

			final_postdata += "--" + boundary + "\r\n";
			final_postdata += "Content-Disposition: form-data; name=\"" + (elem.first) + "\"\r\n";
			final_postdata += "Content-Type: text/plain\r\n\r\n";
			final_postdata += (elem.second) + "\r\n";
		}
		else { //if we're sending a file

			fname = elem.second.substr(elem.second.find_last_of("/\\") + 1); //used for filename
			ctype = "application/octet-stream"; //used as default for file content-type

			//try to see if we can match the MIME type, by getting the file extension
			if (fname.find_last_of(".") != std::string::npos) { //use find_last_of to ignore dots in filename 
				std::string extension = fname.substr(fname.find_last_of("."));
				if (GetMimeType(extension) != "application/unknown") {
					ctype = GetMimeType(extension);
				}
			}

			final_postdata += "--" + boundary + "\r\n";
			final_postdata += "Content-Disposition: form-data; name=\"" + (elem.first.substr(5)) + "\"; filename=\"" + fname + "\"\r\n";
			final_postdata += "Content-Type: " + ctype + "\r\n\r\n";

			// read entire file into string
			if (std::ifstream inputstream{ elem.second, std::ios::binary | std::ios::ate }) {
				auto fsize = inputstream.tellg();
				std::string finput(fsize, '\0'); // construct string to stream size
				inputstream.seekg(0);
				inputstream.read(&finput[0], fsize);
				final_postdata += finput + "\r\n"; //add to postdata
			}
		}

		if (pcount == mp.mp.size()) { //if we've reached the last map element, add -- to the end of the last boundary, as per RFC spec
			final_postdata += "--" + boundary + "--";
		}
	}
	wrap::toSource.PostData = final_postdata;
	//std::cout << "multipart payload:" << std::endl;
	//std::cout << final_postdata << std::endl;
}

void wrap::Params(wrap::Authentication auth) {
	std::string basicauth = auth.usr + ":" + auth.pwd;
	basicauth = base64_encode(basicauth);
	wrap::toSource.Header.hdr["Authorization"] = "Basic " + basicauth;
}

void wrap::Params(wrap::Bearer token) { //https://www.oauth.com/oauth2-servers/making-authenticated-requests/
	wrap::toSource.Header.hdr["Authorization"] = "Bearer " + token.token;
}

void wrap::Params(wrap::Timeout timeout) {
	
	if (timeout.type == "connection" || timeout.type == "all") {
		wrap::toSource.TimeoutConnect = timeout.timeout;
	}
	if (timeout.type == "request" || timeout.type == "all") {
		wrap::toSource.TimeoutRequest = timeout.timeout;
	}
}

wrap::Response wrap::httpsreq(wrap::req request) {
	wrap::Response output;
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
		std::string host, path, scheme, urlfile = "";

		//scheme and host
		host = request.Url;
		if (host.find("://") != std::string::npos) { //get scheme if available
			scheme = host.substr(0, host.find(":"));
			host = host.substr(host.find("://") + 3);
			//cout << "clipped host: " + host << endl;
		}
		else { //otherwise assume it's https 
			scheme = "https";
		}
		//default is https
		DWORD service = INTERNET_SERVICE_HTTP;
		INTERNET_PORT port = INTERNET_DEFAULT_HTTPS_PORT;

		if (scheme == "https") {
			port = INTERNET_DEFAULT_HTTPS_PORT;
		}
		else if (scheme == "http") {
			port = INTERNET_DEFAULT_HTTP_PORT;
		}
		else {
			output.err = "Error: URL input has scheme other than HTTP/S";
			return output;
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
				if (path.find_last_of("/")!= std::string::npos) { // dot after last /
					if (path.substr(path.find_last_of("/") + 1).find(".") != std::string::npos) {
						urlfile = path.substr(path.find_last_of("/") + 1);
					}
				}
				else {
					if (path.find_last_of("?") != std::string::npos) { // dot after ?
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

		//add params that were passed as Parameters map
		if (request.Params != "") {
			path += request.Params;
		}

		output.url = scheme + "://" + host + path;

		//entering dl means the file is saved as its original filename
		if (request.Dl == "dl") {
			request.Dl = urlfile;
		}
		//std::cout << port << std::endl;//std::cout << service << std::endl;
		//std::cout << "host: "+host << std::endl << "path: "+path << std::endl << "scheme: "+scheme << std::endl << "file: " + urlfile << std::endl << "input url: " + site << std::endl;
		HINTERNET hConnect = NULL;
		if (wrap::toSource.TimeoutConnect == 0) { //if no timeout set then just do normal call
			hConnect = InternetConnectA(hInternet, host.c_str(), port, NULL, NULL, service, 0, NULL);
		} else { //if timeout set then use threads due to MS bug https://mskb.pkisolutions.com/kb/224318
			TPARAMS params;
			params.hInternet = hInternet;
			params.host = host;
			params.port = port;
			params.service = service;
			HANDLE hThread;
			hThread = CreateThread(
				NULL,            // Pointer to thread security attributes 
				0,               // Initial thread stack size, in bytes 
				WorkerInternetConnect,  // Pointer to thread function 
				&params,     // lpParameter A pointer to a variable to be passed to the thread.
				0,               // Creation flags 
				0      // lpThreadId A pointer to a DWORD variable that receives the thread identifier.If this parameter is NULL, the thread identifier is not returned.
			);

			if (hThread == 0) { //this is done to avoid possibility of WaitForSingleObject or GetExitCodeThread called if hThread is 0
				output.err = "Could not create thread for timeout.";
				return output;
			}

			// Wait for the call to InternetConnect in worker function to complete
			if (WaitForSingleObject(hThread, wrap::toSource.TimeoutConnect) == WAIT_TIMEOUT)
			{
				std::cout << "Can not connect to server in " << wrap::toSource.TimeoutConnect << " milliseconds" << std::endl;
				if (hInternet)
					InternetCloseHandle(hInternet);
				// Wait until the worker thread exits
				WaitForSingleObject(hThread, INFINITE);
				//std::cout << "Thread has exited" << std::endl;
				wrap::toSource.reset(); //reset timeout for next request
				output.err = "InternetConnect Thread has exited ";
				return output;
			}

			// The state of the specified object (thread) is signaled
			DWORD dwExitCode = 0;
			if (!GetExitCodeThread(hThread, &dwExitCode))
			{
				//std::cerr << "Error on GetExitCodeThread: " << GetLastError() << std::endl;
				output.err = "Error on GetExitCodeThread: " + GetLastError();
				return output;
			}

			CloseHandle(hThread);

			if (dwExitCode) {
				// Worker function failed
				//std::cout << "non 0 exit code" << std::endl;
				output.err = "Worker function failed";
				return output;
			}
			
			hConnect = params.hInternetOut;

		}
		


		if (hConnect == NULL)
		{
			output.err = "InternetConnect failed: " + GetLastError();
			return output;
		}
		else
		{
			//eg path "Dir1/Dir2/Login.php?page=1" - might need leading /
			//accept types param passed as NULL because we use the Accept header field instead

			HINTERNET hRequest = HttpOpenRequestA(hConnect, request.Method.c_str(), path.c_str(), NULL , NULL, NULL, INTERNET_FLAG_SECURE , 0);

			if (hRequest == NULL)
			{
				output.err = "HttpOpenRequest failed: " + GetLastError();
				return output;
			}
			else
			{
				// POST stuff done before headers to potentially add content-type if needed, before header assembly
				if (request.Method == "POST") {
					//if no content-type header was set, set it to application/x-www-form-urlencoded
					//rAnDOm capitalization will break this check
					if ((request.Header.hdr.find("content-type") == request.Header.hdr.end()) && (request.Header.hdr.find("Content-Type") == request.Header.hdr.end())) {
						request.Header.hdr.insert(std::pair<std::string, std::string>("Content-Type", "application/x-www-form-urlencoded"));
					}
				}

				//if its a GET request,  add content-type if needed, before header assembly
				if (request.Method == "GET") {
					//if no content-type header was set, set it to text/plain
					//rAnDOm capitalization will break this check
					if ((request.Header.hdr.find("content-type") == request.Header.hdr.end()) && (request.Header.hdr.find("Content-Type") == request.Header.hdr.end())) {
						request.Header.hdr.insert(std::pair<std::string, std::string>("Content-Type", "text/plain"));
					}
				}

				//assemble headers from map - note headers will not work with non ASCII chars 
				//see remarks here: https://docs.microsoft.com/en-us/windows/win32/api/wininet/nf-wininet-httpsendrequestw
				std::string final_headers = "";
				for (auto elem : request.Header.hdr)
				{
					if (elem.first != "") {
						final_headers += elem.first + ":" + elem.second + "\r\n";
					}		
				}
				if (final_headers != "") {
					final_headers += "\r\n\r\n"; //null terminated string
				}

				if (request.Method == "POST") {
					//std::cout << "charpost data:" << std::endl;
					//std::cout << &wrap::toSource.PostData[0] << '\n';
				}

				BOOL sendr = NULL;

				if (wrap::toSource.TimeoutRequest == 0) { //if no timeout set then just do normal call 
					//sending data as bytes (c string) to ensure files e.g. images or zip files get transfered ok - see here: https://stackoverflow.com/a/16502000/13666347
					sendr = HttpSendRequestA(hRequest, final_headers.c_str(), -1L, &wrap::toSource.PostData[0], sizeof(char) * wrap::toSource.PostData.size());
				}
				else { //if timeout set then use threads due to MS bug https://mskb.pkisolutions.com/kb/224318
					TRPARAMS rparams;
					rparams.hRequest = hRequest;
					rparams.headers = final_headers;
					HANDLE rhThread;
					rhThread = CreateThread(
						NULL,            // Pointer to thread security attributes 
						0,               // Initial thread stack size, in bytes 
						WorkerInternetRequest,  // Pointer to thread function 
						&rparams,     // lpParameter A pointer to a variable to be passed to the thread.
						0,               // Creation flags 
						0      // lpThreadId A pointer to a DWORD variable that receives the thread identifier.If this parameter is NULL, the thread identifier is not returned.
					);

					if (rhThread == 0) { //this is done to avoid possibility of WaitForSingleObject or GetExitCodeThread called if hThread is 0
						output.err = "Could not create thread for timeout.";
						return output;
					}

					// Wait for the call to InternetConnect in worker function to complete
					if (WaitForSingleObject(rhThread, wrap::toSource.TimeoutRequest) == WAIT_TIMEOUT)
					{
						std::cout << "Can not send request to server in " << wrap::toSource.TimeoutRequest << " milliseconds" << std::endl;
						if (hInternet)
							InternetCloseHandle(hInternet);
						// Wait until the worker thread exits
						WaitForSingleObject(rhThread, INFINITE);
						//std::cout << "Thread has exited" << std::endl;
						wrap::toSource.reset(); //reset to remove timeout for next request
						output.err = "InternetConnect Thread has exited ";
						return output;
					}

					// The state of the specified object (thread) is signaled
					DWORD dwExitCode = 0;
					if (!GetExitCodeThread(rhThread, &dwExitCode))
					{
						//std::cerr << "Error on GetExitCodeThread: " << GetLastError() << std::endl;
						output.err = "Error on GetExitCodeThread: " + GetLastError();
						return output;
					}

					CloseHandle(rhThread);

					if (dwExitCode) {
						// Worker function failed
						//std::cout << "non 0 exit code" << std::endl;
						output.err = "Worker function failed";
						return output;
					}

					sendr = rparams.res;

				}
				
				
				if (!sendr)
				{
					output.err = "HttpSendRequest failed with error code " + GetLastError();
					return output;
				}
				else
				{
					std::string strResponse;
					const int nBuffSize = 1024;
					char buff[nBuffSize]; //use bytes rather than string to help with binary file downloads
			
					FILE* pfile = nullptr;
					if (request.Dl != "") {
						pfile = fopen(request.Dl.c_str(), "wb");
					}
					
					BOOL bKeepReading = true;
					DWORD dwBytesRead = -1;

					while (bKeepReading && dwBytesRead != 0)
					{
						
						bKeepReading = InternetReadFile(hRequest, buff, nBuffSize, &dwBytesRead);
						if (dwBytesRead != 0) {
							output.downloaded_bytes += dwBytesRead;
						}
						
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
					std::string received_headers;
					//get the size headers
					DWORD d = 0;

					get_received_headers:

					// This call will fail on the first pass, because no buffer is allocated.
					if (!HttpQueryInfoA(hRequest, HTTP_QUERY_RAW_HEADERS_CRLF, &received_headers[0], &d, NULL))
					{
						if (GetLastError() == ERROR_HTTP_HEADER_NOT_FOUND)
						{
							// Code to handle the case where the header isn't available.
						}
						else
						{
							// Check for an insufficient buffer.
							if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
							{
								received_headers.resize(d, '\0'); // Allocate the necessary buffer.
								goto get_received_headers;// Retry the call.
							}
							else
							{
								// Error handling code.
							}
						}
					}
					else { //no errors
						if (received_headers != "") {
							//break headers into map
							std::string delimiter = "\n";
							size_t pos = 0;
							std::string token, fieldname;
							while ((pos = received_headers.find(delimiter)) != std::string::npos) {
								token = received_headers.substr(0, pos);
								if (token.find(":") != std::string::npos) { //filters out lines without :, e.g. 200 OK
									//here we convert the header field name to lowercase
									//NOTE: this method does NOT support UTF-8 (only ascii) 
									//but should not be a problem as header field names are all standard ascii
									//header values are left as recd
									fieldname = token.substr(0, token.find(":"));
									for (auto& c : fieldname)
									{c = std::tolower(c);}
									//cookies are dealt with on the spot
									if (fieldname == "set-cookie") {
										std::string cval = token.substr(token.find(":") + 1);
										std::string cname = cval.substr(0, cval.find("="));
										//std::cout <<"cookie set: "<< cname << std::endl;
										std::string cookie_url = scheme + "://" + host;//ignoring path for now
										InternetSetCookieA(&cookie_url[0], &cname[0], &cval[0]); //set cookie via WinINet
										//also save cookie value (only) into 'cookies' map obj in response
										output.cookies.insert(std::pair<std::string, std::string>(cname, cval.substr(cval.find("=")+1, cval.find(";")- cval.find("=") - 1)));
									}
									output.header.insert(std::pair<std::string, std::string>(fieldname, token.substr(token.find(":") + 1)));
								}
								received_headers.erase(0, pos + delimiter.length());
							}
						}
					}


					std::string sent_headers;
					d = 0;

					get_sent_headers:

					// This call will fail on the first pass, because no buffer is allocated.
					if (!HttpQueryInfoA(hRequest, HTTP_QUERY_RAW_HEADERS_CRLF | HTTP_QUERY_FLAG_REQUEST_HEADERS,&sent_headers[0], &d, NULL))
					{
						if (GetLastError() == ERROR_HTTP_HEADER_NOT_FOUND)
						{
							// Code to handle the case where the header isn't available.
						}
						else
						{
							// Check for an insufficient buffer.
							if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
							{
								sent_headers.resize(d, '\0'); // Allocate the necessary buffer.
								goto get_sent_headers;// Retry the call.
							}
							else
							{
								// Error handling code.
							}
						}
					}
					else { //no errors
						if (sent_headers != "") {
							//std::cout << std::endl << sent_headers << std::endl;
							//std::string s(sent_headers, d);
							//break headers std::string into map
							std::string delimiter = "\n";
							size_t pos = 0;
							std::string token;
							while ((pos = sent_headers.find(delimiter)) != std::string::npos) {

								token = sent_headers.substr(0, pos);
								//cout << "processing:\n" + token << endl;
								if (token.find(":") != std::string::npos) {
									std::string first = token.substr(0, token.find(":"));
									std::string second = token.substr(token.find(":") + 1);
									//cout << "adding: " + first +" " + second << endl;
									//NOTE: SENT HEADER KEYS ARE RETURNED WITH A TRAILING SPACE BY WININET - RECD HEADER KEYS ARENT
									//FOR THIS REASON FOR SENT HEADERS WE DO SUBSTR 0, token.find(":") - 1 
									output.sent_headers.insert(std::pair<std::string, std::string>(token.substr(0, token.find(":")), token.substr(token.find(":") + 1)));
								}
								sent_headers.erase(0, pos + delimiter.length());
							}
						}
					}


					//get the status code
					DWORD statusCode = 0;
					DWORD length = sizeof(DWORD);
					if (HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &statusCode, &length, NULL)) {
						output.status_code = std::to_string(statusCode);
						if (statusCode==302 || statusCode==301) {
							std::cout << "redirect" << std::endl;
							output.redirect_count += 1;
						}
					}
					else {
						//error handling
					}
					

					//get security info - see here : https://stackoverflow.com/questions/41187935/can-not-programmatically-determine-which-tls-version-my-app-uses
					INTERNET_SECURITY_CONNECTION_INFO connInfo = { 0 };
					DWORD certInfoLength = sizeof(INTERNET_SECURITY_CONNECTION_INFO);
					InternetQueryOption(hRequest, INTERNET_OPTION_SECURITY_CONNECTION_INFO, &connInfo, &certInfoLength);
					//cout << connInfo.connectionInfo.dwProtocol << endl;
					switch (connInfo.connectionInfo.dwProtocol) {
					case(SP_PROT_TLS1_2_CLIENT): output.secinfo["protocol"] = "Transport Layer Security 1.2 client-side"; break;
					case(SP_PROT_TLS1_1_CLIENT): output.secinfo["protocol"] = "Transport Layer Security 1.1 client-side"; break;
					case(SP_PROT_TLS1_CLIENT): output.secinfo["protocol"] = "Transport Layer Security 1.0 client-side"; break;
					case(SP_PROT_TLS1_SERVER): output.secinfo["protocol"] = "Transport Layer Security 1.0 server-side"; break;
					case(SP_PROT_SSL3_CLIENT): output.secinfo["protocol"] = "Secure Sockets Layer 3.0 client-side."; break;
					case(SP_PROT_SSL3_SERVER): output.secinfo["protocol"] = "Secure Sockets Layer 3.0 server-side."; break;
					case(SP_PROT_TLS1_1_SERVER): output.secinfo["protocol"] = "Transport Layer Security 1.1 server-side."; break;
					case(SP_PROT_TLS1_2_SERVER): output.secinfo["protocol"] = "Transport Layer Security 1.2 server-side."; break;
					case(SP_PROT_SSL2_CLIENT): output.secinfo["protocol"] = "Secure Sockets Layer 2.0 client-side. Superseded by SP_PROT_TLS1_CLIENT."; break;
					case(SP_PROT_SSL2_SERVER): output.secinfo["protocol"] = "Secure Sockets Layer 2.0 server-side. Superseded by SP_PROT_TLS1_SERVER. "; break;
					case(SP_PROT_PCT1_CLIENT): output.secinfo["protocol"] = "Private Communications Technology 1.0 client-side. Obsolete."; break;
					case(SP_PROT_PCT1_SERVER): output.secinfo["protocol"] = "Private Communications Technology 1.0 server-side. Obsolete."; break;
					}

					switch (connInfo.connectionInfo.aiCipher) {
					case(CALG_3DES): output.secinfo["cipher"] = "3DES block encryption algorithm"; break;
					case(CALG_AES_128): output.secinfo["cipher"] = "AES 128-bit encryption algorithm"; break;
					case(CALG_AES_256): output.secinfo["cipher"] = "AES 256-bit encryption algorithm"; break;
					case(CALG_DES): output.secinfo["cipher"] = "DES encryption algorithm"; break;
					case(CALG_RC2): output.secinfo["cipher"] = "RC2 block encryption algorithm"; break;
					case(CALG_RC4): output.secinfo["cipher"] = "RC4 stream encryption algorithm"; break;
					case(0): output.secinfo["cipher"] = "No encryption"; break;
					}

					output.secinfo["cipher_strength"] = std::to_string(connInfo.connectionInfo.dwCipherStrength);
					
					switch (connInfo.connectionInfo.aiHash) {
					case(CALG_MD5): output.secinfo["hash"] = "MD5 hashing algorithm"; break;
					case(CALG_SHA): output.secinfo["hash"] = "SHA hashing algorithm"; break;
					}

					if (output.secinfo["hash"] != "") {
						output.secinfo["hash_strength"] = std::to_string(connInfo.connectionInfo.dwHashStrength);
					}
					
					switch (connInfo.connectionInfo.aiExch) {
					case(CALG_RSA_KEYX): output.secinfo["key_exch"] = "RSA key exchange"; break;
					case(CALG_DH_EPHEM): output.secinfo["key_exch"] = "Diffie-Hellman key exchange"; break;
					}

					if (output.secinfo["key_exch"] != "") {
						output.secinfo["key_exch_strength"] = std::to_string(connInfo.connectionInfo.dwExchStrength);
					}

					std::string cert_info_string;
					DWORD cert_info_length = 2048;
					cert_info_string.resize(cert_info_length, '\0');

					if (!InternetQueryOptionA(hRequest, INTERNET_OPTION_SECURITY_CERTIFICATE, &cert_info_string[0], &cert_info_length))
					{
						output.err = "InternetQueryOption failed " + GetLastError();
						return output;
					}

					output.secinfo["certificate"] = cert_info_string;

					output.raw = strResponse;
					//very basic check to see if its a html doc - if yes then do some very basic parsing to try to get the text content
					std::string doctype = strResponse.substr(0, 14);
					//std::transform(doctype.begin(), doctype.end(), doctype.begin(), ::tolower);
					for (auto& c : doctype)
					{c = std::tolower(c);}
					if (doctype=="<!doctype html") {
						std::cout << "html detected!" << std::endl;
						output.text = wrap::text_from_html(strResponse);
					}
					else { //if no html found, .text = .raw
						output.text = output.raw;
					}
					
					//return request object back to defaults by replacing it with a new struct
					//this gets rid of params etc
					wrap::toSource.reset();

					return output;
				}
				InternetCloseHandle(hRequest);
			}
			InternetCloseHandle(hConnect);
		}
		InternetCloseHandle(hInternet);
	}



}
template <typename... Ts>
wrap::Response wrap::HttpsRequest(Ts&& ...args);


/////////////////// WorkerFunctions ////////////////////// 
DWORD WINAPI WorkerInternetConnect(IN LPVOID vThreadParm) // https://docs.microsoft.com/en-us/previous-versions/windows/desktop/legacy/ms686736(v=vs.85)
{
	TPARAMS* params;
	params = (TPARAMS*)vThreadParm;
	HINTERNET g_hConnect = 0;
	if (!(g_hConnect = InternetConnectA(params->hInternet, params->host.c_str(), params->port, NULL, NULL, params->service, 0, NULL)))
	{
		//std::cerr << "Error on InternetConnnect: " << GetLastError() << std::endl;
		return 1; // failure
	}
	else {
		//std::cout << "Connected OK" << std::endl;
		params->hInternetOut = g_hConnect;
	}
	return 0;  // success
}

DWORD WINAPI WorkerInternetRequest(IN LPVOID vThreadParm) // https://docs.microsoft.com/en-us/previous-versions/windows/desktop/legacy/ms686736(v=vs.85)
{
	TRPARAMS* params;
	params = (TRPARAMS*)vThreadParm;
	BOOL g_hConnect = false; 
	if (!(g_hConnect = HttpSendRequestA(params->hRequest, params->headers.c_str(), -1L, &wrap::toSource.PostData[0], sizeof(char) * wrap::toSource.PostData.size())))
	{
		//std::cerr << "Error on HttpSendRequestA: " << GetLastError() << std::endl;
		return 1; // failure
	}
	else {
		//std::cout << "Connected OK" << std::endl;
		params->res = g_hConnect;
	}
	return 0;  // success
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

