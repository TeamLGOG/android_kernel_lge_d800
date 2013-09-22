/*
 * security/ccsecurity/realpath.c
 *
 * Copyright (C) 2005-2012  NTT DATA CORPORATION
 *
 * Version: 1.8.3+   2012/05/05
 */

#include "internal.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36) && LINUX_VERSION_CODE < KERNEL_VERSION(3, 2, 0)
#include <linux/nsproxy.h>
#include <linux/mnt_namespace.h>
#endif

/*                                        */

#define SOCKFS_MAGIC 0x534F434B

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 5, 0)
#define s_fs_info u.generic_sbp
#endif

/*                                        */

/*                                                */

char *ccs_encode(const char *str);
char *ccs_encode2(const char *str, int str_len);
char *ccs_realpath(struct path *path);
const char *ccs_get_exe(void);
void ccs_fill_path_info(struct ccs_path_info *ptr);

static char *ccs_get_absolute_path(struct path *path, char * const buffer,
				   const int buflen);
static char *ccs_get_dentry_path(struct dentry *dentry, char * const buffer,
				 const int buflen);
static char *ccs_get_local_path(struct dentry *dentry, char * const buffer,
				const int buflen);
static char *ccs_get_socket_name(struct path *path, char * const buffer,
				 const int buflen);
static int ccs_const_part_length(const char *filename);

/*                                                */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 5, 0)

/* 
                                  
  
                                     
  
                                      
  
                                                
 */
static inline struct socket *SOCKET_I(struct inode *inode)
{
	return inode->i_sock ? &inode->u.socket_i : NULL;
}

#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)

/* 
                                                 
  
                   
 */
static inline void ccs_realpath_lock(void)
{
	/*                                      */
	/*                                        */
}

/* 
                                                      
  
                   
 */
static inline void ccs_realpath_unlock(void)
{
	/*                                          */
	/*                                        */
}

#elif LINUX_VERSION_CODE == KERNEL_VERSION(2, 6, 36)

/* 
                                                 
  
                   
 */
static inline void ccs_realpath_lock(void)
{
	spin_lock(&dcache_lock);
	/*                                        */
}

/* 
                                                      
  
                   
 */
static inline void ccs_realpath_unlock(void)
{
	/*                                          */
	spin_unlock(&dcache_lock);
}

#elif defined(D_PATH_DISCONNECT) && !defined(CONFIG_SUSE_KERNEL)

/* 
                                                 
  
                   
  
                                                                              
                                                                            
                                                                
  
                                                                              
                                                      
  
                                                                             
                                                                         
                                                                             
                                                                           
                                                                             
         
 */
static inline void ccs_realpath_lock(void)
{
	spin_lock(ccsecurity_exports.vfsmount_lock);
	spin_lock(&dcache_lock);
}

/* 
                                                      
  
                   
 */
static inline void ccs_realpath_unlock(void)
{
	spin_unlock(&dcache_lock);
	spin_unlock(ccsecurity_exports.vfsmount_lock);
}

#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2, 5, 0)

/* 
                                                 
  
                   
 */
static inline void ccs_realpath_lock(void)
{
	spin_lock(&dcache_lock);
	spin_lock(ccsecurity_exports.vfsmount_lock);
}

/* 
                                                      
  
                   
 */
static inline void ccs_realpath_unlock(void)
{
	spin_unlock(ccsecurity_exports.vfsmount_lock);
	spin_unlock(&dcache_lock);
}

#else

/* 
                                                 
  
                   
 */
static inline void ccs_realpath_lock(void)
{
	spin_lock(&dcache_lock);
}

/* 
                                                      
  
                   
 */
static inline void ccs_realpath_unlock(void)
{
	spin_unlock(&dcache_lock);
}

#endif

/*                                                */

/*                                               */

/* 
                                                                               
  
                                     
                                                 
                           
  
                                                          
  
                                                  
                                     
  
                                                      
 */
static char *ccs_get_absolute_path(struct path *path, char * const buffer,
				   const int buflen)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 2, 0)
	char *pos = ERR_PTR(-ENOMEM);
	if (buflen >= 256) {
		pos = ccsecurity_exports.d_absolute_path(path, buffer,
							 buflen - 1);
		if (!IS_ERR(pos) && *pos == '/' && pos[1]) {
			struct inode *inode = path->dentry->d_inode;
			if (inode && S_ISDIR(inode->i_mode)) {
				buffer[buflen - 2] = '/';
				buffer[buflen - 1] = '\0';
			}
		}
	}
	return pos;
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
	/*
                                                                       
                                                                  
                    
   
                                                                       
                                                                     
                                                                   
                                                                        
                                                                   
                                                                       
                                                                    
                                                          
  */
	char *pos = ERR_PTR(-ENOMEM);
	if (buflen >= 256) {
		static bool ccs_no_empty;
		if (!ccs_no_empty) {
			struct path root = { };
			pos = ccsecurity_exports.__d_path(path, &root, buffer,
							  buflen - 1);
		} else {
			pos = NULL;
		}
		if (!pos) {
			struct task_struct *task = current;
			struct path root;
			struct path tmp;
			spin_lock(&task->fs->lock);
			root.mnt = task->nsproxy->mnt_ns->root;
			root.dentry = root.mnt->mnt_root;
			path_get(&root);
			spin_unlock(&task->fs->lock);
			tmp = root;
			pos = ccsecurity_exports.__d_path(path, &tmp, buffer,
							  buflen - 1);
			path_put(&root);
			if (!pos)
				return ERR_PTR(-EINVAL);
			/*                                              */
			ccs_no_empty = true;
		}
		if (!IS_ERR(pos) && *pos == '/' && pos[1]) {
			struct inode *inode = path->dentry->d_inode;
			if (inode && S_ISDIR(inode->i_mode)) {
				buffer[buflen - 2] = '/';
				buffer[buflen - 1] = '\0';
			}
		}
	}
	return pos;
#else
	char *pos = buffer + buflen - 1;
	struct dentry *dentry = path->dentry;
	struct vfsmount *vfsmnt = path->mnt;
	const char *name;
	int len;

	if (buflen < 256)
		goto out;

	*pos = '\0';
	if (dentry->d_inode && S_ISDIR(dentry->d_inode->i_mode))
		*--pos = '/';
	for (;;) {
		struct dentry *parent;
		if (dentry == vfsmnt->mnt_root || IS_ROOT(dentry)) {
			if (vfsmnt->mnt_parent == vfsmnt)
				break;
			dentry = vfsmnt->mnt_mountpoint;
			vfsmnt = vfsmnt->mnt_parent;
			continue;
		}
		parent = dentry->d_parent;
		name = dentry->d_name.name;
		len = dentry->d_name.len;
		pos -= len;
		if (pos <= buffer)
			goto out;
		memmove(pos, name, len);
		*--pos = '/';
		dentry = parent;
	}
	if (*pos == '/')
		pos++;
	len = dentry->d_name.len;
	pos -= len;
	if (pos < buffer)
		goto out;
	memmove(pos, dentry->d_name.name, len);
	return pos;
out:
	return ERR_PTR(-ENOMEM);
#endif
}

/* 
                                                  
  
                                       
                                                 
                           
  
                                                          
  
                                        
  
                                                      
 */
static char *ccs_get_dentry_path(struct dentry *dentry, char * const buffer,
				 const int buflen)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 38)
	char *pos = ERR_PTR(-ENOMEM);
	if (buflen >= 256) {
		/*                                                      */
		pos = dentry_path_raw(dentry, buffer, buflen - 1);
		if (!IS_ERR(pos) && *pos == '/' && pos[1]) {
			struct inode *inode = dentry->d_inode;
			if (inode && S_ISDIR(inode->i_mode)) {
				buffer[buflen - 2] = '/';
				buffer[buflen - 1] = '\0';
			}
		}
	}
	return pos;
#else
	char *pos = buffer + buflen - 1;
	if (buflen < 256)
		return ERR_PTR(-ENOMEM);
	*pos = '\0';
	if (dentry->d_inode && S_ISDIR(dentry->d_inode->i_mode))
		*--pos = '/';
	spin_lock(&dcache_lock);
	while (!IS_ROOT(dentry)) {
		struct dentry *parent = dentry->d_parent;
		const char *name = dentry->d_name.name;
		const int len = dentry->d_name.len;
		pos -= len;
		if (pos <= buffer) {
			pos = ERR_PTR(-ENOMEM);
			break;
		}
		memmove(pos, name, len);
		*--pos = '/';
		dentry = parent;
	}
	spin_unlock(&dcache_lock);
	return pos;
#endif
}

/* 
                                                 
  
                                       
                                                 
                           
  
                                                          
 */
static char *ccs_get_local_path(struct dentry *dentry, char * const buffer,
				const int buflen)
{
	struct super_block *sb = dentry->d_sb;
	char *pos = ccs_get_dentry_path(dentry, buffer, buflen);
	if (IS_ERR(pos))
		return pos;
	/*                                                      */
	if (sb->s_magic == PROC_SUPER_MAGIC && *pos == '/') {
		char *ep;
		const pid_t pid = (pid_t) simple_strtoul(pos + 1, &ep, 10);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25)
		if (*ep == '/' && pid && pid ==
		    task_tgid_nr_ns(current, sb->s_fs_info)) {
			pos = ep - 5;
			if (pos < buffer)
				goto out;
			memmove(pos, "/self", 5);
		}
#else
		if (*ep == '/' && pid == ccs_sys_getpid()) {
			pos = ep - 5;
			if (pos < buffer)
				goto out;
			memmove(pos, "/self", 5);
		}
#endif
		goto prepend_filesystem_name;
	}
	/*                                          */
	if (!MAJOR(sb->s_dev))
		goto prepend_filesystem_name;
	{
		struct inode *inode = sb->s_root->d_inode;
		/*
                                                                 
               
   */
		if (inode->i_op && !inode->i_op->rename)
			goto prepend_filesystem_name;
	}
	/*                      */
	{
		char name[64];
		int name_len;
		const dev_t dev = sb->s_dev;
		name[sizeof(name) - 1] = '\0';
		snprintf(name, sizeof(name) - 1, "dev(%u,%u):", MAJOR(dev),
			 MINOR(dev));
		name_len = strlen(name);
		pos -= name_len;
		if (pos < buffer)
			goto out;
		memmove(pos, name, name_len);
		return pos;
	}
	/*                          */
prepend_filesystem_name:
	{
		const char *name = sb->s_type->name;
		const int name_len = strlen(name);
		pos -= name_len + 1;
		if (pos < buffer)
			goto out;
		memmove(pos, name, name_len);
		pos[name_len] = ':';
	}
	return pos;
out:
	return ERR_PTR(-ENOMEM);
}

/* 
                                                  
  
                                     
                                                 
                           
  
                      
 */
static char *ccs_get_socket_name(struct path *path, char * const buffer,
				 const int buflen)
{
	struct inode *inode = path->dentry->d_inode;
	struct socket *sock = inode ? SOCKET_I(inode) : NULL;
	struct sock *sk = sock ? sock->sk : NULL;
	if (sk) {
		snprintf(buffer, buflen, "socket:[family=%u:type=%u:"
			 "protocol=%u]", sk->sk_family, sk->sk_type,
			 sk->sk_protocol);
	} else {
		snprintf(buffer, buflen, "socket:[unknown]");
	}
	return buffer;
}

#define SOCKFS_MAGIC 0x534F434B

/* 
                                                                                       
  
                                   
  
                                                                      
  
                                                                        
                      
 */
char *ccs_realpath(struct path *path)
{
	char *buf = NULL;
	char *name = NULL;
	unsigned int buf_len = PAGE_SIZE / 2;
	struct dentry *dentry = path->dentry;
	struct super_block *sb;
	if (!dentry)
		return NULL;
	sb = dentry->d_sb;
	while (1) {
		char *pos;
		struct inode *inode;
		buf_len <<= 1;
		kfree(buf);
		buf = kmalloc(buf_len, CCS_GFP_FLAGS);
		if (!buf)
			break;
		/*                                           */
		buf[buf_len - 1] = '\0';
		/*                             */
		if (sb->s_magic == SOCKFS_MAGIC) {
			pos = ccs_get_socket_name(path, buf, buf_len - 1);
			goto encode;
		}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 22)
		/*                  */
		if (dentry->d_op && dentry->d_op->d_dname) {
			pos = dentry->d_op->d_dname(dentry, buf, buf_len - 1);
			goto encode;
		}
#endif
		inode = sb->s_root->d_inode;
		/*
                                                                
                                                                 
           
   */
		if (!path->mnt || (inode->i_op && !inode->i_op->rename))
			pos = ERR_PTR(-EINVAL);
		else {
			/*                                 */
			ccs_realpath_lock();
			pos = ccs_get_absolute_path(path, buf, buf_len - 1);
			ccs_realpath_unlock();
		}
		if (pos == ERR_PTR(-EINVAL))
			pos = ccs_get_local_path(path->dentry, buf,
						 buf_len - 1);
encode:
		if (IS_ERR(pos))
			continue;
		name = ccs_encode(pos);
		break;
	}
	kfree(buf);
	if (!name)
		ccs_warn_oom(__func__);
	return name;
}

/* 
                                                      
  
                                     
                                  
  
                                                                      
  
                                                                        
                      
 */
char *ccs_encode2(const char *str, int str_len)
{
	int i;
	int len = 0;
	const char *p = str;
	char *cp;
	char *cp0;
	if (!p)
		return NULL;
	for (i = 0; i < str_len; i++) {
		const unsigned char c = p[i];
		if (c == '\\')
			len += 2;
		else if (c > ' ' && c < 127)
			len++;
		else
			len += 4;
	}
	len++;
	/*                                  */
	cp = kzalloc(len + 10, CCS_GFP_FLAGS);
	if (!cp)
		return NULL;
	cp0 = cp;
	p = str;
	for (i = 0; i < str_len; i++) {
		const unsigned char c = p[i];
		if (c == '\\') {
			*cp++ = '\\';
			*cp++ = '\\';
		} else if (c > ' ' && c < 127) {
			*cp++ = c;
		} else {
			*cp++ = '\\';
			*cp++ = (c >> 6) + '0';
			*cp++ = ((c >> 3) & 7) + '0';
			*cp++ = (c & 7) + '0';
		}
	}
	return cp0;
}

/* 
                                                     
  
                                 
  
                                                                      
  
                                                                        
                      
 */
char *ccs_encode(const char *str)
{
	return str ? ccs_encode2(str, strlen(str)) : NULL;
}

/* 
                                                                                    
  
                                     
  
                                                             
 */
static int ccs_const_part_length(const char *filename)
{
	char c;
	int len = 0;
	if (!filename)
		return 0;
	while (1) {
		c = *filename++;
		if (!c)
			break;
		if (c != '\\') {
			len++;
			continue;
		}
		c = *filename++;
		switch (c) {
		case '\\':  /*      */
			len += 2;
			continue;
		case '0':   /*        */
		case '1':
		case '2':
		case '3':
			c = *filename++;
			if (c < '0' || c > '7')
				break;
			c = *filename++;
			if (c < '0' || c > '7')
				break;
			len += 4;
			continue;
		}
		break;
	}
	return len;
}

/* 
                                                               
  
                                                      
  
                                                
 */
void ccs_fill_path_info(struct ccs_path_info *ptr)
{
	const char *name = ptr->name;
	const int len = strlen(name);
	ptr->total_len = len;
	ptr->const_len = ccs_const_part_length(name);
	ptr->is_dir = len && (name[len - 1] == '/');
	ptr->is_patterned = (ptr->const_len < len);
	ptr->hash = full_name_hash(name, len);
}

/* 
                                                       
  
                                                                            
  
                                                           
                                       
 */
const char *ccs_get_exe(void)
{
	struct mm_struct *mm = current->mm;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 7, 0)
	struct vm_area_struct *vma;
#endif
	const char *cp = NULL;
	if (!mm)
		return NULL;
	down_read(&mm->mmap_sem);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 7, 0)
	if (mm->exe_file)
		cp = ccs_realpath(&mm->exe_file->f_path);
#else
	for (vma = mm->mmap; vma; vma = vma->vm_next) {
		if ((vma->vm_flags & VM_EXECUTABLE) && vma->vm_file) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 20)
			struct path path = { vma->vm_file->f_vfsmnt,
					     vma->vm_file->f_dentry };
			cp = ccs_realpath(&path);
#else
			cp = ccs_realpath(&vma->vm_file->f_path);
#endif
			break;
		}
	}
#endif
	up_read(&mm->mmap_sem);
	return cp;
}
