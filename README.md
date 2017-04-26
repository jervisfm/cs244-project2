# CS244 Project 2

# Introduction
This is a project to explore performance of different congestion control algorithms
over an emulated cellular wireless link.

# Collaborators
* Luke
* Jervis

Our team name codename is Rocinante.

# Building
For setup instructions, please see http://web.stanford.edu/class/cs244/pa2.html

You can use a VM with prebuilt dependencies at: https://web.stanford.edu/class/cs244/vm/cs244-vm-15.10.ova

To build the code, execute the following:
```
	$ ./autogen.sh
	$ ./configure
	$ make
```

To run an evaluation trace
```
$ cd datagroump
$ runcontest
```


To view real time live traces, make sure that you run the command under a suitable X session.
One way to do that is to re-use the X session in the VM while SSH'ed in.
```
$ export DISPLAY=:0
$ runcontest
```


# Custom Congestion Control
## BBR

For exercise D, we'll try to implement the core ideas epoused in BBR congestion control.
This involing keeping track of both bottleneck bandwidth RTT prop and making sure we
stay under the BDP limits.


