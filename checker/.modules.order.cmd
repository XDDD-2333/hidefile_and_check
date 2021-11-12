cmd_/home/dhz/my_lkm/checker/modules.order := {   echo /home/dhz/my_lkm/checker/checker.ko; :; } | awk '!x[$$0]++' - > /home/dhz/my_lkm/checker/modules.order
