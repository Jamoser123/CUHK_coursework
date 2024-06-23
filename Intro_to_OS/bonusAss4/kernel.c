#include <stdbool.h>
#include "kernel.h"
/*
  1. Check if a free process slot exists and if the there's enough free space (check allocated_pages).
  2. Alloc space for page_table (the size of it depends on how many pages you need) and update allocated_pages.
  3. The mapping to kernel-managed memory is not built up, all the PFN should be set to -1 and present byte to 0.
  4. Return a pid (the index in MMStruct array) which is >= 0 when success, -1 when failure in any above step.
*/

int getPages(int size){
  int pages = size / PAGE_SIZE;
  if(size == PAGE_SIZE*pages) return pages;
  else return pages + 1;
}

int proc_create_vm(struct Kernel* kernel, int size) { //I think it's alright for the moment
  int pid = -1;
  int pages = getPages(size);

  for(int i = 0; i < MAX_PROCESS_NUM; i++){ //get free process slot
    if(kernel->running[i] == 0){
      pid = i;
      break;
    }
  }

  if(pid == -1 || size > VIRTUAL_SPACE_SIZE || (KERNEL_SPACE_SIZE < (kernel->allocated_pages + pages)*PAGE_SIZE)){ //no free process slot or not enough spaces
    return -1;
  }

  kernel->allocated_pages += pages; //increase allocated pages
  kernel->running[pid] = 1;

  //create array of PTE's and zero them out
  struct PTE* pte = (struct PTE*) malloc(pages*sizeof(struct PTE));
  for(int i = 0; i < pages; i++){
    pte[i].PFN = -1;
    pte[i].present = 0;
  }

  struct PageTable* pt = (struct PageTable*) malloc(sizeof(struct PageTable));
  pt->ptes = pte;
  
  kernel->mm[pid].size = size;
  kernel->mm[pid].page_table = pt;

  return pid;
}

/*
  This function will read the range [addr, addr+size) from user space of a specific process to the buf (buf should be >= size).
  1. Check if the reading range is out-of-bounds.
  2. If the pages in the range [addr, addr+size) of the user space of that process are not present,
     you should firstly map them to the free kernel-managed memory pages (first fit policy).
  Return 0 when success, -1 when failure (out of bounds).
*/
bool outOfBound(struct Kernel* kernel, int pid, char* addr, int size){ //check if out of bounds
  if(kernel->mm[pid].size < ((int) addr) + size) return true;
  else return false;
}

int getPFN(struct Kernel* kernel){ //get first free PFN
  for(int i = 0; i < KERNEL_SPACE_SIZE/PAGE_SIZE; i++){
    if(kernel->occupied_pages[i] == 0){
      kernel->occupied_pages[i] = 1;
      return i;
    }
  }
}

int vm_read(struct Kernel* kernel, int pid, char* addr, int size, char* buf) {
  if(outOfBound(kernel, pid, addr, size)){
    return -1;
  }

  for(int i = 0; i < size; i++){
    int pageNum = ((int) addr + i)/PAGE_SIZE; //check which page this bit is on
    if(kernel->mm[pid].page_table->ptes[pageNum].present == 0){
      kernel->mm[pid].page_table->ptes[pageNum].present = 1;
      kernel->mm[pid].page_table->ptes[pageNum].PFN = getPFN(kernel);
    }

    buf[i] = kernel->space[kernel->mm[pid].page_table->ptes[pageNum].PFN*PAGE_SIZE + i%PAGE_SIZE];
  }

  return 0;

}

/*
  This function will write the content of buf to user space [addr, addr+size) (buf should be >= size).
  1. Check if the writing range is out-of-bounds.
  2. If the pages in the range [addr, addr+size) of the user space of that process are not present,
     you should firstly map them to the free kernel-managed memory pages (first fit policy).
  Return 0 when success, -1 when failure (out of bounds).
*/
int vm_write(struct Kernel* kernel, int pid, char* addr, int size, char* buf) {
  if(outOfBound(kernel, pid, addr, size)){
    return -1;
  }

  for(int i = 0; i < size; i++){
    int pageNum = ((int) addr + i)/PAGE_SIZE;
    if(kernel->mm[pid].page_table->ptes[pageNum].present == 0){
      kernel->mm[pid].page_table->ptes[pageNum].present = 1;
      kernel->mm[pid].page_table->ptes[pageNum].PFN = getPFN(kernel);
    }

    kernel->space[kernel->mm[pid].page_table->ptes[pageNum].PFN*PAGE_SIZE + i%PAGE_SIZE] = buf[i];
  }

  return 0;
}

/*
  This function will free the space of a process.
  1. Reset the corresponding pages in occupied_pages to 0.
  2. Release the page_table in the corresponding MMStruct and set to NULL.
  Return 0 when success, -1 when failure.
*/

int proc_exit_vm(struct Kernel* kernel, int pid) {
  
  if(kernel->running[pid] == 0) return -1;

  int pages = getPages(kernel->mm[pid].size);

  for(int i = 0; i < pages; i++){
    if(kernel->mm[pid].page_table->ptes[i].present == 1){
      memset(kernel->space + kernel->mm[pid].page_table->ptes[i].PFN, 0, PAGE_SIZE);
      kernel->occupied_pages[kernel->mm[pid].page_table->ptes[i].PFN] = 0;
      kernel->mm[pid].page_table->ptes[i].present = 0;
    }
  }

  kernel->allocated_pages -= pages;
  kernel->mm[pid].size = 0;

  free(kernel->mm[pid].page_table->ptes);
  free(kernel->mm[pid].page_table);
  kernel->mm[pid].page_table = NULL;
  kernel->running[pid] = 0;

  return 0;
}