#include <elf32.h>
#include <sdcard.h>
#include <log.h>
#include <string.h>

void elf_load_program(elf_program_header_t program_header)
{
    uint32_t file_size = program_header.file_size;
    uint8_t *ram_address = (uint8_t *)program_header.paddr;
    uint32_t block_offset = program_header.offset;
    uint8_t block[512];
    memory_set(block, 0, 512);
    while(file_size) {
        // Translate address to sdcard block address
        uint32_t block_addr = block_offset >> 9;
        // Check if data is in the middle of the block
        uint32_t block_index = block_offset % 512;
        // Get the size to copy to RAM
        uint32_t size = (file_size < 512) ? file_size - block_index : 512 - block_index;

        sdcard_read_block(block_addr, block);
        memory_copy(ram_address, &block[block_index], size);

        file_size -= size;
        ram_address += size;
        block_offset += size;
    }
    
}

void elf_load()
{
    elf_file_header_t file_header;
    elf_program_header_t *program_header_table;
    uint8_t block[512];
    memory_set(block, 0, 512);
    sdcard_read_block(0, block);
    
    if (block[0] != 0x7f ||
        block[1] != 'E' ||
        block[2] != 'L' ||
        block[3] != 'F') {
        log_error(ERROR_INVALID, "ELF file not located at beginning of SD card");
        return;
    }

    // TODO: Validate elf file

    memory_copy(&file_header, &block[0], sizeof(elf_file_header_t));
    program_header_table = (elf_program_header_t *)(block + file_header.program_header_offset);
    if (file_header.program_header_count * file_header.program_header_size > (512 - file_header.size)) {
        log_error(ERROR_INVALID, "Too many program headers %u", file_header.program_header_count);
        return;
    }

    for (uint32_t i = 0; i < file_header.program_header_count; i++) {
        elf_load_program(program_header_table[i]);
    }
}
