# General section
[general]
# General debug mode
debug_mode=false
# Debug stream packets
debug_dump_stream_headers=false
# Debug I/O reactor, false by default
#debug_reactor=false
# Debug HTTP protocol, false by default
#debug_http=false

# seed mode. WARNING. Used true only when you start the new network
#seed_mode=false

# Auto bring up links and sync everything over them
auto_online={AUTO_ONLINE}

# Server part
[server]
#   By default you don't need to open you to the world
enabled={SERVER_ENABLED}
news_url_enabled=false
bugreport_url_enabled=false
listen_address={SERVER_ADDR}
listen_port_tcp={SERVER_PORT}
# External IPv4 address
#ext_address=8.9.10.11
# External IPv6 address
#ext_address6=aaaa:bbbb:deee:96ff:feee:3fff
#
# If not set - used listen_port_tcp for node table auto fill
#ext_port_tcp=8089

[notify_server]
# Listening path have priority above listening address 
#listen_path={PREFIX}/var/run/node_notify
#listen_path_mode=600
listen_address={NOTIFY_SRV_ADDR}
listen_port={NOTIFY_SRV_PORT}

[stream]
# For now its IAES but thats depricated
#preferred_encryption=SALSA2012 
# Debug stream protocol
#debug=true

# Build in DNS client (need for bootstraping)
[dns_client]
#request_timeout=10

# Builtin DNS server
[dns_server]
#enabled=false
#bootstrap_balancer=false

# Ledger defaults
[ledger]
# More debug output
# debug_more=true
# Cache is enabled by default but always off (config ignored) with MASTER or ROOT node roles
# cache_enabled=false

# DAG defaults
[dag]
# More debug output
# debug_more=true

[srv]
order_signed_only=false

[srv_dns]
enabled=false
pricelist=[]

# Mempool
[mempool]
# Automaticaly false, for enabling need role master or higher
auto_proc=false

# Chain network settings
[chain_net]
# debug_more=true

[stream_ch_chain]
# Uncomment to have more debug information in stream channel Chain
# False by default
#debug_more=true
ban_list_sync_groups=[*.orders-test-stat]

# Number of hashes packed into the one update packet
# Increase it to reduce update latency
# Decrease if bad networking
# update_pack_size=100

# VPN stream channel processing module
[srv_vpn]
#   Turn to true if you want to share VPN service from you node 
enabled=false
debug_more=false
# Grace period for service , 60 second by default
#grace_period=60 
#   List of loca security access groups. Built in: expats,admins,services,nobody,everybody
network_address=10.11.12.0
network_mask=255.255.255.0
pricelist=[scorpion:1:CELL:3600:SEC:mywallet0]

# Console interface server
[conserver]
enabled=true
#listen_port_tcp=12345
listen_unix_socket_path={PREFIX}/var/run/node_cli
# Default permissions 770
# IMPORTANT! Its accessible for all the users in system!
listen_unix_socket_permissions=777

# Application Resources
[resources]
#   0 means auto detect
threads_cnt=0 

pid_path={PREFIX}/var/run/cellframe-node.pid
log_file={PREFIX}/var/log/cellframe-node.log
wallets_path={PREFIX}/var/lib/wallet
ca_folders=[{PREFIX}/var/lib/ca,{PREFIX}/share/ca]
dap_global_db_path={PREFIX}/var/lib/global_db
dap_chains_path={PREFIX}/var/lib/network
#global_db_driver=mdbx
#global_db_drvmode_async=false

[Diagnostic]
enabled=true
#name=

# Plugins
#[plugins]
# Load Python-based plugins
#py_load=false   
# Path to Pyhon-based plugins
#py_path={PREFIX}/var/lib/plugins
