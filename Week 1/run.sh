#!/usr/bin/env bash
set -u
cd "$(dirname "$0")"

# Remove stale queue names from a previous forced/abnormal run, if the utility exists.
if command -v python3 >/dev/null 2>&1; then
python3 - <<'PY'
import ctypes
try:
    librt = ctypes.CDLL("librt.so.1", use_errno=True)
    unlink = librt.mq_unlink
    unlink.argtypes = [ctypes.c_char_p]
    for name in (b"/osdsa_cmd_q", b"/osdsa_resp_q", b"/osdsa_log_q"):
        unlink(name)
except Exception:
    pass
PY
fi

./logger & logger_pid=$!
sleep 0.2
./core & core_pid=$!
sleep 0.2
./ui
ui_status=$?
wait "$core_pid" 2>/dev/null || true
wait "$logger_pid" 2>/dev/null || true
exit "$ui_status"
