#!/usr/bin/env python3

import asyncio
from aiocoap import Context, Message, GET
from coap_utils import print_coap

async def main():
    # uri = "coap://[2001:db8::11]:5683/get"
    uri = "coap://[2001:db9::11]:5683/get"
    # uri = "coap://[::1]:5683/get"

    protocol = await Context.create_server_context(
        site=None,
        bind=("::", 12345)  # Bind to all IPv6 addresses on port 12345
    )

    request = Message(
        code=GET,
        uri=uri
    )

    print_coap(request, "COAP REQUEST")

    try:
        response = await protocol.request(request).response

        print_coap(response, "COAP RESPONSE")

    except Exception as e:
        print(f"\nRequest failed: {e}")


if __name__ == "__main__":
    asyncio.run(main())