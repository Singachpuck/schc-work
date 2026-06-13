#!/usr/bin/env python3

import asyncio
from aiocoap import Context, Message, GET


def print_message(msg, title):
    print(f"\n{'=' * 60}")
    print(title)
    print(f"{'=' * 60}")

    print(f"Code      : {msg.code}")

    if hasattr(msg, "mid") and msg.mid is not None:
        print(f"MID       : {msg.mid}")

    if hasattr(msg, "token"):
        print(f"Token     : {msg.token.hex()}")

    if hasattr(msg, "mtype") and msg.mtype is not None:
        print(f"Type      : {msg.mtype}")

    print(f"URI       : {msg.get_request_uri()}")

    print("\nOptions:")
    for option in msg.opt.option_list():
        print(f"  {option}")

    if msg.payload:
        try:
            payload = msg.payload.decode("utf-8")
        except UnicodeDecodeError:
            payload = msg.payload.hex()

        print("\nPayload:")
        print(payload)
    else:
        print("\nPayload: <empty>")


async def main():
    uri = "coap://[2001:db9::1]:5683/"

    protocol = await Context.create_client_context()

    request = Message(
        code=GET,
        uri=uri
    )

    print_message(request, "COAP REQUEST")

    try:
        response = await protocol.request(request).response

        print_message(response, "COAP RESPONSE")

    except Exception as e:
        print(f"\nRequest failed: {e}")


if __name__ == "__main__":
    asyncio.run(main())