# Dockerfile to run Conan server and pre-upload ahoi-module during image build
FROM python:3.11-alpine AS build

# Install runtime and build dependencies
RUN apk add --no-cache \
    git \
    netcat-openbsd \
    build-base \
    cmake

# Install Conan (pip package)
RUN pip install --no-cache-dir conan conan-server; \
    rm -rf /root/.cache/pip
RUN conan profile detect

WORKDIR /tmp
RUN git clone https://github.com/dmochkas/io-lib.git
RUN git clone https://github.com/dmochkas/ahoi-module.git
RUN git clone --branch ascon-v1.4-alpha.0 --single-branch  https://github.com/dmochkas/ascon-conanfile.git
WORKDIR /tmp/io-lib
RUN conan create .
WORKDIR /tmp/ahoi-module
RUN conan create .
WORKDIR /tmp/ascon-conanfile
RUN conan create .

ENV CONAN_SERVER_HOME=/var/conan_server
ENV CONAN_SERVER_PORT=9300

RUN mkdir -p /var/conan_server
WORKDIR /var/conan_server

COPY server.conf .

ARG CONAN_LOGIN_USERNAME
ARG CONAN_PASSWORD

RUN set -eux; \
    conan_server & \
    SERVER_PID=$!; \
    retries=5; \
    until nc -z 0.0.0.0 ${CONAN_SERVER_PORT} || [ "$retries" -le 0 ]; do \
        sleep 1; \
        retries=$((retries-1)); \
    done; \
    if [ "$retries" -le 0 ]; then exit 1; fi; \
    conan remote add local http://localhost:${CONAN_SERVER_PORT} --insecure; \
    conan remote auth local; \
    conan upload "io-lib/*" -r local --all --confirm; \
    conan upload "ahoi-module/*" -r local --all --confirm; \
    conan upload "ascon-c/*" -r local --all --confirm; \
    kill "$SERVER_PID"; \
    sleep 1;

FROM python:3.11-alpine AS runtime

RUN pip install --no-cache-dir conan-server

ENV CONAN_SERVER_HOME=/var/conan_server
ENV CONAN_SERVER_PORT=9300

RUN mkdir -p /var/conan_server
WORKDIR /var/conan_server

COPY --from=build /var/conan_server .

EXPOSE ${CONAN_SERVER_PORT}

CMD ["conan_server"]
