URL shortening in memory service
================================

A quickly hacked url shortening script based on the Slim php framework and the apc php mod for in memory
caching. The index.php script does the majority of the work and I also included the nginx configuration for the specific subdomain.
It does basic input sanitization but nothing really serious.

Requirements:

Create a URL shortening service. You are free to choose any programming language. Persistent storage is not required for
this exercise (i.e. in-memory database is fine). Your service must provide the following API:

POST /shorten
Parameters: Parameter link should contain the link to shorten.
Returns: Id for the shortened link in text/plain format.

GET /{id}
Returns: 301 redirects the user agent to a previously stored URL. 404 error if no link stored with given id.

##License

This implementation is licensed under the [Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported](https://creativecommons.org/licenses/by-nc-sa/3.0/)
