import asyncio

import aiocoap, logging
from aiocoap import Context

from coap_utils import print_coap

logging.basicConfig(level=logging.DEBUG)

async def main():
    # uri = "coap://[::1]/observe"
    uri = "coap://[2001:db9::11]:5683/observe"
    # uri = "coap://[2001:db8::11]:5683/observe"

    protocol = await Context.create_server_context(
        site=None,
        bind=("::", 12345)  # Bind to all IPv6 addresses on port 12345
    )

    request = aiocoap.Message(
        code=aiocoap.GET,
        uri=uri,
        observe=0,
    )

    requester = protocol.request(request)

    try:
        first_response = await requester.response
        print_coap(first_response, "COAP OBSERVE RESPONSE")

        async for notification in requester.observation:
            print_coap(notification, "COAP OBSERVE NOTIFICATION")

    finally:
        requester.observation.cancel()
        await protocol.shutdown()


if __name__ == "__main__":
    asyncio.run(main())