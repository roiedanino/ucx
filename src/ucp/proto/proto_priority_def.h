/**
 * Copyright (c) NVIDIA CORPORATION & AFFILIATES, 2023. ALL RIGHTS RESERVED.
 *
 * See file LICENSE for terms.
 */


#ifndef UCP_PROTO_PRIORITY_DEF_H_
#define UCP_PROTO_PRIORITY_DEF_H_

#include "proto_priority.h"


#define UCP_PRIORITY_PROTO_DECL(_proto) \
    static ucs_status_t _proto ## _priority_init( \
            const ucp_proto_init_params_t *init_params) \
    { \
        ucs_status_t status; \
        status = _proto.init(init_params); \
        if (status != UCS_OK) { \
            return status; \
        } \
        return ucp_proto_priority_init(init_params); \
    } \
    void _proto ## _priority_query(const ucp_proto_query_params_t *params, \
                            ucp_proto_query_attr_t *attr) \
    { \
        _proto.query(params, attr); \
        ucp_proto_priority_query(params, attr); \
    } \
    ucp_proto_t _proto ## __priority \
    { \
        .name = _proto.name, \
        .desc = _proto.desc, \
        .flags = _proto.flags | UCP_PROTO_FLAG_PRIORITY, \
        .init = _proto ## _priority_init, \
        .query = _proto ## _priority_query, \
        .progress = _proto.progress, \
        .abort = _proto.abort, \
        .reset = _proto.reset \
    }

#endif