#include <libelf.h>
#include <gelf.h>
#include "bindpp.h"

#define DELF Elf * elf = c.arg(0)->as_pointer()->extract<Elf*>()
#define DSCN SectionObj * scn = c.arg(0)->as_pointer()->extract<SectionObj*>()
#define DDATA DataObj * data = c.arg(0)->as_pointer()->extract<DataObj*>()
#define DSCNITER SectionIterCtx * iter = c.arg(0)->as_pointer()->extract<SectionIterCtx*>()
#define DDATAITER DataIterCtx * iter = c.arg(0)->as_pointer()->extract<DataIterCtx*>()
#define DSHDR64 Elf64_Shdr * shdr = c.arg(0)->as_pointer()->extract<Elf64_Shdr*>()
#define DSYM SymbolObj * sym = c.arg(0)->as_pointer()->extract<SymbolObj*>()

/*
typedef struct {
    pl::Pointer * elfp;
    pl::Pointer * scnp;
    Elf_Data * data;
} DataIterCtx;
*/

typedef struct {
    SV *elfp;
    Elf_Scn * scn;
} SectionObj;

typedef struct {
    SV *elfp;
    Elf_Data * data;
} DataObj;

/*
typedef struct {
    Elf * elf;
    GElf_Sym sym;
} SymbolObj;
*/

/////////////////////////////////////////////////////////////
// Data::ELF
/////////////////////////////////////////////////////////////

// Data::ELF->new($fd, $cmd)
XS(xs_new) {
    pl::Ctx c(3);

    // arg(0) is class name

    elf_version(EV_CURRENT);

    int fd = c.arg(1)->as_int()->to_c();
    int cmd = c.arg(2)->as_int()->to_c();
    Elf* e = elf_begin(fd, Elf_Cmd(cmd), NULL);
    if (e == NULL) {
        pl::Carp::croak("Data::ELF: %s\n", elf_errmsg(-1));
    }

    pl::Pointer ptr(e, "Data::ELF");
    c.ret(&ptr);
}

XS(xs_DESTROY) {
    pl::Ctx c(1);
    DELF;

    if (elf_end(elf) != 0) {
        pl::Carp::croak("Data::ELF: %s", elf_errmsg(-1));
    }
    c.return_true();
}

XS(xs_getshnum) {
    pl::Ctx c(1);
    DELF;

    size_t dst;
    elf_getshnum(elf, &dst);
    c.ret((int)dst);
}

XS(xs_getscn) {
    pl::Ctx c(2);
    DELF;

    int idx = c.arg(1)->as_int()->to_c();
    Elf_Scn * scn = elf_getscn(elf, idx);
    SectionObj * scnobj = new SectionObj;
    scnobj->scn = scn;
    scnobj->elfp = c.arg(0)->val;
    c.arg(0)->refcnt_inc();
    pl::Pointer ptr(scnobj, "Data::ELF::Section");
    c.ret(&ptr);
}

/*
XS(xs_class) {
    pl::Ctx c(1);
    DELF;

    int klass = (int)elf->class;
    c.ret( klass );
}
*/

/////////////////////////////////////////////////////////////
// Data::ELF::DataIter
/////////////////////////////////////////////////////////////

/*
XS(xs_dataiter_next) {
    pl::Ctx c(1);
    DDATAITER;

    iter->data = elf_getdata(iter->elf, iter->data);
    memcpy(buf, iter->data, sizeof(Elf_Data));
    pl::Pointer ptr(buf, "Data::ELF::Data");
    c.ret(&ptr);
}

XS(xs_dataiter_DESTROY) {
    pl::Ctx c(1);
    DDATAITER;

    delete data;

    c.return_true();
}
*/

/////////////////////////////////////////////////////////////
// Data::ELF::Section
/////////////////////////////////////////////////////////////

/*
XS(xs_scn_dataiter) {
    pl::Ctx c(1);
    DSCN;

    DataIterCtx * iter = new DataIterCtx;
    iter->scnp = arg(0)->as_pointer();
    iter->data = NULL;
    pl::Pointer ptr(iter, "Data::ELF::DataIter");
    c.ret(&ptr);
}
*/

XS(xs_scn_name) {
    pl::Ctx c(1);
    DSCN;

    Elf * elf = pl::Pointer(scn->elfp).extract<Elf*>();

    GElf_Ehdr ehdr;
    if (gelf_getehdr(elf, &ehdr) == NULL) {
        pl::Carp::croak("cannot get the ELF header: %s\n", elf_errmsg(-1));
    }

    GElf_Shdr shdr;
    if (gelf_getshdr(scn->scn, &shdr) != NULL) {
        char * name = elf_strptr(elf, ehdr.e_shstrndx, (size_t) shdr.sh_name);
        c.ret(name);
    } else {
        c.return_undef();
    }
}

XS(xs_scn_getdata) {
    pl::Ctx c(1);
    DSCN;

    Elf * elf = pl::Pointer(scn->elfp).extract<Elf*>();
    Elf_Data* data = elf_getdata(scn->scn, NULL);

    DataObj * dataobj = new DataObj;
    dataobj->elfp = scn->elfp;
    SvREFCNT_inc(dataobj->elfp);
    dataobj->data = data;

    pl::Pointer ptr(dataobj, "Data::ELF::Data");
    c.ret(&ptr);
}

XS(xs_scn_DESTROY) {
    pl::Ctx c(1);
    DSCN;

    SvREFCNT_dec(scn->elfp);
    delete scn;

    c.return_true();
}

/////////////////////////////////////////////////////////////
// Data::ELF::Data
/////////////////////////////////////////////////////////////

/*
XS(xs_data_getsym) {
    pl::Ctx c(2);
    DDATA;

    int idx = c.arg(1)->as_int()->to_c();
    SymbolObj *sym = new SymbolObj;
    gelf_getsym(data, idx, &sym->sym);
    pl::Pointer ptr(sym, "Data::ELF::Symbol");
    c.ret(&ptr);
}
*/

XS(xs_data_buf) {
    pl::Ctx c(1);
    DDATA;

    pl::Str buf((const char*)data->data->d_buf, data->data->d_size);
    c.ret(&buf);
}

XS(xs_data_DESTROY) {
    pl::Ctx c(1);
    DDATA;

    SvREFCNT_dec(data->elfp);
    delete data;

    c.return_true();
}

/////////////////////////////////////////////////////////////
// Data::ELF::Symbol
/////////////////////////////////////////////////////////////

/*
// $symbol->name($elf, $shdr)
// TODO: $elf hide to SymbolObject
XS(xs_sym_name) {
    pl::Ctx c(3);
    DSYM;

    Elf* elf         = c.arg(1)->as_pointer()->extract<Elf*>();
    Elf64_Shdr* shdr = c.arg(2)->as_pointer()->extract<Elf64_Shdr*>();
    char * name = elf_strptr(elf, shdr->sh_link, sym->sym.st_name);
    c.ret(name);
}
XS(xs_sym_value) {
    pl::Ctx c(3);
    DSYM;

    c.ret((int)sym->sym.st_value);
}
XS(xs_sym_DESTROY) {
    pl::Ctx c(0);
    DSYM;

    delete sym;
    c.return_true();
}
*/

/////////////////////////////////////////////////////////////
// Data::ELF::SectionHeader64
/////////////////////////////////////////////////////////////

XS(xs_shdr64_type) {
    pl::Ctx c(1);
    DSHDR64;

    c.ret(shdr->sh_type);
}

/*
XS(xs_shdr64_size) {
    pl::Ctx c(1);
    DSHDR64;

    c.ret((int)shdr->sh_size);
}

XS(xs_shdr64_entsize) {
    pl::Ctx c(1);
    DSHDR64;

    c.ret((int)shdr->sh_entsize);
}

XS(xs_shdr64_link) {
    pl::Ctx c(1);
    DSHDR64;

    c.ret((int)shdr->sh_link);
}
*/

/////////////////////////////////////////////////////////////
// bootstrap
/////////////////////////////////////////////////////////////

extern "C"
XS(boot_Data__ELF)
{
    pl::BootstrapCtx b;

    {
        pl::Package p("Data::ELF", __FILE__);
        p.add_method("new", xs_new);
        // p.add_method("class", xs_class);
        // p.add_method("sectioniter", xs_sectioniter);
        p.add_method("getshnum", xs_getshnum);
        p.add_method("getscn", xs_getscn);
        p.add_method("DESTROY", xs_DESTROY);
        p.add_constant("ELF_C_READ", ELF_C_READ);
        p.add_constant("ELFCLASS32", ELFCLASS32);
        p.add_constant("ELFCLASS64", ELFCLASS64);
        p.add_constant("SHT_SYMTAB", SHT_SYMTAB);
    }

//  {
//      pl::Package p("Data::ELF::SectionIter", __FILE__);
//      p.add_method("next", xs_scniter_next);
//      p.add_method("DESTROY", xs_scniter_DESTROY);
//  }

    {
//      pl::Package p("Data::ELF::SectionHeader64", __FILE__);
//      p.add_method("type", xs_shdr64_type);
        // p.add_method("size", xs_shdr64_size);
        // p.add_method("link", xs_shdr64_link);
        // p.add_method("entsize", xs_shdr64_entsize);
    }

    {
        pl::Package p("Data::ELF::Section", __FILE__);
        // p.add_method("dataiter", xs_scn_dataiter);
        p.add_method("name", xs_scn_name);
        p.add_method("getdata", xs_scn_getdata);
        p.add_method("DESTROY", xs_scn_DESTROY);
    }

    {
        pl::Package p("Data::ELF::Data", __FILE__);
        // p.add_method("getsym", xs_data_getsym);
        p.add_method("buf", xs_data_buf);
        p.add_method("DESTROY", xs_data_DESTROY);
    }

    /*

    {
        pl::Package p("Data::ELF::Symbol", __FILE__);
        p.add_method("name", xs_sym_name);
        p.add_method("value", xs_sym_value);
        p.add_method("DESTROY", xs_sym_DESTROY);
    }

    {
        pl::Package p("Data::ELF::DataIter", __FILE__);
        p.add_method("next", xs_dataiter_next);
        p.add_method("DESTROY", xs_dataiter_DESTROY);
    }

    */
}

