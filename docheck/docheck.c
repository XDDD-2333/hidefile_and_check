#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>


unsigned long * syscall_table;
static  unsigned long __lkm_order;

static struct 
kprobe kp = {
    .symbol_name = "kallsyms_lookup_name"
};

// extern unsigned long __force_order;
static inline void mywrite_cr0(unsigned long value) {
    asm volatile("mov %0,%%cr0":"+r"(value),"+m"(__lkm_order));
}
       


static int __init docheck(void) {
  unsigned long orig_cr0;
  int i=0;
  unsigned long * org_syscall_table;
  typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);
  kallsyms_lookup_name_t kallsyms_lookup_name;

  /*注册*/
  register_kprobe(&kp);

  /* assign kallsyms_lookup_name symbol to kp.addr */
  kallsyms_lookup_name = (kallsyms_lookup_name_t) kp.addr;
    
  /*注销*/
  unregister_kprobe(&kp);
  syscall_table = (unsigned long *) kallsyms_lookup_name("sys_call_table");
  org_syscall_table = (unsigned long *)kallsyms_lookup_name("myorg_syscall_table"); 
  if (syscall_table == NULL) {
    printk(KERN_ALERT "mytest docheck: could not get syscall_table address");
    return 0;
    }
  if (org_syscall_table == NULL) {
    printk(KERN_ALERT "mytest docheck: could not get org_syscall_table address");
    return 0;
    }
  // printk(KERN_ALERT "mycheck docheck: syscall_table[217] is %p , org_syscall_table[217] is %p ",(void *)syscall_table[217],(void *)org_syscall_table[217]);
  // printk(KERN_ALERT "mycheck docheck: org_syscall_table is %p , syscall_table is %p ++++ ",org_syscall_table,syscall_table);
  // printk(KERN_ALERT "mycheck docheck: syscall_table[217] is %p , org_syscall_table[217] is %p ",(void *)syscall_table[217],(void *)org_syscall_table[217]);
  for(i=0;i<NR_syscalls;i++) {
    if(org_syscall_table[i]!=syscall_table[i]){
        printk(KERN_ALERT "mytest docheck: find someone want to change syscall_table !!! +++++++++++");
        orig_cr0 = read_cr0();
        mywrite_cr0(orig_cr0 & (~0x10000));
        syscall_table[i]=org_syscall_table[i];
        mywrite_cr0(orig_cr0);
    }  
  }
  return 0;
}

static void __exit rmdocheck(void) {
  printk(KERN_ALERT "mytest docheck: rmmod rmdocheck");
}

module_init(docheck);
module_exit(rmdocheck);
MODULE_LICENSE("GPL");
