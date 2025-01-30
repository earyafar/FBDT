
route delete default dev enp4s0
route delete default dev enp5s0
route delete default dev enx681ca2121563
route add 192.168.20.30 dev enx681ca2121563


ip rule add from 192.168.1.2 table 1
ip rule add from 192.168.10.231 table 2
#ip rule add from 192.168.20.20 table 3

ip route add 192.168.1.0/24 dev enp5s0 scope link table 1
ip route add default via 192.168.1.1 dev enp5s0 table 1

ip route add 192.168.10.0/24 dev enp4s0 scope link table 2
ip route add default via 192.168.10.1 dev enp4s0 table 2

#ip route add 192.168.20.0/24 dev enx681ca2121563 scope link table 3
#ip route add default via 192.168.20.30 dev  enx681ca2121563 table 3

ip route add default scope global nexthop via 192.168.10.1 dev enp4s0


ip mptcp endpoint flush
ip mptcp limits set subflow 3 add_addr_accepted 3
ip mptcp endpoint add 192.168.10.231 dev enp4s0 subflow id 26
ip mptcp endpoint add 192.168.1.2 dev enp5s0 subflow id 28
#ip mptcp endpoint add 192.168.20.20 dev enx681ca2121563 subflow id 30

#cd mptcp-socket-example/src/packet_scheduler/

#echo 8 > /proc/sys/kernel/printk
#echo 'file net/mptcp/protocol.c +p' > /sys/kernel/debug/dynamic_debug/control

