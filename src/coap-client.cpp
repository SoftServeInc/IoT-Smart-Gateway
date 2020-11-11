//
// Created by faitc on 10/30/2020.
//

/* minimal CoAP client
 *
 * Copyright (C) 2018 Olaf Bergmann <bergmann@tzi.org>
 */

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>

#include <coap2/coap.h>

extern int resolve_address(const char*, const char*, coap_address_t*);

int send_coap_request(const std::string & url, const std::string & path, const std::string & port) {
  coap_context_t  *ctx = nullptr;
  coap_session_t *session = nullptr;
  coap_address_t dst;
  coap_pdu_t *pdu = nullptr;
  int result = EXIT_FAILURE;;

  coap_startup();

  /* resolve destination address where server should be sent */
  if (resolve_address(url.c_str(), port.c_str(), &dst) < 0) {
    std::cout << "failed to resolve address\n";
    goto finish;
  }

  /* create CoAP context and a client session */
  ctx = coap_new_context(nullptr);

  if (!ctx || !(session = coap_new_client_session(ctx, nullptr, &dst,
                                                  COAP_PROTO_UDP))) {
    std::cout << "cannot create client session\n";
    goto finish;
  }

  /* coap_register_response_handler(ctx, response_handler); */
  coap_register_response_handler(ctx, [](auto, auto, auto,
                                         coap_pdu_t *received,
                                         auto) {
      uint8_t *data = new uint8_t[1000];
      size_t len;
      coap_get_data(received, &len, &data);
      std::cout << "COAP: " << data << "\n";
                                        //coap_show_pdu(LOG_INFO, received);
                                      });
  /* construct CoAP message */
  pdu = coap_pdu_init(COAP_MESSAGE_CON,
                      COAP_REQUEST_GET,
                      0 /* message id */,
                      coap_session_max_pdu_size(session));
  if (!pdu) {
    std::cout << "cannot create PDU\n";
    goto finish;
  }

  /* add a Uri-Path option */
  coap_add_option(pdu, COAP_OPTION_URI_PATH, 5,
                  reinterpret_cast<const uint8_t *>(path.c_str()));

  /* and send the PDU */
  coap_send(session, pdu);

  coap_run_once(ctx, 0);

  result = EXIT_SUCCESS;
 finish:

  coap_session_release(session);
  coap_free_context(ctx);
  coap_cleanup();

  return result;
}