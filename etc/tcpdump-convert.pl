#!/usr/bin/perl

$| = 1;

my $regexp_time = '(\d\d:\d\d:\d\d\.\d+ )';
my $regexp_hex = '(0x\d+:\s+)([0-9a-f ]+)+  ';

while (<STDIN>) {
    my $input = $_;
    
    if ($input =~ /^$regexp_time/) {
	print "$1\n";
    }
    
    if ($input =~ /$regexp_hex/) {
	my $counter = $1;
	my $line = $2;
	
	$line =~ s/ //g;
	$counter =~ s/(0x|:)//g;
	
	print $counter . join(' ', ( $line =~ m/../g )) . "\n";
    }
}
