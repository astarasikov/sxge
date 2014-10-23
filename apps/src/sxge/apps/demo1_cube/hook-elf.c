#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <elf.h>

#if 1
        #define HOOK_DBG(fmt, ...) \
            fprintf(stderr, fmt "\n", __VA_ARGS__)
#else
        #define HOOK_DBG(fmt, ...) do {} while (0)
#endif

#define CHECK(expr) do { \
    if (!(expr)) { \
        HOOK_DBG("Failed: %s", #expr); \
        goto err; \
    } \
} while (0)

#ifndef __x86_64__
        #error "Hmm, this is implemented only for __x86_64__"
#endif

#if 0
static int setProtection(void *ptr, bool writable) {
        size_t PAGE_SIZE = sysconf(_SC_PAGESIZE);
        size_t PAGE_MASK = PAGE_SIZE - 1;
        void *page_ptr = (void*)(((size_t)ptr) & ~PAGE_MASK);
        if (mprotect(page_ptr, PAGE_SIZE,
                       PROT_READ | (writable ? PROT_WRITE : 0)) < 0)
        {
                HOOK_DBG("failed to mprotect %p, error %s", ptr, strerror(errno));
                return -1;
        }
        return 0;
}
#endif

static Elf64_Shdr *elfSectionByType(
        Elf64_Ehdr *elf_header,
        Elf64_Shdr *sections,
        size_t type)
{
    size_t i = 0;
    for (i = 0; i < elf_header->e_shnum; i++)
    {
        if (sections[i].sh_type == type)
        {
            return sections + i;
        }
    }
    return NULL;
}

static Elf64_Shdr *elfSectionByName(
        size_t lib_start,
        Elf64_Ehdr *elf_header,
        Elf64_Shdr *sections,
        const char *name)
{
    size_t i = 0;
    Elf64_Shdr *string_section = NULL;
    const char *strings = NULL;

    CHECK(string_section = sections + elf_header->e_shstrndx);
    CHECK(strings = (const char*)(lib_start + string_section->sh_offset));

    for (i = 0; i < elf_header->e_shnum; i++)
    {
        if (!strcmp(name, strings + sections[i].sh_name))
        {
            return sections + i;
        }
    }
err:
    return NULL;
}

static Elf64_Sym *elfSymbolByName(
        size_t lib_start,
        Elf64_Shdr *sections,
        Elf64_Shdr *section,
        const char *name,
        size_t *idx_out)
{
    Elf64_Shdr *strings_section = NULL;
    size_t i = 0;
    size_t count = 0;
    const char *strings = NULL;
    Elf64_Sym *symbols = NULL;

    Elf64_Sym *current_symbol = NULL;
    const char *current_symbol_name = NULL;
    
    CHECK(strings_section = sections + section->sh_link);
    CHECK(strings = (const char*)(lib_start + strings_section->sh_offset));

    CHECK(symbols = (Elf64_Sym*)(lib_start + section->sh_offset));
    CHECK(count = section->sh_size / sizeof(Elf64_Sym));

    for (i = 0; i < count; i++)
    {
        CHECK(current_symbol = symbols + i);
        CHECK(current_symbol_name = strings + current_symbol->st_name);
        if (!strcmp(name, current_symbol_name))
        {
            *idx_out = i;
            return current_symbol;
        }
    }
err:
    return NULL;
}

int replaceSymbol(const char* libname, const char *path, size_t newFunc)
{
    int rc = -1;
    struct stat sdata;
    size_t libdata = 0;
    size_t *lib_handle = NULL;
    size_t lib_in_process = 0;

    Elf64_Ehdr *elf_header = NULL;
    Elf64_Shdr *section_header = NULL;
    Elf64_Shdr *dynsym = NULL;
    
    Elf64_Shdr *rel_plt = NULL;
    Elf64_Rel *rel_plt_table = NULL;
    
    Elf64_Sym *symbol = NULL;
    size_t symbol_idx = 0;

    size_t i = 0;
    int fd = -1;

    CHECK((fd = open(path, O_RDONLY)) >= 0);
    CHECK(fstat(fd, &sdata) >= 0);
    CHECK(libdata = (size_t)mmap(NULL, sdata.st_size, PROT_READ,
                    MAP_SHARED, fd, 0));

    CHECK(lib_handle = (size_t*)dlopen(libname, RTLD_NOW));
    CHECK(lib_in_process = *lib_handle);

    CHECK(elf_header = (Elf64_Ehdr*)libdata);
    CHECK(section_header = (Elf64_Shdr*)(libdata + elf_header->e_shoff));
    CHECK(dynsym = elfSectionByType(elf_header, section_header, SHT_DYNSYM));
    
    CHECK(symbol = elfSymbolByName(
            libdata,
            section_header,
            dynsym,
            "ioctl",
            &symbol_idx));
    
    CHECK(rel_plt = elfSectionByName(
                libdata,
                elf_header,
                section_header,
                ".rela.plt"));
    
    CHECK(rel_plt_table = (Elf64_Rel*)(lib_in_process + rel_plt->sh_addr));

    for (i = 0; i < rel_plt->sh_size / sizeof(Elf64_Rel); i++)
    {
        if (ELF64_R_SYM(rel_plt_table[i].r_info) != symbol_idx)
        {
            continue;
        }
        size_t *function_ptr = NULL;
        CHECK(function_ptr = (size_t*)(lib_in_process + rel_plt_table[i].r_offset));

        *function_ptr = newFunc;
    }

    rc = 0;
err:
    if (fd >= 0)
    {
        close(fd);
    }
    if (libdata)
    {
        munmap((void*)libdata, sdata.st_size);
    }
    if (lib_handle)
    {
        dlclose(lib_handle);
    }
    return rc;
}
