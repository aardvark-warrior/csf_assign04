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

#include "elf_names.h"

int main(int argc, char **argv) {
  // TODO: implement
  int fd = open(argv[1], O_RDONLY);

  if (fd < 0) {
    // error - do something
    return 1;
  }

struct stat statbuf;
int rc = fstat(fd, &statbuf);
if (rc != 0) {
  // error - do something
  return 1;
}
size_t file_size = statbuf.st_size;
// ...
void *data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
if (data == (void*) -1) {
  // error - do something
  return 1;
}

Elf64_Ehdr *elf_header = (Elf64_Ehdr *) data;




}
