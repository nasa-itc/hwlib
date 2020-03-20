#ifndef _lib_mem_h_
#define _lib_mem_h_

void Xil_MMAP_Out32(unsigned int Addr, unsigned int Value); 
unsigned int Xil_MMAP_In32(unsigned int Addr); 
int devmem_write(unsigned int addr, unsigned char *in, int length); 
int devmem_read(unsigned int addr, unsigned char *out, int length); 

#endif /*_lib_mem_h_*/
