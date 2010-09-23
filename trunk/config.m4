dnl
dnl $Id: config.m4 2010-09-22 21:08:35 zhangsilly@gmail.com $
dnl

PHP_ARG_ENABLE(chunzhen_shm, whether to enable chunzhen_shm support,
[  --enable-chunzhen_shm           Enable chunzhen_shm support])

if test "$PHP_CHUNZHEN_SHM" != "no"; then
  AC_DEFINE(HAVE_CHUNZHEN_SHMLIB,1,[ ])
  PHP_NEW_EXTENSION(chunzhen_shm, chunzhen.cpp chunzhen_shm.c, $ext_shared)
fi
