.TH CFTP 1 1/5/82
.SH NAME
cftp \- user file transfer program for the Chaos net
.SH SYNOPSIS
.B cftp
[
.B -D
] [ host ]
.SH
DESCRIPTION
.I Cftp
can be used to transfer files to and from remote systems which
provide ``Lisp-machine file servers.''
.I Cftp
interprets commands in a simple command loop.
Command names can be abbreviated.
All arguments in the description below are optional in the sense that
any required arguments not specified with the command will be
prompted for.
.PP
The commands are
.TP 10
.RI connect " host"
Establish a connection to the remote site
.I host.
.TP 10
.RI open " host"
Same as connect.
.TP 10
.RI login " user"
Login as
.I user
on the remote site.
.I Cftp
will prompt for a password, which will not be echoed.
Most hosts provide the user ANONYMOUS with password GUEST as a user
with low privilege that may be used to read files.
.TP 10
.RI ldir " local-directory"
.I Cftp
will prepend the specified local directory string to all local file
names.
The default local directory string is empty.
Since
.I ldir
is a string operation and does not invoke
.IR chdir (2),
you normall must include a trailing slash (/).
.TP 10
.RI fdir " foreign-directory"
.I Cftp
will prepend the specified foreign directory string to all foreign file
names.
The default foreign directory string is empty.
.TP 10
.RI get " foreign-file"
Reads the specified file in the current transfer mode from the
remote host to a local file.
.I Cftp
will prompt for the name of the local file.
If an empty local file name is specified, the foreign file name is
used as the local file name.
.TP 10
.RI send " local-file"
Sends the specified local file in the current transfer mode to a file
on the remote host.
.I Cftp
will prompt for the name of the foreign file.
If an empty foreign file name is specified, the local file name is
used as the foreign file name.
.TP 10
.RI directory " wild-card"
Prints a file directory of all files on the remote host matching the
specification given by
.IR wild-card .
The interpretation of
.I wild-card
is entirely dependent on the remote host.
.TP 10
.RI probe " foreign-file"
Prints information about 
.I foreign-file
on the remote host.
.TP 10
.RI delete " foreign-file"
Deletes
.IR foreign-file .
.TP 10
.RI rename " old-name"
Renames the file
.I old-name
on the remote host to a new name, which
.I cftp
prompts for.
.TP 10
ascii
Transfers all data in CHARACTER mode.  Actually translates all
characters to and from the Lisp machine character set, which
interprets most control characters as text characters, and uses the
character codes greater than 127 for special characters such as
newline.
.TP 10
raw
Transfers all data in CHARACTER RAW mode.  
No translations are performed.
This is good for transferring text files between UNIX systems, and
data files between UNIX systems on the same base machine (e.g.
vax-to-vax or pdp11-to-pdp11).
However, from ITS or TOPS-20, you will get an untranslated stream of
7-bit characters.
.TP 10
image
Transfers all data in CHARACTER SUPER-IMAGE mode.  No translations
are performed.
.TP 10
bytes
Equivalent to BINARY BYTE-SIZE 16.
This is effective for transferring PRESS files to and from ITS or
TOPS-20 systems, although the 8-bit bytes within each 16-bit byte
will appear swapped.
The dover spooler on UNIX automatically swaps bytes if necessary.
In general, if you want to move a sequence of 8-bit bytes, you should
use the mode BINARY BYTE-SIZE 8.
.TP 10
.RI binary " byte-size"
Transfers all data in BINARY BYTE-SIZE
.I byte-size
mode.
This currently works by simulating the action of PDP-10 byte pointer
operations into succesive 16-bit words.
This clearly has a host of problems when transferring binary data
from machines with different numeric representations.
Transferring PRESS files from ITS and TOPS-20 works using binary 16
mode, but swaps the order of 8-bit bytes within each 16-bit byte.
PRESS files can be transferred to and from ITS and TOPS-20 using
binary 8 mode, but any other data files should be perused carefully
before use.
.TP 10
status
Reports on the status of the current connection.
i.e. the remote host, login name and transfer mode.
.TP 10
verbose
Prints verbose debugging information about packets sent and received.
.TP 10
brief
Stops printing verbose debugging information.
.TP 10
quit
Exit from
.IR cftp ,
closing any open connection.
.TP 10
exit
Same as quit
.TP 10
disconnect
Close the current connection (if any).
.TP 10
bye
Same as disconnect.
.TP 10
.RI help " command"
Gives help on the specified
.IR command .
If no
.I command
is specified, help prints a brief description of all commands.
.TP 10
?
Same as help.
.PP
An initial argument
.B -D
sets the
.I verbose
switch.
.PP
If an argument is given to
.I cftp
it connects to the specified host and exits when the connection is
closed.
.PP
Otherwise illegal commands are treated as arguments to
.I connect
if there currently is no active connection,
or as arguments to
.I login
if the active connection is not yet logged in.
.SH FILES
.ta \w'/dev/chlisten'
/dev/chaosa	control connections
.br
/dev/chlisten	data connections
.br
/etc/hosttab	host table
.SH BUGS
The translations performed on the various transfer modes are all
rather ad hoc, especially the binary transfer modes.
