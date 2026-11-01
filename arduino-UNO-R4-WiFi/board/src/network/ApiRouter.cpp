#include "ApiRouter.h"

ApiRouter::ApiRouter(LEDService& ledService) : ledHandler_(ledService) {}

HttpResponse ApiRouter::dispatch(const String& requestLine) {
  HttpResponse response{};
  if (ledHandler_.handle(requestLine, response)) {
    return response;
  }

  return {404, "Not Found", "{\"error\":\"Unknown endpoint\"}"};
}
