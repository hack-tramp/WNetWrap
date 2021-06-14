#include <iostream>

#include "wnetwrap.h"


using namespace wrap;
using namespace std;
int main()
{

	/*
	cout << "status code: " + r.status_code << endl;
	cout << r.text << endl;
	cout << "received header:" << endl;
	cout << r.header["content-type"] << endl;

	cout << "sent headers map:" << endl;
	for (auto elem : r.sent_headers)
	{
		cout << elem.first + " : " + elem.second + "\r\n";
	}
*/
	//HttpsRequest(Url{ "https://github.com/whoshuu/cpr/archive/refs/tags/1.6.0.zip" }, Download{});

	Response r;
	r = HttpsRequest(Url{ "www.postman-echo.com/get" }, Header{ {"Referer","www.bla.com"},{"Content-Type","*/*"} }, Parameters{ {"fruit","mango"},{"price","£3"} });
	cout << endl << r.text << endl;

	//posting raw data - does not get url encoded
	r = HttpsRequest(Url{ "www.postman-echo.com/post" }, Body{ "£" }, Method{ "POST" });
	cout << endl << r.text << endl;
	//url form encode - key value pairs
	r = HttpsRequest(Url{ "www.httpbin.org/post" }, Payload{ {"name","习近平"} }, Method{ "POST" });
	cout << endl << r.text << endl;

	//note: to upload to file.io do not use www in url, and always use filename file
	r = HttpsRequest(Url{ "file.io" }, Multipart{ {"file:file","sample.txt"} }, Method{ "POST" });
	cout << endl << r.text << endl;

	r = HttpsRequest(Url{ "https://www.httpbin.org/basic-auth/user/passwd" }, Authentication{  "user","passwd"  });
	cout << endl << r.text << endl;

/*
	my_request.method = "POST";
	my_request.set_header("Content-Type:", "application/json");
	my_request.postdata = "{\"b\":\"a\"}";
	my_response = HttpsRequest("https://postman-echo.com/post", my_request);

	cout << my_response.raw << endl;

	cout << "sent headers map:" << endl;
	for (auto elem : my_response.sent_headers)
	{
		cout << elem.first + " : " + elem.second + "\r\n";
	}

	cout << "recd headers map:" << endl;
	for (auto elem : my_response.received_headers)
	{
		cout << elem.first + " : " + elem.second + "\r\n";
	}

	*/
	system("PAUSE");
	return 0;
}


