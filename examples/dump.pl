use strict;
use warnings;
use Data::ELF;

open my $fh, '<', 'a.out' or die $!;
my $elf = Data::ELF->new(fileno($fh), Data::ELF::ELF_C_READ);
my $max = $elf->getshnum();
for my $i (1..$max-1) {
    my $scn = $elf->getscn($i);
    print "-- ", $scn->name, "\n";
    print unpack('H*', $scn->getdata->buf), "\n";
}

