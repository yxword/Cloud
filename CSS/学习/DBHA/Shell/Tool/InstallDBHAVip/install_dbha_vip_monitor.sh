#!/bin/bash

shell_dir=$(cd "$(dirname "$0")"; pwd)

echo "install to crontab: " "*/1 * * * * ${shell_dir}/monitor_dbha_mapping_vip.sh"

echo "*/1 * * * * ${shell_dir}/monitor_dbha_mapping_vip.sh" >> /var/spool/cron/root

service crond restart # >/dev/null 2>&1
