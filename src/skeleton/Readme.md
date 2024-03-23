# First skeleton implementation notes / issues.
* `skeleton.c` exists with duplicate files, nothing there that does not exist in other files.
* Peer and poll has direct code errors that needs to be solved.
* `system_process.c` contains selection, clustering and combining algorithms.
* Loop filter exists within `vfo.c`. The changing of the local clock exists there as well. 
* Peer and poll functions are split into `peer.c` and `poll.c` respectively.
* `main_and_utility.c` has 3 functions `recv_packet`, `xmit_packet`, and `md5` that are completely empty.