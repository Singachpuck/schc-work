def print_coap(msg, title):
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
    print(flush=True)