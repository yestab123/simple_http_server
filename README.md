BeBe Simple Http Server
=================
BeBe_http服务器，可以对静态网页请求正常解析并正确响应。在多个浏览器测试均能够正常访问服务器网页。<br />
由初版一直重构，目前共有3个版本。（目前主要专注于服务器的高性能处理，多个版本着重于改进对连接的处理性能。以后才会对CGI等模块进行编写。<br />

环境：
-------------------
开发语言：C <br />
操作系统：Ubuntu13.04(Linux)<br />
编程：epoll<br />

###用法:
    1. 下载并进行src/v* （目前共3个版本）
    2. make
    3. ./service [server_ip] [server_port] (默认网页绑定的目录/Project/web/)





###Other Dir:
    /tool/   测试服务器是否正常工作的工具。（used to test whether the server is working properly）
    /test/   验证一些想法是否可行的test文件。（used to test the feasibility of algorithms）
    /web/    测试服务器正常运行用的html模板。(the DIR of web page.)
    
###每个版本的说明（Each Version Instructions）
[Version Description](https://github.com/yestab123/simple_http_server/blob/master/src/README.md) 
