.TH SUPDUP 1
.SH NAME
supdup \- user supdup program to the Arpanet or Chaos net
.SH SYNOPSIS
.B supdup
[ [ network ] host [ contact | socket ] [ via ] ]
.SH
DESCRIPTION
.I supdup
connects the standard input and output to a remote
computer system using the SUPDUP protocol.
The specified host may be on either the
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
name (for Chaos net hosts) defaults to ``SUPDUP'', and the
.I socket
number (for Arpanet hosts) defaults to ``137''.
Connections to the Arpanet are made
.I via 
``mit-mc'' by default.
.PP
After making the connection,
.I supdup
runs as two processes: the
.I send
process reads the standard input and passes most of it to the remote system;
the
.I receive
process reads from the remote system and passes most data to the
standard output.
The
.I receive
process uses the
.I termcap
(5) database and the environment variable TERM to determine the capabilities
of the terminal attached to the standard output and to interpret the SUPDUP
protocol.
.PP
The escape character (initially ^^ \- control-^) introduces a two
character command sequence. Command characters are:
.IP ? 10
Lists the command characters
.IP c 10
Equivalent to the bye command (closes the connection)
.IP p 10
Equivalent to the pause command
.IP q 10
Equivalent to the quit command
.IP s 10
Equivalent to the status command
.IP x 10
Enter extended command mode
.PP
In extended command mode, a prompt line is given and one of
the following commands may be given:
.TP 10
bye
Close the connection to the remote system.
.TP 10
disconnect
Close the connection to the remote NVT.
.TP 10
.RI escape " c"
Sets the escape character to
.I c.
.I C
is either a literal character preceded by a single quote (') or an octal
integer.
.TP 10
exit
Close the connection to the remote NVT and exit.
.TP 10
.RI location " terminal-location"
Sets the terminal location string which will be sent to the remote
system when a connection is established.
.I csh
(I))
.TP 10
pause
Suspends
.I supdup.
.TP 10
quit
Close the connection to the remote NVT and exit.
.TP 10
status
Prints status information about the current connection (if any).
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
.I supdup
is not given any arguments, then
.I supdup
enters a command loop similar to extended command mode except the allowable
commands are:
.TP 10
open
Takes arguments identical in format to the command line arguments for
.I supdup
itself and opens a connection to the specified remote NVT and
sends characters from the standard input to the NVT,
.I et cetera.
When the connection is closed,
.I supdup
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
location
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
.br
/etc/termcap
.SH "SEE ALSO"
tn(1), termcap(5)
.SH DIAGNOSTICS
Exit code is zero for normal exit, nonzero (various values) otherwise.
.PP
Option negotiations that
.I supdup
does not understand are reported on the standard output.
.SH BUGS
Most options are refused, and
.I supdup
won't even let you specify a desired state for the options that it does know
about.
