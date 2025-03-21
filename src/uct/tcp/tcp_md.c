/**
 * Copyright (c) NVIDIA CORPORATION & AFFILIATES, 2001-2019. ALL RIGHTS RESERVED.
 * See file LICENSE for terms.
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "tcp.h"
#include "tcp_sockcm.h"
#include <uct/base/uct_md.h>
#include <uct/api/v2/uct_v2.h>


static ucs_config_field_t uct_tcp_md_config_table[] = {
    {"", "", NULL, ucs_offsetof(uct_tcp_md_config_t, super),
     UCS_CONFIG_TYPE_TABLE(uct_md_config_table)},

    {"AF_PRIO", "inet,inet6",
     "Priority of address families used for socket connections",
     ucs_offsetof(uct_tcp_md_config_t, af_prio), UCS_CONFIG_TYPE_STRING_ARRAY},

    {"BRIDGE_ENABLE", "n", "Enable using bridge devices",
     ucs_offsetof(uct_tcp_md_config_t, bridge_enable), UCS_CONFIG_TYPE_BOOL},

    {NULL}
};

static ucs_status_t uct_tcp_md_query(uct_md_h md, uct_md_attr_v2_t *attr)
{
    uct_md_base_md_query(attr);
    attr->access_mem_types = UCS_BIT(UCS_MEMORY_TYPE_HOST);
    return UCS_OK;
}

static void uct_tcp_md_close(uct_md_h md)
{
    uct_tcp_md_t *tcp_md = ucs_derived_of(md, uct_tcp_md_t);
    ucs_free(tcp_md);
}

static uct_md_ops_t uct_tcp_md_ops = {
    .close              = uct_tcp_md_close,
    .query              = uct_tcp_md_query,
    .mem_alloc          = (uct_md_mem_alloc_func_t)ucs_empty_function_return_unsupported,
    .mem_free           = (uct_md_mem_free_func_t)ucs_empty_function_return_unsupported,
    .mem_advise         = (uct_md_mem_advise_func_t)ucs_empty_function_return_unsupported,
    .mem_reg            = (uct_md_mem_reg_func_t)ucs_empty_function_return_unsupported,
    .mem_dereg          = (uct_md_mem_dereg_func_t)ucs_empty_function_return_unsupported,
    .mem_query          = (uct_md_mem_query_func_t)ucs_empty_function_return_unsupported,
    .mkey_pack          = (uct_md_mkey_pack_func_t)ucs_empty_function_return_unsupported,
    .mem_attach         = (uct_md_mem_attach_func_t)ucs_empty_function_return_unsupported,
    .detect_memory_type = (uct_md_detect_memory_type_func_t)ucs_empty_function_return_unsupported
};

static ucs_status_t
uct_tcp_md_open(uct_component_t *component, const char *md_name,
                const uct_md_config_t *uct_md_config, uct_md_h *md_p)
{
    const uct_tcp_md_config_t *md_config = ucs_derived_of(uct_md_config,
                                                          uct_tcp_md_config_t);
    uct_tcp_md_t *tcp_md;
    ucs_status_t status;
    int i;

    tcp_md = ucs_malloc(sizeof(uct_tcp_md_t), "uct_tcp_md_t");
    if (NULL == tcp_md) {
        ucs_error("failed to allocate memory for uct_tcp_md_t");
        status = UCS_ERR_NO_MEMORY;
        goto err;
    }

    tcp_md->super.ops            = &uct_tcp_md_ops;
    tcp_md->super.component      = &uct_tcp_component;
    tcp_md->config.af_prio_count = ucs_min(md_config->af_prio.count, 2);
    tcp_md->config.bridge_enable = md_config->bridge_enable;

    for (i = 0; i < tcp_md->config.af_prio_count; i++) {
        if (!strcasecmp(md_config->af_prio.af[i], "inet")) {
            tcp_md->config.af_prio_list[i] = AF_INET;
        } else if (!strcasecmp(md_config->af_prio.af[i], "inet6")) {
            tcp_md->config.af_prio_list[i] = AF_INET6;
        } else {
            ucs_error("invalid address family: %s", md_config->af_prio.af[i]);
            status = UCS_ERR_INVALID_PARAM;
            goto err_free;
        }
    }

    *md_p = &tcp_md->super;
    return UCS_OK;

err_free:
    ucs_free(tcp_md);
err:
    return status;
}

uct_component_t uct_tcp_component = {
    .query_md_resources = uct_md_query_single_md_resource,
    .md_open            = uct_tcp_md_open,
    .cm_open            = UCS_CLASS_NEW_FUNC_NAME(uct_tcp_sockcm_t),
    .rkey_unpack        = (uct_component_rkey_unpack_func_t)ucs_empty_function_return_unsupported,
    .rkey_ptr           = (uct_component_rkey_ptr_func_t)ucs_empty_function_return_unsupported,
    .rkey_release       = (uct_component_rkey_release_func_t)ucs_empty_function_return_unsupported,
    .rkey_compare       = (uct_component_rkey_compare_func_t)ucs_empty_function_return_unsupported,
    .name               = UCT_TCP_NAME,
    .md_config          = {
        .name           = "TCP memory domain",
        .prefix         = "TCP_",
        .table          = uct_tcp_md_config_table,
        .size           = sizeof(uct_tcp_md_config_t)
    },
    .cm_config          = {
        .name           = "TCP-SOCKCM connection manager",
        .prefix         = "TCP_CM_",
        .table          = uct_tcp_sockcm_config_table,
        .size           = sizeof(uct_tcp_sockcm_config_t),
     },
    .tl_list            = UCT_COMPONENT_TL_LIST_INITIALIZER(&uct_tcp_component),
    .flags              = UCT_COMPONENT_FLAG_CM,
    .md_vfs_init        = (uct_component_md_vfs_init_func_t)ucs_empty_function
};
