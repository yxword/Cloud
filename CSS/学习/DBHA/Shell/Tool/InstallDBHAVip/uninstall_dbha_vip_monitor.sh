#!/bin/bash

shell_dir=$(cd "$(dirname "$0")"; pwd)

echo "uninstall from crontab: " "monitor_dbha_mapping_vip.sh.sh"

sed -i '/monitor_dbha_mapping_vip.sh/d' /var/spool/cron/root

service crond restart # >/dev/null 2>&1
