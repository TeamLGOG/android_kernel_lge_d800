/*
                                                   
  
                                       
  
                                                                 
                                                                 
   
             
                                                            
                                                        
                                                             
                                                              
                                                             
                                                                  
                              
   
                                                                 
                                                                 
 */

#ifndef _LINUX_SYSCTL_H
#define _LINUX_SYSCTL_H

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/compiler.h>

struct completion;

#define CTL_MAXNAME 10		/*                                          
                                                
                                                
                                                 */

struct __sysctl_args {
	int __user *name;
	int nlen;
	void __user *oldval;
	size_t __user *oldlenp;
	void __user *newval;
	size_t newlen;
	unsigned long __unused[4];
};

/*                           */

/*                  */

enum
{
	CTL_KERN=1,		/*                                 */
	CTL_VM=2,		/*               */
	CTL_NET=3,		/*            */
	CTL_PROC=4,		/*                                      */
	CTL_FS=5,		/*             */
	CTL_DEBUG=6,		/*           */
	CTL_DEV=7,		/*         */
	CTL_BUS=8,		/*        */
	CTL_ABI=9,		/*                  */
	CTL_CPU=10,		/*                                */
	CTL_ARLAN=254,		/*                       */
	CTL_S390DBF=5677,	/*            */
	CTL_SUNRPC=7249,	/*              */
	CTL_PM=9899,		/*                      */
	CTL_FRV=9898,		/*                      */
};

/*                */
enum
{
	CTL_BUS_ISA=1		/*     */
};

/*                       */
enum
{
	INOTIFY_MAX_USER_INSTANCES=1,	/*                        */
	INOTIFY_MAX_USER_WATCHES=2,	/*                      */
	INOTIFY_MAX_QUEUED_EVENTS=3	/*                                */
};

/*                 */
enum
{
	KERN_OSTYPE=1,		/*                        */
	KERN_OSRELEASE=2,	/*                        */
	KERN_OSREV=3,		/*                      */
	KERN_VERSION=4,		/*                           */
	KERN_SECUREMASK=5,	/*                             */
	KERN_PROF=6,		/*                              */
	KERN_NODENAME=7,	/*                  */
	KERN_DOMAINNAME=8,	/*                    */

	KERN_PANIC=15,		/*                    */
	KERN_REALROOTDEV=16,	/*                                        */

	KERN_SPARC_REBOOT=21,	/*                         */
	KERN_CTLALTDEL=22,	/*                                  */
	KERN_PRINTK=23,		/*                                           */
	KERN_NAMETRANS=24,	/*                  */
	KERN_PPC_HTABRECLAIM=25, /*                                      */
	KERN_PPC_ZEROPAGED=26,	/*                                      */
	KERN_PPC_POWERSAVE_NAP=27, /*                               */
	KERN_MODPROBE=28,	/*                       */
	KERN_SG_BIG_BUFF=29,	/*                                     */
	KERN_ACCT=30,		/* BSD process accounting parameters */
	KERN_PPC_L2CR=31,	/*                      */

	KERN_RTSIGNR=32,	/*                          */
	KERN_RTSIGMAX=33,	/*              */
	
	KERN_SHMMAX=34,         /*                                     */
	KERN_MSGMAX=35,         /*                                */
	KERN_MSGMNB=36,         /*                                 */
	KERN_MSGPOOL=37,        /*                                       */
	KERN_SYSRQ=38,		/*                    */
	KERN_MAX_THREADS=39,	/*                                          */
 	KERN_RANDOM=40,		/*               */
 	KERN_SHMALL=41,		/*                                    */
 	KERN_MSGMNI=42,		/*                            */
 	KERN_SEM=43,		/*                               */
 	KERN_SPARC_STOP_A=44,	/*                          */
 	KERN_SHMMNI=45,		/*                            */
	KERN_OVERFLOWUID=46,	/*                   */
	KERN_OVERFLOWGID=47,	/*                   */
	KERN_SHMPATH=48,	/*                        */
	KERN_HOTPLUG=49,	/*                                            */
	KERN_IEEE_EMULATION_WARNINGS=50, /*                                      */
	KERN_S390_USER_DEBUG_LOGGING=51,  /*                           */
	KERN_CORE_USES_PID=52,		/*                            */
	KERN_TAINTED=53,	/*                                   */
	KERN_CADPID=54,		/*                                          */
	KERN_PIDMAX=55,		/*                  */
  	KERN_CORE_PATTERN=56,	/*                                     */
	KERN_PANIC_ON_OOPS=57,  /*                                       */
	KERN_HPPA_PWRSW=58,	/*                             */
	KERN_HPPA_UNALIGNED=59,	/*                                 */
	KERN_PRINTK_RATELIMIT=60, /*                               */
	KERN_PRINTK_RATELIMIT_BURST=61,	/*                               */
	KERN_PTY=62,		/*                 */
	KERN_NGROUPS_MAX=63,	/*                  */
	KERN_SPARC_SCONS_PWROFF=64, /*                                    */
	KERN_HZ_TIMER=65,	/*                         */
	KERN_UNKNOWN_NMI_PANIC=66, /*                             */
	KERN_BOOTLOADER_TYPE=67, /*                       */
	KERN_RANDOMIZE=68, /*                                      */
	KERN_SETUID_DUMPABLE=69, /*                                         */
	KERN_SPIN_RETRY=70,	/*                                 */
	KERN_ACPI_VIDEO_FLAGS=71, /*                                                  */
	KERN_IA64_UNALIGNED=72, /*                                          */
	KERN_COMPAT_LOG=73,	/*                                   */
	KERN_MAX_LOCK_DEPTH=74, /*                                   */
	KERN_NMI_WATCHDOG=75, /*                                  */
	KERN_PANIC_ON_NMI=76, /*                                              */
	KERN_BOOT_REASON = 77, /*                                        */
};



/*               */
enum
{
	VM_UNUSED1=1,		/*                                      */
	VM_UNUSED2=2,		/*                                             */
	VM_UNUSED3=3,		/*                                       */
	VM_UNUSED4=4,		/*       */
	VM_OVERCOMMIT_MEMORY=5,	/*                                          */
	VM_UNUSED5=6,		/*                                           */
	VM_UNUSED7=7,		/*                                          */
	VM_UNUSED8=8,		/*                                       */
	VM_UNUSED9=9,		/*                                              */
	VM_PAGE_CLUSTER=10,	/*                                           */
	VM_DIRTY_BACKGROUND=11,	/*                        */
	VM_DIRTY_RATIO=12,	/*             */
	VM_DIRTY_WB_CS=13,	/*                           */
	VM_DIRTY_EXPIRE_CS=14,	/*                        */
	VM_NR_PDFLUSH_THREADS=15, /*                    */
	VM_OVERCOMMIT_RATIO=16, /*                                       */
	VM_PAGEBUF=17,		/*                                    */
	VM_HUGETLB_PAGES=18,	/*                                     */
	VM_SWAPPINESS=19,	/*                                 */
	VM_LOWMEM_RESERVE_RATIO=20,/*                                          */
	VM_MIN_FREE_KBYTES=21,	/*                                    */
	VM_MAX_MAP_COUNT=22,	/*                                            */
	VM_LAPTOP_MODE=23,	/*                */
	VM_BLOCK_DUMP=24,	/*                 */
	VM_HUGETLB_GROUP=25,	/*                         */
	VM_VFS_CACHE_PRESSURE=26, /*                                */
	VM_LEGACY_VA_LAYOUT=27, /*                                                   */
	VM_SWAP_TOKEN_TIMEOUT=28, /*                                 */
	VM_DROP_PAGECACHE=29,	/*                             */
	VM_PERCPU_PAGELIST_FRACTION=30,/*                                                */
	VM_ZONE_RECLAIM_MODE=31, /*                                                 */
	VM_MIN_UNMAPPED=32,	/*                                   */
	VM_PANIC_ON_OOM=33,	/*                        */
	VM_VDSO_ENABLED=34,	/*                              */
	VM_MIN_SLAB=35,		 /*                                       */
};


/*                */
enum
{
	NET_CORE=1,
	NET_ETHER=2,
	NET_802=3,
	NET_UNIX=4,
	NET_IPV4=5,
	NET_IPX=6,
	NET_ATALK=7,
	NET_NETROM=8,
	NET_AX25=9,
	NET_BRIDGE=10,
	NET_ROSE=11,
	NET_IPV6=12,
	NET_X25=13,
	NET_TR=14,
	NET_DECNET=15,
	NET_ECONET=16,
	NET_SCTP=17,
	NET_LLC=18,
	NET_NETFILTER=19,
	NET_DCCP=20,
	NET_IRDA=412,
};

/*                         */
enum
{
	RANDOM_POOLSIZE=1,
	RANDOM_ENTROPY_COUNT=2,
	RANDOM_READ_THRESH=3,
	RANDOM_WRITE_THRESH=4,
	RANDOM_BOOT_ID=5,
	RANDOM_UUID=6
};

/*                      */
enum
{
	PTY_MAX=1,
	PTY_NR=2
};

/*                   */
enum
{
	BUS_ISA_MEM_BASE=1,
	BUS_ISA_PORT_BASE=2,
	BUS_ISA_PORT_SHIFT=3
};

/*                    */
enum
{
	NET_CORE_WMEM_MAX=1,
	NET_CORE_RMEM_MAX=2,
	NET_CORE_WMEM_DEFAULT=3,
	NET_CORE_RMEM_DEFAULT=4,
/*                            */
	NET_CORE_MAX_BACKLOG=6,
	NET_CORE_FASTROUTE=7,
	NET_CORE_MSG_COST=8,
	NET_CORE_MSG_BURST=9,
	NET_CORE_OPTMEM_MAX=10,
	NET_CORE_HOT_LIST_LENGTH=11,
	NET_CORE_DIVERT_VERSION=12,
	NET_CORE_NO_CONG_THRESH=13,
	NET_CORE_NO_CONG=14,
	NET_CORE_LO_CONG=15,
	NET_CORE_MOD_CONG=16,
	NET_CORE_DEV_WEIGHT=17,
	NET_CORE_SOMAXCONN=18,
	NET_CORE_BUDGET=19,
	NET_CORE_AEVENT_ETIME=20,
	NET_CORE_AEVENT_RSEQTH=21,
	NET_CORE_WARNINGS=22,
};

/*                        */

/*                   */

/*                    */

enum
{
	NET_UNIX_DESTROY_DELAY=1,
	NET_UNIX_DELETE_DELAY=2,
	NET_UNIX_MAX_DGRAM_QLEN=3,
};

/*                         */
enum
{
	NET_NF_CONNTRACK_MAX=1,
	NET_NF_CONNTRACK_TCP_TIMEOUT_SYN_SENT=2,
	NET_NF_CONNTRACK_TCP_TIMEOUT_SYN_RECV=3,
	NET_NF_CONNTRACK_TCP_TIMEOUT_ESTABLISHED=4,
	NET_NF_CONNTRACK_TCP_TIMEOUT_FIN_WAIT=5,
	NET_NF_CONNTRACK_TCP_TIMEOUT_CLOSE_WAIT=6,
	NET_NF_CONNTRACK_TCP_TIMEOUT_LAST_ACK=7,
	NET_NF_CONNTRACK_TCP_TIMEOUT_TIME_WAIT=8,
	NET_NF_CONNTRACK_TCP_TIMEOUT_CLOSE=9,
	NET_NF_CONNTRACK_UDP_TIMEOUT=10,
	NET_NF_CONNTRACK_UDP_TIMEOUT_STREAM=11,
	NET_NF_CONNTRACK_ICMP_TIMEOUT=12,
	NET_NF_CONNTRACK_GENERIC_TIMEOUT=13,
	NET_NF_CONNTRACK_BUCKETS=14,
	NET_NF_CONNTRACK_LOG_INVALID=15,
	NET_NF_CONNTRACK_TCP_TIMEOUT_MAX_RETRANS=16,
	NET_NF_CONNTRACK_TCP_LOOSE=17,
	NET_NF_CONNTRACK_TCP_BE_LIBERAL=18,
	NET_NF_CONNTRACK_TCP_MAX_RETRANS=19,
	NET_NF_CONNTRACK_SCTP_TIMEOUT_CLOSED=20,
	NET_NF_CONNTRACK_SCTP_TIMEOUT_COOKIE_WAIT=21,
	NET_NF_CONNTRACK_SCTP_TIMEOUT_COOKIE_ECHOED=22,
	NET_NF_CONNTRACK_SCTP_TIMEOUT_ESTABLISHED=23,
	NET_NF_CONNTRACK_SCTP_TIMEOUT_SHUTDOWN_SENT=24,
	NET_NF_CONNTRACK_SCTP_TIMEOUT_SHUTDOWN_RECD=25,
	NET_NF_CONNTRACK_SCTP_TIMEOUT_SHUTDOWN_ACK_SENT=26,
	NET_NF_CONNTRACK_COUNT=27,
	NET_NF_CONNTRACK_ICMPV6_TIMEOUT=28,
	NET_NF_CONNTRACK_FRAG6_TIMEOUT=29,
	NET_NF_CONNTRACK_FRAG6_LOW_THRESH=30,
	NET_NF_CONNTRACK_FRAG6_HIGH_THRESH=31,
	NET_NF_CONNTRACK_CHECKSUM=32,
};

/*                    */
enum
{
	/*                            */
	NET_IPV4_FORWARD=8,
	NET_IPV4_DYNADDR=9,

	NET_IPV4_CONF=16,
	NET_IPV4_NEIGH=17,
	NET_IPV4_ROUTE=18,
	NET_IPV4_FIB_HASH=19,
	NET_IPV4_NETFILTER=20,

	NET_IPV4_TCP_TIMESTAMPS=33,
	NET_IPV4_TCP_WINDOW_SCALING=34,
	NET_IPV4_TCP_SACK=35,
	NET_IPV4_TCP_RETRANS_COLLAPSE=36,
	NET_IPV4_DEFAULT_TTL=37,
	NET_IPV4_AUTOCONFIG=38,
	NET_IPV4_NO_PMTU_DISC=39,
	NET_IPV4_TCP_SYN_RETRIES=40,
	NET_IPV4_IPFRAG_HIGH_THRESH=41,
	NET_IPV4_IPFRAG_LOW_THRESH=42,
	NET_IPV4_IPFRAG_TIME=43,
	NET_IPV4_TCP_MAX_KA_PROBES=44,
	NET_IPV4_TCP_KEEPALIVE_TIME=45,
	NET_IPV4_TCP_KEEPALIVE_PROBES=46,
	NET_IPV4_TCP_RETRIES1=47,
	NET_IPV4_TCP_RETRIES2=48,
	NET_IPV4_TCP_FIN_TIMEOUT=49,
	NET_IPV4_IP_MASQ_DEBUG=50,
	NET_TCP_SYNCOOKIES=51,
	NET_TCP_STDURG=52,
	NET_TCP_RFC1337=53,
	NET_TCP_SYN_TAILDROP=54,
	NET_TCP_MAX_SYN_BACKLOG=55,
	NET_IPV4_LOCAL_PORT_RANGE=56,
	NET_IPV4_ICMP_ECHO_IGNORE_ALL=57,
	NET_IPV4_ICMP_ECHO_IGNORE_BROADCASTS=58,
	NET_IPV4_ICMP_SOURCEQUENCH_RATE=59,
	NET_IPV4_ICMP_DESTUNREACH_RATE=60,
	NET_IPV4_ICMP_TIMEEXCEED_RATE=61,
	NET_IPV4_ICMP_PARAMPROB_RATE=62,
	NET_IPV4_ICMP_ECHOREPLY_RATE=63,
	NET_IPV4_ICMP_IGNORE_BOGUS_ERROR_RESPONSES=64,
	NET_IPV4_IGMP_MAX_MEMBERSHIPS=65,
	NET_TCP_TW_RECYCLE=66,
	NET_IPV4_ALWAYS_DEFRAG=67,
	NET_IPV4_TCP_KEEPALIVE_INTVL=68,
	NET_IPV4_INET_PEER_THRESHOLD=69,
	NET_IPV4_INET_PEER_MINTTL=70,
	NET_IPV4_INET_PEER_MAXTTL=71,
	NET_IPV4_INET_PEER_GC_MINTIME=72,
	NET_IPV4_INET_PEER_GC_MAXTIME=73,
	NET_TCP_ORPHAN_RETRIES=74,
	NET_TCP_ABORT_ON_OVERFLOW=75,
	NET_TCP_SYNACK_RETRIES=76,
	NET_TCP_MAX_ORPHANS=77,
	NET_TCP_MAX_TW_BUCKETS=78,
	NET_TCP_FACK=79,
	NET_TCP_REORDERING=80,
	NET_TCP_ECN=81,
	NET_TCP_DSACK=82,
	NET_TCP_MEM=83,
	NET_TCP_WMEM=84,
	NET_TCP_RMEM=85,
	NET_TCP_APP_WIN=86,
	NET_TCP_ADV_WIN_SCALE=87,
	NET_IPV4_NONLOCAL_BIND=88,
	NET_IPV4_ICMP_RATELIMIT=89,
	NET_IPV4_ICMP_RATEMASK=90,
	NET_TCP_TW_REUSE=91,
	NET_TCP_FRTO=92,
	NET_TCP_LOW_LATENCY=93,
	NET_IPV4_IPFRAG_SECRET_INTERVAL=94,
	NET_IPV4_IGMP_MAX_MSF=96,
	NET_TCP_NO_METRICS_SAVE=97,
	NET_TCP_DEFAULT_WIN_SCALE=105,
	NET_TCP_MODERATE_RCVBUF=106,
	NET_TCP_TSO_WIN_DIVISOR=107,
	NET_TCP_BIC_BETA=108,
	NET_IPV4_ICMP_ERRORS_USE_INBOUND_IFADDR=109,
	NET_TCP_CONG_CONTROL=110,
	NET_TCP_ABC=111,
	NET_IPV4_IPFRAG_MAX_DIST=112,
 	NET_TCP_MTU_PROBING=113,
	NET_TCP_BASE_MSS=114,
	NET_IPV4_TCP_WORKAROUND_SIGNED_WINDOWS=115,
	NET_TCP_DMA_COPYBREAK=116,
	NET_TCP_SLOW_START_AFTER_IDLE=117,
	NET_CIPSOV4_CACHE_ENABLE=118,
	NET_CIPSOV4_CACHE_BUCKET_SIZE=119,
	NET_CIPSOV4_RBM_OPTFMT=120,
	NET_CIPSOV4_RBM_STRICTVALID=121,
	NET_TCP_AVAIL_CONG_CONTROL=122,
	NET_TCP_ALLOWED_CONG_CONTROL=123,
	NET_TCP_MAX_SSTHRESH=124,
	NET_TCP_FRTO_RESPONSE=125,
};

enum {
	NET_IPV4_ROUTE_FLUSH=1,
	NET_IPV4_ROUTE_MIN_DELAY=2, /*                       */
	NET_IPV4_ROUTE_MAX_DELAY=3, /*                       */
	NET_IPV4_ROUTE_GC_THRESH=4,
	NET_IPV4_ROUTE_MAX_SIZE=5,
	NET_IPV4_ROUTE_GC_MIN_INTERVAL=6,
	NET_IPV4_ROUTE_GC_TIMEOUT=7,
	NET_IPV4_ROUTE_GC_INTERVAL=8, /*                       */
	NET_IPV4_ROUTE_REDIRECT_LOAD=9,
	NET_IPV4_ROUTE_REDIRECT_NUMBER=10,
	NET_IPV4_ROUTE_REDIRECT_SILENCE=11,
	NET_IPV4_ROUTE_ERROR_COST=12,
	NET_IPV4_ROUTE_ERROR_BURST=13,
	NET_IPV4_ROUTE_GC_ELASTICITY=14,
	NET_IPV4_ROUTE_MTU_EXPIRES=15,
	NET_IPV4_ROUTE_MIN_PMTU=16,
	NET_IPV4_ROUTE_MIN_ADVMSS=17,
	NET_IPV4_ROUTE_SECRET_INTERVAL=18,
	NET_IPV4_ROUTE_GC_MIN_INTERVAL_MS=19,
};

enum
{
	NET_PROTO_CONF_ALL=-2,
	NET_PROTO_CONF_DEFAULT=-3

	/*                          */
};

enum
{
	NET_IPV4_CONF_FORWARDING=1,
	NET_IPV4_CONF_MC_FORWARDING=2,
	NET_IPV4_CONF_PROXY_ARP=3,
	NET_IPV4_CONF_ACCEPT_REDIRECTS=4,
	NET_IPV4_CONF_SECURE_REDIRECTS=5,
	NET_IPV4_CONF_SEND_REDIRECTS=6,
	NET_IPV4_CONF_SHARED_MEDIA=7,
	NET_IPV4_CONF_RP_FILTER=8,
	NET_IPV4_CONF_ACCEPT_SOURCE_ROUTE=9,
	NET_IPV4_CONF_BOOTP_RELAY=10,
	NET_IPV4_CONF_LOG_MARTIANS=11,
	NET_IPV4_CONF_TAG=12,
	NET_IPV4_CONF_ARPFILTER=13,
	NET_IPV4_CONF_MEDIUM_ID=14,
	NET_IPV4_CONF_NOXFRM=15,
	NET_IPV4_CONF_NOPOLICY=16,
	NET_IPV4_CONF_FORCE_IGMP_VERSION=17,
	NET_IPV4_CONF_ARP_ANNOUNCE=18,
	NET_IPV4_CONF_ARP_IGNORE=19,
	NET_IPV4_CONF_PROMOTE_SECONDARIES=20,
	NET_IPV4_CONF_ARP_ACCEPT=21,
	NET_IPV4_CONF_ARP_NOTIFY=22,
};

/*                              */
enum
{
	NET_IPV4_NF_CONNTRACK_MAX=1,
	NET_IPV4_NF_CONNTRACK_TCP_TIMEOUT_SYN_SENT=2,
	NET_IPV4_NF_CONNTRACK_TCP_TIMEOUT_SYN_RECV=3,
	NET_IPV4_NF_CONNTRACK_TCP_TIMEOUT_ESTABLISHED=4,
	NET_IPV4_NF_CONNTRACK_TCP_TIMEOUT_FIN_WAIT=5,
	NET_IPV4_NF_CONNTRACK_TCP_TIMEOUT_CLOSE_WAIT=6,
	NET_IPV4_NF_CONNTRACK_TCP_TIMEOUT_LAST_ACK=7,
	NET_IPV4_NF_CONNTRACK_TCP_TIMEOUT_TIME_WAIT=8,
	NET_IPV4_NF_CONNTRACK_TCP_TIMEOUT_CLOSE=9,
	NET_IPV4_NF_CONNTRACK_UDP_TIMEOUT=10,
	NET_IPV4_NF_CONNTRACK_UDP_TIMEOUT_STREAM=11,
	NET_IPV4_NF_CONNTRACK_ICMP_TIMEOUT=12,
	NET_IPV4_NF_CONNTRACK_GENERIC_TIMEOUT=13,
	NET_IPV4_NF_CONNTRACK_BUCKETS=14,
	NET_IPV4_NF_CONNTRACK_LOG_INVALID=15,
	NET_IPV4_NF_CONNTRACK_TCP_TIMEOUT_MAX_RETRANS=16,
	NET_IPV4_NF_CONNTRACK_TCP_LOOSE=17,
	NET_IPV4_NF_CONNTRACK_TCP_BE_LIBERAL=18,
	NET_IPV4_NF_CONNTRACK_TCP_MAX_RETRANS=19,
 	NET_IPV4_NF_CONNTRACK_SCTP_TIMEOUT_CLOSED=20,
 	NET_IPV4_NF_CONNTRACK_SCTP_TIMEOUT_COOKIE_WAIT=21,
 	NET_IPV4_NF_CONNTRACK_SCTP_TIMEOUT_COOKIE_ECHOED=22,
 	NET_IPV4_NF_CONNTRACK_SCTP_TIMEOUT_ESTABLISHED=23,
 	NET_IPV4_NF_CONNTRACK_SCTP_TIMEOUT_SHUTDOWN_SENT=24,
 	NET_IPV4_NF_CONNTRACK_SCTP_TIMEOUT_SHUTDOWN_RECD=25,
 	NET_IPV4_NF_CONNTRACK_SCTP_TIMEOUT_SHUTDOWN_ACK_SENT=26,
	NET_IPV4_NF_CONNTRACK_COUNT=27,
	NET_IPV4_NF_CONNTRACK_CHECKSUM=28,
};
 
/*                    */
enum {
	NET_IPV6_CONF=16,
	NET_IPV6_NEIGH=17,
	NET_IPV6_ROUTE=18,
	NET_IPV6_ICMP=19,
	NET_IPV6_BINDV6ONLY=20,
	NET_IPV6_IP6FRAG_HIGH_THRESH=21,
	NET_IPV6_IP6FRAG_LOW_THRESH=22,
	NET_IPV6_IP6FRAG_TIME=23,
	NET_IPV6_IP6FRAG_SECRET_INTERVAL=24,
	NET_IPV6_MLD_MAX_MSF=25,
};

enum {
	NET_IPV6_ROUTE_FLUSH=1,
	NET_IPV6_ROUTE_GC_THRESH=2,
	NET_IPV6_ROUTE_MAX_SIZE=3,
	NET_IPV6_ROUTE_GC_MIN_INTERVAL=4,
	NET_IPV6_ROUTE_GC_TIMEOUT=5,
	NET_IPV6_ROUTE_GC_INTERVAL=6,
	NET_IPV6_ROUTE_GC_ELASTICITY=7,
	NET_IPV6_ROUTE_MTU_EXPIRES=8,
	NET_IPV6_ROUTE_MIN_ADVMSS=9,
	NET_IPV6_ROUTE_GC_MIN_INTERVAL_MS=10
};

enum {
	NET_IPV6_FORWARDING=1,
	NET_IPV6_HOP_LIMIT=2,
	NET_IPV6_MTU=3,
	NET_IPV6_ACCEPT_RA=4,
	NET_IPV6_ACCEPT_REDIRECTS=5,
	NET_IPV6_AUTOCONF=6,
	NET_IPV6_DAD_TRANSMITS=7,
	NET_IPV6_RTR_SOLICITS=8,
	NET_IPV6_RTR_SOLICIT_INTERVAL=9,
	NET_IPV6_RTR_SOLICIT_DELAY=10,
	NET_IPV6_USE_TEMPADDR=11,
	NET_IPV6_TEMP_VALID_LFT=12,
	NET_IPV6_TEMP_PREFERED_LFT=13,
	NET_IPV6_REGEN_MAX_RETRY=14,
	NET_IPV6_MAX_DESYNC_FACTOR=15,
	NET_IPV6_MAX_ADDRESSES=16,
	NET_IPV6_FORCE_MLD_VERSION=17,
	NET_IPV6_ACCEPT_RA_DEFRTR=18,
	NET_IPV6_ACCEPT_RA_PINFO=19,
	NET_IPV6_ACCEPT_RA_RTR_PREF=20,
	NET_IPV6_RTR_PROBE_INTERVAL=21,
	NET_IPV6_ACCEPT_RA_RT_INFO_MAX_PLEN=22,
	NET_IPV6_PROXY_NDP=23,
	NET_IPV6_ACCEPT_SOURCE_ROUTE=25,
	__NET_IPV6_MAX
};

/*                         */
enum {
	NET_IPV6_ICMP_RATELIMIT=1
};

/*                                      */
enum {
	NET_NEIGH_MCAST_SOLICIT=1,
	NET_NEIGH_UCAST_SOLICIT=2,
	NET_NEIGH_APP_SOLICIT=3,
	NET_NEIGH_RETRANS_TIME=4,
	NET_NEIGH_REACHABLE_TIME=5,
	NET_NEIGH_DELAY_PROBE_TIME=6,
	NET_NEIGH_GC_STALE_TIME=7,
	NET_NEIGH_UNRES_QLEN=8,
	NET_NEIGH_PROXY_QLEN=9,
	NET_NEIGH_ANYCAST_DELAY=10,
	NET_NEIGH_PROXY_DELAY=11,
	NET_NEIGH_LOCKTIME=12,
	NET_NEIGH_GC_INTERVAL=13,
	NET_NEIGH_GC_THRESH1=14,
	NET_NEIGH_GC_THRESH2=15,
	NET_NEIGH_GC_THRESH3=16,
	NET_NEIGH_RETRANS_TIME_MS=17,
	NET_NEIGH_REACHABLE_TIME_MS=18,
};

/*                    */
enum {
	NET_DCCP_DEFAULT=1,
};

/*                   */
enum {
	NET_IPX_PPROP_BROADCASTING=1,
	NET_IPX_FORWARDING=2
};

/*                   */
enum {
	NET_LLC2=1,
	NET_LLC_STATION=2,
};

/*                        */
enum {
	NET_LLC2_TIMEOUT=1,
};

/*                           */
enum {
	NET_LLC_STATION_ACK_TIMEOUT=1,
};

/*                                */
enum {
	NET_LLC2_ACK_TIMEOUT=1,
	NET_LLC2_P_TIMEOUT=2,
	NET_LLC2_REJ_TIMEOUT=3,
	NET_LLC2_BUSY_TIMEOUT=4,
};

/*                         */
enum {
	NET_ATALK_AARP_EXPIRY_TIME=1,
	NET_ATALK_AARP_TICK_TIME=2,
	NET_ATALK_AARP_RETRANSMIT_LIMIT=3,
	NET_ATALK_AARP_RESOLVE_TIME=4
};


/*                      */
enum {
	NET_NETROM_DEFAULT_PATH_QUALITY=1,
	NET_NETROM_OBSOLESCENCE_COUNT_INITIALISER=2,
	NET_NETROM_NETWORK_TTL_INITIALISER=3,
	NET_NETROM_TRANSPORT_TIMEOUT=4,
	NET_NETROM_TRANSPORT_MAXIMUM_TRIES=5,
	NET_NETROM_TRANSPORT_ACKNOWLEDGE_DELAY=6,
	NET_NETROM_TRANSPORT_BUSY_DELAY=7,
	NET_NETROM_TRANSPORT_REQUESTED_WINDOW_SIZE=8,
	NET_NETROM_TRANSPORT_NO_ACTIVITY_TIMEOUT=9,
	NET_NETROM_ROUTING_CONTROL=10,
	NET_NETROM_LINK_FAILS_COUNT=11,
	NET_NETROM_RESET=12
};

/*                    */
enum {
	NET_AX25_IP_DEFAULT_MODE=1,
	NET_AX25_DEFAULT_MODE=2,
	NET_AX25_BACKOFF_TYPE=3,
	NET_AX25_CONNECT_MODE=4,
	NET_AX25_STANDARD_WINDOW=5,
	NET_AX25_EXTENDED_WINDOW=6,
	NET_AX25_T1_TIMEOUT=7,
	NET_AX25_T2_TIMEOUT=8,
	NET_AX25_T3_TIMEOUT=9,
	NET_AX25_IDLE_TIMEOUT=10,
	NET_AX25_N2=11,
	NET_AX25_PACLEN=12,
	NET_AX25_PROTOCOL=13,
	NET_AX25_DAMA_SLAVE_TIMEOUT=14
};

/*                    */
enum {
	NET_ROSE_RESTART_REQUEST_TIMEOUT=1,
	NET_ROSE_CALL_REQUEST_TIMEOUT=2,
	NET_ROSE_RESET_REQUEST_TIMEOUT=3,
	NET_ROSE_CLEAR_REQUEST_TIMEOUT=4,
	NET_ROSE_ACK_HOLD_BACK_TIMEOUT=5,
	NET_ROSE_ROUTING_CONTROL=6,
	NET_ROSE_LINK_FAIL_TIMEOUT=7,
	NET_ROSE_MAX_VCS=8,
	NET_ROSE_WINDOW_SIZE=9,
	NET_ROSE_NO_ACTIVITY_TIMEOUT=10
};

/*                   */
enum {
	NET_X25_RESTART_REQUEST_TIMEOUT=1,
	NET_X25_CALL_REQUEST_TIMEOUT=2,
	NET_X25_RESET_REQUEST_TIMEOUT=3,
	NET_X25_CLEAR_REQUEST_TIMEOUT=4,
	NET_X25_ACK_HOLD_BACK_TIMEOUT=5,
	NET_X25_FORWARD=6
};

/*                          */
enum
{
	NET_TR_RIF_TIMEOUT=1
};

/*                       */
enum {
	NET_DECNET_NODE_TYPE = 1,
	NET_DECNET_NODE_ADDRESS = 2,
	NET_DECNET_NODE_NAME = 3,
	NET_DECNET_DEFAULT_DEVICE = 4,
	NET_DECNET_TIME_WAIT = 5,
	NET_DECNET_DN_COUNT = 6,
	NET_DECNET_DI_COUNT = 7,
	NET_DECNET_DR_COUNT = 8,
	NET_DECNET_DST_GC_INTERVAL = 9,
	NET_DECNET_CONF = 10,
	NET_DECNET_NO_FC_MAX_CWND = 11,
	NET_DECNET_MEM = 12,
	NET_DECNET_RMEM = 13,
	NET_DECNET_WMEM = 14,
	NET_DECNET_DEBUG_LEVEL = 255
};

/*                                 */
enum {
	NET_DECNET_CONF_LOOPBACK = -2,
	NET_DECNET_CONF_DDCMP = -3,
	NET_DECNET_CONF_PPP = -4,
	NET_DECNET_CONF_X25 = -5,
	NET_DECNET_CONF_GRE = -6,
	NET_DECNET_CONF_ETHER = -7

	/*                            */
};

/*                                  */
enum {
	NET_DECNET_CONF_DEV_PRIORITY = 1,
	NET_DECNET_CONF_DEV_T1 = 2,
	NET_DECNET_CONF_DEV_T2 = 3,
	NET_DECNET_CONF_DEV_T3 = 4,
	NET_DECNET_CONF_DEV_FORWARDING = 5,
	NET_DECNET_CONF_DEV_BLKSIZE = 6,
	NET_DECNET_CONF_DEV_STATE = 7
};

/*                    */
enum {
	NET_SCTP_RTO_INITIAL = 1,
	NET_SCTP_RTO_MIN     = 2,
	NET_SCTP_RTO_MAX     = 3,
	NET_SCTP_RTO_ALPHA   = 4,
	NET_SCTP_RTO_BETA    = 5,
	NET_SCTP_VALID_COOKIE_LIFE       =  6,
	NET_SCTP_ASSOCIATION_MAX_RETRANS =  7,
	NET_SCTP_PATH_MAX_RETRANS        =  8,
	NET_SCTP_MAX_INIT_RETRANSMITS    =  9,
	NET_SCTP_HB_INTERVAL             = 10,
	NET_SCTP_PRESERVE_ENABLE         = 11,
	NET_SCTP_MAX_BURST               = 12,
	NET_SCTP_ADDIP_ENABLE		 = 13,
	NET_SCTP_PRSCTP_ENABLE		 = 14,
	NET_SCTP_SNDBUF_POLICY		 = 15,
	NET_SCTP_SACK_TIMEOUT		 = 16,
	NET_SCTP_RCVBUF_POLICY		 = 17,
};

/*                      */
enum {
	NET_BRIDGE_NF_CALL_ARPTABLES = 1,
	NET_BRIDGE_NF_CALL_IPTABLES = 2,
	NET_BRIDGE_NF_CALL_IP6TABLES = 3,
	NET_BRIDGE_NF_FILTER_VLAN_TAGGED = 4,
	NET_BRIDGE_NF_FILTER_PPPOE_TAGGED = 5,
};

/*                   */
enum {
	NET_IRDA_DISCOVERY=1,
	NET_IRDA_DEVNAME=2,
	NET_IRDA_DEBUG=3,
	NET_IRDA_FAST_POLL=4,
	NET_IRDA_DISCOVERY_SLOTS=5,
	NET_IRDA_DISCOVERY_TIMEOUT=6,
	NET_IRDA_SLOT_TIMEOUT=7,
	NET_IRDA_MAX_BAUD_RATE=8,
	NET_IRDA_MIN_TX_TURN_TIME=9,
	NET_IRDA_MAX_TX_DATA_SIZE=10,
	NET_IRDA_MAX_TX_WINDOW=11,
	NET_IRDA_MAX_NOREPLY_TIME=12,
	NET_IRDA_WARN_NOREPLY_TIME=13,
	NET_IRDA_LAP_KEEPALIVE_TIME=14,
};


/*               */
enum
{
	FS_NRINODE=1,	/*                                        */
	FS_STATINODE=2,
	FS_MAXINODE=3,	/*                                                    */
	FS_NRDQUOT=4,	/*                                        */
	FS_MAXDQUOT=5,	/*                                                    */
	FS_NRFILE=6,	/*                                                 */
	FS_MAXFILE=7,	/*                                                             */
	FS_DENTRY=8,
	FS_NRSUPER=9,	/*                                              */
	FS_MAXSUPER=10,	/*                                                          */
	FS_OVERFLOWUID=11,	/*                   */
	FS_OVERFLOWGID=12,	/*                   */
	FS_LEASES=13,	/*                     */
	FS_DIR_NOTIFY=14,	/*                                     */
	FS_LEASE_TIME=15,	/*                                             */
	FS_DQSTATS=16,	/*                                         */
	FS_XFS=17,	/*                                */
	FS_AIO_NR=18,	/*                                            */
	FS_AIO_MAX_NR=19,	/*                                            */
	FS_INOTIFY=20,	/*                 */
	FS_OCFS2=988,	/*       */
};

/*                     */
enum {
	FS_DQ_LOOKUPS = 1,
	FS_DQ_DROPS = 2,
	FS_DQ_READS = 3,
	FS_DQ_WRITES = 4,
	FS_DQ_CACHE_HITS = 5,
	FS_DQ_ALLOCATED = 6,
	FS_DQ_FREE = 7,
	FS_DQ_SYNCS = 8,
	FS_DQ_WARNINGS = 9,
};

/*                  */

/*                */
enum {
	DEV_CDROM=1,
	DEV_HWMON=2,
	DEV_PARPORT=3,
	DEV_RAID=4,
	DEV_MAC_HID=5,
	DEV_SCSI=6,
	DEV_IPMI=7,
};

/*                     */
enum {
	DEV_CDROM_INFO=1,
	DEV_CDROM_AUTOCLOSE=2,
	DEV_CDROM_AUTOEJECT=3,
	DEV_CDROM_DEBUG=4,
	DEV_CDROM_LOCK=5,
	DEV_CDROM_CHECK_MEDIA=6
};

/*                       */
enum {
	DEV_PARPORT_DEFAULT=-3
};

/*                    */
enum {
	DEV_RAID_SPEED_LIMIT_MIN=1,
	DEV_RAID_SPEED_LIMIT_MAX=2
};

/*                               */
enum {
	DEV_PARPORT_DEFAULT_TIMESLICE=1,
	DEV_PARPORT_DEFAULT_SPINTIME=2
};

/*                                 */
enum {
	DEV_PARPORT_SPINTIME=1,
	DEV_PARPORT_BASE_ADDR=2,
	DEV_PARPORT_IRQ=3,
	DEV_PARPORT_DMA=4,
	DEV_PARPORT_MODES=5,
	DEV_PARPORT_DEVICES=6,
	DEV_PARPORT_AUTOPROBE=16
};

/*                                          */
enum {
	DEV_PARPORT_DEVICES_ACTIVE=-3,
};

/*                                                  */
enum {
	DEV_PARPORT_DEVICE_TIMESLICE=1,
};

/*                       */
enum {
	DEV_MAC_HID_KEYBOARD_SENDS_LINUX_KEYCODES=1,
	DEV_MAC_HID_KEYBOARD_LOCK_KEYCODES=2,
	DEV_MAC_HID_MOUSE_BUTTON_EMULATION=3,
	DEV_MAC_HID_MOUSE_BUTTON2_KEYCODE=4,
	DEV_MAC_HID_MOUSE_BUTTON3_KEYCODE=5,
	DEV_MAC_HID_ADB_MOUSE_SENDS_KEYCODES=6
};

/*                    */
enum {
	DEV_SCSI_LOGGING_LEVEL=1,
};

/*                    */
enum {
	DEV_IPMI_POWEROFF_POWERCYCLE=1,
};

/*               */
enum
{
	ABI_DEFHANDLER_COFF=1,	/*                                   */
	ABI_DEFHANDLER_ELF=2, 	/*                                  */
	ABI_DEFHANDLER_LCALL7=3,/*                                        */
	ABI_DEFHANDLER_LIBCSO=4,/*                                           */
	ABI_TRACE=5,		/*               */
	ABI_FAKE_UTSNAME=6,	/*                                 */
};

#ifdef __KERNEL__
#include <linux/list.h>
#include <linux/rcupdate.h>
#include <linux/wait.h>
#include <linux/rbtree.h>

/*                           */
struct ctl_table;
struct nsproxy;
struct ctl_table_root;
struct ctl_table_header;
struct ctl_dir;

typedef struct ctl_table ctl_table;

typedef int proc_handler (struct ctl_table *ctl, int write,
			  void __user *buffer, size_t *lenp, loff_t *ppos);

extern int proc_dostring(struct ctl_table *, int,
			 void __user *, size_t *, loff_t *);
extern int proc_dointvec(struct ctl_table *, int,
			 void __user *, size_t *, loff_t *);
extern int proc_dointvec_minmax(struct ctl_table *, int,
				void __user *, size_t *, loff_t *);
extern int proc_dointvec_jiffies(struct ctl_table *, int,
				 void __user *, size_t *, loff_t *);
extern int proc_dointvec_userhz_jiffies(struct ctl_table *, int,
					void __user *, size_t *, loff_t *);
extern int proc_dointvec_ms_jiffies(struct ctl_table *, int,
				    void __user *, size_t *, loff_t *);
extern int proc_doulongvec_minmax(struct ctl_table *, int,
				  void __user *, size_t *, loff_t *);
extern int proc_doulongvec_ms_jiffies_minmax(struct ctl_table *table, int,
				      void __user *, size_t *, loff_t *);
extern int proc_do_large_bitmap(struct ctl_table *, int,
				void __user *, size_t *, loff_t *);

/*
                                                                  
                                                                   
                                                         
                                                                     
  
                                                                   
                                           
  
                                                               
                  
  
                                                                     
                                                                     
                                                       
  
                                                                     
                                                                 
                                                                     
                                                               
   
                                                              
                                                                    
                                                                 
                      
 */

/*                                                   */
struct ctl_table_poll {
	atomic_t event;
	wait_queue_head_t wait;
};

static inline void *proc_sys_poll_event(struct ctl_table_poll *poll)
{
	return (void *)(unsigned long)atomic_read(&poll->event);
}

#define __CTL_TABLE_POLL_INITIALIZER(name) {				\
	.event = ATOMIC_INIT(0),					\
	.wait = __WAIT_QUEUE_HEAD_INITIALIZER(name.wait) }

#define DEFINE_CTL_TABLE_POLL(name)					\
	struct ctl_table_poll name = __CTL_TABLE_POLL_INITIALIZER(name)

/*                                                 */
struct ctl_table 
{
	const char *procname;		/*                                */
	void *data;
	int maxlen;
	umode_t mode;
	struct ctl_table *child;	/*            */
	proc_handler *proc_handler;	/*                              */
	struct ctl_table_poll *poll;
	void *extra1;
	void *extra2;
};

struct ctl_node {
	struct rb_node node;
	struct ctl_table_header *header;
};

/*                                                             
                           */
struct ctl_table_header
{
	union {
		struct {
			struct ctl_table *ctl_table;
			int used;
			int count;
			int nreg;
		};
		struct rcu_head rcu;
	};
	struct completion *unregistering;
	struct ctl_table *ctl_table_arg;
	struct ctl_table_root *root;
	struct ctl_table_set *set;
	struct ctl_dir *parent;
	struct ctl_node *node;
};

struct ctl_dir {
	/*                                        */
	struct ctl_table_header header;
	struct rb_root root;
};

struct ctl_table_set {
	int (*is_seen)(struct ctl_table_set *);
	struct ctl_dir dir;
};

struct ctl_table_root {
	struct ctl_table_set default_set;
	struct ctl_table_set *(*lookup)(struct ctl_table_root *root,
					   struct nsproxy *namespaces);
	int (*permissions)(struct ctl_table_root *root,
			struct nsproxy *namespaces, struct ctl_table *table);
};

/*                                                                   */
struct ctl_path {
	const char *procname;
};

#ifdef CONFIG_SYSCTL

void proc_sys_poll_notify(struct ctl_table_poll *poll);

extern void setup_sysctl_set(struct ctl_table_set *p,
	struct ctl_table_root *root,
	int (*is_seen)(struct ctl_table_set *));
extern void retire_sysctl_set(struct ctl_table_set *set);

void register_sysctl_root(struct ctl_table_root *root);
struct ctl_table_header *__register_sysctl_table(
	struct ctl_table_set *set,
	const char *path, struct ctl_table *table);
struct ctl_table_header *__register_sysctl_paths(
	struct ctl_table_set *set,
	const struct ctl_path *path, struct ctl_table *table);
struct ctl_table_header *register_sysctl(const char *path, struct ctl_table *table);
struct ctl_table_header *register_sysctl_table(struct ctl_table * table);
struct ctl_table_header *register_sysctl_paths(const struct ctl_path *path,
						struct ctl_table *table);

void unregister_sysctl_table(struct ctl_table_header * table);

extern int sysctl_init(void);
#else /*               */
static inline struct ctl_table_header *register_sysctl_table(struct ctl_table * table)
{
	return NULL;
}

static inline struct ctl_table_header *register_sysctl_paths(
			const struct ctl_path *path, struct ctl_table *table)
{
	return NULL;
}

static inline void unregister_sysctl_table(struct ctl_table_header * table)
{
}

static inline void setup_sysctl_set(struct ctl_table_set *p,
	struct ctl_table_root *root,
	int (*is_seen)(struct ctl_table_set *))
{
}

#endif /*               */

#endif /*            */

#endif /*                 */