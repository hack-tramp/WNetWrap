# WNetWrap  [![MSBuild](https://github.com/hack-tramp/wnetwrap/actions/workflows/msbuild.yml/badge.svg)](https://github.com/hack-tramp/wnetwrap/actions/workflows/msbuild.yml) [![Generic Badge](https://img.shields.io/badge/c%2B%2B-11-blue)](https://github.com/topics/c-plus-plus-11) [![GitHub license](https://img.shields.io/github/license/Naereen/StrapDown.js.svg)](https://github.com/hack-tramp/wnetwrap/blob/main/LICENSE)<br>
  
A tiny, dependency-free library using [WinINet](https://docs.microsoft.com/en-us/windows/win32/wininet/about-wininet) for HTTP(S) requests in C++. This is for developers targeting Windows only, who need a lightweight HTTPS solution that doesn't use additional libraries.

## Basic HTTP GET request

```c++
#include <iostream>
#include "wnetwrap.h"

using namespace wrap;
using namespace std;

int main()
{
	req my_request; //GET method and firefox user agent used by default
	resp my_response = HttpsRequest("https://www.example.com/", my_request);
	cout << my_response.text << endl; //very basic html parser
	cout << my_response.status_code << endl; // 200
 }
  ```
 
URL query parameters can be passed as normal - here is a Google search as an example:

```c++
HttpsRequest("https://www.google.com/search?q=testing+123", my_request);
```


## Downloading a file

To download the contents of the request you simply add a third parameter to `HttpsRequest`. If this parameter is `"dl"` then the file is downloaded with its original filename, otherwise the value provided will be the new file's name. For example, to download the CPR library: <br>
```c++
HttpsRequest("https://github.com/whoshuu/cpr/archive/refs/tags/1.6.0.zip", my_request, "dl");
```
When you download a file, the `.raw` and `.text` properties of the response object will be returned empty.

## Preparing the request

The `req` request object can be used for the following (all inputs are strings)

**Specifying the HTTP method**<br>
```c++ 
req my_request; my_request.method = "GET"; // already set to GET by default 
```

**Setting the user agent**
 
Firefox is used by default but you can specify your own, for example an Apple iPhone XR (Safari):<br>
```c++ 
my_request.ua = "Mozilla/5.0 (iPhone; CPU iPhone OS 12_0 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/12.0 Mobile/15E148 Safari/604.1";
```


**Setting a header:**<br>
```c++ 
my_request.set_header( "Referer" , "my.referer.com" );
```
<br>

**Updating a header**
 
Note that as HTTP header fields are case-insensitive, they will always be stored and sent in lowercase - this means this will still work:<br>
```c++ 
my_request.set_header("RefErEr", "my.bla.com");
``` 

**Posting data**
 
If you are sending data via POST you can set the data like this:<br>
```c++ 
my_request.postdata = "{\"b\":\"a\"}"
```

## Handling the response
 
**Retrieving the headers**

The response is stored into a `resp` object. To get info from the headers received use `get_header("header_field")`


```c++
resp my_response = HttpsRequest("https://www.example.com/", my_request);
cout << my_response.get_header("date")
```
<br>

For headers sent, use `get_header("header_field", "sent")`

<br>

```c++
resp my_response = HttpsRequest("https://www.example.com/", my_request);
cout << my_response.get_header("referer","sent")
```
<br>

To output all the headers in one go, you will need to cycle through the `received_headers` or `sent_headers` map:

<br>


```c++ 
cout << "recd headers map:" << endl;
for (auto elem : my_response.received_headers)
{
	cout << elem.first + " : " + elem.second + "\r\n";
}
``` 

## Getting security info

To see the response's security info, you will need to access the `secinfo` map. For example, to get the security certificate:
```c++
my_response.secinfo["certificate"]
```
For `www.example.com` this returns:
```
Subject:
US
California
Los Angeles
Internet Corporation for Assigned Names and Numbers
www.example.org
Issuer:
US
DigiCert Inc
DigiCert TLS RSA SHA256 2020 CA1
Effective Date: 24/11/2020 00:00:00
Expiration Date:        25/12/2021 23:59:59
Security Protocol:      (null)
Signature Type: (null)
Encryption Type:        (null)
Privacy Strength:       High (128 bits)
cipher : AES 128-bit encryption algorithm
```
Due to WinInet limitations, some data such as the protocol and encryption type may appear as `(null)` - however this may be found in other parts of the certificate, such as under `Issuer` above. This can also be found as one of several additional elements in the `secinfo` map:
```c++
cout << my_response.secinfo["protocol"]; // example.com: Transport Layer Security 1.2 client-side 
```
Cycling through the `secinfo` map will show all other available security info:
```c++ 
cout << "security info:" << endl;
for (auto elem : my_response.secinfo)
{
	cout << elem.first + " : " + elem.second + "\r\n";
}
``` 
This gives the map keys and values (I've omitted the certificate):
```
cipher : AES 128-bit encryption algorithm
cipher_strength : 128
hash : SHA hashing algorithm
hash_strength : 128
key_exch : RSA key exchange
key_exch_strength : 2048
protocol : Transport Layer Security 1.2 client-side
```

## HTTP POST request
Here we are sending a POST request with JSON data `{"b":"a"}` which is then echoed back to us:<br>
```c++ 
my_request.method = "POST";
my_request.set_header("Content-Type:", "application/json");
my_request.postdata = "{\"b\":\"a\"}";
my_response = HttpsRequest("https://postman-echo.com/post", my_request);

cout << my_response.raw << endl;
``` 

Note that we are outputting the response without parsing anything, using 
```c++
my_response.raw
```
<br> The result should be something like this: <br>

```
{"args":{},"data":{"b":"a"},"files":{},"form":{},"headers":{"x-forwarded-proto":"https","x-forwarded-port":"443","host":"postman-echo.com","x-amzn-trace-id":"Root=1-60634f18-2270c652666720526210e242","content-length":"9","accept":"*/*, */*","connection ":"keep-alive","content-type":"application/json","referer ":"my.bla.com","user-agent":"Mozilla/5.0 (Windows NT 6.1; Win64; x64; rv:86.0) Gecko/20100101 Firefox/86.0","cache-control":"no-cache"},"json":{"b":"a"},"url":"https://postman-echo.com/post"}
```

  
