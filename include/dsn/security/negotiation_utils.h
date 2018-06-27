/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Microsoft Corporation
 *
 * -=- Robust Distributed System Nucleus (rDSN) -=-
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once

#include <dsn/dist/replication/replication.codes.h>
#include <dsn/security/security.types.h>
#include <dsn/security/security_types.h>
#include <dsn/security/sasl_utils.h>
#include <dsn/tool-api/network.h>
#include <dsn/tool-api/rpc_message.h>

namespace dsn {
namespace security {

struct sasl_deleter
{
    void operator()(sasl_conn_t *conn) { sasl_dispose(&conn); }
};

typedef std::function<void(negotiation_message &)> negotiation_recv_callback;

inline const char *enum_to_string(negotiation_status::type s)
{
    switch (s) {
    case negotiation_status::type::SASL_SUCC:
        return "negotiation_succ";
    case negotiation_status::type::SASL_AUTH_FAIL:
        return "negotiation_auth_fail";
    case negotiation_status::type::SASL_INITIATE:
        return "negotiation_initiate";
    case negotiation_status::type::SASL_CHALLENGE:
        return "negotiation_challenge";
    case negotiation_status::type::SASL_RESPONSE:
        return "negotiation_response";
    case negotiation_status::type::INVALID:
        return "negotiation_invalid";
    }
    return "negotiation-unkown";
}

inline void async_send_negotiation_msg(rpc_session_ptr session, const negotiation_message &req)
{
    dassert(session != nullptr, "invalid rpc_session");
    dsn_message_t msg = dsn_msg_create_request(RPC_NEGOTIATION, 0);
    ::dsn::marshall(msg, req);
    session->send_negotiation_message(reinterpret_cast<message_ex *>(msg));
}

// Notice: this function will try to release req automatically, if caller want use req after this
// function, you should add_ref before call this function
inline void async_response_negotiation_msg(rpc_session_ptr session,
                                           message_ex *req,
                                           const negotiation_message &resp)
{
    dassert(session != nullptr, "invalid rpc_session");
    message_ex *response = req->create_response();
    strncpy(response->header->server.error_name,
            ERR_OK.to_string(),
            sizeof(response->header->server.error_name));
    response->header->server.error_code.local_code = ERR_OK; // rpc is ok
    response->header->server.error_code.local_hash = message_ex::s_local_hash;
    ::dsn::marshall(response, resp);
    session->send_message(response);

    // Don't use delete directly
    req->add_ref();
    req->release_ref();
}

} // end namespace security
} // end namespace dsn
