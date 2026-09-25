import asyncio

import aiocoap.resource as resource
import aiocoap

port = 5683

class RootResource(resource.Resource):
    async def render_get(self, request):
        print(
            "Received CoAP request:",
            {
                "remote": str(request.remote),
                "code": str(request.code),
                "token": request.token.hex(),
                "mid": request.mid,
                "uri_path": request.opt.uri_path,
                "uri_query": request.opt.uri_query,
                "payload_len": len(request.payload),
            },
            flush=True,
        )
        return aiocoap.Message(payload=b"\x01")


async def main():
    root = resource.Site()
    root.add_resource([], RootResource())
    await aiocoap.Context.create_server_context(root, bind=("::", port))
    await asyncio.get_running_loop().create_future()


if __name__ == "__main__":
    asyncio.run(main())