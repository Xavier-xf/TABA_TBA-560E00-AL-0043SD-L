#!/bin/sh

install()
{
    modprobe sunrpc
    modprobe grace
#    modprobe lockd
    modprobe nfs
    modprobe nfsv2
    modprobe nfsv3
}

uninstall()
{
    modprobe -r sunrpc
    modprobe -r grace
#    modprobe -r lockd
    modprobe -r nfs
    modprobe -r nfsv2
    modprobe -r nfsv3
}

usage()
{
    echo "usage :" 
    echo "    ./nfs_start.sh <install/uninstall>"
    echo "    default : install"

}

if test $# -gt 1 ; then
    usage
elif test $# -eq 0 ; then
    install
elif test $# -eq 1 ; then
    if test $1 = "install" ; then
        install
    elif test $1 = "uninstall" ; then
        uninstall
    else
        usage
    fi
fi

