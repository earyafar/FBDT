echo 8 > /proc/sys/kernel/printk
echo 'file net/mptcp/protocol.c +p' > /sys/kernel/debug/dynamic_debug/control
