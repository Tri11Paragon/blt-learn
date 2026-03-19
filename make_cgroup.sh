#!/usr/bin/env bash
set -euo pipefail

mkdir cgroup
sudo mount -t cgroup2 none cgroup

CGROUP_ROOT="$(pwd)/cgroup"
GROUP_NAME="blt_codes"
GROUP_PATH="${CGROUP_ROOT}/${GROUP_NAME}"
TARGET_USER="${SUDO_USER:-$USER}"

sudo mkdir -p "$GROUP_PATH"
echo 4G | sudo tee "${GROUP_PATH}/memory.max" >/dev/null
echo "+memory" | sudo tee "${GROUP_PATH}/cgroup.subtree_control"

sudo tee "${GROUP_PATH}/cgroup.procs"
sudo chown -R "$TARGET_USER":"$TARGET_USER" "$GROUP_PATH"
sudo chmod u+rwx "$GROUP_PATH"

echo "Created cgroup at $GROUP_PATH"
echo "Memory limit set to 4G"
echo "User $TARGET_USER can now write to cgroup.procs"