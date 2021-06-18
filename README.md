# WNetWrap  [![MSBuild](https://github.com/hack-tramp/wnetwrap/actions/workflows/msbuild.yml/badge.svg)](https://github.com/hack-tramp/wnetwrap/actions/workflows/msbuild.yml) [![Generic Badge](https://img.shields.io/badge/c%2B%2B-14-blue)](https://github.com/topics/c-plus-plus-11)  [![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](https://opensource.org/licenses/MIT)<br>
  
A tiny, dependency-free wrapper around [WinINet](https://docs.microsoft.com/en-us/windows/win32/wininet/about-wininet) for developers targeting Windows only, who need a lightweight native solution. Inspired by the excellent [CPR library](https://github.com/whoshuu/cpr), it has mostly identical function names, and will likewise work with random parameter order.

Below is a basic GET request - for detailed examples see the documentation below.

```c++
#include <iostream>
#include "wnetwrap.h"

using namespace wrap;
using namespace std;

int main()
{	//GET method and firefox user agent used by default
	Response r = HttpsRequest(Url{"https://www.example.com/"}); 
	cout << r.text << endl; // basic parser
	cout << r.status_code << endl; // 200
 }
  ```
 
## Features
* Custom headers
* Url encoded parameters
* Url encoded POST values
* Multipart form POST upload
* File POST upload
* Basic authentication
* Bearer authentication
* Connection and request timeout 

## Usage

Just put `wnetwrap.h` and `wnetwrap.cpp` in your project folder. That's it!

## Documentation

For now it's all here on the readme, but it will eventually be put on a different page.

### GET requests

Making a GET request with cpr is simple - the GET method is used by default so doesn't need to be specified:

```c++
#include <wnetwrap.h>
wrap::Response r = wrap::HttpsRequest(wrap::Url{"http://www.httpbin.org/get"});
```
This gives us a Response object which we’ve called r. There’s a lot of good stuff in there:
```c++
std::cout << r.url << std::endl; // http://www.httpbin.org/get
std::cout << r.status_code << std::endl; // 200
std::cout << r.header["content-type"] << std::endl; // application/json
std::cout << r.text << std::endl;

/*
 * {
 *   "args": {},
 *   "headers": {
 *     ..
 *   },
 *   "url": "http://httpbin.org/get"
 * }
 */
```
To add URL-encoded parameters, throw in a Parameters object to the HttpsRequest call:
```c++
wrap::Response r = wrap::HttpsRequest(wrap::Url{"http://www.httpbin.org/get"},
                  wrap::Parameters{{"hello", "world"}});
std::cout << r.url << std::endl; // http://www.httpbin.org/get?hello=world
std::cout << r.text << std::endl;

/*
 * {
 *   "args": {
 *     "hello": "world"
 *   },
 *   "headers": {
 *     ..
 *   },
 *   "url": "http://httpbin.org/get?hello=world"
 * }
 */
```
Parameters is an object with a map-like interface. You can construct it using a list of key/value pairs inside the Get method or have it outlive Get by constructing it outside:
```c++
// Constructing it in place
wrap::Response r = wrap::HttpsRequest(wrap::Url{"http://www.httpbin.org/get"},
                  wrap::Parameters{{"hello", "world"}, {"stay", "cool"}});
std::cout << r.url << std::endl; // http://www.httpbin.org/get?hello=world&stay=cool
std::cout << r.text << std::endl;

/*
 * {
 *   "args": {
 *     "hello": "world"
 *     "stay": "cool"
 *   },
 *   "headers": {
 *     ..
 *   },
 *   "url": "http://httpbin.org/get?hello=world&stay=cool"
 * }
 */

 // Constructing it outside
wrap::Parameters parameters = wrap::Parameters{{"hello", "world"}, {"stay", "cool"}};
wrap::Response r_outside = wrap::HttpsRequest(wrap::Url{"http://www.httpbin.org/get"}, parameters);
std::cout << r_outside.url << std::endl; // http://www.httpbin.org/get?hello=world&stay=cool
std::cout << r_outside.text << std::endl; // Same text response as above
```

### Downloading a file

To download the contents of the request you simply add a `Download` parameter to `HttpsRequest`. If this parameter's value is blank then the file is downloaded with its original filename, otherwise the value provided will be the new file's name. For example, to download the CPR library: <br>
```c++
HttpsRequest(Url{ "https://github.com/whoshuu/cpr/archive/refs/tags/1.6.0.zip" }, Download{});
```
When you download a file, the `.raw` and `.text` properties of the response object will be returned empty.

### POST Requests

Making a POST request is just a matter of specifying the HTTP method:
```c++
wrap::Response r = wrap::HttpsRequest(wrap::Url{"http://www.httpbin.org/post"},
                   wrap::Payload{{"key", "value"}}, wrap::Method{"POST"});
std::cout << r.text << std::endl;

/*
 * {
 *   "args": {},
 *   "data": "",
 *   "files": {},
 *   "form": {
 *     "key": "value"
 *   },
 *   "headers": {
 *     ..
 *     "Content-Type": "application/x-www-form-urlencoded",
 *     ..
 *   },
 *   "json": null,
 *   "url": "http://www.httpbin.org/post"
 * }
 */
```
This sends up `"key=value"` as a `"x-www-form-urlencoded"` pair in the POST request. To send data raw and unencoded, use `Body` instead of `Payload`:
```c++
wrap::Response r = wrap::HttpsRequest(wrap::Url{"http://www.httpbin.org/post"},
                   wrap::Body{"This is raw POST data"},
                   wrap::Header{{"Content-Type", "text/plain"}},
		   wrap::Method{"POST"});
std::cout << r.text << std::endl;

/*
 * {
 *   "args": {},
 *   "data": "This is raw POST data",
 *   "files": {},
 *   "form": {},
 *   "headers": {
 *     ..
 *     "Content-Type": "text/plain",
 *     ..
 *   },
 *   "json": null,
 *   "url": "http://www.httpbin.org/post"
 * }
 */
```
If the data package is large or contains a file, it’s more appropriate to use a Multipart upload. In this example we are uploading a textfile to file.io.
```c++
wrap::Response r = wrap::HttpsRequest(wrap::Url{ "file.io" },
				      wrap::Multipart{ {"file:sample1","sample.txt"} }, 
			              wrap::Method{ "POST" });
std::cout << r.text << std::endl;

/*
{"success":true,"status":200,"id":"0a1dc4a0-d056-11eb-b8a8-95e106f75f99","key":"JBDaFwjAneQH","name":"sample.txt","link":"https://
file.io/JBDaFwjAneQH","private":false,"expires":"2021-07-02T16:55:52.042Z","downloads":0,"maxDownloads":1,"autoDelete":true,"size"
:53,"mimeType":"text/plain","created":"2021-06-18T16:55:52.042Z","modified":"2021-06-18T16:55:52.042Z"}
 */
```
Notice how the textfile which in this case was passed as sample1, had `file:` prefixed before it - this tells WNetWrap that this is a file and not a key - value pair.


## Handling the Response object
 
**Retrieving the headers**

The response is stored into a `Response` object. To get info from the headers received use `get_header("header_field")`


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

  
