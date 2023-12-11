/**
 * Copyright (c) NVIDIA CORPORATION & AFFILIATES, 2023. ALL RIGHTS RESERVED.
 *
 * See file LICENSE for terms.
 */

#include <limits.h>

#include "proto_common.h"
#include "proto_debug.h"
#include "proto_init.h"

#include "proto_priority.h"

#include "ucs/debug/memtrack_int.h"
#include "ucs/sys/math.h"
#include "ucs/debug/log.h"


ucs_status_t
ucp_proto_priority_init_priv(const ucp_proto_priority_init_params_t *params,
                             ucp_proto_priority_priv_t *priv)
{
    ucp_proto_common_tl_perf_t lanes_perf[UCP_PROTO_MAX_LANES];
    ucp_proto_perf_node_t *lanes_perf_nodes[UCP_PROTO_MAX_LANES];
    ucp_lane_index_t lanes[UCP_PROTO_MAX_LANES];
    ucp_proto_perf_node_t *perf_node;
    ucp_lane_index_t num_lanes;
    ucp_md_map_t reg_md_map;
    ucp_proto_common_tl_perf_t *lane_perf, perf;
    ucs_status_t status;
    ucp_lane_index_t lane;
    ucp_lane_map_t lane_map;
    int i, min_lat_lane_index;
    double min_latency;

    num_lanes = ucp_proto_common_find_lanes(&params->super, params->lane_type,
                                            params->tl_cap_flags, UCP_PROTO_MAX_LANES,
                                            params->super.exclude_map, lanes);

    if (num_lanes == 0) {
        ucs_trace("no priority lanes for %s", params->super.super.proto_name);
        return UCS_ERR_NO_ELEM;
    }

    /* Get latency of all lanes and min_latency */
    min_latency        = INFINITY;
    min_lat_lane_index = 0;
    for (i = 0; i < num_lanes; ++i) {
        lane      = lanes[i];
        lane_perf = &lanes_perf[lane];

        status = ucp_proto_common_get_lane_perf(&params->super, lane, lane_perf,
                                                &lanes_perf_nodes[lane]);
        if (status != UCS_OK) {
            return status;
        }

        /* Calculate minimum latency of all lanes, to skip slow lanes */
        if (min_latency > lane_perf->latency) {
            min_latency        = lane_perf->latency;
            min_lat_lane_index = i;
        }
    }

    lane_map   = UCS_BIT(lanes[min_lat_lane_index]);
    reg_md_map = ucp_proto_common_reg_md_map(
            &params->super, lane_map);
    perf_node  = lanes_perf_nodes[lanes[min_lat_lane_index]];
    ucp_proto_perf_node_ref(perf_node);

    status = ucp_proto_common_init_caps(&params->super, &perf, perf_node,
                                        reg_md_map);

    /* Deref unused nodes */
    for (i = 0; i < num_lanes; ++i) {
        ucp_proto_perf_node_deref(&lanes_perf_nodes[lanes[i]]);
    }
    ucp_proto_perf_node_deref(&perf_node);

    return status;
}

ucs_status_t
ucp_proto_priority_init(const ucp_proto_priority_init_params_t *params)
{
    ucs_status_t status;

    if (!ucp_proto_common_init_check_err_handling(&params->super)) {
        return UCS_ERR_UNSUPPORTED;
    }

    if (params->super.super.num_priority_lanes == 0) {
        /* no need for priority lanes */
        return UCS_OK;
    }

    status = ucp_proto_priority_init_priv(params, params->super.super.priv);
    if (status != UCS_OK) {
        return status;
    }

    *params->super.super.priv_size = sizeof(ucp_proto_priority_priv_t);
    return UCS_OK;
}

void ucp_proto_priority_query(const ucp_proto_query_params_t *params,
                              ucp_proto_query_attr_t *attr)
{
}