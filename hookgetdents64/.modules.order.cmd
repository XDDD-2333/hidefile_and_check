cmd_/home/dhz/my_lkm/hookgetdents64/modules.order := {   echo /home/dhz/my_lkm/hookgetdents64/hook.ko; :; } | awk '!x[$$0]++' - > /home/dhz/my_lkm/hookgetdents64/modules.order
