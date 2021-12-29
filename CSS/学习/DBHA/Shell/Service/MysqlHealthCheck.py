#/usr/bin/env python
#coding=utf-8
# Requires "pymysql" for Python
#
import re
import logging
from logging import handlers
import time
import sys

def init_logging(logFilePath = None):
	logger = logging.getLogger('mysql_health_check')
	logger.setLevel(logging.DEBUG)
	formatter = logging.Formatter('%(asctime)s [%(levelname)s] %(name)s::%(funcName)s::%(lineno)d %(message)s')

	if logFilePath:
		file_handler = logging.handlers.RotatingFileHandler(logFilePath, maxBytes=100*1024*1024, backupCount=4)
		file_handler.setLevel(logging.INFO)
		file_handler.suffix = "%Y-%m-%d_%H-%M-%S.log"
		file_handler.setFormatter(formatter)
		logger.addHandler(file_handler)
	stream_handler = logging.StreamHandler(sys.stdout)
	stream_handler.setLevel(logging.DEBUG)
	stream_handler.setFormatter(formatter)
	logger.addHandler(stream_handler)
	
	return logger

MYSQL_CONFIG = {
	'Host':           'localhost',
	'Port':           3306,
	'User':           'root',
	'Password':       '',
	'unix_socket':    '/tmp/mysql.sock',
	'HeartbeatTable': '',
	'Verbose':        False,
}

MYSQL_STATUS_VARS = {
	'Aborted_clients': 'counter',
	'Aborted_connects': 'counter',
	'Binlog_cache_disk_use': 'counter',
	'Binlog_cache_use': 'counter',
	'Bytes_received': 'counter',
	'Bytes_sent': 'counter',
	'Connections': 'counter',
	'Created_tmp_disk_tables': 'counter',
	'Created_tmp_files': 'counter',
	'Created_tmp_tables': 'counter',
	'Innodb_buffer_pool_pages_data': 'gauge',
	'Innodb_buffer_pool_pages_dirty': 'gauge',
	'Innodb_buffer_pool_pages_free': 'gauge',
	'Innodb_buffer_pool_pages_total': 'gauge',
	'Innodb_buffer_pool_read_requests': 'counter',
	'Innodb_buffer_pool_reads': 'counter',
	'Innodb_checkpoint_age': 'gauge',
	'Innodb_checkpoint_max_age': 'gauge',
	'Innodb_data_fsyncs': 'counter',
	'Innodb_data_pending_fsyncs': 'gauge',
	'Innodb_data_pending_reads': 'gauge',
	'Innodb_data_pending_writes': 'gauge',
	'Innodb_data_read': 'counter',
	'Innodb_data_reads': 'counter',
	'Innodb_data_writes': 'counter',
	'Innodb_data_written': 'counter',
	'Innodb_deadlocks': 'counter',
	'Innodb_history_list_length': 'gauge',
	'Innodb_ibuf_free_list': 'gauge',
	'Innodb_ibuf_merged_delete_marks': 'counter',
	'Innodb_ibuf_merged_deletes': 'counter',
	'Innodb_ibuf_merged_inserts': 'counter',
	'Innodb_ibuf_merges': 'counter',
	'Innodb_ibuf_segment_size': 'gauge',
	'Innodb_ibuf_size': 'gauge',
	'Innodb_lsn_current': 'counter',
	'Innodb_lsn_flushed': 'counter',
	'Innodb_max_trx_id': 'counter',
	'Innodb_mem_adaptive_hash': 'gauge',
	'Innodb_mem_dictionary': 'gauge',
	'Innodb_mem_total': 'gauge',
	'Innodb_mutex_os_waits': 'counter',
	'Innodb_mutex_spin_rounds': 'counter',
	'Innodb_mutex_spin_waits': 'counter',
	'Innodb_os_log_pending_fsyncs': 'gauge',
	'Innodb_pages_created': 'counter',
	'Innodb_pages_read': 'counter',
	'Innodb_pages_written': 'counter',
	'Innodb_row_lock_time': 'counter',
	'Innodb_row_lock_time_avg': 'gauge',
	'Innodb_row_lock_time_max': 'gauge',
	'Innodb_row_lock_waits': 'counter',
	'Innodb_rows_deleted': 'counter',
	'Innodb_rows_inserted': 'counter',
	'Innodb_rows_read': 'counter',
	'Innodb_rows_updated': 'counter',
	'Innodb_s_lock_os_waits': 'counter',
	'Innodb_s_lock_spin_rounds': 'counter',
	'Innodb_s_lock_spin_waits': 'counter',
	'Innodb_uncheckpointed_bytes': 'gauge',
	'Innodb_unflushed_log': 'gauge',
	'Innodb_unpurged_txns': 'gauge',
	'Innodb_x_lock_os_waits': 'counter',
	'Innodb_x_lock_spin_rounds': 'counter',
	'Innodb_x_lock_spin_waits': 'counter',
	'Key_blocks_not_flushed': 'gauge',
	'Key_blocks_unused': 'gauge',
	'Key_blocks_used': 'gauge',
	'Key_read_requests': 'counter',
	'Key_reads': 'counter',
	'Key_write_requests': 'counter',
	'Key_writes': 'counter',
	'Max_used_connections': 'gauge',
	'Open_files': 'gauge',
	'Open_table_definitions': 'gauge',
	'Open_tables': 'gauge',
	'Opened_files': 'counter',
	'Opened_table_definitions': 'counter',
	'Opened_tables': 'counter',
	'Qcache_free_blocks': 'gauge',
	'Qcache_free_memory': 'gauge',
	'Qcache_hits': 'counter',
	'Qcache_inserts': 'counter',
	'Qcache_lowmem_prunes': 'counter',
	'Qcache_not_cached': 'counter',
	'Qcache_queries_in_cache': 'counter',
	'Qcache_total_blocks': 'counter',
	'Questions': 'counter',
	'Select_full_join': 'counter',
	'Select_full_range_join': 'counter',
	'Select_range': 'counter',
	'Select_range_check': 'counter',
	'Select_scan': 'counter',
	'Slave_open_temp_tables': 'gauge',
	'Slave_retried_transactions': 'counter',
	'Slow_launch_threads': 'counter',
	'Slow_queries': 'counter',
	'Sort_merge_passes': 'counter',
	'Sort_range': 'counter',
	'Sort_rows': 'counter',
	'Sort_scan': 'counter',
	'Table_locks_immediate': 'counter',
	'Table_locks_waited': 'counter',
	'Table_open_cache_hits': 'counter',
	'Table_open_cache_misses': 'counter',
	'Table_open_cache_overflows': 'counter',
	'Threadpool_idle_threads': 'gauge',
	'Threadpool_threads': 'gauge',
	'Threads_cached': 'gauge',
	'Threads_connected': 'gauge',
	'Threads_created': 'counter',
	'Threads_running': 'gauge',
	'Uptime': 'gauge',
	'wsrep_apply_oooe': 'gauge',
	'wsrep_apply_oool': 'gauge',
	'wsrep_apply_window': 'gauge',
	'wsrep_causal_reads': 'gauge',
	'wsrep_cert_deps_distance': 'gauge',
	'wsrep_cert_index_size': 'gauge',
	'wsrep_cert_interval': 'gauge',
	'wsrep_cluster_size': 'gauge',
	'wsrep_commit_oooe': 'gauge',
	'wsrep_commit_oool': 'gauge',
	'wsrep_commit_window': 'gauge',
	'wsrep_flow_control_paused': 'gauge',
	'wsrep_flow_control_paused_ns': 'counter',
	'wsrep_flow_control_recv': 'counter',
	'wsrep_flow_control_sent': 'counter',
	'wsrep_local_bf_aborts': 'counter',
	'wsrep_local_cert_failures': 'counter',
	'wsrep_local_commits': 'counter',
	'wsrep_local_recv_queue': 'gauge',
	'wsrep_local_recv_queue_avg': 'gauge',
	'wsrep_local_recv_queue_max': 'gauge',
	'wsrep_local_recv_queue_min': 'gauge',
	'wsrep_local_replays': 'gauge',
	'wsrep_local_send_queue': 'gauge',
	'wsrep_local_send_queue_avg': 'gauge',
	'wsrep_local_send_queue_max': 'gauge',
	'wsrep_local_send_queue_min': 'gauge',
	'wsrep_received': 'counter',
	'wsrep_received_bytes': 'counter',
	'wsrep_repl_data_bytes': 'counter',
	'wsrep_repl_keys': 'counter',
	'wsrep_repl_keys_bytes': 'counter',
	'wsrep_repl_other_bytes': 'counter',
	'wsrep_replicated': 'counter',
	'wsrep_replicated_bytes': 'counter',
}

MYSQL_VARS = [
	'binlog_stmt_cache_size',
	'innodb_additional_mem_pool_size',
	'innodb_buffer_pool_size',
	'innodb_concurrency_tickets',
	'innodb_io_capacity',
	'innodb_log_buffer_size',
	'innodb_log_file_size',
	'innodb_open_files',
	'innodb_open_files',
	'join_buffer_size',
	'max_connections',
	'open_files_limit',
	'query_cache_limit',
	'query_cache_size',
	'query_cache_size',
	'read_buffer_size',
	'table_cache',
	'table_definition_cache',
	'table_open_cache',
	'thread_cache_size',
	'thread_cache_size',
	'thread_concurrency',
	'tmp_table_size',
]

MYSQL_PROCESS_STATES = {
	'closing_tables': 0,
	'copying_to_tmp_table': 0,
	'end': 0,
	'freeing_items': 0,
	'init': 0,
	'locked': 0,
	'login': 0,
	'none': 0,
	'other': 0,
	'preparing': 0,
	'reading_from_net': 0,
	'sending_data': 0,
	'sorting_result': 0,
	'statistics': 0,
	'updating': 0,
	'writing_to_net': 0,
	'creating_table': 0,
	'opening_tables': 0,
	'longest_wait_time': 0,
	'longest_wait_command': ''
}

MYSQL_INNODB_STATUS_VARS = {
	'active_transactions': 'gauge',
	'current_transactions': 'gauge',
	'file_reads': 'counter',
	'file_system_memory': 'gauge',
	'file_writes': 'counter',
	'innodb_lock_structs': 'gauge',
	'innodb_lock_wait_secs': 'gauge',
	'innodb_locked_tables': 'gauge',
	'innodb_sem_wait_time_ms': 'gauge',
	'innodb_sem_waits': 'gauge',
	'innodb_tables_in_use': 'gauge',
	'lock_system_memory': 'gauge',
	'locked_transactions': 'gauge',
	'log_writes': 'counter',
	'page_hash_memory': 'gauge',
	'pending_aio_log_ios': 'gauge',
	'pending_buf_pool_flushes': 'gauge',
	'pending_chkp_writes': 'gauge',
	'pending_ibuf_aio_reads': 'gauge',
	'pending_log_writes':'gauge',
	'queries_inside': 'gauge',
	'queries_queued': 'gauge',
	'read_views': 'gauge',
}

MYSQL_INNODB_STATUS_MATCHES = {
	# 0 read views open inside InnoDB
	'read views open inside InnoDB': {
		'read_views': 0,
	},
	# 5635328 OS file reads, 27018072 OS file writes, 20170883 OS fsyncs
	' OS file reads, ': {
		'file_reads': 0,
		'file_writes': 4,
	},
	# ibuf aio reads: 0, log i/o's: 0, sync i/o's: 0
	'ibuf aio reads': {
		'pending_ibuf_aio_reads': 3,
		'pending_aio_log_ios': 6,
		'pending_aio_sync_ios': 9,
	},
	# Pending flushes (fsync) log: 0; buffer pool: 0
	'Pending flushes (fsync)': {
		'pending_buf_pool_flushes': 7,
	},
	# 16086708 log i/o's done, 106.07 log i/o's/second
	" log i/o's done, ": {
		'log_writes': 0,
	},
	# 0 pending log writes, 0 pending chkp writes
	' pending log writes, ': {
		'pending_log_writes': 0,
		'pending_chkp_writes': 4,
	},
	# Page hash           2302856 (buffer pool 0 only)
	'Page hash    ': {
		'page_hash_memory': 2,
	},
	# File system         657820264 	(812272 + 657007992)
	'File system    ': {
		'file_system_memory': 2,
	},
	# Lock system         143820296 	(143819576 + 720)
	'Lock system    ': {
		'lock_system_memory': 2,
	},
	# 0 queries inside InnoDB, 0 queries in queue
	'queries inside InnoDB, ': {
		'queries_inside': 0,
		'queries_queued': 4,
	},
	# --Thread 139954487744256 has waited at dict0dict.cc line 472 for 0.0000 seconds the semaphore:
	'seconds the semaphore': {
		'innodb_sem_waits': lambda row, stats: stats['innodb_sem_waits'] + 1,
		'innodb_sem_wait_time_ms': lambda row, stats: int(float(row[9]) * 1000),
	},
	# mysql tables in use 1, locked 1
	'mysql tables in use': {
		'innodb_tables_in_use': lambda row, stats: stats['innodb_tables_in_use'] + int(row[4]),
		'innodb_locked_tables': lambda row, stats: stats['innodb_locked_tables'] + int(row[6]),
	},
	"------- TRX HAS BEEN": {
		"innodb_lock_wait_secs": lambda row, stats: stats['innodb_lock_wait_secs'] + int(row[5]),
	},
}

def get_mysql_conn():
	return pymysql.connect(
		host=MYSQL_CONFIG['Host'],
		port=MYSQL_CONFIG['Port'],
		user=MYSQL_CONFIG['User'],
		passwd=MYSQL_CONFIG['Password'],
		unix_socket = MYSQL_CONFIG['unix_socket']
	)

def mysql_query(conn, query):
	cur = conn.cursor(pymysql.cursors.DictCursor)
	cur.execute(query)
	return cur

def fetch_mysql_status(conn):
	result = mysql_query(conn, "SHOW GLOBAL STATUS LIKE 'Prepared_stmt_count'")
	status = {}
	for row in result.fetchall():
		status[row['Variable_name']] = row['Value']

	# calculate the number of unpurged txns from existing variables
	if 'Innodb_max_trx_id' in status:
		status['Innodb_unpurged_txns'] = int(status['Innodb_max_trx_id']) - int(status['Innodb_purge_trx_id'])

	if 'Innodb_lsn_last_checkpoint' in status:
		status['Innodb_uncheckpointed_bytes'] = int(status['Innodb_lsn_current'])- int(status['Innodb_lsn_last_checkpoint'])

	if 'Innodb_lsn_flushed' in status:
		status['Innodb_unflushed_log'] = int(status['Innodb_lsn_current']) - int(status['Innodb_lsn_flushed'])

	return status

def fetch_mysql_master_stats(conn):
	try:
		result = mysql_query(conn, 'SHOW BINARY LOGS')
	except pymysql.err.OperationalError:
		return {}

	stats = {
		'binary_log_space': 0,
	}

	for row in result.fetchall():
		if 'File_size' in row and row['File_size'] > 0:
			stats['binary_log_space'] += int(row['File_size'])

	return stats

def fetch_mysql_slave_stats(conn):
	result    = mysql_query(conn, 'SHOW SLAVE STATUS')
	slave_row = result.fetchone()
	if slave_row is None:
		return {}

	status = {
		'relay_log_space': slave_row['Relay_Log_Space'],
		'slave_lag':       slave_row['Seconds_Behind_Master'] if slave_row['Seconds_Behind_Master'] != None else 0,
	}

	if MYSQL_CONFIG['HeartbeatTable']:
		query = """
			SELECT MAX(UNIX_TIMESTAMP() - UNIX_TIMESTAMP(ts)) AS delay
			FROM %s
			WHERE server_id = %s
		""" % (MYSQL_CONFIG['HeartbeatTable'], slave_row['Master_Server_Id'])
		result = mysql_query(conn, query)
		row    = result.fetchone()
		if 'delay' in row and row['delay'] != None:
			status['slave_lag'] = row['delay']

	status['slave_running'] = 1 if slave_row['Slave_SQL_Running'] == 'Yes' else 0
	status['slave_stopped'] = 1 if slave_row['Slave_SQL_Running'] != 'Yes' else 0
	return status

def fetch_mysql_process_states(conn):
	global MYSQL_PROCESS_STATES
	result = mysql_query(conn, 'select * from information_schema.processlist')
	states = MYSQL_PROCESS_STATES.copy()

	for row in result.fetchall():
		state = row['STATE']
		if state == '' or state == None: state = 'none'
		state = re.sub(r'^(Table lock|Waiting for .*lock)$', "Locked", state)
		state = state.lower().replace(" ", "_")
		if state not in states: state = 'other'
		states[state] += 1

	command = ''
	time_wait = 0
	result = mysql_query(conn, 'select trx_mysql_thread_id, trx_started, trx_query from '
								'information_schema.innodb_trx order by trx_started limit 1;')
	rows = result.fetchall()
	if len(rows) == 0: return states
	
	row = rows[0]
	startTime = int(time.mktime(row['trx_started'].timetuple()))
	currentTime = int(time.mktime(time.localtime()))
	states['longest_wait_time'] = currentTime-startTime
	states['longest_wait_command'] = {'trx_started':row['trx_started'].strftime('%Y-%m-%d %H:%M:%S'), 
									  'trx_query':row['trx_query'], 
									  'trx_mysql_thread_id':row['trx_mysql_thread_id'], 'trx_state': 'unknown'}
	result = mysql_query(conn, 'select STATE from information_schema.processlist where ID = {0}'.format(row['trx_mysql_thread_id']))
	rows = result.fetchall()
	if len(rows) == 0: return states
	
	states['longest_wait_command']['trx_state'] = rows[0]['STATE']
	return states

def fetch_mysql_variables(conn):
	global MYSQL_VARS
	result = mysql_query(conn, 'SHOW GLOBAL VARIABLES')
	variables = {}
	for row in result.fetchall():
		if row['Variable_name'] in MYSQL_VARS:
			variables[row['Variable_name']] = row['Value']

	return variables

def fetch_mysql_response_times(conn):
	response_times = {}
	try:
		result = mysql_query(conn, """
			SELECT *
			FROM INFORMATION_SCHEMA.QUERY_RESPONSE_TIME
			WHERE `time` != 'TOO LONG'
			ORDER BY `time`
		""")
	except pymysql.err.OperationalError:
		return {}

	for i in range(1, 14):
		row = result.fetchone()

		# fill in missing rows with zeros
		if not row:
			row = { 'count': 0, 'total': 0 }

		response_times[i] = {
			'time':  float(row['time']),
			'count': int(row['count']),
			'total': round(float(row['total']) * 1000000, 0),
		}

	return response_times

def fetch_innodb_stats(conn):
	global MYSQL_INNODB_STATUS_MATCHES, MYSQL_INNODB_STATUS_VARS
	result = mysql_query(conn, 'SHOW ENGINE INNODB STATUS')
	row    = result.fetchone()
	status = row['Status']
	stats  = dict.fromkeys(MYSQL_INNODB_STATUS_VARS.keys(), 0)

	for line in status.split("\n"):
		line = line.strip()
		row  = re.split(r' +', re.sub(r'[,;] ', ' ', line))
		if line == '': continue

		# ---TRANSACTION 124324402462, not started
		# ---TRANSACTION 124324402468, ACTIVE 0 sec committing
		if line.find("---TRANSACTION") != -1:
			stats['current_transactions'] += 1
			if line.find("ACTIVE") != -1:
				stats['active_transactions'] += 1
		# LOCK WAIT 228 lock struct(s), heap size 46632, 65 row lock(s), undo log entries 1
		# 205 lock struct(s), heap size 30248, 37 row lock(s), undo log entries 1
		elif line.find("lock struct(s)") != -1:
			if line.find("LOCK WAIT") != -1:
				stats['innodb_lock_structs'] += int(row[2])
				stats['locked_transactions'] += 1
			else:
				stats['innodb_lock_structs'] += int(row[0])
		else:
			for match in MYSQL_INNODB_STATUS_MATCHES:
				if line.find(match) == -1: continue
				for key in MYSQL_INNODB_STATUS_MATCHES[match]:
					value = MYSQL_INNODB_STATUS_MATCHES[match][key]
					if type(value) is int:
						if value < len(row) and row[value].isdigit():
							stats[key] = int(row[value])
					else:
						stats[key] = value(row, stats)
				break

	return stats

def configure_callback(conf):
	global MYSQL_CONFIG
	for node in conf.children:
		if node.key in MYSQL_CONFIG:
			MYSQL_CONFIG[node.key] = node.values[0]

	MYSQL_CONFIG['Port']    = int(MYSQL_CONFIG['Port'])
	MYSQL_CONFIG['Verbose'] = bool(MYSQL_CONFIG['Verbose'])

def read_callback():
	mysql_helper = mysql_stat_helper()
	mysql_helper.dispatch_mysql_status()

def notification_callback(notification, data=None):
	message = notification.message
	if notification.severity == collectd.NOTIF_FAILURE:
		severity = 'FAILURE'
	elif notification.severity == collectd.NOTIF_WARNING:
		severity = 'WARNING'
	elif notification.severity == collectd.NOTIF_OKAY:
		severity = 'OKAY'
	else:
		severity = 'UNKNOWN'
	#send_to_slack(message)
	collectd.info("python debug: Notification is sent to mysql... Severity =  {0}, Message = {1}".format(severity,message))

class mysql_stat_helper(object):
	def __init__(self, **args):
		if args:
			global MYSQL_CONFIG
			for key in args:
				if not key in MYSQL_CONFIG:continue
				MYSQL_CONFIG[key] = args[key]
			MYSQL_CONFIG['Port']    = int(MYSQL_CONFIG['Port'])
			MYSQL_CONFIG['Verbose'] = bool(MYSQL_CONFIG['Verbose'])

		self.conn = get_mysql_conn();
	
	def get_mysql_status(self):
		mysql_status = fetch_mysql_status(self.conn)
		for key in mysql_status: 
			if mysql_status[key] == '': mysql_status[key] = 0
	
		#mysql_variables = fetch_mysql_variables(self.conn)
		#mysql_master_status = fetch_mysql_master_stats(self.conn)
		#slave_status = fetch_mysql_slave_stats(self.conn)
		mysql_states = fetch_mysql_process_states(self.conn)
		#response_times = fetch_mysql_response_times(self.conn)
		innodb_status = fetch_innodb_stats(self.conn)
		
		return {
				'status':mysql_status,
				#'variables': mysql_variables,
				#'master': mysql_master_status,
				#'slave': slave_status,
				'state': mysql_states,
				#'response_time': response_times,
				'innodb': innodb_status
				}

	def mysql_kill_thread(self, thread_id):
		result = mysql_query(self.conn, 'kill {0}'.format(thread_id))
		return

	def log_verbose(self, msg):
		if MYSQL_CONFIG['Verbose'] == False:
			return
		collectd.info('mysql plugin: %s' % msg)
	
	def dispatch_value(self, prefix, key, value, type, type_instance=None):
		if not type_instance:
			type_instance = key
	
		#self.log_verbose('Sending value: %s/%s=%s' % (prefix, type_instance, value))
		if value is None:
			return
		try:
			value = int(value)
		except ValueError:
			value = float(value)
	
		val               = collectd.Values(plugin='mysql', plugin_instance=prefix)
		val.type          = type
		val.type_instance = type_instance
		val.values        = [value]
		val.dispatch()

	def dispatch_mysql_status(self):
		mysql_full_status = self.get_mysql_status()
		for upper_key in mysql_full_status:
			current_status = mysql_full_status[upper_key]
			if upper_key == 'status':
				for sub_key in current_status:
					# collect anything beginning with Com_/Handler_ as these change
					# regularly between  mysql versions and this is easier than a fixed
					# list
					if sub_key.split('_', 2)[0] in ['Com', 'Handler']:
						ds_type = 'counter'
					elif sub_key in MYSQL_STATUS_VARS:
						ds_type = MYSQL_STATUS_VARS[sub_key]
					else:
						continue
			
					self.dispatch_value(upper_key, sub_key, current_status[sub_key], ds_type)
			elif upper_key in ['variables', 'master', 'slave']:
				ds_type = 'gauge'
				for sub_key in current_status:
					self.dispatch_value(upper_key, sub_key, current_status[sub_key], ds_type)
			elif upper_key == 'state':
				ds_type = 'gauge'
				for sub_key in current_status:
					if sub_key == 'longest_wait_command':
						if current_status[sub_key] != '':
							self.log_verbose('the longest wait command is {0}'.format(current_status[sub_key]))
						continue				
					self.dispatch_value(upper_key, sub_key, current_status[sub_key], ds_type)
			elif upper_key == 'response_time':
				ds_type = 'counter'
				for sub_key in current_status:
					self.dispatch_value('response_time_total', str(sub_key), current_status[sub_key]['total'], 'counter')
					self.dispatch_value('response_time_count', str(sub_key), current_status[sub_key]['count'], 'counter')
			elif upper_key == 'innodb':
				for sub_key in current_status:
					if not sub_key in MYSQL_INNODB_STATUS_VARS: continue
					self.dispatch_value(upper_key, sub_key, current_status[sub_key], MYSQL_INNODB_STATUS_VARS[sub_key])
		
if __name__ == '__main__':
	import optparse,logging
	
	parser = optparse.OptionParser(usage="usage: %prog [options]", version="%prog 1.0")
	parser.add_option('-g', "--debug", 
					  action = "store", 
					  type = "string", 
					  dest = "debug", 
					  default = "False", 
					  help = "debug True or False")
	parser.add_option('-H', "--host",
					  action = "store",
					  type = "string",
					  dest = "host",
					  help = "specify the mysql ip to connect")
	parser.add_option('-u', "--user",
					  action = "store",
					  type = "string",
					  dest = "user",
					  help = "specify the username of mysql")
	parser.add_option('-p', "--passwd",
					  action = "store",
					  type = "string",
					  dest = "passwd",
					  help = "specify the passwd of mysql")
	parser.add_option('-w', "--wait_threshold",
					  action = "store",
					  type = "string", 
					  dest = "wait_threshold",
					  help = "specify the threshold value of wait time")
	parser.add_option('-f', "--log_file",
					action = "store",
					type = "string",
					dest = "log_file",
					default = None,
					help = "specify the path of the log file")
	options, args = parser.parse_args()
	
	if options.debug == 'True':
		import pdb
		pdb.set_trace()
	
	logger = init_logging(options.log_file)
	try:
		import pymysql
	except Exception, e:
		logger.warn('the pymysql module does not exists, should install it.')
		exit(1)

	try:
		mysql_helper = mysql_stat_helper(Host = options.host, User = options.user, 
									Password = options.passwd)
		result = mysql_helper.get_mysql_status()
		logger.info('the mysql status is: {0}'.format(result))
		if result['state']['longest_wait_time'] > int(options.wait_threshold):
			logger.warn('long wait trx exists, the long wait query {0} should be killed'.
				format(result['state']['longest_wait_command'])) 
			mysql_helper.mysql_kill_thread(result['state']['longest_wait_command']['trx_mysql_thread_id'])
	except Exception, e:
		logger.error('do mysql check failed, the args: {0}, the err info: {1}'.format(options, e))
	finally:
		exit(0)
else:
	import collectd,pymysql
	collectd.register_read(read_callback)
	collectd.register_config(configure_callback)
	collectd.register_notification(notification_callback)
	collectd.info("mysql plugin...LOADED")

# vim:noexpandtab ts=8 sw=8 sts=8