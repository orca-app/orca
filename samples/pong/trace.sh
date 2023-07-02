#!/bin/bash

# You will need dtrace2spall installed:
#
#   go install github.com/bvisness/dtrace2spall@latest
#
# You will also need `$(go env GOPATH)/bin` on your PATH.

# Run DTrace and save to profile.dtrace
sudo dtrace -n 'profile-997 /pid == $target/ { @[timestamp, pid, tid, ustack(100)] = count(); }' \
    -x ustackframes=100 \
    -o profile.dtrace \
    -x aggsortkey -x aggsortkeypos=0 \
    -c ./Pong.app/Contents/MacOS/orca
# Convert to Spall and save to profile.spall
cat profile.dtrace | dtrace2spall --freq 997 -o profile.spall --fields=_,pid,tid
