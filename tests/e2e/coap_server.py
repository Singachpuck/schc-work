import asyncio
import os

import aiocoap.resource as resource
import aiocoap, logging
from coap_utils import print_coap

logging.basicConfig(level=logging.DEBUG)

port = 5683

class RootResource(resource.Resource):
    async def render_get(self, request):
        print_coap(request, "COAP GET REQUEST")
        return aiocoap.Message(payload=os.urandom(1))


class ObserveResource(resource.ObservableResource):
    def __init__(self):
        super().__init__()
        self._task = None
        self._notification_count = 0

    def update_observation_count(self, count):
        if count and self._task is None:
            self._task = asyncio.create_task(self._notify_loop())
        elif not count and self._task is not None:
            self._task.cancel()
            self._task = None

    async def _notify_loop(self):
        try:
            while True:
                await asyncio.sleep(5)
                self.updated_state()
        except asyncio.CancelledError:
            pass

    async def render_get(self, request):
        print("Sending notification")

        return aiocoap.Message(payload=os.urandom(2))


async def main():
    root = resource.Site()
    root.add_resource(["get"], RootResource())
    root.add_resource(["observe"], ObserveResource())
    await aiocoap.Context.create_server_context(root, bind=("::", port))
    await asyncio.get_running_loop().create_future()


if __name__ == "__main__":
    asyncio.run(main())