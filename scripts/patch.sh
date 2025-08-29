#!/usr/bin/env bash
set -euo pipefail

# Find the first UnityPlayerActivity.smali under the decompiled package directory
target=$(find . -type f -name 'UnityPlayerActivity.smali' | head -n1 || true)

if [ -z "$target" ]; then
  echo "UnityPlayerActivity.smali not found, skipping patch"
  exit 0
fi

echo "Patching target: $target"

# If already patched (contains marker), skip
if grep -q -E 'Perseus|JMBQ' "$target"; then
  echo "Patch already present in $target, skipping"
  exit 0
fi

# Get the onCreate line content (strip leading line number '123:...')
oncreate=$(grep -n -m1 'onCreate' "$target" | sed 's/^[0-9]*:\(.*\)/\1/' || true)

if [ -z "$oncreate" ]; then
  echo "onCreate not found in $target, skipping"
  exit 0
fi

# Use awk to insert the native init method and the library load/invoke lines
awk -v o="$oncreate" '
BEGIN { done = 0 }
{
  if (!done && $0 == o) {
    # insert native method before the onCreate line
    print ".method private static native init(Landroid/content/Context;)V"
    print ".end method"
    print ""
    # print the original onCreate line
    print $0
    # insert the library load + init invocation immediately after
    print "    const-string v0, \"JMBQ\""
    print ""
    print "    invoke-static {v0}, Ljava/lang/System;->loadLibrary(Ljava/lang/String;)V"
    print ""
    print "    invoke-static {p0}, Lcom/unity3d/player/UnityPlayerActivity;->init(Landroid/content/Context;)V"
    done = 1
  } else {
    print $0
  }
}
' "$target" > "${target}.tmp" && mv "${target}.tmp" "$target"

echo "Patched $target successfully."
