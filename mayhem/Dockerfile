# Commit image for DirectXTK (savantenvs). Builds the WAV / XACT wave bank parsers on Linux
# against mayhem/shim; see mayhem/build.sh.
FROM ghcr.io/savantenvs/base:20261023

ARG COMMIT
ARG REPO_URL
ARG SOURCE_DATE_EPOCH

ARG SANITIZER_FLAGS
ARG COVERAGE_FLAGS=
ARG MAYHEM_JOBS=

# DirectXMath (MIT, header-only; Inc/Audio.h includes it) pinned by commit + sha256, cached under
# /opt/toolchains/deps so the air-gapped build.sh re-run resolves it with no network.
USER root
RUN set -eux; \
    d=/opt/toolchains/deps/directxmath-6ac6a3c; \
    curl -fsSL -o /tmp/dxm.tgz https://github.com/microsoft/DirectXMath/archive/6ac6a3c51d4d99237656f22486ae5177c14c565d.tar.gz; \
    echo "0e122170d259c73a101c807fce56a1c6b2f91c9a073430b27c15aaffc043f8d4  /tmp/dxm.tgz" | sha256sum -c -; \
    mkdir -p "$d/src"; tar -xzf /tmp/dxm.tgz -C "$d/src" --strip-components=1; rm /tmp/dxm.tgz; \
    chmod -R a+rX,go-w "$d"

USER mayhem
WORKDIR /mayhem
COPY --chown=2000:2000 . /mayhem

RUN if [ -x /opt/mayhem/ccache/prefill ]; then /opt/mayhem/ccache/prefill bash mayhem/build.sh; \
    else bash mayhem/build.sh; fi

RUN if [ -f mayhem/test.sh ]; then bash mayhem/test.sh; else echo "no mayhem/test.sh — repo ships no functional tests"; fi

LABEL org.opencontainers.image.source="${REPO_URL}" \
      org.opencontainers.image.revision="${COMMIT}" \
      rlenv.tier="commit" \
      rlenv.source_date_epoch="${SOURCE_DATE_EPOCH}" \
      rlenv.user="mayhem"

USER mayhem
WORKDIR /mayhem
