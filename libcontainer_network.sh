ip netns add netns1
ip link add A type veth peer name B
ip link set B netns netns1

brctl addbr bridge0
ip addr add 172.17.42.1/16 dev bridge0
ip link set dev bridge0 up
brctl addif bridge0 A
ip link set A up

ip netns exec netns1 ip link set dev lo up
ip netns exec netns1 ip link set dev B name eth0
ip netns exec netns1 ip link set eth0 up
ip netns exec netns1 ip addr add 172.17.0.2/16 dev eth0
ip netns exec netns1 ip route add default via 172.17.42.1

iptables -t nat -A POSTROUTING -s 172.17.42.1/16  -d 0.0.0.0/0 -j MASQUERADE

#iptables -t nat -A POSTROUTING -o p2p1 -j MASQUERADE
