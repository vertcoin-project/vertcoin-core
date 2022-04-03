#!/bin/bash
# from https://github.com/metabrainz/picard/blob/e1354632d2db305b7a7624282701d34d73afa225/scripts/package/macos-notarize-app.sh

VERSION="$(git rev-parse --short=12 HEAD)"

# Code Signing: See https://developer.apple.com/library/archive/documentation/Security/Conceptual/CodeSigningGuide/Procedures/Procedures.html
if [ -n "$CODESIGN_CERT" ]; then
    # Test the identity is valid for signing by doing this hack. There is no other way to do this.
    cp -f /bin/ls ./CODESIGN_TEST
    set +e
    codesign -s "$CODESIGN_CERT" --dryrun -f ./CODESIGN_TEST > /dev/null 2>&1
    res=$?
    set -e
    rm -f ./CODESIGN_TEST
    if ((res)); then
        echo "Code signing identity \"$CODESIGN_CERT\" appears to be invalid."
        exit 1
    fi
    unset res
    echo "Code signing enabled using identity \"$CODESIGN_CERT\""
else
    exit 1
fi

function DoCodeSign { # ARGS: infoName fileOrDirName
    infoName="$1"
    file="$2"
    deep=""
    if [ -z "$CODESIGN_CERT" ]; then
        # no cert -> we won't codesign
        return
    fi
    if [ -d "$file" ]; then
        deep="--deep"
    fi
    if [ -z "$infoName" ] || [ -z "$file" ] || [ ! -e "$file" ]; then
        echo "Argument error to internal function DoCodeSign()"
        exit 1
    fi
    hardened_arg="-o runtime"

   echo "Code signing ${infoName}..."
    codesign -f -v $deep -s "$CODESIGN_CERT" $hardened_arg "$file"
}

DoCodeSign ".app" "output/Vertcoin-Qt.app"

if [ -z "$APPLE_ID_USER" ] || [ -z "$APPLE_ID_PASSWORD" ]; then
    echo "You need to set your Apple ID credentials with \$APPLE_ID_USER and \$APPLE_ID_PASSWORD."
    exit 1
fi

# Package app for submission
echo "Generating ZIP archive Vertcoin-Qt.zip..."
ditto -c -k --rsrc --keepParent "output/Vertcoin-Qt.app" "output/Vertcoin-Qt.zip"

# Submit for notarization
echo "Submitting Vertcoin-Qt for notarization..."
RESULT=$(xcrun altool --notarize-app --type osx \
  --file "output/Vertcoin-Qt.zip" \
  --primary-bundle-id org.vertcoin.Vertcoin-Qt \
  --username $APPLE_ID_USER \
  --password @env:APPLE_ID_PASSWORD \
  --output-format xml)

if [ $? -ne 0 ]; then
  echo "Submitting Vertcoin-Qt failed:"
  echo "$RESULT"
  exit 1
fi

REQUEST_UUID=$(echo "$RESULT" | xpath \
  "//key[normalize-space(text()) = 'RequestUUID']/following-sibling::string[1]/text()" 2> /dev/null)

if [ -z "$REQUEST_UUID" ]; then
  echo "Submitting Vertcoin-Qt failed:"
  echo "$RESULT"
  exit 1
fi

echo "$(echo "$RESULT" | xpath \
  "//key[normalize-space(text()) = 'success-message']/following-sibling::string[1]/text()" 2> /dev/null)"

# Poll for notarization status
echo "Submitted notarization request $REQUEST_UUID, waiting for response..."
sleep 60
while :
do
  RESULT=$(xcrun altool --notarization-info "$REQUEST_UUID" \
    --username "$APPLE_ID_USER" \
    --password @env:APPLE_ID_PASSWORD \
    --output-format xml)
  STATUS=$(echo "$RESULT" | xpath \
    "//key[normalize-space(text()) = 'Status']/following-sibling::string[1]/text()" 2> /dev/null)

  if [ "$STATUS" = "success" ]; then
    echo "Notarization of Vertcoin-Qt succeeded!"
    break
  elif [ "$STATUS" = "in progress" ]; then
    echo "Notarization in progress..."
    sleep 20
  else
    echo "Notarization of Vertcoin-Qt failed:"
    echo "$RESULT"
    exit 1
  fi
done

# Staple the notary ticket
xcrun stapler staple "output/Vertcoin-Qt.app"

# rm zip
rm "output/Vertcoin-Qt.zip"

echo "Creating .DMG"
hdiutil create -fs HFS+ -volname Vertcoin-Qt -srcfolder output/Vertcoin-Qt.app output/vertcoin-$VERSION-osx.dmg
DoCodeSign ".DMG" "output/vertcoin-$VERSION-osx.dmg"