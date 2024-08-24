/* $Id$
 * $Source$
 *------------------------------------------------------------------
 *
 * contains definitions of structs used to send/receive NetFlow data
 * 
 * Cisco NetFlow FlowCollector
 *
 * Copyright (c) 1997 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 * $Log$
 *------------------------------------------------------------------
 * $Endlog$
 */

#ifndef __FLOWDATA_H__
#define __FLOWDATA_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define FLOW_VERSION_1		1
#define V1FLOWS_PER_PAK		24
#define FLOW_VERSION_5		5
#define V5FLOWS_PER_PAK		30
#define FLOW_VERSION_7      7
#define V7FLOWS_PER_PAK     27
#define FLOW_VERSION_8                  8
#define MAX_FLOWS_PER_V8_AS_PAK         51
#define MAX_FLOWS_PER_V8_PROTO_PORT_PAK 51
#define MAX_FLOWS_PER_V8_SRC_PREFIX_PAK 44
#define MAX_FLOWS_PER_V8_PREFIX_PAK     35
#define MAX_FLOWS_PER_V8_DST_PREFIX_PAK 44


/* Used in flags field in IPFlowStatV7 */
/* Fields srcaddr, srcport, dstport, prot are invalid in IPFlowStatV7 */
#define DESTINATION_ONLY_FLOW    0x1
/* Fields srcport, dstport, prot are invalid in IPFlowStatV7 */
#define SOURCE_DESTINATION_FLOW  0x2

/* For V5 engine_type, RSP uses 0, VIPs uses 1 */
#define V5_RSP_EXPORT           0
#define V5_VIP_EXPORT           1

#ifndef __SVR4
//typedef unsigned int/*long*/ ulong;
#endif
typedef ulong ipaddrtype;
typedef unsigned char uchar;

#ifndef MAX
#define MAX(a,b) ((a) >= (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b) ((a) >= (b) ? (b) : (a))
#endif

typedef struct {
    ushort version;         /* 1 for now. */
    ushort count;           /* The number of records in PDU. */
    ulong  SysUptime;       /* Current time in millisecs since router booted */
    ulong  unix_secs;       /* Current seconds since 0000 UTC 1970 */
    ulong  unix_nsecs;      /* Residual nanoseconds since 0000 UTC 1970 */
} Flow1StatHdr;

typedef struct {
    ipaddrtype srcaddr;    /* Source IP Address */
    ipaddrtype dstaddr;    /* Destination IP Address */
    ipaddrtype nexthop;    /* Next hop router's IP Address */
    ushort input;          /* Input interface index */
    ushort output;         /* Output interface index */
    
    ulong dPkts;           /* Packets sent in Duration */
    ulong dOctets;         /* Octets sent in Duration. */
    ulong First;           /* SysUptime at start of flow */
    ulong Last;            /* and of last packet of flow */

    ushort srcport;        /* TCP/UDP source port number or equivalent */    
    ushort dstport;        /* TCP/UDP destination port number or equivalent */
    ushort pad;
    uchar  prot;           /* IP protocol, e.g., 6=TCP, 17=UDP, ... */
    uchar  tos;            /* IP Type-of-Service */
    
    uchar  flags;          /* Reason flow was discarded, etc...  */
    uchar  tcp_retx_cnt;   /* Number of mis-seq with delay > 1sec */
    uchar  tcp_retx_secs;  /* Cumulative seconds between mis-sequenced pkts */
    uchar  tcp_misseq_cnt; /* Number of mis-sequenced tcp pkts seen */
    ulong  reserved;
} IPFlow1Stat;

typedef struct {
    Flow1StatHdr header;
    IPFlow1Stat  records[V1FLOWS_PER_PAK];
} IPStat1Msg;

typedef struct {
    ushort version;         
    ushort count;           /* The number of records in PDU. */
    ulong  SysUptime;       /* Current time in millisecs since router booted */
    ulong  unix_secs;       /* Current seconds since 0000 UTC 1970 */
    ulong  unix_nsecs;      /* Residual nanoseconds since 0000 UTC 1970 */
    ulong  flow_sequence;   /* Seq counter of total flows seen */
    uchar  engine_type;     /* Type of flow switching engine */
    uchar  engine_id;       /* ID number of the flow switching engine */
    ushort reserved;
} Flow5StatHdr;


typedef struct {
    ipaddrtype srcaddr;    /* Source IP Address */
    ipaddrtype dstaddr;    /* Destination IP Address */
    ipaddrtype nexthop;    /* Next hop router's IP Address */
    ushort input;          /* Input interface index */
    ushort output;         /* Output interface index */
    
    ulong dPkts;           /* Packets sent in Duration */
    ulong dOctets;         /* Octets sent in Duration. */
    ulong First;           /* SysUptime at start of flow */
    ulong Last;            /* and of last packet of flow */

    ushort srcport;        /* TCP/UDP source port number or equivalent */
    ushort dstport;        /* TCP/UDP destination port number or equivalent */
    uchar  pad;
    uchar  tcp_flags;      /* Cumulative OR of tcp flags */
    uchar  prot;           /* IP protocol, e.g., 6=TCP, 17=UDP, ... */
    uchar  tos;            /* IP Type-of-Service */
    ushort src_as;         /* originating AS of source address */
    ushort dst_as;         /* originating AS of destination address */
    uchar  src_mask;       /* source address prefix mask bits */
    uchar  dst_mask;       /* destination address prefix mask bits */
    ushort reserved;
} IPFlow5Stat;

typedef struct {
    Flow5StatHdr header;
    IPFlow5Stat  records[V5FLOWS_PER_PAK];
} IPStat5Msg;


typedef struct {
    ushort version;
    ushort count;           /* The number of records in PDU. */
    ulong  SysUptime;       /* Current time in millisecs since router booted */
    ulong  unix_secs;       /* Current seconds since 0000 UTC 1970 */
    ulong  unix_nsecs;      /* Residual nanoseconds since 0000 UTC 1970 */
    ulong  flow_sequence;   /* Seq counter of total flows seen */
    ulong  reserved;
} Flow7StatHdr;

typedef struct {
    ipaddrtype srcaddr;    /* Source IP Address */
    ipaddrtype dstaddr;    /* Destination IP Address */
    ipaddrtype nexthop;    /* Next hop router's IP Address */
    ushort input;          /* Input interface index */
    ushort output;         /* Output interface index */

    ulong dPkts;           /* Packets sent in Duration */
    ulong dOctets;         /* Octets sent in Duration. */
    ulong First;           /* SysUptime at start of flow */
    ulong Last;            /* and of last packet of flow */

    ushort srcport;        /* TCP/UDP source port number or equivalent */
    ushort dstport;        /* TCP/UDP destination port number or equivalent */
    uchar  flags;          /* See above for flag definitions */
    uchar  tcp_flags;      /* Cumulative OR of tcp flags */
    uchar  prot;           /* IP protocol, e.g., 6=TCP, 17=UDP, ... */
    uchar  tos;            /* IP Type-of-Service */
    ushort src_as;         /* originating AS of source address */
    ushort dst_as;         /* originating AS of destination address */
    uchar  src_mask;       /* source address prefix mask bits */
    uchar  dst_mask;       /* destination address prefix mask bits */
    ushort reserved;
    ipaddrtype router_sc;  /* IP address of router shortcut by switch */
} IPFlow7Stat;

typedef struct {
    Flow7StatHdr header;
    IPFlow7Stat  records[V7FLOWS_PER_PAK];
} IPStat7Msg;


typedef struct {
    ushort version;         /* Current version */
    ushort count;           /* The number of records in PDU. */
    ulong  SysUptime;       /* Current time in millisecs since router booted */
    ulong  unix_secs;       /* Current seconds since 0000 UTC 1970 */
    ulong  unix_nsecs;      /* Residual nanoseconds since 0000 UTC 1970 */
    ulong  flow_sequence;   /* Seq counter of total flows seen */
    uchar  engine_type;     /* Type of flow switching engine */
    uchar  engine_id;       /* ID number of the flow switching engine */
    uchar  aggregation;     /* Aggregation method being used */
    uchar  agg_version;     /* Version of the aggregation export */
    ulong  reserved;
} FlowStatHdrV8;


/*
 * Flow aggregation schemes
 */
enum Router_Aggregation {
    IPFLOW_NO_AGGREGATION,
    IPFLOW_AS_AGGREGATION,
    IPFLOW_PROTO_PORT_AGGREGATION,
    IPFLOW_SRC_PREFIX_AGGREGATION,
    IPFLOW_DST_PREFIX_AGGREGATION,
    IPFLOW_PREFIX_AGGREGATION,
    IPFLOW_MAX_AGGREGATION
};

/*
 * Flow stat record for AS aggregation
 */
typedef struct {
    ulong      flows;          /* Number of flows */
    ulong      dPkts;          /* Packets sent in Duration */
    ulong      dOctets;        /* Octets sent in Duration. */
    ulong      First;          /* SysUptime at start of flow */
    ulong      Last;           /* and of last packet of flow */
    ushort     src_as;         /* originating AS of source address */
    ushort     dst_as;         /* originating AS of destination address */
    ushort     input;          /* Input interface index */
    ushort     output;         /* Output interface index */
} IPFlowStatV8_AS;

#define V8_AS_VERSION 2          /* Current version of the AS record */

/*
 * Flow stat record for Protocol Port aggregation
 */
typedef struct {
    ulong      flows;          /* Number of flows */
    ulong      dPkts;          /* Packets sent in Duration */
    ulong      dOctets;        /* Octets sent in Duration. */
    ulong      First;          /* SysUptime at start of flow */
    ulong      Last;           /* and of last packet of flow */
    uchar      prot;           /* IP protocol, e.g., 6=TCP, 17=UDP, ... */
    uchar      pad;
    ushort     reserved;
    ushort     srcport;        /* TCP/UDP source port number or equivalent */
    ushort     dstport;        /* TCP/UDP dest port number or equivalent */
} IPFlowStatV8_ProtoPort;

#define V8_PROTO_PORT_VERSION 2  /* Current version of the proto-port record */

/*
 * Flow stat record for SourcePrefix aggregation
 */
typedef struct {
    ulong      flows;          /* Number of flows */
    ulong      dPkts;          /* Packets sent in Duration */
    ulong      dOctets;        /* Octets sent in Duration. */
    ulong      First;          /* SysUptime at start of flow */
    ulong      Last;           /* and of last packet of flow */
    ipaddrtype src_prefix;     /* Source prefix */
    uchar      src_mask;       /* source address prefix mask bits */
    uchar      pad;
    ushort     src_as;         /* originating AS of source address */
    ushort     input;          /* Input interface index */
    ushort     reserved;
} IPFlowStatV8_SrcPrefix;

#define V8_SRC_PREFIX_VERSION 2  /* Current version of the src prefix record */

/*
 * Flow stat record for DestinationPrefix aggregation
 */
typedef struct {
    ulong      flows;          /* Number of flows */
    ulong      dPkts;          /* Packets sent in Duration */
    ulong      dOctets;        /* Octets sent in Duration. */
    ulong      First;          /* SysUptime at start of flow */
    ulong      Last;           /* and of last packet of flow */
    ipaddrtype dst_prefix;     /* Destination prefix */
    uchar      dst_mask;       /* destination address prefix mask bits */
    uchar      pad;
    ushort     dst_as;         /* originating AS of destination address */
    ushort     output;         /* Output interface index */
    ushort     reserved;
} IPFlowStatV8_DstPrefix;

#define V8_DST_PREFIX_VERSION 2  /* Current version of the dst prefix record */

/*
 * Flow stat record for Prefix aggregation
 */
typedef struct {
    ulong      flows;          /* Number of flows */
    ulong      dPkts;          /* Packets sent in Duration */
    ulong      dOctets;        /* Octets sent in Duration. */
    ulong      First;          /* SysUptime at start of flow */
    ulong      Last;           /* and of last packet of flow */
    ipaddrtype src_prefix;     /* Source prefix */
    ipaddrtype dst_prefix;     /* Destination prefix */
    uchar      dst_mask;       /* destination address prefix mask bits */
    uchar      src_mask;       /* source address prefix mask bits */
    ushort     reserved;
    ushort     src_as;         /* originating AS of source address */
    ushort     dst_as;         /* originating AS of destination address */
    ushort     input;          /* Input interface index */
    ushort     output;         /* Output interface index */
} IPFlowStatV8_Prefix;

#define V8_PREFIX_VERSION 2  /* Current version of the dst prefix record */

typedef struct {
    FlowStatHdrV8 header;
    union {
        IPFlowStatV8_AS        as[MAX_FLOWS_PER_V8_AS_PAK]; 
        IPFlowStatV8_ProtoPort proto_port[MAX_FLOWS_PER_V8_PROTO_PORT_PAK];
        IPFlowStatV8_SrcPrefix src_prefix[MAX_FLOWS_PER_V8_SRC_PREFIX_PAK];
        IPFlowStatV8_DstPrefix dst_prefix[MAX_FLOWS_PER_V8_DST_PREFIX_PAK];
        IPFlowStatV8_Prefix    prefix[MAX_FLOWS_PER_V8_PREFIX_PAK];
    } records;
} IPStatMsgV8;


#define MAX_V1_FLOW_PAK_SIZE (sizeof(Flow1StatHdr) + \
                              sizeof(IPFlow1Stat) * V1FLOWS_PER_PAK)
 
#define MAX_V5_FLOW_PAK_SIZE (sizeof(Flow5StatHdr) + \
                              sizeof(IPFlow5Stat) * V5FLOWS_PER_PAK)
 
#define MAX_V7_FLOW_PAK_SIZE (sizeof(Flow7StatHdr) + \
                              sizeof(IPFlow7Stat) * V7FLOWS_PER_PAK)

#define MAX_V8_AS_FLOW_PAK_SIZE (sizeof(FlowStatHdrV8) + \
                                 sizeof(IPFlowStatV8_AS) * \
                                 MAX_FLOWS_PER_V8_AS_PAK)

#define MAX_V8_PROTO_PORT_FLOW_PAK_SIZE (sizeof(FlowStatHdrV8) + \
                                         sizeof(IPFlowStatV8_ProtoPort) * \
                                         MAX_FLOWS_PER_V8_PROTO_PORT_PAK)

#define MAX_V8_SRC_PREFIX_FLOW_PAK_SIZE (sizeof(FlowStatHdrV8) +          \
                                         sizeof(IPFlowStatV8_SrcPrefix) * \
                                         MAX_FLOWS_PER_V8_SRC_PREFIX_PAK)

#define MAX_V8_DST_PREFIX_FLOW_PAK_SIZE (sizeof(FlowStatHdrV8) +          \
                                         sizeof(IPFlowStatV8_DstPrefix) * \
                                         MAX_FLOWS_PER_V8_DST_PREFIX_PAK)

#define MAX_V8_PREFIX_FLOW_PAK_SIZE (sizeof(FlowStatHdrV8) +       \
                                     sizeof(IPFlowStatV8_Prefix) * \
                                     MAX_FLOWS_PER_V8_PREFIX_PAK)

#define MAX_V8_FLOW_PAK_SIZE MAX(MAX(MAX(MAX_V8_AS_FLOW_PAK_SIZE, \
                                              MAX_V8_PROTO_PORT_FLOW_PAK_SIZE),\
                                   MAX(MAX_V8_SRC_PREFIX_FLOW_PAK_SIZE, \
                                         MAX_V8_DST_PREFIX_FLOW_PAK_SIZE)), \
                                   MAX_V8_PREFIX_FLOW_PAK_SIZE)

#define MAX_FLOW_PAK_SIZE MAX(MAX(MAX_V1_FLOW_PAK_SIZE,  \
                                      MAX_V5_FLOW_PAK_SIZE), \
                                MAX(MAX_V7_FLOW_PAK_SIZE,  \
                                      MAX_V8_FLOW_PAK_SIZE))

#define MAX_FLOWS_PER_V8_PAK MAX(MAX(MAX(MAX_FLOWS_PER_V8_AS_PAK, \
                                              MAX_FLOWS_PER_V8_PROTO_PORT_PAK),\
                                         MAX(MAX_FLOWS_PER_V8_SRC_PREFIX_PAK,\
                                             MAX_FLOWS_PER_V8_DST_PREFIX_PAK)),\
                                         MAX_FLOWS_PER_V8_PREFIX_PAK)

#define MAX_FLOW_PER_PAK_SIZE MAX(MAX(MAX_FLOWS_PER_V1_PAK, \
                                          MAX_FLOWS_PER_V5_PAK),\
                                    MAX(MAX_FLOWS_PER_V7_PAK, \
                                          MAX_FLOWS_PER_V8_PAK))

#endif /* __FLOWDATA_H__ */
