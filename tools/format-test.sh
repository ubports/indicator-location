#!/bin/sh

# Copyright (C) 2016 Canonical Ltd
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Tests to see if a file is in the project's format.

usage()
{
    echo usage: format-test project_dir formatcode 2>&1
    exit 1
}

[ $# -ne 2 ] && usage

dir="$1"
formatcode="$2"

exitval=0
for file in $(find "$dir" -name '*.h' -o -name '*.cpp' -o -name '*.c' -o -name '*.cc' -print0 | xargs -0);
do
    results=$(cat $file | $formatcode | diff -u $file -)
    linecount=$(echo "$var" | wc -l)
    if test $linecount -gt 1
    then
        basecmd=$(basename $0)
        echo "$basecmd FAIL $file"
        echo "${results}"
        exitval=1
    fi
done

exit $exitval
