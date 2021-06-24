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
	//HttpsRequest(Url{ "https://archive-4.kali.org/kali-images/kali-2021.1/kali-linux-2021.1-live-amd64.iso" }, Download{});

	Response r;
	//r = HttpsRequest(Url{ "www.postman-echo.com/get" }, Header{ {"Referer","www.bla.com"},{"Content-Type","*/*"} }, Parameters{ {"fruit","mango"},{"price","£3"} });
	//cout << endl << r.text << endl;

	//posting raw data - does not get url encoded
	//r = HttpsRequest(Url{ "www.postman-echo.com/post" }, Body{ "£" }, Method{ "POST" });
	//cout << endl << r.text << endl;
	//url form encode - key value pairs
	r = HttpsRequest(Url{ "www.httpbin.org/post" }, Payload{ {"name","习近平"} }, Method{ "POST" }, Timeout{1000});
	cout << endl << r.text << endl;

	//note: to upload to file.io do not use www in url, and always use filename file
	//r = HttpsRequest(Url{ "file.io" }, Multipart{ {"file:file","sample.txt"} }, Method{ "POST" });
	//cout << endl << r.text << endl;

	r = HttpsRequest(Url{ "https://www.httpbin.org/basic-auth/user/passwd" }, Authentication{  "user","passwd"  });
	cout << endl << r.text << endl;
	
	//receiving a cookie 
	/*r = HttpsRequest(Url{ "https://www.httpbin.org/cookies/set?cookie=yummy" });
	cout << endl << r.text << endl;
	cout << "status code: " + r.status_code << endl;
	cout << "redirect count: " << r.redirect_count << endl;
	cout << "rcd byte count: " << r.downloaded_bytes << endl;
	cout << "recd cookies map:" << endl;
	for (auto elem : r.cookies)
	{
		cout << elem.first + " : " + elem.second + "\r\n";
	}
	*/
	//sending a cookie with request
/*

	cout << "sent headers map:" << endl;
	for (auto elem : r.sent_headers)
	{
		cout << elem.first + " : " + elem.second + "\r\n";
	}

	cout << "recd headers map:" << endl;
	for (auto elem : r.header)
	{
		cout << elem.first + " : " + elem.second + "\r\n";
	}

	*/
	system("PAUSE");
	return 0;
}


