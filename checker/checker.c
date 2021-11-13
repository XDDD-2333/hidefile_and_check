#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/dirent.h>


unsigned long * syscall_table;
unsigned long myorg_syscall_table[NR_syscalls];


static struct 
kprobe kp = {
    .symbol_name = "kallsyms_lookup_name"
};
     
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

static int __init checker(void) {
  int i=0;
  syscall_table = get_syscall_table();
  if (syscall_table == NULL) {
    printk(KERN_ALERT "mytest checker: could not get syscall table address");
    return 0;
}
  for(i=0;i<NR_syscalls;i++) {
    myorg_syscall_table[i]=syscall_table[i];   
    // printk(KERN_ALERT "mytest checker: syscall_table[i] is %p , org_syscall_table[i] is %p ",(void *)syscall_table[i],(void *)org_syscall_table[i]);
  }
  printk(KERN_ALERT "mycheck checker: syscall_table[217] is %p , org_syscall_table[217] is %p ",(void *)syscall_table[217],(void *)myorg_syscall_table[217]);
  printk(KERN_ALERT "mycheck checker: org_syscall_table is %p ++++",(void *)myorg_syscall_table);
  printk(KERN_ALERT "mycheck checker: save sys_call_table is %p done ++++",syscall_table);
  return 0;
}

static void __exit rmchecker(void) {
  printk(KERN_ALERT "mytest checker: rmmod checker");
}

module_init(checker);
module_exit(rmchecker);
MODULE_LICENSE("GPL");
