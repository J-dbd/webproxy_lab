### The Tiny main Routine 

Tiny is an iterative server that listens for connection requests on the port that is passed in the command line. 

After opening a listening socket by calling the open_listenfd function, 
Tiny executes the typical inﬁnite server loop, 
repeatedly accepting a connection request (line 32), 
performing a transaction (line 36), 
and closing its end of the connection (line 37).

Tiny supports only the GET method

### the doit function

First, read and parse the request line (lines 11–14)

Tiny supports only the GET method. 
If the client requests another method (such as POST), we send it an error message and return to the main routine (lines 15–19), which then closes the connection and awaits the next connection request. Otherwise, we read and (as we shall see) ignore any request headers(line 20).


Next, we parse the URI into a filename and a possibly empty CGI argument string, 
and we set a ﬂag that indicates whether the request is for static or dynamic content (line 23). If the file does not exist on disk, we immediately send an error message to the client and return. 

Finally, if the request is for static content, we verify that the ﬁle is a regular ﬁle and that we have read permission (line 31). If so, we serve the static content (line 36) to the client.

Similarly, if the request is for dynamic content, we verify that the ﬁle is executable (line 39), and, if so, we go ahead and serve the dynamic content (line 44)


### The clienterror Function

Tiny lacks many of the error-handling features of a real server. However, it does check for some obvious errors and reports them to the client. 

The clienterror function in Figure 11.31 sends an HTTP response to the client with the appropriate status code and status message in the response line, along with an HTML ﬁle in
the response body that explains the error to the browser’s user. 

Recall that an HTML response should indicate the size and type of the content in the body. Thus, we have opted to build the HTML content as a single string so that we can easily determine its size. 

Also, notice that we are using the robust rio_writen function from Figure 10.4 for all output.

### The read_requesthdrs Function

Tiny does not use any of the information in the request headers. It simply reads and ignores them by calling the read_requesthdrs function in Figure 11.32. Notice that the empty text line that terminates the request headers consists of a carriage return and line feed pair, which we check for in line 6.

### The parse_uri Function

Tiny assumes that the home directory for static content is its current directory and 
that the home directory for executables is ./cgi-bin. Any URI that contains the 
string cgi-bin is assumed to denote a request for dynamic content. The default 
filename is ./home.html.
The parse_uri function in Figure 11.33 implements these policies. It parses 
the URI into a filename and an optional CGI argument string. If the request is 
for static content (line 5), we clear the CGI argument string (line 6) and then 
convert the URI into a relative Linux pathname such as ./index.html (lines 7–8). 
If the URI ends with a ‘/’ character (line 9), then we append the default filename 
(line 10). On the other hand, if the request is for dynamic content (line 13), we 
extract any CGI arguments (lines 14–20) and convert the remaining portion of the 
URI to a relative Linux filename (lines 21–22).

#### step by step 
The parse_uri function in the provided code is responsible for parsing an HTTP URI into a filename and an optional CGI argument string. The function assumes certain conventions for the URI structure:

If the URI does not contain the substring "cgi-bin," it is considered static content, and the function sets cgiargs to an empty string. The filename is constructed by appending the URI to the current directory. If the URI ends with '/', it is assumed to be a request for the "home.html" file.

If the URI contains the substring "cgi-bin," it is considered dynamic content. The function extracts the CGI argument string, if present, by looking for the '?' character. The filename is constructed by appending the URI to the current directory.

Let's break down the function step by step:

```c

int parse_uri(char *uri, char *filename, char *cgiargs) {
    char *ptr;

    // Check if the URI contains "cgi-bin"
    if (!strstr(uri, "cgi-bin")) { /* Static content */
        // Set CGI arguments to an empty string
        strcpy(cgiargs, "");
        // Construct the filename by appending the URI to the current directory
        strcpy(filename, ".");
        strcat(filename, uri);
        
        // If the URI ends with '/', append "home.html" to the filename
        if (uri[strlen(uri)-1] == '/') {
            strcat(filename, "home.html");
        }
        return 1; // Indicate static content
    } else { /* Dynamic content */
        // Find the position of '?' in the URI
        ptr = index(uri, '?');
        
        // If '?' is found, extract CGI arguments and modify the URI
        if (ptr) {
            strcpy(cgiargs, ptr+1);
            *ptr = '\0';
        } else {
            strcpy(cgiargs, "");
        }

        // Construct the filename by appending the modified URI to the current directory
        strcpy(filename, ".");
        strcat(filename, uri);

        return 0; // Indicate dynamic content
    }
}
```

Example usage: 

Suppose the URI is "/cgi-bin/example.cgi?param1=value1&param2=value2".

```c
Copy code
char uri[MAXLINE] = "/cgi-bin/example.cgi?param1=value1&param2=value2";
char filename[MAXLINE];
char cgiargs[MAXLINE];

int is_static = parse_uri(uri, filename, cgiargs);

// After the function call, filename will be "./cgi-bin/example.cgi"
// and cgiargs will be "param1=value1&param2=value2".

```
In this example, the function correctly identifies the URI as dynamic content, extracts the CGI argument string, and constructs the filename accordingly.


### The serve_static Function
Tiny serves ﬁve common types of static content: HTML ﬁles, unformatted text 
ﬁles, and images encoded in GIF, PNG, and JPEG formats.
The serve_static function in Figure 11.34 sends an HTTP response whose 
body contains the contents of a local ﬁle. First, we determine the ﬁle type by 
inspecting the sufﬁx in the filename (line 7) and then send the response line and 
response headers to the client (lines 8–13). Notice that a blank line terminates the 
headers.
Next, we send the response body by copying the contents of the requested ﬁle 
to the connected descriptor fd. The code here is somewhat subtle and needs to be 
studied carefully. Line 18 opens filename for reading and gets its descriptor. In 
line 19, the Linux mmap function maps the requested ﬁle to a virtual memory area. 
Recall from our discussion of mmap in Section 9.8 that the call to mmap maps the ﬁrst filesize bytes of ﬁle srcfd to a private read-only area of virtual memory 
that starts at address srcp.
Once we have mapped the ﬁle to memory, we no longer need its descriptor, 
so we close the ﬁle (line 20). Failing to do this would introduce a potentially fatal 
memory leak. Line 21 performs the actual transfer of the ﬁle to the client. The 
rio_writen function copies the filesize bytes starting at location srcp (which 
of course is mapped to the requested ﬁle) to the client’s connected descriptor. 
Finally, line 22 frees the mapped virtual memory area. This is important to avoid 
a potentially fatal memory leak.

### The serve_dynamic Function
Tiny serves any type of dynamic content by forking a child process and then 
running a CGI program in the context of the child.
The serve_dynamic function in Figure 11.35 begins by sending a response line 
indicating success to the client, along with an informational Server header. The 
CGI program is responsible for sending the rest of the response. Notice that this 
is not as robust as we might wish, since it doesn’t allow for the possibility that the 
CGI program might encounter some error.
After sending the ﬁrst part of the response, we fork a new child process 
(line 11). The child initializes the QUERY_STRING environment variable with 
the CGI arguments from the request URI (line 13). Notice that a real server would set the other CGI environment variables here as well. For brevity, we have omitted 
this step.
Next, the child redirects the child’s standard output to the connected ﬁle 
descriptor (line 14) and then loads and runs the CGI program (line 15). Since 
the CGI program runs in the context of the child, it has access to the same open 
ﬁles and environment variables that existed before the call to the execve function. 
Thus, everything that the CGI program writes to standard output goes directly to 
the client process, without any intervention from the parent process. Meanwhile, 
the parent blocks in a call to wait, waiting to reap the child when it terminates 
(line 17).
