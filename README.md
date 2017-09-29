# lwhtip README.md

# What is lwhtip
lwhtip is C software implementing TTC standard HTIP (JJ-300.00v3).
This software works on each IoT computer, sends messages to identify network topology.
lwhtip consists of two daemons (HTIP-L2Agent, HTIP-NW) and library.
HTIP-L2Agent sends a device information periodically.
HTIP-NW should be installed on a layer 2 network switch, sends a MAC address table information of the switch periodically.
By collecting these information, IoT network topology can be estimated.

# Requirements
lwhtip requires

* Linux operation system
* C compiler and autotools
* brctl (required by HTIP-NW)

# Installation
To install lwhtip, run following commands on a terminal application.

        autoreconf -i
        ./configure
        make
        make install

# Getting started
lwhtip consists of HTIP-L2Agent and HTIP-NW.

## HTIP-L2Agent
HTIP-L2Agent broadcasts a device information periodically.
Execute a following command to run.

        l2agent

Execute with path to HTIP-L2Agent if you didn't install.

        src/bin/l2agent

## HTIP L2Agent(HTIP-NW)
HTIP L2Agent executed on HTIP-NW(layer 2 switch) broadcasts a device information and link information.
Create a bridge interface with brctl commands before running.
For example, if you will create a bridge interface named br0 and add two network interface (en0, en1) to the bridge, execute following commands:

        brctl addbr br0
        brctl addif br0 en0
        brctl addif br0 en1

After creating a bridge interface, run HTIP L2Agent(HTIP-NW) with a following command:

        l2switch -i br0

Run above commands as root user to control bridges and network interfaces.
If it doesn't work, check the bridge interface information and the state of network interfaces.

# Documentation
API documentation is inline with the code and conforms to Doxygen standards. You can generate an HTML version of the API documentation by running:

        doxygen Doxyfile

Then open doc/html/index.html

# License
MIT license, Copyright (c) 2017 Takashi OKADA.
