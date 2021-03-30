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



  
