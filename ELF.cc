extern "C" {
#include <libelf.h>
#include "bindpp.h"
}

XS(xs_new) {
    pl::Ctx c(0);
}

extern "C"
XS(boot_Data__ELF)
{
    pl::BootstrapCtx b;

    pl::Package p("Data::ELF", __FILE__);
    p.add_method("new", xs_new);
    p.add_method("DESTROY", xs_DESTROY);
}

