/*
 * security/ccsecurity/internal.h
 *
 * Copyright (C) 2005-2012  NTT DATA CORPORATION
 *
 * Version: 1.8.3+   2012/05/05
 */

#ifndef _SECURITY_CCSECURITY_INTERNAL_H
#define _SECURITY_CCSECURITY_INTERNAL_H

#include <linux/version.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/utime.h>
#include <linux/file.h>
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 38)
#include <linux/smp_lock.h>
#endif
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/highmem.h>
#include <linux/poll.h>
#include <linux/binfmts.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/dcache.h>
#include <linux/mount.h>
#include <linux/net.h>
#include <linux/inet.h>
#include <linux/in.h>
#include <linux/in6.h>
#include <linux/un.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 5, 0)
#include <linux/fs.h>
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 5, 0)
#include <linux/namei.h>
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 30)
#include <linux/fs_struct.h>
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 20)
#include <linux/namespace.h>
#endif
#include <linux/proc_fs.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0) || defined(RHEL_MAJOR)
#include <linux/hash.h>
#endif
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 18) || (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33) && defined(CONFIG_SYSCTL_SYSCALL))
#include <linux/sysctl.h>
#endif
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 6)
#include <linux/kthread.h>
#endif
#include <stdarg.h>
#include <asm/uaccess.h>
#include <net/sock.h>
#include <net/af_unix.h>
#include <net/ip.h>
#include <net/ipv6.h>
#include <net/udp.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 5, 0)
#define sk_family family
#define sk_protocol protocol
#define sk_type type
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 20)

/*                                                                  */
struct path {
	struct vfsmount *mnt;
	struct dentry *dentry;
};

#endif

#ifndef __printf
#define __printf(a,b) __attribute__((format(printf,a,b)))
#endif
#ifndef __packed
#define __packed __attribute__((__packed__))
#endif
#ifndef bool
#define bool _Bool
#endif
#ifndef false
#define false 0
#endif
#ifndef true
#define true 1
#endif

#ifndef __user
#define __user
#endif

#ifndef current_uid
#define current_uid()   (current->uid)
#endif
#ifndef current_gid
#define current_gid()   (current->gid)
#endif
#ifndef current_euid
#define current_euid()  (current->euid)
#endif
#ifndef current_egid
#define current_egid()  (current->egid)
#endif
#ifndef current_suid
#define current_suid()  (current->suid)
#endif
#ifndef current_sgid
#define current_sgid()  (current->sgid)
#endif
#ifndef current_fsuid
#define current_fsuid() (current->fsuid)
#endif
#ifndef current_fsgid
#define current_fsgid() (current->fsgid)
#endif

#ifndef DEFINE_SPINLOCK
#define DEFINE_SPINLOCK(x) spinlock_t x = SPIN_LOCK_UNLOCKED
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 16)
#define mutex semaphore
#define mutex_init(mutex) init_MUTEX(mutex)
#define mutex_unlock(mutex) up(mutex)
#define mutex_lock(mutex) down(mutex)
#define mutex_lock_interruptible(mutex) down_interruptible(mutex)
#define mutex_trylock(mutex) (!down_trylock(mutex))
#define DEFINE_MUTEX(mutexname) DECLARE_MUTEX(mutexname)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 15)
#define MS_UNBINDABLE	(1<<17)	/*                      */
#define MS_PRIVATE	(1<<18)	/*                   */
#define MS_SLAVE	(1<<19)	/*                 */
#define MS_SHARED	(1<<20)	/*                  */
#endif

#ifndef container_of
#define container_of(ptr, type, member) ({				\
			const typeof(((type *)0)->member) *__mptr = (ptr); \
			(type *)((char *)__mptr - offsetof(type, member)); })
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0)
#define smp_read_barrier_depends smp_rmb
#endif

#ifndef ACCESS_ONCE
#define ACCESS_ONCE(x) (*(volatile typeof(x) *)&(x))
#endif

#ifndef rcu_dereference
#define rcu_dereference(p)     ({					\
			typeof(p) _________p1 = ACCESS_ONCE(p);		\
			smp_read_barrier_depends(); /*         */	\
			(_________p1);					\
		})
#endif

#ifndef rcu_assign_pointer
#define rcu_assign_pointer(p, v)			\
	({						\
		if (!__builtin_constant_p(v) ||		\
		    ((v) != NULL))			\
			smp_wmb(); /*         */	\
		(p) = (v);				\
	})
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0)
#define f_vfsmnt f_path.mnt
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 14)

/* 
                                                          
  
                            
                     
  
                                                                  
  
                                                
  
                                                                           
                                                                               
 */
#define kzalloc(size, flags) ({					\
			void *ret = kmalloc((size), (flags));	\
			if (ret)				\
				memset(ret, 0, (size));		\
			ret; })

#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 25)

/* 
                                              
  
                                   
  
                   
  
                                                
 */
static inline void path_put(struct path *path)
{
	dput(path->dentry);
	mntput(path->mnt);
}

#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 5, 0)

/* 
                                                                             
  
                                        
                                        
                                        
  
                   
  
                                                
 */
static inline void __list_add_rcu(struct list_head *new,
				  struct list_head *prev,
				  struct list_head *next)
{
	new->next = next;
	new->prev = prev;
	rcu_assign_pointer(prev->next, new);
	next->prev = new;
}

/* 
                                                             
  
                                        
                                        
  
                   
  
                                                
 */
static inline void list_add_tail_rcu(struct list_head *new,
				     struct list_head *head)
{
	__list_add_rcu(new, head->prev, head);
}

/* 
                                                        
  
                                        
                                        
  
                   
  
                                                
 */
static inline void list_add_rcu(struct list_head *new, struct list_head *head)
{
	__list_add_rcu(new, head, head->next);
}

#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 38)

/* 
                                                                        
  
                                         
  
                   
  
                                                
 */
static inline void __list_del_entry(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
}

#endif

#ifndef list_for_each_entry_safe

/* 
                                                                                                 
  
                                                 
                                                         
                                          
                                                          
  
                                                
 */
#define list_for_each_entry_safe(pos, n, head, member)                  \
	for (pos = list_entry((head)->next, typeof(*pos), member),      \
		     n = list_entry(pos->member.next, typeof(*pos), member); \
	     &pos->member != (head);					\
	     pos = n, n = list_entry(n->member.next, typeof(*n), member))

#endif

#ifndef srcu_dereference

/* 
                                                                 
  
                                                    
                                        
  
              
  
                                                
 */
#define srcu_dereference(p, ss) rcu_dereference(p)

#endif

#ifndef list_for_each_entry_srcu

/* 
                                                                  
  
                                               
                                   
                                                          
                                            
  
                                                                         
 */
#define list_for_each_entry_srcu(pos, head, member, ss)		      \
	for (pos = list_entry(srcu_dereference((head)->next, ss),     \
			      typeof(*pos), member);		      \
	     prefetch(pos->member.next), &pos->member != (head);      \
	     pos = list_entry(srcu_dereference(pos->member.next, ss), \
			      typeof(*pos), member))

#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 4, 30) || (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 5, 0) && LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 9))

#ifndef ssleep

/* 
                                        
  
                           
  
                   
  
                                                
  
                                                                          
                                                                               
 */
#define ssleep(secs) {						\
		set_current_state(TASK_UNINTERRUPTIBLE);	\
		schedule_timeout((HZ * secs) + 1);		\
	}

#endif

#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 5, 0)

/* 
                                       
  
                
                      
  
                                               
 */
#define from_kuid(ns, uid) (uid)

/* 
                                       
  
                
                      
  
                                               
 */
#define from_kgid(ns, gid) (gid)

/* 
                                                     
  
                                               
                                                
  
                                                                         
 */
#define uid_eq(left, right) ((left) == (right))
#define GLOBAL_ROOT_UID 0

#endif

/*
                              
 */

#include <linux/ccsecurity.h>

/*                                          */

/*                                    */
enum ccs_acl_entry_type_index {
	CCS_TYPE_PATH_ACL,
	CCS_TYPE_PATH2_ACL,
	CCS_TYPE_PATH_NUMBER_ACL,
	CCS_TYPE_MKDEV_ACL,
	CCS_TYPE_MOUNT_ACL,
#ifdef CONFIG_CCSECURITY_MISC
	CCS_TYPE_ENV_ACL,
#endif
#ifdef CONFIG_CCSECURITY_CAPABILITY
	CCS_TYPE_CAPABILITY_ACL,
#endif
#ifdef CONFIG_CCSECURITY_NETWORK
	CCS_TYPE_INET_ACL,
	CCS_TYPE_UNIX_ACL,
#endif
#ifdef CONFIG_CCSECURITY_IPC
	CCS_TYPE_SIGNAL_ACL,
#endif
#ifdef CONFIG_CCSECURITY_TASK_EXECUTE_HANDLER
	CCS_TYPE_AUTO_EXECUTE_HANDLER,
	CCS_TYPE_DENIED_EXECUTE_HANDLER,
#endif
#ifdef CONFIG_CCSECURITY_TASK_DOMAIN_TRANSITION
	CCS_TYPE_AUTO_TASK_ACL,
	CCS_TYPE_MANUAL_TASK_ACL,
#endif
};

/*                                           */
enum ccs_conditions_index {
	CCS_TASK_UID,             /*                 */
	CCS_TASK_EUID,            /*                 */
	CCS_TASK_SUID,            /*                 */
	CCS_TASK_FSUID,           /*                 */
	CCS_TASK_GID,             /*                 */
	CCS_TASK_EGID,            /*                 */
	CCS_TASK_SGID,            /*                 */
	CCS_TASK_FSGID,           /*                 */
	CCS_TASK_PID,             /*                */
	CCS_TASK_PPID,            /*                */
	CCS_EXEC_ARGC,            /*                               */
	CCS_EXEC_ENVC,            /*                               */
	CCS_TYPE_IS_SOCKET,       /*          */
	CCS_TYPE_IS_SYMLINK,      /*         */
	CCS_TYPE_IS_FILE,         /*         */
	CCS_TYPE_IS_BLOCK_DEV,    /*         */
	CCS_TYPE_IS_DIRECTORY,    /*         */
	CCS_TYPE_IS_CHAR_DEV,     /*         */
	CCS_TYPE_IS_FIFO,         /*         */
	CCS_MODE_SETUID,          /*         */
	CCS_MODE_SETGID,          /*         */
	CCS_MODE_STICKY,          /*         */
	CCS_MODE_OWNER_READ,      /*         */
	CCS_MODE_OWNER_WRITE,     /*         */
	CCS_MODE_OWNER_EXECUTE,   /*         */
	CCS_MODE_GROUP_READ,      /*         */
	CCS_MODE_GROUP_WRITE,     /*         */
	CCS_MODE_GROUP_EXECUTE,   /*         */
	CCS_MODE_OTHERS_READ,     /*         */
	CCS_MODE_OTHERS_WRITE,    /*         */
	CCS_MODE_OTHERS_EXECUTE,  /*         */
	CCS_TASK_TYPE,            /*                         
                                     */
	CCS_TASK_EXECUTE_HANDLER, /*                             */
	CCS_EXEC_REALPATH,
	CCS_SYMLINK_TARGET,
	CCS_PATH1_UID,
	CCS_PATH1_GID,
	CCS_PATH1_INO,
	CCS_PATH1_MAJOR,
	CCS_PATH1_MINOR,
	CCS_PATH1_PERM,
	CCS_PATH1_TYPE,
	CCS_PATH1_DEV_MAJOR,
	CCS_PATH1_DEV_MINOR,
	CCS_PATH2_UID,
	CCS_PATH2_GID,
	CCS_PATH2_INO,
	CCS_PATH2_MAJOR,
	CCS_PATH2_MINOR,
	CCS_PATH2_PERM,
	CCS_PATH2_TYPE,
	CCS_PATH2_DEV_MAJOR,
	CCS_PATH2_DEV_MINOR,
	CCS_PATH1_PARENT_UID,
	CCS_PATH1_PARENT_GID,
	CCS_PATH1_PARENT_INO,
	CCS_PATH1_PARENT_PERM,
	CCS_PATH2_PARENT_UID,
	CCS_PATH2_PARENT_GID,
	CCS_PATH2_PARENT_INO,
	CCS_PATH2_PARENT_PERM,
	CCS_MAX_CONDITION_KEYWORD,
	CCS_NUMBER_UNION,
	CCS_NAME_UNION,
	CCS_ARGV_ENTRY,
	CCS_ENVP_ENTRY,
};

/*                                        */
enum ccs_domain_info_flags_index {
	/*                        */
	CCS_DIF_QUOTA_WARNED,
	/*
                                                    
                                                                       
                                                 
                                                                       
  */
	CCS_DIF_TRANSITION_FAILED,
	CCS_MAX_DOMAIN_INFO_FLAGS
};

/*                               */
enum ccs_grant_log {
	/*                                 */
	CCS_GRANTLOG_AUTO,
	/*                            */
	CCS_GRANTLOG_NO,
	/*                     */
	CCS_GRANTLOG_YES,
};

/*                                  */
enum ccs_group_id {
	CCS_PATH_GROUP,
	CCS_NUMBER_GROUP,
#ifdef CONFIG_CCSECURITY_NETWORK
	CCS_ADDRESS_GROUP,
#endif
	CCS_MAX_GROUP
};

/*                                              */
enum ccs_mac_category_index {
	CCS_MAC_CATEGORY_FILE,
#ifdef CONFIG_CCSECURITY_NETWORK
	CCS_MAC_CATEGORY_NETWORK,
#endif
#ifdef CONFIG_CCSECURITY_MISC
	CCS_MAC_CATEGORY_MISC,
#endif
#ifdef CONFIG_CCSECURITY_IPC
	CCS_MAC_CATEGORY_IPC,
#endif
#ifdef CONFIG_CCSECURITY_CAPABILITY
	CCS_MAC_CATEGORY_CAPABILITY,
#endif
	CCS_MAX_MAC_CATEGORY_INDEX
};

/*                                  */
enum ccs_mac_index {
	CCS_MAC_FILE_EXECUTE,
	CCS_MAC_FILE_OPEN,
	CCS_MAC_FILE_CREATE,
	CCS_MAC_FILE_UNLINK,
#ifdef CONFIG_CCSECURITY_FILE_GETATTR
	CCS_MAC_FILE_GETATTR,
#endif
	CCS_MAC_FILE_MKDIR,
	CCS_MAC_FILE_RMDIR,
	CCS_MAC_FILE_MKFIFO,
	CCS_MAC_FILE_MKSOCK,
	CCS_MAC_FILE_TRUNCATE,
	CCS_MAC_FILE_SYMLINK,
	CCS_MAC_FILE_MKBLOCK,
	CCS_MAC_FILE_MKCHAR,
	CCS_MAC_FILE_LINK,
	CCS_MAC_FILE_RENAME,
	CCS_MAC_FILE_CHMOD,
	CCS_MAC_FILE_CHOWN,
	CCS_MAC_FILE_CHGRP,
	CCS_MAC_FILE_IOCTL,
	CCS_MAC_FILE_CHROOT,
	CCS_MAC_FILE_MOUNT,
	CCS_MAC_FILE_UMOUNT,
	CCS_MAC_FILE_PIVOT_ROOT,
#ifdef CONFIG_CCSECURITY_NETWORK
	CCS_MAC_NETWORK_INET_STREAM_BIND,
	CCS_MAC_NETWORK_INET_STREAM_LISTEN,
	CCS_MAC_NETWORK_INET_STREAM_CONNECT,
	CCS_MAC_NETWORK_INET_STREAM_ACCEPT,
	CCS_MAC_NETWORK_INET_DGRAM_BIND,
	CCS_MAC_NETWORK_INET_DGRAM_SEND,
#ifdef CONFIG_CCSECURITY_NETWORK_RECVMSG
	CCS_MAC_NETWORK_INET_DGRAM_RECV,
#endif
	CCS_MAC_NETWORK_INET_RAW_BIND,
	CCS_MAC_NETWORK_INET_RAW_SEND,
#ifdef CONFIG_CCSECURITY_NETWORK_RECVMSG
	CCS_MAC_NETWORK_INET_RAW_RECV,
#endif
	CCS_MAC_NETWORK_UNIX_STREAM_BIND,
	CCS_MAC_NETWORK_UNIX_STREAM_LISTEN,
	CCS_MAC_NETWORK_UNIX_STREAM_CONNECT,
	CCS_MAC_NETWORK_UNIX_STREAM_ACCEPT,
	CCS_MAC_NETWORK_UNIX_DGRAM_BIND,
	CCS_MAC_NETWORK_UNIX_DGRAM_SEND,
#ifdef CONFIG_CCSECURITY_NETWORK_RECVMSG
	CCS_MAC_NETWORK_UNIX_DGRAM_RECV,
#endif
	CCS_MAC_NETWORK_UNIX_SEQPACKET_BIND,
	CCS_MAC_NETWORK_UNIX_SEQPACKET_LISTEN,
	CCS_MAC_NETWORK_UNIX_SEQPACKET_CONNECT,
	CCS_MAC_NETWORK_UNIX_SEQPACKET_ACCEPT,
#endif
#ifdef CONFIG_CCSECURITY_MISC
	CCS_MAC_ENVIRON,
#endif
#ifdef CONFIG_CCSECURITY_IPC
	CCS_MAC_SIGNAL,
#endif
#ifdef CONFIG_CCSECURITY_CAPABILITY
	CCS_MAC_CAPABILITY_USE_ROUTE_SOCKET,
	CCS_MAC_CAPABILITY_USE_PACKET_SOCKET,
	CCS_MAC_CAPABILITY_SYS_REBOOT,
	CCS_MAC_CAPABILITY_SYS_VHANGUP,
	CCS_MAC_CAPABILITY_SYS_SETTIME,
	CCS_MAC_CAPABILITY_SYS_NICE,
	CCS_MAC_CAPABILITY_SYS_SETHOSTNAME,
	CCS_MAC_CAPABILITY_USE_KERNEL_MODULE,
	CCS_MAC_CAPABILITY_SYS_KEXEC_LOAD,
	CCS_MAC_CAPABILITY_SYS_PTRACE,
#endif
	CCS_MAX_MAC_INDEX
};

/*                                             */
enum ccs_memory_stat_type {
	CCS_MEMORY_POLICY,
	CCS_MEMORY_AUDIT,
	CCS_MEMORY_QUERY,
	CCS_MAX_MEMORY_STAT
};

/*                                                                        */
enum ccs_mkdev_acl_index {
	CCS_TYPE_MKBLOCK,
	CCS_TYPE_MKCHAR,
	CCS_MAX_MKDEV_OPERATION
};

/*                                   */
enum ccs_mode_value {
	CCS_CONFIG_DISABLED,
	CCS_CONFIG_LEARNING,
	CCS_CONFIG_PERMISSIVE,
	CCS_CONFIG_ENFORCING,
	CCS_CONFIG_MAX_MODE,
	CCS_CONFIG_WANT_REJECT_LOG =  64,
	CCS_CONFIG_WANT_GRANT_LOG  = 128,
	CCS_CONFIG_USE_DEFAULT     = 255,
};

/*                                      */
enum ccs_network_acl_index {
	CCS_NETWORK_BIND,    /*                   */
	CCS_NETWORK_LISTEN,  /*                     */
	CCS_NETWORK_CONNECT, /*                      */
	CCS_NETWORK_ACCEPT,  /*                     */
	CCS_NETWORK_SEND,    /*                   */
#ifdef CONFIG_CCSECURITY_NETWORK_RECVMSG
	CCS_NETWORK_RECV,    /*                   */
#endif
	CCS_MAX_NETWORK_OPERATION
};

/*                                                       */
enum ccs_path2_acl_index {
	CCS_TYPE_LINK,
	CCS_TYPE_RENAME,
	CCS_TYPE_PIVOT_ROOT,
	CCS_MAX_PATH2_OPERATION
};

/*                                                      */
enum ccs_path_acl_index {
	CCS_TYPE_EXECUTE,
	CCS_TYPE_READ,
	CCS_TYPE_WRITE,
	CCS_TYPE_APPEND,
	CCS_TYPE_UNLINK,
#ifdef CONFIG_CCSECURITY_FILE_GETATTR
	CCS_TYPE_GETATTR,
#endif
	CCS_TYPE_RMDIR,
	CCS_TYPE_TRUNCATE,
	CCS_TYPE_SYMLINK,
	CCS_TYPE_CHROOT,
	CCS_TYPE_UMOUNT,
	CCS_MAX_PATH_OPERATION
};

/*                                                                     */
enum ccs_path_number_acl_index {
	CCS_TYPE_CREATE,
	CCS_TYPE_MKDIR,
	CCS_TYPE_MKFIFO,
	CCS_TYPE_MKSOCK,
	CCS_TYPE_IOCTL,
	CCS_TYPE_CHMOD,
	CCS_TYPE_CHOWN,
	CCS_TYPE_CHGRP,
	CCS_MAX_PATH_NUMBER_OPERATION
};

/*                           */
enum ccs_path_stat_index {
	/*                           */
	CCS_PATH1,
	CCS_PATH1_PARENT,
	CCS_PATH2,
	CCS_PATH2_PARENT,
	CCS_MAX_PATH_STAT
};

/*                               */
enum ccs_policy_id {
#ifdef CONFIG_CCSECURITY_PORTRESERVE
	CCS_ID_RESERVEDPORT,
#endif
	CCS_ID_GROUP,
#ifdef CONFIG_CCSECURITY_NETWORK
	CCS_ID_ADDRESS_GROUP,
#endif
	CCS_ID_PATH_GROUP,
	CCS_ID_NUMBER_GROUP,
	CCS_ID_AGGREGATOR,
	CCS_ID_TRANSITION_CONTROL,
	CCS_ID_MANAGER,
	CCS_ID_CONDITION,
	CCS_ID_NAME,
	CCS_ID_ACL,
	CCS_ID_DOMAIN,
	CCS_MAX_POLICY
};

/*                                             */
enum ccs_policy_stat_type {
	/*                           */
	CCS_STAT_POLICY_UPDATES,
	CCS_STAT_POLICY_LEARNING,   /*                        */
	CCS_STAT_POLICY_PERMISSIVE, /*                          */
	CCS_STAT_POLICY_ENFORCING,  /*                         */
	CCS_MAX_POLICY_STAT
};

/*                                                */
enum ccs_pref_index {
	CCS_PREF_MAX_AUDIT_LOG,
	CCS_PREF_MAX_LEARNING_ENTRY,
	CCS_PREF_ENFORCING_PENALTY,
	CCS_MAX_PREF
};

/*                                          */
enum ccs_proc_interface_index {
	CCS_DOMAIN_POLICY,
	CCS_EXCEPTION_POLICY,
	CCS_PROCESS_STATUS,
	CCS_STAT,
	CCS_AUDIT,
	CCS_VERSION,
	CCS_PROFILE,
	CCS_QUERY,
	CCS_MANAGER,
#ifdef CONFIG_CCSECURITY_TASK_EXECUTE_HANDLER
	CCS_EXECUTE_HANDLER,
#endif
};

/*                                             */
enum ccs_special_mount {
	CCS_MOUNT_BIND,            /*                              */
	CCS_MOUNT_MOVE,            /*                              */
	CCS_MOUNT_REMOUNT,         /*                              */
	CCS_MOUNT_MAKE_UNBINDABLE, /*                              */
	CCS_MOUNT_MAKE_PRIVATE,    /*                              */
	CCS_MOUNT_MAKE_SLAVE,      /*                              */
	CCS_MOUNT_MAKE_SHARED,     /*                              */
	CCS_MAX_SPECIAL_MOUNT
};

/*                                                       */
enum ccs_transition_type {
	/*                           */
	CCS_TRANSITION_CONTROL_NO_RESET,
	CCS_TRANSITION_CONTROL_RESET,
	CCS_TRANSITION_CONTROL_NO_INITIALIZE,
	CCS_TRANSITION_CONTROL_INITIALIZE,
	CCS_TRANSITION_CONTROL_NO_KEEP,
	CCS_TRANSITION_CONTROL_KEEP,
	CCS_MAX_TRANSITION_TYPE
};

/*                                           */
enum ccs_value_type {
	CCS_VALUE_TYPE_INVALID,
	CCS_VALUE_TYPE_DECIMAL,
	CCS_VALUE_TYPE_OCTAL,
	CCS_VALUE_TYPE_HEXADECIMAL,
};

/*                                        */

/*
                                                                            
                                                                           
                                          
 */
#define CCS_HASH_BITS 8
#define CCS_MAX_HASH (1u << CCS_HASH_BITS)

/*
                                                                        
                                     
 */
#define CCS_SOCK_MAX 6

/*                                                  */
#define CCS_EXEC_TMPSIZE     4096

/*                                                      */
#define CCS_GC_IN_PROGRESS -1

/*                                                 */
#define CCS_MAX_PROFILES 256

/*                                               */
#define CCS_MAX_ACL_GROUPS 256

/*                                                    */
#define CCS_OPEN_FOR_READ_TRUNCATE        1
/*                                   */
#define CCS_OPEN_FOR_IOCTL_ONLY           2
/*                                       */
#define CCS_TASK_IS_IN_EXECVE             4
/*                                                          */
#define CCS_TASK_IS_EXECUTE_HANDLER       8
/*                                                                      */
#define CCS_TASK_IS_MANAGER              16

/*
                                                                           
                                                                        
  
                                                                              
                                                             
 */
#define CCS_RETRY_REQUEST 1

/*                                           */
#ifndef __GFP_HIGHIO
#define __GFP_HIGHIO 0
#endif
#ifndef __GFP_NOWARN
#define __GFP_NOWARN 0
#endif
#ifndef __GFP_NORETRY
#define __GFP_NORETRY 0
#endif
#ifndef __GFP_NOMEMALLOC
#define __GFP_NOMEMALLOC 0
#endif

/*                               */
#define CCS_GFP_FLAGS (__GFP_WAIT | __GFP_IO | __GFP_HIGHIO | __GFP_NOWARN | \
		       __GFP_NORETRY | __GFP_NOMEMALLOC)

/*                                               */
#define CCS_MAX_IO_READ_QUEUE 64

/*                                        */

/*                                        */
struct ccs_acl_head {
	struct list_head list;
	s8 is_deleted; /*                                     */
} __packed;

/*                                   */
struct ccs_shared_acl_head {
	struct list_head list;
	atomic_t users;
} __packed;

/*                                       */
struct ccs_acl_info {
	struct list_head list;
	struct ccs_condition *cond; /*             */
	s8 is_deleted; /*                                     */
	u8 type; /*                                                   */
	u16 perm;
} __packed;

/*                               */
struct ccs_name_union {
	/*                                     */
	const struct ccs_path_info *filename;
	struct ccs_group *group;
};

/*                                 */
struct ccs_number_union {
	unsigned long values[2];
	struct ccs_group *group; /*             */
	/*                                         */
	u8 value_type[2];
};

/*                                      */
struct ccs_ipaddr_union {
	struct in6_addr ip[2]; /*             */
	struct ccs_group *group; /*                           */
	bool is_ipv6; /*                               */
};

/*                                                                      */
struct ccs_group {
	struct ccs_shared_acl_head head;
	/*                                      */
	const struct ccs_path_info *group_name;
	/*
                                                                   
                               
  */
	struct list_head member_list;
};

/*                                       */
struct ccs_path_group {
	struct ccs_acl_head head;
	const struct ccs_path_info *member_name;
};

/*                                         */
struct ccs_number_group {
	struct ccs_acl_head head;
	struct ccs_number_union number;
};

/*                                          */
struct ccs_address_group {
	struct ccs_acl_head head;
	/*                                      */
	struct ccs_ipaddr_union address;
};

/*                                                                  */
struct ccs_mini_stat {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 5, 0)
	kuid_t uid;
	kgid_t gid;
#else
	uid_t uid;
	gid_t gid;
#endif
	ino_t ino;
	umode_t mode;
	dev_t dev;
	dev_t rdev;
};

/*                                                                   */
struct ccs_page_dump {
	struct page *page;    /*                         */
	char *data;           /*                                        */
};

/*                                                                */
struct ccs_obj_info {
	/*                                                                   */
	bool validate_done;
	/*                           */
	bool stat_valid[CCS_MAX_PATH_STAT];
	/*                                                             */
	struct path path1;
	/*                                                              */
	struct path path2;
	/*
                                                                      
                     
  */
	struct ccs_mini_stat stat[CCS_MAX_PATH_STAT];
	/*
                                                                     
                   
  */
	struct ccs_path_info *symlink_target;
};

/*                                                             */
struct ccs_condition_element {
	/*
                                                                
                                                                
                                
  */
	u8 left;
	/*
                                                       
                                                                     
                                                     
  */
	u8 right;
	/*                                                                 */
	bool equals;
};

/*                                   */
struct ccs_condition {
	struct ccs_shared_acl_head head;
	u32 size; /*                                       */
	u16 condc; /*                                      */
	u16 numbers_count; /*                                             */
	u16 names_count; /*                                          */
	u16 argc; /*                              */
	u16 envc; /*                              */
	u8 grant_log; /*                                        */
	bool exec_transit; /*                                        */
	const struct ccs_path_info *transit; /*             */
	/*
                                                  
                                                  
                                             
                               
                               
  */
};

struct ccs_execve;
struct ccs_policy_namespace;

/*                             */
struct ccs_request_info {
	/*
                                                                   
                              
  */
	struct ccs_obj_info *obj;
	/*
                                                        
                                    
  */
	struct ccs_execve *ee;
	/*
                           
                                                                  
  */
	union {
		struct {
			const struct ccs_path_info *filename;
			/*
                                                    
     
                                                         
                                                      
                                                        
                                 
    */
			const struct ccs_path_info *matched_path;
			/*                                             */
			u8 operation;
		} path;
		struct {
			const struct ccs_path_info *filename1;
			const struct ccs_path_info *filename2;
			/*                                              */
			u8 operation;
		} path2;
		struct {
			const struct ccs_path_info *filename;
			unsigned int mode;
			unsigned int major;
			unsigned int minor;
			/*                                              */
			u8 operation;
		} mkdev;
		struct {
			const struct ccs_path_info *filename;
			unsigned long number;
			/*
                                                        
    */
			u8 operation;
		} path_number;
#ifdef CONFIG_CCSECURITY_NETWORK
		struct {
			const u32 *address; /*             */
			u16 port; /*              */
			/*                                          */
			u8 protocol;
			/*                                                */
			u8 operation;
			bool is_ipv6;
		} inet_network;
		struct {
			const struct ccs_path_info *address;
			/*                                          */
			u8 protocol;
			/*                                                */
			u8 operation;
		} unix_network;
#endif
#ifdef CONFIG_CCSECURITY_MISC
		struct {
			const struct ccs_path_info *name;
		} environ;
#endif
#ifdef CONFIG_CCSECURITY_CAPABILITY
		struct {
			/*                                                   */
			u8 operation;
		} capability;
#endif
#ifdef CONFIG_CCSECURITY_IPC
		struct {
			const char *dest_pattern;
			int sig;
		} signal;
#endif
		struct {
			const struct ccs_path_info *type;
			const struct ccs_path_info *dir;
			const struct ccs_path_info *dev;
			unsigned long flags;
			int need_dev;
		} mount;
#ifdef CONFIG_CCSECURITY_TASK_DOMAIN_TRANSITION
		struct {
			const struct ccs_path_info *domainname;
		} task;
#endif
	} param;
	/*
                                                                      
                                                   
                                                                  
                                                                
  */
	struct ccs_acl_info *matched_acl;
	u8 param_type; /*                                                   */
	bool granted; /*                                   */
	/*                                                             */
	bool dont_sleep_on_enforce_error;
	/*
                                                         
                                                                  
                      
  */
	u8 retry;
	/*
                                                     
                                                     
  */
	u8 profile;
	/*
                                                     
                                                    
                                                
  */
	u8 mode;
	/*
                                                      
                                                      
                                                           
  */
	u8 type;
};

/*                                */
struct ccs_path_info {
	const char *name;
	u32 hash;          /*                                      */
	u16 total_len;     /*                                      */
	u16 const_len;     /*                                      */
	bool is_dir;       /*                                      */
	bool is_patterned; /*                                      */
};

/*                                   */
struct ccs_execve {
	struct ccs_request_info r;
	struct ccs_obj_info obj;
	struct linux_binprm *bprm;
	struct ccs_domain_info *previous_domain;
	const struct ccs_path_info *transition;
	/*                     */
	const struct ccs_path_info *handler;
	char *handler_path; /*                                         */
	/*                                */
	struct ccs_page_dump dump;
	/*                    */
	char *tmp; /*                                */
};

/*                                   */
struct ccs_domain_info {
	struct list_head list;
	struct list_head acl_info_list;
	/*                                           */
	const struct ccs_path_info *domainname;
	/*                                        */
	struct ccs_policy_namespace *ns;
	u8 profile;        /*                        */
	u8 group;          /*                        */
	bool is_deleted;   /*                        */
	bool flags[CCS_MAX_DOMAIN_INFO_FLAGS];
};

/*
                                                                      
                                                                 
 */
struct ccs_transition_control {
	struct ccs_acl_head head;
	u8 type; /*                                             */
	bool is_last_name; /*                                            */
	const struct ccs_path_info *domainname; /*            */
	const struct ccs_path_info *program;    /*            */
};

/*                                     */
struct ccs_aggregator {
	struct ccs_acl_head head;
	const struct ccs_path_info *original_name;
	const struct ccs_path_info *aggregated_name;
};

/*                                        */
struct ccs_reserved {
	struct ccs_acl_head head;
	struct ccs_number_union port;
};

/*                               */
struct ccs_manager {
	struct ccs_acl_head head;
	/*                                    */
	const struct ccs_path_info *manager;
};

/*                       */
struct ccs_argv {
	unsigned long index;
	const struct ccs_path_info *value;
	bool is_not;
};

/*                       */
struct ccs_envp {
	const struct ccs_path_info *name;
	const struct ccs_path_info *value;
	bool is_not;
};

/*
                                                                              
             
  
                                                                             
                                                                         
                                                                            
                                                                            
                                                            
                                                                     
                                                                         
  
                                                                             
                                                                              
                                                                              
                                                                               
                           
 */
struct ccs_handler_acl {
	struct ccs_acl_info head;       /*                                   */
	const struct ccs_path_info *handler; /*                              */
};

/*
                                                  
                                             
 */
struct ccs_task_acl {
	struct ccs_acl_info head; /*                            */
	/*                        */
	const struct ccs_path_info *domainname;
};

/*
                                                                          
                                                                
                                                              
 */
struct ccs_path_acl {
	struct ccs_acl_info head; /*                          */
	struct ccs_name_union name;
};

/*
                                                                            
 */
struct ccs_path2_acl {
	struct ccs_acl_info head; /*                           */
	struct ccs_name_union name1;
	struct ccs_name_union name2;
};

/*
                                                                           
                                                                       
 */
struct ccs_path_number_acl {
	struct ccs_acl_info head; /*                                 */
	struct ccs_name_union name;
	struct ccs_number_union number;
};

/*                                                           */
struct ccs_mkdev_acl {
	struct ccs_acl_info head; /*                           */
	struct ccs_name_union name;
	struct ccs_number_union mode;
	struct ccs_number_union major;
	struct ccs_number_union minor;
};

/*                                       */
struct ccs_mount_acl {
	struct ccs_acl_info head; /*                           */
	struct ccs_name_union dev_name;
	struct ccs_name_union dir_name;
	struct ccs_name_union fs_type;
	struct ccs_number_union flags;
};

/*                                                      */
struct ccs_env_acl {
	struct ccs_acl_info head;        /*                          */
	const struct ccs_path_info *env; /*                      */
};

/*                                       */
struct ccs_capability_acl {
	struct ccs_acl_info head; /*                                */
	u8 operation; /*                                                   */
};

/*                                       */
struct ccs_signal_acl {
	struct ccs_acl_info head; /*                            */
	struct ccs_number_union sig;
	/*                                 */
	const struct ccs_path_info *domainname;
};

/*                                         */
struct ccs_inet_acl {
	struct ccs_acl_info head; /*                          */
	u8 protocol;
	struct ccs_ipaddr_union address;
	struct ccs_number_union port;
};

/*                                         */
struct ccs_unix_acl {
	struct ccs_acl_info head; /*                          */
	u8 protocol;
	struct ccs_name_union name;
};

/*                                    */
struct ccs_name {
	struct ccs_shared_acl_head head;
	int size; /*                                       */
	struct ccs_path_info entry;
};

/*                                                         */
struct ccs_acl_param {
	char *data; /*                   */
	struct list_head *list; /*                        */
	struct ccs_policy_namespace *ns; /*                   */
	bool is_delete; /*                                 */
	union ccs_acl_union {
		struct ccs_acl_info acl_info;
		struct ccs_handler_acl handler_acl;
		struct ccs_task_acl task_acl;
		struct ccs_path_acl path_acl;
		struct ccs_path2_acl path2_acl;
		struct ccs_path_number_acl path_number_acl;
		struct ccs_mkdev_acl mkdev_acl;
		struct ccs_mount_acl mount_acl;
		struct ccs_env_acl env_acl;
		struct ccs_capability_acl capability_acl;
		struct ccs_signal_acl signal_acl;
		struct ccs_inet_acl inet_acl;
		struct ccs_unix_acl unix_acl;
		/**/
		struct ccs_acl_head acl_head;
		struct ccs_transition_control transition_control;
		struct ccs_aggregator aggregator;
		struct ccs_reserved reserved;
		struct ccs_manager manager;
		struct ccs_path_group path_group;
		struct ccs_number_group number_group;
		struct ccs_address_group address_group;
	} e;
};

/*                                                                 */
struct ccs_io_buffer {
	/*                                      */
	struct mutex io_sem;
	char __user *read_user_buf;
	size_t read_user_buf_avail;
	struct {
		struct list_head *ns;
		struct list_head *domain;
		struct list_head *group;
		struct list_head *acl;
		size_t avail;
		unsigned int step;
		unsigned int query_index;
		u16 index;
		u16 cond_index;
		u8 acl_group_index;
		u8 cond_step;
		u8 bit;
		u8 w_pos;
		bool eof;
		bool print_this_domain_only;
		bool print_transition_related_only;
		bool print_cond_part;
		const char *w[CCS_MAX_IO_READ_QUEUE];
	} r;
	struct {
		struct ccs_policy_namespace *ns;
		struct ccs_domain_info *domain;
		size_t avail;
		bool is_delete;
	} w;
	/*                                      */
	char *read_buf;
	/*                                      */
	size_t readbuf_size;
	/*                                      */
	char *write_buf;
	/*                                      */
	size_t writebuf_size;
	/*                    */
	enum ccs_proc_interface_index type;
	/*                                                     */
	u8 users;
	/*                                              */
	struct list_head list;
};

/*                                            */
struct ccs_profile {
	const struct ccs_path_info *comment;
	u8 default_config;
	u8 config[CCS_MAX_MAC_INDEX + CCS_MAX_MAC_CATEGORY_INDEX];
	unsigned int pref[CCS_MAX_PREF];
};

/*                                                 */
struct ccs_time {
	u16 year;
	u8 month;
	u8 day;
	u8 hour;
	u8 min;
	u8 sec;
};

/*                                 */
struct ccs_policy_namespace {
	/*                                               */
	struct ccs_profile *profile_ptr[CCS_MAX_PROFILES];
	/*                             */
	struct list_head group_list[CCS_MAX_GROUP];
	/*                 */
	struct list_head policy_list[CCS_MAX_POLICY];
	/*                                                 */
	struct list_head acl_group[CCS_MAX_ACL_GROUPS];
	/*                                                 */
	struct list_head namespace_list;
	/*                                                      */
	unsigned int profile_version;
	/*                                                                 */
	const char *name;
};

/*                                                          */

void __init ccs_permission_init(void);
void __init ccs_mm_init(void);

/*                                        */

bool ccs_dump_page(struct linux_binprm *bprm, unsigned long pos,
		   struct ccs_page_dump *dump);
bool ccs_memory_ok(const void *ptr, const unsigned int size);
char *ccs_encode(const char *str);
char *ccs_encode2(const char *str, int str_len);
char *ccs_realpath(struct path *path);
const char *ccs_get_exe(void);
const struct ccs_path_info *ccs_get_name(const char *name);
int ccs_audit_log(struct ccs_request_info *r);
int ccs_check_acl(struct ccs_request_info *r);
int ccs_init_request_info(struct ccs_request_info *r, const u8 index);
struct ccs_domain_info *ccs_assign_domain(const char *domainname,
					  const bool transit);
u8 ccs_get_config(const u8 profile, const u8 index);
void *ccs_commit_ok(void *data, const unsigned int size);
void ccs_del_acl(struct list_head *element);
void ccs_del_condition(struct list_head *element);
void ccs_fill_path_info(struct ccs_path_info *ptr);
void ccs_get_attributes(struct ccs_obj_info *obj);
void ccs_notify_gc(struct ccs_io_buffer *head, const bool is_register);
void ccs_transition_failed(const char *domainname);
void ccs_warn_oom(const char *function);
void ccs_write_log(struct ccs_request_info *r, const char *fmt, ...)
	__printf(2, 3);

/*                                       */

extern bool ccs_policy_loaded;
extern const char * const ccs_dif[CCS_MAX_DOMAIN_INFO_FLAGS];
extern const u8 ccs_c2mac[CCS_MAX_CAPABILITY_INDEX];
extern const u8 ccs_pn2mac[CCS_MAX_PATH_NUMBER_OPERATION];
extern const u8 ccs_pnnn2mac[CCS_MAX_MKDEV_OPERATION];
extern const u8 ccs_pp2mac[CCS_MAX_PATH2_OPERATION];
extern struct ccs_domain_info ccs_kernel_domain;
extern struct list_head ccs_condition_list;
extern struct list_head ccs_domain_list;
extern struct list_head ccs_name_list[CCS_MAX_HASH];
extern struct list_head ccs_namespace_list;
extern struct mutex ccs_policy_lock;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 19)
extern struct srcu_struct ccs_ss;
#endif
extern unsigned int ccs_memory_quota[CCS_MAX_MEMORY_STAT];
extern unsigned int ccs_memory_used[CCS_MAX_MEMORY_STAT];

/*                                     */

/* 
                                                               
  
                                         
                                         
  
                                             
 */
static inline bool ccs_pathcmp(const struct ccs_path_info *a,
			       const struct ccs_path_info *b)
{
	return a->hash != b->hash || strcmp(a->name, b->name);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 19)

/* 
                                                   
  
                                              
 */
static inline int ccs_read_lock(void)
{
	return srcu_read_lock(&ccs_ss);
}

/* 
                                                        
  
                                                  
  
                   
 */
static inline void ccs_read_unlock(const int idx)
{
	srcu_read_unlock(&ccs_ss, idx);
}

#else

int ccs_lock(void);
void ccs_unlock(const int idx);

/* 
                                                   
  
                                              
 */
static inline int ccs_read_lock(void)
{
	return ccs_lock();
}

/* 
                                                        
  
                                                  
  
                   
 */
static inline void ccs_read_unlock(const int idx)
{
	ccs_unlock(idx);
}

#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 18)

/* 
                                                                          
  
                   
 */
static inline void ccs_tasklist_lock(void)
{
	rcu_read_lock();
}

/* 
                                                                               
  
                   
 */
static inline void ccs_tasklist_unlock(void)
{
	rcu_read_unlock();
}

#else

/* 
                                                                          
  
                   
 */
static inline void ccs_tasklist_lock(void)
{
	read_lock(&tasklist_lock);
}

/* 
                                                                               
  
                   
 */
static inline void ccs_tasklist_unlock(void)
{
	read_unlock(&tasklist_lock);
}

#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24)

/* 
                                       
  
                                
  
                                                                            
                                                       
 */
static inline pid_t ccs_sys_getppid(void)
{
	pid_t pid;
	rcu_read_lock();
	pid = task_tgid_vnr(rcu_dereference(current->real_parent));
	rcu_read_unlock();
	return pid;
}

#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2, 5, 0)

/* 
                                       
  
                                
  
                                                                             
                                                                            
                                                               
 */
static inline pid_t ccs_sys_getppid(void)
{
	pid_t pid;
	rcu_read_lock();
#if (defined(RHEL_MAJOR) && RHEL_MAJOR == 5) || (defined(AX_MAJOR) && AX_MAJOR == 3)
	pid = rcu_dereference(current->parent)->tgid;
#elif defined(CONFIG_UTRACE)
	/*
                                                                
                                                
  */
	pid = rcu_dereference(current->parent)->tgid;
#else
	pid = rcu_dereference(current->real_parent)->tgid;
#endif
	rcu_read_unlock();
	return pid;
}

#else

/* 
                                       
  
                                
  
                                                                              
                                                                      
             
 */
static inline pid_t ccs_sys_getppid(void)
{
	pid_t pid;
	read_lock(&tasklist_lock);
#ifdef TASK_DEAD
	pid = current->group_leader->real_parent->tgid;
#else
	pid = current->p_opptr->pid;
#endif
	read_unlock(&tasklist_lock);
	return pid;
}

#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24)

/* 
                                     
  
                                
  
                                                                           
                                                      
 */
static inline pid_t ccs_sys_getpid(void)
{
	return task_tgid_vnr(current);
}

#else

/* 
                                     
  
                                
 */
static inline pid_t ccs_sys_getpid(void)
{
	return current->tgid;
}

#endif

/* 
                                                       
  
                            
                                  
  
                
 */
static inline u8 ccs_get_mode(const u8 profile, const u8 index)
{
	return ccs_get_config(profile, index) & (CCS_CONFIG_MAX_MODE - 1);
}

#if defined(CONFIG_SLOB)

/* 
                                                                    
  
                                
  
                 
  
                                                                    
 */
static inline int ccs_round2(size_t size)
{
	return size;
}

#else

/* 
                                                                    
  
                                
  
                        
  
                                                                             
                    
 */
static inline int ccs_round2(size_t size)
{
#if PAGE_SIZE == 4096
	size_t bsize = 32;
#else
	size_t bsize = 64;
#endif
	if (!size)
		return 0;
	while (size > bsize)
		bsize <<= 1;
	return bsize;
}

#endif

/* 
                                                                
  
                                                        
  
                   
 */
static inline void ccs_put_condition(struct ccs_condition *cond)
{
	if (cond)
		atomic_dec(&cond->head.users);
}

/* 
                                                        
  
                                                     
  
                   
 */
static inline void ccs_put_group(struct ccs_group *group)
{
	if (group)
		atomic_dec(&group->head.users);
}

/* 
                                                      
  
                                                        
  
                   
 */
static inline void ccs_put_name(const struct ccs_path_info *name)
{
	if (name)
		atomic_dec(&container_of(name, struct ccs_name, entry)->
			   head.users);
}

/*                                        */
extern const struct ccsecurity_exports ccsecurity_exports;

#ifdef CONFIG_CCSECURITY_USE_EXTERNAL_TASK_SECURITY

/*
                                                                             
                                                     
  
                                                                               
                                                                            
                                                                          
                       
  
                                                                             
                                                                               
                                                                       
 */
struct ccs_security {
	struct list_head list;
	const struct task_struct *task;
	struct ccs_domain_info *ccs_domain_info;
	u32 ccs_flags;
	struct rcu_head rcu;
};

#define CCS_TASK_SECURITY_HASH_BITS 12
#define CCS_MAX_TASK_SECURITY_HASH (1u << CCS_TASK_SECURITY_HASH_BITS)
extern struct list_head ccs_task_security_list[CCS_MAX_TASK_SECURITY_HASH];

struct ccs_security *ccs_find_task_security(const struct task_struct *task);

/* 
                                                                       
  
                                                               
 */
static inline struct ccs_security *ccs_current_security(void)
{
	return ccs_find_task_security(current);
}

/* 
                                                                       
  
                                          
  
                                                                 
 */
static inline struct ccs_domain_info *ccs_task_domain(struct task_struct *task)
{
	struct ccs_domain_info *domain;
	rcu_read_lock();
	domain = ccs_find_task_security(task)->ccs_domain_info;
	rcu_read_unlock();
	return domain;
}

/* 
                                                                        
  
                                                                  
 */
static inline struct ccs_domain_info *ccs_current_domain(void)
{
	return ccs_find_task_security(current)->ccs_domain_info;
}

/* 
                                                   
  
                                          
  
                                      
 */
static inline u32 ccs_task_flags(struct task_struct *task)
{
	u32 ccs_flags;
	rcu_read_lock();
	ccs_flags = ccs_find_task_security(task)->ccs_flags;
	rcu_read_unlock();
	return ccs_flags;
}

/* 
                                                    
  
                                    
 */
static inline u32 ccs_current_flags(void)
{
	return ccs_find_task_security(current)->ccs_flags;
}

#else

/*
                                                                               
                                                                              
                                                                   
                                                                         
 */
#define ccs_security task_struct

/* 
                                                                      
  
                                          
  
                                            
 */
static inline struct ccs_security *ccs_find_task_security(struct task_struct *
							  task)
{
	return task;
}

/* 
                                                                       
  
                                                               
 */
static inline struct ccs_security *ccs_current_security(void)
{
	return ccs_find_task_security(current);
}

/* 
                                                                       
  
                                          
  
                                                                 
 */
static inline struct ccs_domain_info *ccs_task_domain(struct task_struct *task)
{
	struct ccs_domain_info *domain = task->ccs_domain_info;
	return domain ? domain : &ccs_kernel_domain;
}

/* 
                                                                        
  
                                                                  
  
                                                                           
                                                                  
                                                
 */
static inline struct ccs_domain_info *ccs_current_domain(void)
{
	struct task_struct *task = current;
	if (!task->ccs_domain_info)
		task->ccs_domain_info = &ccs_kernel_domain;
	return task->ccs_domain_info;
}

/* 
                                                   
  
                                          
  
                                      
 */
static inline u32 ccs_task_flags(struct task_struct *task)
{
	return ccs_find_task_security(task)->ccs_flags;
}

/* 
                                                    
  
                                    
 */
static inline u32 ccs_current_flags(void)
{
	return ccs_find_task_security(current)->ccs_flags;
}

#endif

/* 
                                                                                
  
                                                                       
 */
static inline struct ccs_policy_namespace *ccs_current_namespace(void)
{
	return ccs_current_domain()->ns;
}

#endif
