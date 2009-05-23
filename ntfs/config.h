#define HAVE_STRING_H
#define _LITTLE_ENDIAN
#define __attribute__(x) /*nothing*/
#define inline __inline
#define __inline__ __inline
#define HAVE_ERRNO_H
#define EOPNOTSUPP EINVAL
#define snprintf _snprintf
#define strtoll(ptr, eptr, base) _atoi64(ptr)
//#define ntfs_log_redirect() /*nothing*/
//#define ntfs_malloc malloc
//#define ntfs_calloc(size) calloc(1,size)
#define HAVE_STDINT_H 1
#define non_resident_end compressed_size
#define HAVE_FCNTL_H
#define EOVERFLOW EINVAL
#define ENOMSG EINVAL
#define HAVE_STDLIB_H
#define HAVE_STDIO_H
#define HAVE_CTYPE_H
extern int ffs(int i);


/* Encoding of the file mode.  */

#define __S_IFMT        0170000 /* These bits determine file type.  */

/* File types.  */
#define S_IFDIR       0040000 /* Directory.  */
#define S_IFCHR       0020000 /* Character device.  */
#define S_IFBLK       0060000 /* Block device.  */
#define S_IFREG       0100000 /* Regular file.  */
#define S_IFIFO       0010000 /* FIFO.  */
#define S_IFLNK       0120000 /* Symbolic link.  */
#define S_IFSOCK      0140000 /* Socket.  */

#define __S_ISTYPE(mode, mask)  (((mode) & __S_IFMT) == (mask))

#define S_ISDIR(mode)    __S_ISTYPE((mode), S_IFDIR)
#define S_ISCHR(mode)    __S_ISTYPE((mode), S_IFCHR)
#define S_ISBLK(mode)    __S_ISTYPE((mode), S_IFBLK)
#define S_ISREG(mode)    __S_ISTYPE((mode), S_IFREG)

#define major(dev) ((int)(((dev) >> 8) & 0xff))
#define minor(dev) ((int)(((dev)     ) & 0xff))

#define __CYGWIN32__
#define HAVE_WINDOWS_H
#define EBADRQC EINVAL
#define ENOSHARE EINVAL
#define O_ACCMODE	   0003
#define ENOBUFS EINVAL
typedef unsigned int mode_t;
#undef UNICODE

#define FAILED 0
#define OK 0