# Backbone net config

# General section
[general]
id=0x0404202200000000
name=Backbone
gdb_groups_prefix=scorpion
native_ticker=CELL

# Possible values: light, full, archive, master, root
node-role={NODE_TYPE}
seed_nodes_aliases=[0.root.scorpion,1.root.scorpion,2.root.scorpion,3.root.scorpion,4.root.scorpion]
seed_nodes_hostnames=[0.root.scorpion.cellframe.net,1.root.scorpion.cellframe.net,2.root.scorpion.cellframe.net,3.root.scorpion.cellframe.net,4.root.scorpion.cellframe.net]
seed_nodes_addrs=[0404::2022::0000::0000,0404::2022::0000::0001,0404::2022::0000::0002,0404::2022::0000::0003,0404::2022::0000::0004]
seed_nodes_port=[80,80,80,80,80]
require_links=3

#[auth]
#type=ca
#acl_accept_ca_list=[]
#acl_accept_ca_gdb=
#acl_accept_ca_chains=all

[dag-poa]
#events-sign-cert=scorpion.root.0

[block-ton]
#blocks-sign-cert=scorpion.root.0

#[block-poa]
#blocks-sign-cert=mycert
