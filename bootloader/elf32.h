#ifndef ELF32_H
#define ELF32_h

#include <stdint.h>

typedef struct elf_file_header {
    uint8_t ident[16];
    uint16_t type;
    uint16_t machine;
    uint32_t version;
    uint32_t entry;
    uint32_t program_header_offset;
    uint32_t section_header_offset;
    uint32_t flags;
    uint16_t size;
    uint16_t program_header_size;
    uint16_t program_header_count;
    uint16_t section_header_size;
    uint16_t section_header_count;
    uint16_t section_header_names_index;
} elf_file_header_t;

typedef struct elf_program_header {
    uint32_t type;
    uint32_t offset;
    uint32_t vaddr;
    uint32_t paddr;
    uint32_t file_size;
    uint32_t memory_size;
    uint32_t flags;
    uint32_t align;
} elf_program_header_t;

typedef struct elf_section_header {
    uint32_t name;
    uint32_t type;
    uint32_t flags;
    uint32_t addr;
    uint32_t offset;
    uint32_t size;
    uint32_t link;
    uint32_t info;
    uint32_t addralign;
    uint32_t entry_size;
} elf_section_header_t;

void elf_load();

#endif
