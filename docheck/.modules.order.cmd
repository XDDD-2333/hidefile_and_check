cmd_/home/dhz/my_lkm/docheck/modules.order := {   echo /home/dhz/my_lkm/docheck/docheck.ko; :; } | awk '!x[$$0]++' - > /home/dhz/my_lkm/docheck/modules.order
