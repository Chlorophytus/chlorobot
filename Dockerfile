# =============================================================================
# Builder
# =============================================================================
FROM alpine:3.21 AS build

# install buildtime deps
RUN apk -U add --no-cache build-base cmake openssl-dev luajit-dev sqlite-dev \
    luarocks5.1 && \
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

# CMake build then regular build
RUN cmake -DCMAKE_BUILD_TYPE=Release -Bbuild && \
    make -C build/ -j 
# =============================================================================
# Runner
# =============================================================================
from alpine:3.21

# install runtime deps
RUN apk -U add --no-cache libstdc++ openssl luajit sqlite-libs 

# copy over built luarocks deps
COPY --from=build /usr/local /usr/local

# symlink to correct sqlite3 name
RUN ln -s /usr/lib/libsqlite3.so.0 /usr/lib/libsqlite3.so 

# set up chlorobot serving
WORKDIR /srv/chlorobot
RUN chown nobody /srv/chlorobot 
COPY --from=build --chown=nobody:root /opt/chlorobot/build/chlorobot ./

# we are good now
USER nobody
ENTRYPOINT [ "/srv/chlorobot/chlorobot" ]
