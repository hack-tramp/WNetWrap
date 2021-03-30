# wnetwrap
A small library which helps to use WinInet to make simple HTTP(S) requests in C++.

# Basic HTTP GET request

```
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
`req my_request; my_request.method = "GET";`

**Setting the user agent**
Firefox is used by default but you can specify your own, for example an Apple iPhone XR (Safari):
`my_request.ua = "Mozilla/5.0 (iPhone; CPU iPhone OS 12_0 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/12.0 Mobile/15E148 Safari/604.1";`


**Setting a header**:<br>
`my_request.set_header( "Referer" , "my.referer.com" );` <br><br>
**Updating a header**<br>Note that as HTTP header fields are case-insensitive, they will always be stored and sent in lowercase - this means this will still work:<br>
`my_request.set_header("RefErEr", "my.bla.com");`





  
