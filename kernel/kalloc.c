// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

// counter for cow page reference
#define PA2COWIDX(pa) (((uint64)(pa) - KERNBASE) >> 12)
#define MAXPHYPG ((PHYSTOP - KERNBASE) >> 12)
struct {
    struct spinlock lock;
    int refcount[MAXPHYPG];
} cowpg;

// increase cow page reference count
void cow8ref(void *pa)
{
    acquire(&cowpg.lock);
    ++cowpg.refcount[PA2COWIDX(pa)];
    release(&cowpg.lock);
}

// decrease reference count only when it's on cow page
static int cow6ref(void *pa)
{
    int c;
    acquire(&cowpg.lock);
    c = --cowpg.refcount[PA2COWIDX(pa)];
    release(&cowpg.lock);
    return c;
}

static void initcow(void)
{
  initlock(&cowpg.lock, "cowlock");
  for (int i = 0; i < MAXPHYPG; ++i)
      cowpg.refcount[i] = 0;
}

void
kinit()
{
  initcow();
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // return if the page is still referenced
  if (cow6ref(pa) > 0)
      return;

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r) {
      memset((char*)r, 5, PGSIZE); // fill with junk
      cowpg.refcount[PA2COWIDX(r)] = 1;
  }

  return (void*)r;
}
