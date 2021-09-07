/* © 2014 Silicon Laboratories Inc.
 */

#ifndef IPV46_NAT_H_
#define IPV46_NAT_H_

 /**
  * \defgroup ip46nat IPv4 to IPv6 network address translator.
  * This module maps IPv4 UDP and ICMP ping packages into their IPv6 equivalents and
  * vice versa. This is done to allow the internal IPv6 to process the IPv4
  * frames as well.
  *
  * @{
  */

/**
 * Contains information about each node
 */
typedef struct nat_table_entry {
  /** Node id which the nat table entry points to */
  u8_t nodeid;
  /** The last bits of the ipv4 address if the nat entry.
   * This is combined with the ipv4 net to form the full ipv4 address.
   */
  u16_t ip_suffix;
} nat_table_entry_t;

/**
 * Maximum number of entries in the NAT table
 */
#define MAX_NAT_ENTRIES 232

/**
 * Actual number of entries in the NAT table
 */
extern u8_t nat_table_size ;

/**
 * The NAT entry database
 */
extern nat_table_entry_t nat_table[MAX_NAT_ENTRIES];


/**
 * Generic definition of an IPv6 address
 */
typedef union ip6addr {
  u8_t  u8[16];     /* Initializer, must come first!!! */
  u16_t u16[8];
} ip6addr_t ;

/**
 * Generic definition of an IPv4 address
 */
typedef union ip4addr_t {
  u8_t  u8[4];			/* Initializer, must come first!!! */
  u16_t u16[2];
} ipv4addr_t;

typedef ipv4addr_t uip_ipv4addr_t;

/**
 * Input of the NAT interface driver. if the destination of the IP address is a NAT address, 
 * this will translate the ip4 package in  uip_buf to a ipv6 package, 
 * If the address is not a NAT'ed address this function will not change the uip_buf.
 *
 * If the destination address is the the uip_hostaddr and the destination
 * UDP port is the Z-Wave port, translation will be performed. Otherwise it will not.
 */
void ipv46nat_interface_input()CC_REENTRANT_ARG;
/**
 * Translate the the package in uip_buf from a IPv6 package to a IPv4 package if the package
 * in uip_buf is a mapped IPv4 package.
 *
 * This functions updates uip_buf and uip_len with the translated IPv4 package.
  */
u8_t ipv46nat_interface_output()CC_REENTRANT_ARG;

/**
 * Add NAT table entry.  Start DHCP on the new node.
 *
 * \return 0 if the add entry fails
 */
u8_t ipv46nat_add_entry(u8_t node);


/**
 * Remove a NAT table entry, return 1 if the entry was removed.
 */
u8_t ipv46nat_del_entry(u8_t node);

/**
 * Remove and send out DHCP RELEASE for all nat table entries except the ZIPGW.
 *
 * This is called when the pan side of the GW leaves one network, eg,
 * in order to join another one.  In the new network, the nodes will
 * have new ids, so they should also get new IPv4 addresses.  The GW
 * itself does not change DHCP client ID, so it does not need a new
 * DHCP lease.
 * This includes gateway reset and learn mode.
 */
void ipv46nat_del_all_nodes();

/**
 * Initialize the IPv4to6 translator
 */
void ipv46nat_init();

/**
 * Get the IPv4 address of a nat table entry.
 * @param ip destination to write to
 * @param e Entry to query
 */
void ipv46nat_ipv4addr_of_entry(uip_ipv4addr_t* ip,nat_table_entry_t *e);


/**
 * Get the node id behind the ipv4 address
 * @param ip IPv4 address
 * @return nodeid
 */
u8_t ipv46_get_nat_addr(uip_ipv4addr_t *ip);
/**
 * Return true if this is an IPv4 mapeed IPv6 address
 */
u8_t is_4to6_addr(ip6addr_t* ip);
/**
 * Get the IPv4 address of a node
 * @param ip destination to write the ip to
 * @param node Id of the node to query for
 * @return TRUE if address was found
 */
u8_t ipv46nat_ipv4addr_of_node(uip_ipv4addr_t* ip,u8_t node);

/**
 * Return true if all nodes has an ip
 */
u8_t ipv46nat_all_nodes_has_ip();


/**
 * Rename a node in the nat table to a new ID
 */
void ipv6nat_rename_node(uint8_t old_id, uint8_t new_id) ;

/**
 * @}
 */

#endif /* IPV46_NAT_H_ */
