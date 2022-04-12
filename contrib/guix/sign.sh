#!/bin/bash

if RECENT_TAG="$(git describe --exact-match HEAD)"; then
    VERSION="${RECENT_TAG#v}"
else
    VERSION="$(git rev-parse --short=12 HEAD)"
fi

cd output

KEY_FILE=${KEY_FILE:-~/codesigning/vertcoin-project.p12}
if [[ ! -f "$KEY_FILE" ]]; then
    ls $KEY_FILE
    echo "Make sure that $KEY_FILE exists"
fi

if ! which osslsigncode > /dev/null 2>&1; then
    echo "Please install osslsigncode"
fi

rm -rf signed
mkdir -p signed >/dev/null 2>&1

unzip vertcoin-$VERSION-win64.zip -d signed
cd signed/vertcoin-$VERSION/bin

echo "Found $(ls *.exe | wc -w) files to sign."
for f in $(ls *.exe); do
    echo "Signing $f..."
    osslsigncode sign \
      -h sha256 \
      -pkcs12 "$KEY_FILE" \
      -pass "$VERTCOIN_PROJECT_KEY_PASSWORD" \
      -n "Vertcoin-Core" \
      -i "https://vertcoin.org/" \
      -t "http://timestamp.digicert.com/" \
      -in "$f" \
      -out "../../$f"
    ls ../../$f -lah
done

cd ../..
rm vertcoin-$VERSION/bin/*
mv vertcoin*.exe vertcoin-$VERSION/bin
zip -r vertcoin-$VERSION-win64.zip vertcoin-$VERSION
rm -r vertcoin-$VERSION

echo "Signing vertcoin-$VERSION-win64-setup..."
osslsigncode sign \
  -h sha256 \
  -pkcs12 "$KEY_FILE" \
  -pass "$VERTCOIN_PROJECT_KEY_PASSWORD" \
  -n "Vertcoin-Core" \
  -i "https://vertcoin.org/" \
  -t "http://timestamp.digicert.com/" \
  -in "../vertcoin-$VERSION-win64-setup-unsigned.exe" \
  -out "vertcoin-$VERSION-win64-setup.exe"
