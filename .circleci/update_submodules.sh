#/bin/bash

set -x
for d in external/*/ ; do
	if [[ ! "$d" =~ 'llvm-project/$' ]] ; then
		git submodule update --init $d
	fi
done
