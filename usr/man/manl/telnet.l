.TH TELNET 1
.SH NAME
telnet \- user telnet program to the Arpanet or Chaos net
.SH SYNOPSIS
.B telnet
[ [ network ] host [ contact | socket ] [ via ] ]
.SH
DESCRIPTION
.I Telnet
connects the standard input and output to a network
virtual terminal (NVT) using the standard TELNET protocol.
The NVT is
connected to a NVT on the specified host, which may be on either the
.B arpanet
or
.B chaos
network.
If no
.I network
is specified, the Chaos net is
used if a machine is on both the Arpanet and the Chaos net.
The
.I contact
name (for Chaos net hosts) defaults to ``TELNET'', and the
.I socket
number (for Arpanet hosts) defaults to ``27''.
Connections to the Arpanet are made
.I via 
``mit-mc'' by default.
.PP
After making the connection,
.I telnet
runs as two processes: the
.I send
process reads the standard input and passes most of it to the NVT; the
.I receive
process reads from the NVT and passes most data to the standard output.
.PP
The
.I send
process attempts to negotiate remote echoing (i.e. DO ECHO)
and suppress go-ahead (i.e. DO SUPPRESS-GO-AHEAD and WILL
SUPPRESS-GO-AHEAD). The escape character (initially ^^ \-
control-^) introduces a two character command sequence.
Command characters are:
.IP ? 10
Lists the command characters
.IP a 10
Equivalent to the attn command
.IP b 10
Equivalent to the break command
.IP c 10
Equivalent to the bye command
.IP o 10
Equivalent to the abort command
.IP s 10
Equivalent to the status command
.IP x 10
Enter extended command mode
.PP
In extended command mode, a prompt line is given and one of
the following commands may be given:
.TP 10
bye
Close the connection to the remote NVT.
.TP 10
disconnect
Close the connection to the remote NVT.
.TP 10
exit
Close the connection to the remote NVT and exit.
.TP 10
quit
Close the connection to the remote NVT and exit.
.TP 10
pause
Suspends
.I telnet.
.TP 10
.RI escape " c"
Sets the escape character to
.I c.
.I C
is either a literal character preceded by a single quote (') or an octal
integer.
.TP 10
abort
Sends the NVT abort output sequence (usually causes the remote NVT to stop
sending characters).
.TP 10
attn
Sends the NVT attention (or interrupt process) sequence.
.TP 10
break
Sends the NVT break sequence.
.TP 10
verbose
Causes
.I telnet
to print detailed information about all option negotation
(normally of interest to debuggers only).
.TP 10
brief
Causes
.I telnet
to only print information about unimplemented option negotiations.
.I Brief
is the opposite of
.IR verbose .
.TP 10
status
Prints status information about the current connection (if any).
If a
connection is currently open, sends the NVT are-you-there sequence.
.TP 10
.RI help " command"
Provides information about
.I command.
.TP 10
?
Same as
.I
help.
.PP
If
.I telnet
is not given any arguments, then
.I telnet
enters a command loop similar to extended command mode except the allowable
commands are:
.TP 10
open
Takes arguments identical in format to the command line arguments for
.I telnet
itself and opens a connection to the specified remote NVT and
sends characters from the standard input to the NVT,
.I et cetera.
When the connection is closed,
.I telnet
returns to its main command loop.
.TP 10
connect
Same as
.I open.
.TP 10
exit
Same as in extended command mode.
.TP 10
quit
Same as in extended command mode.
.TP 10
pause
Same as in extended command mode.
.TP 10
escape
Same as in extended command mode.
.TP 10
verbose
Same as in extended command mode.
.TP 10
brief
Same as in extended command mode.
.TP 10
status
Same as in extended command mode.
.TP 10
help
Same as in extended command mode.
.TP 10
?
Same as in extended command mode.
.SH FILES
/dev/chaos
.SH "SEE ALSO"
supdup(1)
.SH DIAGNOSTICS
Exit code is zero for normal exit, nonzero (various values) otherwise.
.PP
Option negotiations that
.I telnet
does not understand are reported on the standard output.
.SH BUGS
Most options are refused, and
.I telnet
won't even let you specify a desired state for the options that it does know
about.
.PP
The pause command requires job control support in the kernel (e.g. 4BSD).
