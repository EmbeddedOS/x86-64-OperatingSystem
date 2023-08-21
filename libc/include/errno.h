/**
 * @file    errno.h
 * @author  Cong Nguyen (congnt264@gmail.com)
 * @brief   These error codes are used for system calls, user can map the error
 *          code to check system call error code meaning.
 * 
 * @version 0.1
 * @date 2023-08-21
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#pragma once

/* Public define -------------------------------------------------------------*/

#define EPERM           1    /* The operation is not permitted.               */
#define ENOENT          2    /* There is no such file or directory exists.    */
#define ESRCH           3    /* There is no such process exists.              */
#define EINTR           4    /* Interrupted system call                       */
#define EIO             5    /* Input/output error.                           */
#define ENXIO           6    /* There is no such device or address exists.    */
#define E2BIG           7    /* Argument list is too long.                    */
#define ENOEXEC         8    /* There is an exec format error                 */
#define EBADF           9    /* Bad file descriptor.                          */
#define ECHILD          10   /* There is no child process exists.             */
#define EAGAIN          11   /* Resource is temporarily unavailable.          */
#define ENOMEM          12   /* The system cannot allocate memory.            */
#define EACCES          13   /* Permission is denied.                         */
#define EFAULT          14   /* There is a bad address.                       */
#define ENOTBLK         15   /* Block device is required.                     */
#define EBUSY           16   /* Device or resource is busy.                   */
#define EEXIST          17   /* File already exists.                          */
#define EXDEV           18   /* There is invalid cross-device link.           */
#define ENODEV          19   /* There is no such device.                      */
#define ENOTDIR         20   /* There is not a directory.                     */
#define EISDIR          21   /* There is a directory.                         */
#define EINVAL          22   /* There is an invalid argument.                 */
#define ENFILE          23   /* There are too many open files in system.      */
#define EMFILE          24   /* There are too many open files.                */
#define ENOTTY          25   /* There is an inappropriate ioctl for device.   */
#define ETXTBSY         26   /* Text file is busy.                            */
#define EFBIG           27   /* The file is too large.                        */
#define ENOSPC          28   /* There is no space left on device.             */
#define ESPIPE          29   /* Illegal seek.                                 */
#define EROFS           30   /* Read-only file system.                        */
#define EMLINK          31   /* There are too many links.                     */
#define EPIPE           32   /* Broken pipe.                                  */
#define EDOM            33   /* Numerical argument is out of domain.          */
#define ERANGE          34   /* Numerical result is out of range.             */
#define EDEADLK         35   /* Resource deadlock is avoided.                 */
#define ENAMETOOLONG    36   /* File name is too long.                        */
#define ENOLCK          37   /* No locks are available.                       */
#define ENOSYS          38   /* Function is not implemented.                  */
#define ENOTEMPTY       39   /* Directory is not empty.                       */
#define ELOOP           40   /* There are too many levels of symbolic links.  */
#define ENOMSG          42   /* There is no message of desired type.          */
#define EIDRM           43   /* Identifier is removed.                        */
#define ECHRNG          44   /* Channel number is out of range.               */
#define EL2NSYNC        45   /* Level 2 is not synchronized.                  */
#define EL3HLT          46   /* Level 3 is halted.                            */
#define EL3RST          47   /* Level 3 is reset.                             */
#define ELNRNG          48   /* The link number is out of range.              */
#define EUNATCH         49   /* Protocol driver is not attached.              */
#define ENOCSI          50   /* There is no CSI structure available.          */
#define EL2HLT          51   /* Level 2 is halted.                            */
#define EBADE           52   /* Invalid exchange.                             */
#define EBADR           53   /* Invalid request descriptor.                   */
#define EXFULL          54   /* Exchange is full.                             */
#define ENOANO          55   /* No anode.                                     */
#define EBADRQC         56   /* Invalid request code.                         */
#define EBADSLT         57   /* Invalid slot.                                 */
#define EBFONT          59   /* There is bad font file format.                */
#define ENOSTR          60   /* Device is not a stream.                       */
#define ENODATA         61   /* There is no data available.                   */
#define ETIME           62   /* Timer is expired.                             */
#define ENOSR           63   /* Out of streams resources.                     */
#define ENONET          64   /* Machine is not on the network.                */
#define ENOPKG          65   /* Package is not installed.                     */
#define EREMOTE         66   /* Object is remote.                             */
#define ENOLINK         67   /* The link has been severed.                    */
#define EADV            68   /* Advertise error.                              */
#define ESRMNT          69   /* Srmount error.                                */
#define ECOMM           70   /* There is communication error on send.         */
#define EPROTO          71   /* Protocol error.                               */
#define EMULTIHOP       72   /* Multihop attempted.                           */
#define EDOTDOT         73   /* RFS specific error.                           */
#define EBADMSG         74   /* Bad message.                                  */
#define EOVERFLOW       75   /* Value is too large for defined data type.     */
#define ENOTUNIQ        76   /* Name is not unique on network.                */
#define EBADFD          77   /* File descriptor is in bad state.              */
#define EREMCHG         78   /* Remote address is changed.                    */
#define ELIBACC         79   /* We cannot access a needed shared library.     */
#define ELIBBAD         80   /* Accessing a corrupted shared library.         */
#define ELIBSCN         81   /* There is .lib section in a.out is corrupted.  */
#define ELIBMAX         82   /* Attempt to link in too many shared libraries. */
#define ELIBEXEC        83   /* we cannot exec a shared library directly.     */
#define EILSEQ          84   /* An invalid or incomplete multibyte.           */
#define ERESTART        85   /* Interrupted system call should be restarted.  */
#define ESTRPIPE        86   /* There is streams pipe error.                  */
#define EUSERS          87   /* Too many users.                               */
#define ENOTSOCK        88   /* Socket operation on non-socket.               */
#define EDESTADDRREQ    89   /* The destination address is required.          */
#define EMSGSIZE        90   /* Message is too long.                          */
#define EPROTOTYPE      91   /* The protocol is wrong type for socket.        */
#define ENOPROTOOPT     92   /* The protocol is not available.                */
#define EPROTONOSUPPORT 93   /* Protocol is not supported.                    */
#define ESOCKTNOSUPPORT 94   /* The Socket type is not supported.             */
#define EOPNOTSUPP      95   /* The Operation is not supported.               */
#define EPFNOSUPPORT    96   /* The protocol family is not supported.         */
#define EAFNOSUPPORT    97   /* Address family is not supported by protocol.  */
#define EADDRINUSE      98   /* The address is already in use.                */
#define EADDRNOTAVAIL   99   /* The system cannot assign requested address.   */
#define ENETDOWN        100  /* Network is down.                              */
#define ENETUNREACH     101  /* Network is unreachable.                       */
#define ENETRESET       102  /* Network has dropped connection on reset.      */
#define ECONNABORTED    103  /* Software caused connection abort.             */
#define ECONNRESET      104  /* Connection is reset by peer.                  */
#define ENOBUFS         105  /* There is no buffer space available.           */
#define EISCONN         106  /* The transport endpoint is already connected.  */
#define ENOTCONN        107  /* The transport endpoint is not connected.      */
#define ESHUTDOWN       108  /* System can't send after the endpoint shutdown.*/
#define ETOOMANYREFS    109  /* There are too many references: cannot splice. */
#define ETIMEDOUT       110  /* Connection is timed out.                      */
#define ECONNREFUSED    111  /* Connection is refused.                        */
#define EHOSTDOWN       112  /* The Host is down.                             */
#define EHOSTUNREACH    113  /* There is no route to host.                    */
#define EALREADY        114  /* Operation is already in progress.             */
#define EINPROGRESS     115  /* The operation is now in progress.             */
#define ESTALE          116  /* Stale file handle.                            */
#define EUCLEAN         117  /* Structure needs cleaning.                     */
#define ENOTNAM         118  /* Not a XENIX named type file.                  */
#define ENAVAIL         119  /* There are no XENIX semaphores are available.  */
#define EISNAM          120  /* There is a named type file.                   */
#define EREMOTEIO       121  /* Remote I/O error.                             */
#define EDQUOT          122  /* Disk quota is exceeded.                       */
#define ENOMEDIUM       123  /* There is no medium found.                     */
#define EMEDIUMTYPE     124  /* There is wrong medium type.                   */
#define ECANCELED       125  /* Operation is cancelled.                       */
#define ENOKEY          126  /* Required key is not available.                */
#define EKEYEXPIRED     127  /* The key has expired.                          */
#define EKEYREVOKED     128  /* The key has been revoked.                     */
#define EKEYREJECTED    129  /* The key was rejected by service.              */
#define EOWNERDEAD      130  /* The Owner is died.                            */
#define ENOTRECOVERABLE 131  /* No recoverable state.                         */
#define ERFKILL         132  /* The operation is not possible due to RF-kill. */
#define EHWPOISON       133  /* The memory page has hardware error.           */