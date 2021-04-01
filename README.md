# WNetWrap  [![MSBuild](https://github.com/hack-tramp/wnetwrap/actions/workflows/msbuild.yml/badge.svg)](https://github.com/hack-tramp/wnetwrap/actions/workflows/msbuild.yml) [![Generic Badge](https://img.shields.io/badge/c%2B%2B-14-blue)](https://github.com/topics/c-plus-plus-14) <br>
  
A tiny library using WinInet to make simple HTTP(S) requests in C++. As WinInet is a native windows library, there are no dependencies, and WNetWrap is very lightweight compared to other libraries like CPR.

⚠️ **Noob Notice** ⚠️ : This repo uses MS Visual Studio for the example, but *you don't need that to use WNetWrap* - all you need to do is put the `wnetwrap.h` and `wnetwrap.cpp` in your project folder, and include as in the example below.

# Basic HTTP GET request

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
 }
  ```

# Preparing the request

The `req` request object is used and can be used for the following (all inputs are strings)

**Specifying the HTTP method**<br>
```c++ 
req my_request; my_request.method = "GET";
```

**Setting the user agent**<br>
Firefox is used by default but you can specify your own, for example an Apple iPhone XR (Safari):<br>
```c++ 
my_request.ua = "Mozilla/5.0 (iPhone; CPU iPhone OS 12_0 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/12.0 Mobile/15E148 Safari/604.1";
```


**Setting a header**:<br>
```c++ 
my_request.set_header( "Referer" , "my.referer.com" );
```
<br>

**Updating a header**<br>Note that as HTTP header fields are case-insensitive, they will always be stored and sent in lowercase - this means this will still work:<br>
```c++ 
my_request.set_header("RefErEr", "my.bla.com");
``` 

**Posting data**<br>
If you are sending data via POST you can set the data like this:<br>
```c++ 
my_request.postdata = "{\"b\":\"a\"}"
```

# Handling the response
<br>

**Retrieving the headers**

<br>

The response is stored into a `resp` object. To get info from the headers received use `get_header("header_field")`

<br>

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

# HTTP POST request
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

  
