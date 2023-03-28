route delete default dev wlp1s0
route delete default dev wlp2s0
#route add 192.168.20.30 dev enp0s31f6


ip rule add from 192.168.1.4 table 1
ip rule add from 192.168.10.245 table 2

ip route add 192.168.1.0/24 dev wlp2s0 scope link table 1
ip route add default via 192.168.1.1 dev wlp2s0 table 1

ip route add 192.168.10.0/24 dev wlp1s0 scope link table 2
ip route add default via 192.168.10.1 dev wlp1s0 table 2

ip route add default scope global nexthop via 192.168.10.1 dev wlp1s0
#ip route add default scope global nexthop via 192.168.1.1 dev wlp1s0


#ip mptcp endpoint flush
#ip mptcp limits set subflow 2 add_addr_accepted 2
#ip mptcp endpoint add 192.168.1.3 dev wlp2s0 signal id 26
#ip mptcp endpoint add 192.168.10.245 dev wlp1s0 signal id 28


#echo 8 > /proc/sys/kernel/printk

#echo 'file net/mptcp/protocol.c +p' > /sys/kernel/debug/dynamic_debug/control



