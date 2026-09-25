FROM python:3.11-alpine AS build

# Install runtime and build dependencies
RUN apk add --no-cache \
    linux-headers \
    build-base \
    cmake

ENV CONAN_SERVER_PORT=9300

WORKDIR /opt/schc_worker

RUN pip install --no-cache-dir conan && \
    conan profile detect && \
    conan remote add conan-server http://host.docker.internal:$CONAN_SERVER_PORT --insecure

COPY conanfile.txt .
RUN conan install . --build=missing

COPY . .

ARG MODE
ARG L2_UDP_REMOTE_IP

RUN cmake -S . -B build/Release -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=generators/conan_toolchain.cmake \
    -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_BUILD_TYPE=Release -DWITH_TESTS=OFF -DPROFILE_NAME=udp_schc_al_oscore_$MODE \
    -DL2_UDP_REMOTE_IP=$L2_UDP_REMOTE_IP \
    -DCMAKE_INSTALL_PREFIX=/install && \
    cmake --build build/Release && \
    cmake --install build/Release

# Final minimal runtime image
FROM python:3.11-alpine AS runtime

RUN apk add --no-cache tcpdump socat
RUN pip install --no-cache-dir aiocoap

WORKDIR /usr/local/bin/

COPY --from=build /opt/schc_worker/tests/e2e/ .
COPY --from=build /install/bin/ .

ENTRYPOINT ["schc-worker-linux"]
#ENTRYPOINT ["run_ipv6_whitelist.sh", "schc-worker-linux", "2001:db8::/64"]
#ENTRYPOINT ["tail", "-f", "/dev/null"]
