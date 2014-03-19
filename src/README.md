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
        Progress:99%
        Problem:1.Too more pthread_mutex(because it use multi process with multi thread,need solve syn)[Next version 
        try to only use multi process]
        2.When huge connection comming,it still will shutdown by segmentation fault.
        3.Signal is being noting in this version.It very complication because of Multi-*.
        Multi process and multi thread model.Big change in I/O processing mode.
        Has same protocol processing module with v2.

###/src/v4  
        Status:planning
        Hope to add CGI and cache
