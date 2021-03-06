/*
 * security/ccsecurity/load_policy.c
 *
 * Copyright (C) 2005-2012  NTT DATA CORPORATION
 *
 * Version: 1.8.3+   2012/05/05
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/binfmts.h>
#include <linux/sched.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 5, 0)
#include <linux/kmod.h>
/*
                                                                               
                                                                          
 */
#define __KERNEL_SYSCALLS__
#include <linux/unistd.h>
#else
#include <linux/fs.h>
#include <linux/namei.h>
#endif
#ifndef LOOKUP_POSITIVE
#define LOOKUP_POSITIVE 0
#endif

/*
                              
 */

#include <linux/ccsecurity.h>

/* 
                                            
  
                                          
  
             
 */
static int __init ccs_setup(char *str)
{
	if (!strcmp(str, "off"))
		ccsecurity_ops.disabled = 1;
	else if (!strcmp(str, "on"))
		ccsecurity_ops.disabled = 0;
	return 0;
}

__setup("ccsecurity=", ccs_setup);

#ifndef CONFIG_CCSECURITY_OMIT_USERSPACE_LOADER

/*                                                                        */
static const char *ccs_loader;

/* 
                                        
  
                                                                  
  
             
 */
static int __init ccs_loader_setup(char *str)
{
	ccs_loader = str;
	return 0;
}

__setup("CCS_loader=", ccs_loader_setup);

/* 
                                                                  
  
                                                          
 */
static _Bool ccs_policy_loader_exists(void)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 28)
	struct path path;
	if (!ccs_loader)
		ccs_loader = CONFIG_CCSECURITY_POLICY_LOADER;
	if (kern_path(ccs_loader, LOOKUP_FOLLOW | LOOKUP_POSITIVE,
		      &path) == 0) {
		path_put(&path);
		return 1;
	}
#else
	struct nameidata nd;
	if (!ccs_loader)
		ccs_loader = CONFIG_CCSECURITY_POLICY_LOADER;
	if (path_lookup(ccs_loader, LOOKUP_FOLLOW | LOOKUP_POSITIVE,
			&nd) == 0) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25)
		path_put(&nd.path);
#else
		path_release(&nd);
#endif
		return 1;
	}
#endif
	printk(KERN_INFO "Not activating Mandatory Access Control "
	       "as %s does not exist.\n", ccs_loader);
	return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 5, 0)

/* 
                                         
  
                     
  
                                                                      
 */
static int ccs_run_loader(void *unused)
{
	char *argv[2];
	char *envp[3];
	printk(KERN_INFO "Calling %s to load policy. Please wait.\n",
	       ccs_loader);
	argv[0] = (char *) ccs_loader;
	argv[1] = NULL;
	envp[0] = "HOME=/";
	envp[1] = "PATH=/sbin:/bin:/usr/sbin:/usr/bin";
	envp[2] = NULL;
	return exec_usermodehelper(argv[0], argv, envp);
}

#endif

/*                                                                       */
static const char *ccs_trigger;

/* 
                                                  
  
                                                                    
  
             
 */
static int __init ccs_trigger_setup(char *str)
{
	ccs_trigger = str;
	return 0;
}

__setup("CCS_trigger=", ccs_trigger_setup);

/* 
                                                               
  
                                         
  
                   
  
                                                                  
                                                                       
                                               
                                                               
                                   
 */
static void ccs_load_policy(const char *filename)
{
	static _Bool done;
	if (ccsecurity_ops.disabled || done)
		return;
	if (!ccs_trigger)
		ccs_trigger = CONFIG_CCSECURITY_ACTIVATION_TRIGGER;
	if (strcmp(filename, ccs_trigger))
		return;
	if (!ccs_policy_loader_exists())
		return;
	done = 1;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 5, 0)
	{
		char *argv[2];
		char *envp[3];
		printk(KERN_INFO "Calling %s to load policy. Please wait.\n",
		       ccs_loader);
		argv[0] = (char *) ccs_loader;
		argv[1] = NULL;
		envp[0] = "HOME=/";
		envp[1] = "PATH=/sbin:/bin:/usr/sbin:/usr/bin";
		envp[2] = NULL;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 23) || defined(UMH_WAIT_PROC)
		call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);
#else
		call_usermodehelper(argv[0], argv, envp, 1);
#endif
	}
#elif defined(TASK_DEAD)
	{
		/*                           */
		struct task_struct *task = current;
		pid_t pid = kernel_thread(ccs_run_loader, NULL, 0);
		sigset_t tmpsig;
		spin_lock_irq(&task->sighand->siglock);
		tmpsig = task->blocked;
		siginitsetinv(&task->blocked,
			      sigmask(SIGKILL) | sigmask(SIGSTOP));
		recalc_sigpending();
		spin_unlock_irq(&task->sighand->siglock);
		if (pid >= 0)
			waitpid(pid, NULL, __WCLONE);
		spin_lock_irq(&task->sighand->siglock);
		task->blocked = tmpsig;
		recalc_sigpending();
		spin_unlock_irq(&task->sighand->siglock);
	}
#else
	{
		/*                           */
		struct task_struct *task = current;
		pid_t pid = kernel_thread(ccs_run_loader, NULL, 0);
		sigset_t tmpsig;
		spin_lock_irq(&task->sigmask_lock);
		tmpsig = task->blocked;
		siginitsetinv(&task->blocked,
			      sigmask(SIGKILL) | sigmask(SIGSTOP));
		recalc_sigpending(task);
		spin_unlock_irq(&task->sigmask_lock);
		if (pid >= 0)
			waitpid(pid, NULL, __WCLONE);
		spin_lock_irq(&task->sigmask_lock);
		task->blocked = tmpsig;
		recalc_sigpending(task);
		spin_unlock_irq(&task->sigmask_lock);
	}
#endif
	if (ccsecurity_ops.check_profile)
		ccsecurity_ops.check_profile();
	else
		panic("Failed to load policy.");
}

#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0)

/* 
                                                                                    
  
                                           
  
                                                  
 */
static int __ccs_search_binary_handler(struct linux_binprm *bprm)
{
#ifndef CONFIG_CCSECURITY_OMIT_USERSPACE_LOADER
	ccs_load_policy(bprm->filename);
#endif
	/*
                                                                  
                                                                   
                                                                    
                                                                 
                                                                     
                                            
  */
	if (ccsecurity_ops.search_binary_handler
	    != __ccs_search_binary_handler)
		return ccsecurity_ops.search_binary_handler(bprm);
	return search_binary_handler(bprm);
}

#else

/* 
                                                                                    
  
                                           
                                      
  
                                                  
 */
static int __ccs_search_binary_handler(struct linux_binprm *bprm,
				       struct pt_regs *regs)
{
#ifndef CONFIG_CCSECURITY_OMIT_USERSPACE_LOADER
	ccs_load_policy(bprm->filename);
#endif
	/*
                                                                  
                                                                   
                                                                    
                                                                 
                                                                     
                                            
  */
	if (ccsecurity_ops.search_binary_handler
	    != __ccs_search_binary_handler)
		return ccsecurity_ops.search_binary_handler(bprm, regs);
	return search_binary_handler(bprm, regs);
}

#endif

/*
                                                
  
                                                                            
                                                                            
                                                       
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 5, 0) && LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 35)
extern spinlock_t vfsmount_lock;
#endif

/*                                        */
const struct ccsecurity_exports ccsecurity_exports = {
#ifndef CONFIG_CCSECURITY_OMIT_USERSPACE_LOADER
	.load_policy = ccs_load_policy,
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 2, 0)
	.d_absolute_path = d_absolute_path,
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
	.__d_path = __d_path,
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2, 5, 0)
	.vfsmount_lock = &vfsmount_lock,
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24)
	.find_task_by_vpid = find_task_by_vpid,
	.find_task_by_pid_ns = find_task_by_pid_ns,
#endif
};
#ifdef CONFIG_CCSECURITY_LKM
/*                                                    */
EXPORT_SYMBOL_GPL(ccsecurity_exports);
#endif

/*                                                */
struct ccsecurity_operations ccsecurity_ops = {
	.search_binary_handler = __ccs_search_binary_handler,
#ifdef CONFIG_CCSECURITY_DISABLE_BY_DEFAULT
	.disabled = 1,
#endif
};
/*
 * Non-GPL modules might need to access this struct via inlined functions
 * embedded into include/linux/security.h and include/net/ip.h
 */
EXPORT_SYMBOL(ccsecurity_ops);
