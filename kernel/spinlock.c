// Mutual exclusion spin locks.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "proc.h"
#include "defs.h"

void
initlock(struct spinlock *lk, char *name)
{
  lk->name = name; // 锁名
  lk->locked = 0;  // 初始未上锁
  lk->cpu = 0;     
}

// 获取锁
// 循环 (自旋) 直到获取到锁
void
acquire(struct spinlock *lk)
{
  push_off(); // 嵌套计数器++
  if(holding(lk))
    panic("acquire"); // 杜绝重复获取锁，直接panic

  // 在 RISC-V 架构中，sync_lock_test_and_set 会转换为原子交换指令：
  //   a5 = 1
  //   s1 = &lk->locked
  //   amoswap.w.aq a5, a5, (s1)
  while(__sync_lock_test_and_set(&lk->locked, 1) != 0) // 只要旧值不为0(说明锁被占用)，就自旋
    ;

  // Tell the C compiler and the processor to not move loads or stores
  // past this point, to ensure that the critical section's memory
  // references happen strictly after the lock is acquired.
  // On RISC-V, this emits a fence instruction.
  __sync_synchronize(); // 在此指令之前的所有内存读写操作，必须在执行此指令之后的内存读写操作之前完成

  // 记录哪个核心获得了锁，用于 holding() 函数和调试
  lk->cpu = mycpu();
}

// 释放锁
void
release(struct spinlock *lk)
{
  if(!holding(lk))
    panic("release"); // 释放未持有的锁直接panic

  lk->cpu = 0; // 重置

  // Tell the C compiler and the CPU to not move loads or stores
  // past this point, to ensure that all the stores in the critical
  // section are visible to other CPUs before the lock is released,
  // and that loads in the critical section occur strictly before
  // the lock is released.
  // On RISC-V, this emits a fence instruction.
  __sync_synchronize(); // 和上面一样

  // 释放锁，等同于 lk->locked = 0
  // 此处代码没有使用 C 语言的赋值语句，因为 C 标准
  // 意味着赋值操作可能会被分解为多条存储指令来实现
  // 在 RISC-V 架构中，sync_lock_release 会转换为原子交换指令：
  //   s1 = &lk->locked
  //   amoswap.w zero, zero, (s1)
  __sync_lock_release(&lk->locked);

  pop_off(); // 启用中断
}

// Check whether this cpu is holding the lock.
// Interrupts must be off. 如果在检查holding期间中断，且进程被调度到别的core
// mycpu()结果就会改变，判断就会出错
int
holding(struct spinlock *lk)
{
  int r;
  r = (lk->locked && lk->cpu == mycpu());
  return r;
}

// push_off/pop_off are like intr_off()/intr_on() except that they are matched:
// it takes two pop_off()s to undo two push_off()s.  Also, if interrupts
// are initially off, then push_off, pop_off leaves them off.

void
push_off(void)
{
  int old = intr_get(); // 记录初始中断状态

  // disable interrupts to prevent an involuntary context
  // switch while using mycpu().
  intr_off();

  if(mycpu()->noff == 0)
    mycpu()->intena = old; // 只有noff为0时，old才会被保存
                           // 也就是在第一次获取锁时记录之前的中断状态
                           // 注意必须在intr_off()之后才能使用mucpu()
                           // 所以需要old这个临时变量
  mycpu()->noff += 1;
}

void
pop_off(void)
{
  struct cpu *c = mycpu();
  if(intr_get())
    panic("pop_off - interruptible");
  if(c->noff < 1)
    panic("pop_off");
  c->noff -= 1;
  if(c->noff == 0 && c->intena) // 只有noff归零且之前就开启了中断，才会开启中断
    intr_on();
}
