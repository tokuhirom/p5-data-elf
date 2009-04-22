use inc::Module::Install;
name 'Data-ELF';
all_from 'lib/Data/ELF.pm';

can_run 'g++' or die 'this module requires g++';
makemaker_args(
    OBJECT => '$(O_FILES)',
    clean => {
        FILES => q{
            *.stackdump
            *.gcov *.gcda *.gcno
            *.out
            nytprof
            cover_db
        },
    },
    LIBS => ['-lstdc++ -lelf'],
    CCFLAGS => '-g',
);

tests 't/*.t';
author_tests 'xt';

build_requires 'Test::More';
use_test_base;
auto_include;
WriteAll;
