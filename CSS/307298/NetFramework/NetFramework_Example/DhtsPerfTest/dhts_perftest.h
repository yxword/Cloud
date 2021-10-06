//
//  "$Id: TcpCli.h 7672 2010-5-17 02:28:18Z he_linqiang $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//  Description:    
//  Revisions:      Year-Month-Day  SVN-Author  Modification
//                  2010-5-17        he_linqiang Create
//
//
#ifndef __INCLUDED_DHTS_PERFTEST_H__
#define __INCLUDED_DHTS_PERFTEST_H__

struct PerfTestConfig
{
	int nchans;		//  -p option, num of parallel channel 
	int bandwidth;	//  -b option, in Mbps
	int debug;		// -d option, enable debug 
	int duration;	// -t option, duration time in second 
	int interval;	// -i optoin, interval of print info 
	int mode;		// -s|-c option,  server or client mode 
	int port;		// -p option, socket port 
	int fps;		// -f option, fps   25|30 
	int drvcnt;		// -n option, num of drive restart tx
	int seqtimetest; // -T option, enable seqtime test 
	struct sockaddr_in addr;	
};

struct BandwithSample
{
	double intval_ms;
	double rxbw;
	double txbw;
	int 	 txbytes;
	int 	 rxbytes;
	BandwithSample():intval_ms(0.0),rxbw(0.0),txbw(0.0),txbytes(0), rxbytes(0){}
};

typedef struct 
{
	double total_bw;
	int count;
} AvgRateStatistic;

#endif //__INCLUDED_DHTS_PERFTEST_H__
