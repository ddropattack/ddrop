#!/bin/bash
# This script rebuilds the kvm kernel module (includes kvm_amd)
# And installs it to the module directory
# You still need to reload with `sudo modprobe -r kvm_amd kvm && sudo modprobe kvm_amd`
# for the changes to take effect

#abort on first error
set -e

#We want the version to match the one of the snp kernel, such that
# modules_install overwrites the existing modules directory with the
# new versions. This way modprobe will load the new versions.
# The AMD build scripts sets LOCALVERSION in the config file.
# If we dont set LOCALVERSION explictly, the build system will a a prefix
# to the new
EV=""
make -j $(nproc) LOCALVERSION=${EV} scripts
make -j $(nproc) LOCALVERSION=${EV} prepare
make -j $(nproc) LOCALVERSION=${EV} modules_prepare
make -j $(nproc) LOCALVERSION=${EV} M=arch/x86/kvm
sudo make LOCALVERSION=${EV} M=arch/x86/kvm modules_install
make -j $(nproc) LOCALVERSION="" M=drivers/crypto/ccp
sudo make LOCALVERSION=${EV} M=drivers/crypto/ccp modules_install
