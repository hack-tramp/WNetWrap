# ![Image](https://github.com/hack-tramp/WNetWrap/blob/main/logo.png?raw=true) WNetWrap  [![MSBuild](https://github.com/hack-tramp/wnetwrap/actions/workflows/msbuild.yml/badge.svg)](https://github.com/hack-tramp/wnetwrap/actions/workflows/msbuild.yml) [![Generic Badge](https://img.shields.io/badge/c%2B%2B-14-blue)](https://github.com/topics/c-plus-plus-11)  [![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](https://opensource.org/licenses/MIT)<br>
  
A tiny, dependency-free wrapper around [WinINet](https://docs.microsoft.com/en-us/windows/win32/wininet/about-wininet) for developers targeting Windows only, who need a lightweight native solution. Inspired by the excellent [CPR library](https://github.com/whoshuu/cpr), it has mostly identical function names, and will likewise work with random parameter order.

Below is a basic GET request - for detailed examples see the documentation below.

```c++
#include <iostream>
#include "wnetwrap.h"

int main()
{	//GET method and firefox user agent used by default
	wrap::Response r = wrap::HttpsRequest(wrap::Url{"https://www.example.com/"}); 
	std::cout << r.text << std::endl; // basic parser
	std::cout << r.status_code << std::endl; // 200
 }
  ```
 
## Features

|Implemented| Upcoming|
|:------------:|:----------:|
|Custom headers|Proxy support|
|Url encoded parameters|Asynchronous requests|
|Url encoded POST values|Callbacks|
|Multipart form POST upload|NTLM authentication|
|File POST upload|Digest authentication|
|Basic authentication|PUT, PATCH and DELETE methods|
|Bearer authentication|
|Connection and request timeout| 
|Cookie support|

## Usage

Just put `wnetwrap.h` and `wnetwrap.cpp` in your project folder. That's it!

## Documentation

For now it's all here on the readme, but it will eventually be put on a different page to make navigation more user friendly.
To navigate through it use the table of contents dropdown menu.

中文文档稍后会贴在这里但是现在只有英文的，对不起。

### GET requests

Making a GET request with WNetWrap is simple - the GET method is used by default so doesn't need to be specified:

```c++
#include <wnetwrap.h>
wrap::Response r = wrap::HttpsRequest(wrap::Url{"http://www.httpbin.org/get"});
```
This gives us a `Response` object which we’ve called r. There’s a lot of useful stuff in there:
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
To add URL-encoded parameters, add a `Parameters` object to the `HttpsRequest` call:
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
`Parameters` is an object with a map-like interface. You can construct it using a list of key/value pairs inside the `HttpsRequest` or have it outlive `HttpsRequest` by constructing it outside:
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
wrap::HttpsRequest(wrap::Url{ "https://github.com/whoshuu/cpr/archive/refs/tags/1.6.0.zip" }, wrap::Download{});
```
When you download a file, the `.raw` and `.text` properties of the response object will be returned empty.

### POST Requests

Making a POST request is just a matter of specifying the HTTP method:
```c++
wrap::Response r = wrap::HttpsRequest(wrap::Url{"http://www.httpbin.org/post"},
                   wrap::Payload{{"key", "value"}}, 
		   wrap::Method{"POST"});
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
Here you will notice that the `"Content-Type"` is being set explicitly to `"text/plain"`. This is because by default, `"x-www-form-urlencoded"` is used for raw data POSTs. For cases where the data being sent up is small, either `"x-www-form-urlencoded"` or `"text/plain"` is suitable. If the data package is large or contains a file, it’s more appropriate to use a `Multipart` upload. In this example we are uploading a textfile to file.io:
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
Notice how the text file, which in this case was passed as `sample1`, had `file:` prefixed before it - this tells WNetWrap that this is a file and not a key - value pair.

### Authentication

To use [Basic Authentication](https://en.wikipedia.org/wiki/Basic_access_authentication) which uses a username and password, just add `Authentication` to the call:
```c++
wrap::Response r = wrap::HttpsRequest(wrap::Url{"http://www.httpbin.org/basic-auth/user/pass"},
                  wrap::Authentication{"user", "pass"});
std::cout << r.text << std::endl;

/*
 * {
 *   "authenticated": true,
 *   "user": "user"
 * }
 */

```
Authentication via an [OAuth - Bearer Token](https://en.wikipedia.org/wiki/OAuth) can be done using the `Bearer` authentication object:
```c++
wrap::Response r = wrap::HttpsRequest(wrap::Url{"http://www.httpbin.org/bearer"},
                  wrap::Bearer{"ACCESS_TOKEN"});
std::cout << r.text << std::endl;

/*
 * {
 *   "authenticated": true,
 *   "token": "ACCESS_TOKEN"
 * }
 */

```

### Response Objects

A `Response` has these public fields and methods:
```c++
std::string status_code;        // The HTTP status code for the request
std::string raw;                // The body of the HTTP response
std::string text;               // The text body in case of HTML response - if not HTML, same as raw above
std::map header;                // A map of the header fields received
std::map sent_headers;          // A map of the headers sent
std::map secinfo;               // A map of certificate information strings (HTTPS only)
std::string url;                // The effective URL of the ultimate request
std::string err;                // An error string containing the error code and a message
unsigned long uploaded_bytes;   // How many bytes have been sent to the server
unsigned long downloaded_bytes; // How many bytes have been received from the server
unsigned long redirect_count;   // How many redirects occurred
```

The `header` is a map with an important modification. Its keys are case insensitive as required by [RFC 7230](http://tools.ietf.org/html/rfc7230#section-3.2):
```c++
wrap::Response r = wrap::HttpsRequest(wrap::Url{"http://www.httpbin.org/get"});
std::cout << r.header["content-type"] << std::endl;
std::cout << r.header["Content-Type"] << std::endl;
std::cout << r.header["CoNtEnT-tYpE"] << std::endl;
```
All of these should print the same value, `"application/json"`. 

### Request Headers

Using `Header` in your `HttpsRequest` you can specify custom headers:
```c++
wrap::Response r = wrap::HttpsRequest(wrap::Url{"http://www.httpbin.org/headers"},
                  wrap::Header{{"accept", "application/json"}});
std::cout << r.text << std::endl;

/*
 * "headers": {
 *   "Accept": "application/json",
 *   "Host": "www.httpbin.org",
 *   "User-Agent": "Mozilla/5.0 (Windows NT 6.1; Win64; x64; rv:89.0) Gecko/20100101 Firefox/89.0"
 * }
 */

```

### Setting Timeouts

It’s possible to set a timeout for your request if you have strict timing requirements:
```c++
wrap::Response r = wrap::HttpsRequest(wrap::Url{"http://www.httpbin.org/get"},
                  wrap::Timeout{1000}); // will timeout after 1000 ms
```
Setting the `Timeout` option sets the maximum allowed time the connection or request operation can take in milliseconds. By default a Timeout will only apply to the request itself, but you can specify either one by adding either `connection` or `request`, or both with `all`:
```c++
wrap::Timeout{1000,"connection"}
```
Since WNetWrap is built on top of WinINet, it’s important to know what setting this `Timeout` does to the request. It creates a worker thread which executes the connection or request call. This thread is then monitored and killed if it takes longer than the timeout specified. The reason this approach is taken is that the normal method of setting a timeout with WinINet does not work, due to a 20+ year old MS bug. You can find out more about this workaround [here](https://mskb.pkisolutions.com/kb/224318). What it means in practical terms is that `Timeout` cannot be set to a value higher than WinINet's default (currently 1 hour).

### Security Certificate Info

To see the response's security info, you will need to access the `secinfo` map. For example, to get the security certificate:
```c++
r.secinfo["certificate"]
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
cout << r.secinfo["protocol"]; // example.com: Transport Layer Security 1.2 client-side 
```
Cycling through the `secinfo` map will show all other available security info:
```c++ 
cout << "security info:" << endl;
for (auto elem : r.secinfo)
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

