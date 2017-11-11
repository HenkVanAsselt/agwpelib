Another old project from around the year 2000 which I found back on a 3.5" floppy disk.

This project makes a telnet interface to the AGW AX.25 Packet Engine, as it was available around that time.

This interface is written on a Win'95 machine with DJGPP V2.0 running in a DOS box with the a libsock interface.

To make the distribution, use the following commands:

	make all	-	to make all object files and info files
	make lint	-	to check your syntax with LCLINT (if installed)
	make cxref	-	to make cross reference files with CXREF
	make info	- 	to generate the info files (including HTML version)

History:

20001024: The file configure.in and make-cfg.in have been created.
These are very simple 'input' files for autoconf
Autoconf will create the files 'configure'
configure is a shell script which should be run withing bash or sh.
It will create the files configure.status and makefile.cfg (with make-cfg.in as the base)

20000725: The source files are documented with CXREF and are stored in the
directory 'doc'  I got CXREF only to work in the CYGWIN environment, so I
call it from there.

20000725: Checking the source files with LCLINT
The following enviroment variables have to be set (examples show my case)

set LARCH_PATH=d:\bin\lclint\lib
set LCLIMPORTDIR=d:\bin\lclint\imports

Invoke LCLINT as follows to show the path to the include directory:

lclint <file> -Id:\djgpp\include -warnposix	-weak

'make lint' will start the checking

20000804 - started on info development
The info source file is .\src\agwpetcp.txi
'make info' will make the info file

20000817 - Started on development of telnet interface.
Cees of PI8WLF asked if it was possible to make a telnet interface to AGEPE.
I think this is possible. As a basis I will use the servern.c example as it can
be found in \djgpp\contrib\ls074b4\demo.

It should do the following:

Open a socket and listen on port 23
If a connection is made to this port, open a socket and connect to AGWPE.
first we start in command mode and we should be able to escape to the 
command mode with the ESC key.
Commands will be limited and are like the normal TNC commands. 
I think of :
  - Connect		C <port> callsign <via> <callsign1> <callsign2>
  - Disconnect	D
  - List radio ports	L
  - Print status		S

A command will be ended by a CR/LF. As it will work on a line by line base,
it does not have to be put in a circular buffer, just a straight one.
The command will be processed and a AGEPE_IF structure will be generated
and send to the AGWPE engine.
The responses from AGWPE will be echoed back to the client, connected
on port 23.

Thu 08-24-2000: The first tests are successfull. You can connect to the test
application with telnet on port 23, and enter something, which will be
echoed to the stdout of the application DOS screen, and back to the telnet
client.


