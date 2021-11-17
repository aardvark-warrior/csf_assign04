/*
 * Implementation of main and helper functions for processing ELF (or non-ELF) files
 * CSF Assignment 4
 * M.Albert and A.Wang
 * malber20@jhu.edu
 * awang91@jhu.edu
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <cerrno>
#include <cstdint>
#include <elf.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <fcntl.h>
#include <iostream>
#include <string>

using std::cout;
using std::endl;
using std::cerr;

#include "elf_names.h"

void print_header_info(void* data, unsigned char*& strtab_data, int& num_syms, Elf64_Sym*& sym_table, Elf64_Ehdr* elf_header);
void print_sym_info(int num_syms, Elf64_Sym* sym_table, unsigned char* strtab_data);

int main(int argc, char **argv) {
  if (argc != 2) {
    cerr << "ERROR: Invalid number of command args" << endl;
    return 1;
  }
  int fd = open(argv[1], O_RDONLY);

  if (fd < 0) {  // If cannot open file
    cerr << "ERROR: Cannot open file" << endl;
    return 2;  
  }

  // Get file size using statbuf
  struct stat statbuf;
  int rc = fstat(fd, &statbuf);
  if (rc != 0) {
    cerr << "ERROR: fstat failed" << endl;
    return 3;
  }
  size_t file_size = statbuf.st_size;

  // Create private, read-only mapping to access file contents in memory
  void *data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (data == (void*)-1) {  // If pointer points to memory region inaccessible to program
    cerr << "ERROR: mapped to inaccessible memory region" << endl;
    return 4;
  }

  Elf64_Ehdr *elf_header = (Elf64_Ehdr*) data;

  // Check if opened file is ELF
  if (elf_header->e_ident[0] != 127 ||
      elf_header->e_ident[1] != 69 ||
      elf_header->e_ident[2] != 76 ||
      elf_header->e_ident[3] != 70) {  
    cout << "Not an ELF file" << endl;
    return 0;
  }

  // Print ELF file info
  cout << "Object file type: " << get_type_name(elf_header->e_type) << endl;
  cout << "Instruction set: " << get_machine_name(elf_header->e_machine) << endl;
  cout << "Endianness: " << ((elf_header->e_ident[EI_DATA] == 1) ? "Little endian" : "Big Endian") << endl;

  unsigned char* strtab_data = nullptr;
  Elf64_Sym* sym_table = nullptr;
  int num_syms = 0;

  print_header_info(data, strtab_data, num_syms, sym_table, elf_header);
  print_sym_info(num_syms, sym_table, strtab_data);

  return 0;
}

/**
 * Function to print section header data.
 * @param data ELF data from memory mapping
 * @param strtab_data ptr for string table to be read into
 * @param num_syms integer to hold number of symbols
 * @param sym_table ptr to hold symbol table data
 * @param elf_header ELF file header data
 */
void print_header_info(void* data, unsigned char*& strtab_data, int& num_syms, Elf64_Sym*& sym_table, Elf64_Ehdr* elf_header) {
  unsigned char* sh_data = (unsigned char*) elf_header + elf_header->e_shoff;  // Find section headers in ELF header
  std::string symtab = ".symtab";
  std::string strtab = ".strtab";

  Elf64_Shdr* section_name_table = &((Elf64_Shdr*)sh_data)[elf_header->e_shstrndx];  // Find the section of strtab w/ names of sections
  unsigned char* shstrtab_data = (unsigned char*) data + section_name_table->sh_offset;

  for (int i = 0; i < elf_header->e_shnum; i++) {  
    Elf64_Shdr* section_headers = (Elf64_Shdr*) sh_data + i;
    std::string name((char*) (section_headers->sh_name + shstrtab_data));
    if (symtab.compare(name) == 0) {
      sym_table = (Elf64_Sym*) ((unsigned char*) data + section_headers->sh_offset);
      num_syms = section_headers->sh_size / section_headers->sh_entsize;
    }

    else if (strtab.compare(name) == 0) {
      strtab_data = (unsigned char*) data + section_headers->sh_offset;
    }

    cout << "Section header " << i << ": name=" << name;
    printf(", type=%lx, offset=%lx, size=%lx\n", 
      (long unsigned int) section_headers->sh_type, section_headers->sh_offset, section_headers->sh_size);
  }
}

/**
 * Function to print the symbol table info.
 * @param num_syms number of symbols
 * @param sym_table symbol table data
 * @param strtab_data string table data
 */
void print_sym_info(int num_syms, Elf64_Sym* sym_table, unsigned char* strtab_data) {
  for (int i = 0; i < num_syms; i++) {
    char* name;
    if (sym_table->st_name != 0) name = (char*) (sym_table->st_name + strtab_data);
    cout << "Symbol " << i << ": name=" << ((sym_table->st_name != 0) ? name : "");
    printf(", size=%lx, info=%lx, other=%d\n", 
      sym_table->st_size, (long unsigned int) sym_table->st_info, sym_table->st_other);
    sym_table++;
  }
}
