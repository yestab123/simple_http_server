Now we have three version of the simple_http_server.

Each version was rebuild in previous version.

/src/v1  #Single thread processing.The most simple HTTP protocol processing module. 
/src/v2  #Multi thread processing and it can deal with thousands of connection same time.It has a update protocol processing module.
/src/v3  #Multi process and multi thread model.Big change in I/O processing mode.Has same protocol processing module with v2.

/src/v4  #planning