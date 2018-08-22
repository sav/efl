#!/bin/sh

set -e
. .ci/travis.sh
if [ "$1" = "release-ready" ] ; then
  exit 0
fi
travis_fold make make
if [ "$DISTRO" != "" ] ; then
  docker exec --env MAKEFLAGS="-j5 -rR" --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) git clone https://github.com/vtorri/ewpi.git
  docker exec --env MAKEFLAGS="-j5 -rR" --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) cd ewpi
  docker exec --env MAKEFLAGS="-j5 -rR" --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) gcc -o ewpi ewpi.c ewpi_map.c $(pkg-config --cflags --libs libcurl)
  docker exec --env MAKEFLAGS="-j5 -rR" --env EIO_MONITOR_POLL=1 $(cat $HOME/cid) ./ewpi $HOME/EFL/ewpi_64 x86_64-w64-mingw32
else
  export PATH="$(brew --prefix gettext)/bin:$PATH"
  make
fi
travis_endfold make
