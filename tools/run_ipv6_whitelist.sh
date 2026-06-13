#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 2 ]]; then
  echo "Usage: $0 <executable> <ipv6-dst-prefix> [args...]" >&2
  exit 1
fi

if [[ $EUID -ne 0 ]]; then
  echo "This script must be run as root (ip6tables)." >&2
  exit 1
fi

exe="$1"
shift
ipv6_dst="$1"
shift

if [[ ! -x "$exe" ]]; then
  echo "Executable not found or not executable: $exe" >&2
  exit 1
fi

original_policy="$(ip6tables -S OUTPUT | awk 'NR==1 {print $3}')"
rule_added=false
policy_changed=false
cleanup_done=false

cleanup() {
  if $cleanup_done; then
    return 0
  fi
  cleanup_done=true
  if $rule_added; then
    ip6tables -D OUTPUT -d "$ipv6_dst" -j ACCEPT || true
  fi
  if $policy_changed; then
    ip6tables -P OUTPUT "$original_policy" || true
  fi
}
trap cleanup EXIT INT TERM

# Change policy to DROP and add exception rule
if ! ip6tables -P OUTPUT DROP; then
  echo "Failed to set OUTPUT policy to DROP" >&2
  exit 1
fi
policy_changed=true

if ! ip6tables -A OUTPUT -d "$ipv6_dst" -j ACCEPT; then
  echo "Failed to add OUTPUT rule" >&2
  exit 1
fi
rule_added=true

# Execute the wrapped program
"$exe" "$@"
