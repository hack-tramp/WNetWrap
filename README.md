# wnetwrap
A small library which helps to use WinInet to make simple HTTP(S) requests in C++. As WinInet is a native windows library, there are no dependencies, and wnetwrap is lightweight compared to other libraries like CPR.

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
  This will send a GET request to the specified URL and do some basic parsing to show the website text - the result for www.example.com should look like this:
```
Example Domain
This domain is for use in illustrative examples in documents. You may use this domain in literature without prior coordination or asking for permission.
More information...
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

  
