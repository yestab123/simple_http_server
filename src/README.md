Each version of the HTTP server description
==================================
Each new version is based on the reconstruction of the previous version

###/src/v1
        Status:Finish
        Single thread processing.The most simple HTTP protocol processing module. 

###/src/v2
        Status:Finish
        Multi thread processing and it can deal with thousands of connection same time.
        It has a update protocol processing module.

###/src/v3  
        Status:In preparation
        Progress:70% (30% in Signal,Syn,and load balancing)
        Multi process and multi thread model.Big change in I/O processing mode.
        Has same protocol processing module with v2.

###/src/v4  
        Status:planning
        Hope to add CGI and cache
