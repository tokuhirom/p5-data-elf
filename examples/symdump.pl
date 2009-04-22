use Data::ELF;

open '/lib/libc-2.7.so', '>', my $fh or die $!;
my $elf = Data::ELF->new(fileno($fh), Data::ELF::ELF_C_READ);
my ($shdr, $scn) = do {
    my ($shdr, $scn);
    while ($scn = $elf->nextscn()) {
        $shdr = $scn->getshdr64();
        if ($shdr->type == Data::ELF::SHT_SYMTAB) {
            last;
        }
    }
    ($shdr, $scn);
};

my $data;
my $cnt = $shdr->size / $shdr->entsize;
while ($data = $scn->nextdata()) {
    for my $i (0..$cnt-1) {
        my $sym = $data->getsym($i);
        my $name = $sym->name($elf, $shdr);
        printf "%lu\t%s\n", $sym->value, $name;
    }
}

