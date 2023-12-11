/**
 * Copyright (c) NVIDIA CORPORATION & AFFILIATES, 2023. ALL RIGHTS RESERVED.
 *
 * See file LICENSE for terms.
 */

#ifndef UCP_PROTO_PRIORITY_H_
#define UCP_PROTO_PRIORITY_H_

#include "proto.h"
#include "proto_common.h"


typedef struct {
    ucp_proto_common_init_params_t super;
    /* Required iface capabilities */
    uint64_t                       tl_cap_flags;

    /* Required lane type */
    ucp_lane_type_t                lane_type;
} ucp_proto_priority_init_params_t;

typedef struct {
    ucp_proto_common_lane_priv_t super;
    ucp_md_map_t     reg_md_map; /* Memory domains to register on */
    ucp_lane_map_t   lane_map; /* Map of used lanes */
    ucp_lane_index_t num_lanes; /* Number of lanes to use */
} ucp_proto_priority_priv_t;


ucs_status_t
ucp_proto_priority_init(const ucp_proto_priority_init_params_t *params);


ucs_status_t
ucp_proto_priority_init_priv(const ucp_proto_priority_init_params_t *params,
                             ucp_proto_priority_priv_t *spriv);


void ucp_proto_priority_query(const ucp_proto_query_params_t *params,
                              ucp_proto_query_attr_t *attr);

#endif