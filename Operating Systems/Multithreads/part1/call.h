#include<linux/unistd.h>

#define __NR_mycall 341
#define __NR_carlosalvacall 342

extern long int syscall(long int __sysno, ...) __THROW;

