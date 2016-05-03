BeBe_HTTP_Server
================

BeBe_HTTP_Server Version Four


BUG:
-------------------------
    EPOLL连接机制还没有进行优化。（目前仍统一采用EPOLLIN来处理连接读写）
    HTTP_VERSION解析数字统计出错。
    工作进程容易崩溃，形成僵尸进程。
    当前还不支持keepalive连接，每次请求处理完暂作粗暴断开处理。
    
常见错误：
------------------------
    忘记给函数指针赋值，直接调用相关指针造成出错。
    sendfile缺少参数，编译不报错。
