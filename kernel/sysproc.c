#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sysinfo.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// system call trace
uint64 sys_trace(void)
{
    int mask;

    if (argint(0, &mask) < 0)
        return -1;
    myproc()->tracemask = mask;
    return 0;
}

// system call sysinfo
uint64 sys_sysinfo(void)
{
    struct sysinfo info;
    uint64 uvainfo = 0;

    if (argaddr(0, &uvainfo) < 0)
        return -1;
    info.freemem = freemem();
    info.nproc = nproc();
    if (copyout(myproc()->pagetable, uvainfo, (char *)&info, sizeof(struct sysinfo)) < 0)
        return -1;
    return 0;
}

// system call pgaccess
uint64 sys_pgaccess(void)
{
    pagetable_t pagetable;
    int upages, level;
    uint64 uresult, result = 0, upg = 0;
    pte_t *pte;
    struct proc *p;

    if (argaddr(0, &upg) < 0)
        return -1;
    if (argint(1, &upages) < 0)
        return -1;
    if (argaddr(2, &uresult) < 0)
        return -1;

    // max bits of bitmask result in uint64
    if (upages > 32)
        return -1;

    p = myproc();
    //vmprint(p->pagetable);

    for (int page = 0; page < upages; ++page) {
        pagetable = p->pagetable;

        // walk to level 0 pagetable directory
        for (level = 2; level > 0; --level) {
            pte = &pagetable[PX(level, upg)];
            if (*pte & PTE_V)
                pagetable = (pagetable_t) PTE2PA(*pte);
            else
                goto pga_nextpage;
        }
        // level 0 pte
        pte = &pagetable[PX(level, upg)];

        // check access bit
        if (*pte & PTE_A) {
            result |= 1L << page;
            *pte &= ~PTE_A;
        }

pga_nextpage:
        upg += PGSIZE;
    }

    return copyout(p->pagetable, uresult, (char *)&result, sizeof(uint));
}
