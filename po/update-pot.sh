#!/bin/sh
set -e
cd $(dirname $0)
PO_DIR=$PWD

PKGNAME="indicator-location"
PROGNAME=$(basename $0)

# Create a list of files to scan
GETTEXT_FILES=$(mktemp --tmpdir uitk-unity.lst.XXXXX)
trap 'rm -f "$GETTEXT_FILES"' EXIT
cd ..
find \( -name '*.h' -o -name '*.cpp' -o -name '*.cc' -o -name '*.c' -o -name '*.qml' -o -name '*.js' \) \
    -a ! \( -path './debian/*' -o -path './builddir/*' -o -path './build/*' -o -path './.bzr/*' \) | sort \
> $GETTEXT_FILES
cat $GETTEXT_FILES

# Generate pot from our list
xgettext \
    --output $PO_DIR/$PKGNAME.pot \
    --files-from $GETTEXT_FILES \
    --c++ \
    --add-comments=TRANSLATORS \
    --keyword=_ \
    --package-name="$PKGNAME" \
    --copyright-holder="Canonical Ltd."

echo "$PROGNAME: $PO_DIR/$PKGNAME.pot updated"
