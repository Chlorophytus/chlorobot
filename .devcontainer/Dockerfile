# =============================================================================
# Builder
# =============================================================================
FROM alpine:3 AS develop

# install buildtime deps
RUN apk -U add --no-cache build-base cmake openssl-dev luajit-dev sqlite-dev \
    luarocks5.1 linux-headers git && \
    mkdir -p /opt/chlorobot/build

# set up build dir
WORKDIR /opt/chlorobot

# copy source
COPY CMakeLists.txt .
COPY src/ ./src
COPY include/ ./include

# luarocks deps install globally
RUN luarocks-5.1 --global config variables.LUA_INCDIR /usr/include/luajit-2.1 && \
    luarocks-5.1 --global install luv && \
    luarocks-5.1 --global install sqlite && \
    luarocks-5.1 --global install luasec && \
    luarocks-5.1 --global install luasocket

RUN addgroup -S vscode && \
    adduser -S vscode -G vscode -s /bin/sh