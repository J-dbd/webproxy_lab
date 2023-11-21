Recording terminal

```terminal
$ ./tiny [portnumber]
Accepted connection from (Ip, port)
Request headers:
GET / HTTP/1.1
Connection: keep-alive
Upgrade-Insecure-Requests: 1
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7
Accept-Encoding: gzip, deflate
Accept-Language: ko-KR,ko;q=0.9,en-US;q=0.8,en;q=0.7

Response headers:
HTTP/1.0 200 OK
Server: Tiny Web Server
Connection: close
Content-length: 120
Content-type: text/html

Accepted connection from (Ip, port)
Request headers:
Accepted connection from (Ip, port)
Request headers:
GET /godzilla.gif HTTP/1.1
Connection: keep-alive
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Safari/537.36
Accept: image/avif,image/webp,image/apng,image/svg+xml,image/*,*/*;q=0.8
Referer: http://[my ec2]:port/
Accept-Encoding: gzip, deflate
Accept-Language: ko-KR,ko;q=0.9,en-US;q=0.8,en;q=0.7

Response headers:
HTTP/1.0 200 OK
Server: Tiny Web Server
Connection: close
Content-length: 12155
Content-type: image/gif

Accepted connection from (Ip, port)
Request headers:
:Accepted connection from (Ip, port)
Request headers:
GET /favicon.ico HTTP/1.1
Connection: keep-alive
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Safari/537.36
Accept: image/avif,image/webp,image/apng,image/svg+xml,image/*,*/*;q=0.8
Referer: http://[my ec2]:port/
Accept-Encoding: gzip, deflate
Accept-Language: ko-KR,ko;q=0.9,en-US;q=0.8,en;q=0.7

```




### 11.6-c

Inspect the output from Tiny to determine the version of HTTP your browser uses.

-----

request from my browser

```sql
GET / HTTP/1.1
GET /godzilla.gif HTTP/1.1
GET /favicon.ico HTTP/1.1

```

response from the server
```sql
HTTP/1.0 200 OK
HTTP/1.0 200 OK
HTTP/1.0 200 OK

```

### 11.7
attaching mp4 to server
```
$ ./tiny [portnumber]
Accepted connection from (Ip, port)
Request headers:
GET /water_field.mp4 HTTP/1.1
Connection: keep-alive
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Safari/537.36
Accept-Encoding: identity;q=1, *;q=0
Accept: */*
Referer: http://[my ec2]:port/
Accept-Language: ko-KR,ko;q=0.9,en-US;q=0.8,en;q=0.7
Range: bytes=0-

Response headers:
HTTP/1.0 200 OK
Server: Tiny Web Server
Connection: close
Content-length: 1090424
Content-type: video/mp4

Accepted connection from (Ip, port)
Request headers:
GET /water_field.mp4 HTTP/1.1
Connection: keep-alive
Upgrade-Insecure-Requests: 1
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7
Accept-Encoding: gzip, deflate
Accept-Language: ko-KR,ko;q=0.9,en-US;q=0.8,en;q=0.7

Response headers:
HTTP/1.0 200 OK
Server: Tiny Web Server
Connection: close
Content-length: 1090424
Content-type: video/mp4

Accepted connection from (Ip, port)
Request headers:
GET /water_field.mp4 HTTP/1.1
Connection: keep-alive
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Safari/537.36
Accept-Encoding: identity;q=1, *;q=0
Accept: */*
Referer: http://[my ec2]:port/water_field.mp4
Accept-Language: ko-KR,ko;q=0.9,en-US;q=0.8,en;q=0.7
Range: bytes=0-

Response headers:
HTTP/1.0 200 OK
Server: Tiny Web Server
Connection: close
Content-length: 1090424
Content-type: video/mp4

```