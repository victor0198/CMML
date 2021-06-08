# CMML - Chat messages manipulation language
The CMML gives the possibility to exchage programmable messages with your partner.
It is implemented as a server application written in C++, using Restbed and JSON libraies. It builds and serves custom HTTP methods and responds with JSON formatted data structures.

For message analysis, there is one POST method, which receives the request payload as raw text. The "Content-Length" header is mandatory. The response is formatted in JSON. The POST method is available at "https://<your_address>/cmml".

## Example:
```
$question(Let’s go for a walk!)
	[Accept][Decline]
	{  
		[super!]
		ok :)
	}
@color(white)
@size(14)
@bgcolor(green)
```
<img title="Server app's logs" alt="Server app's logs" src="/examples/example_1.png">
