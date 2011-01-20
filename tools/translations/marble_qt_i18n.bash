#!/bin/bash

# A script to download Marble Qt translations from KDE's SVN,
# and transform them to binary .qm format.
#
# This program is free software licensed under the GNU LGPL. You can
# find a copy of this license in LICENSE.txt in the top directory of
# the source code.
#
# Copyright 2011     Dennis Nienhüser <earthwings@gentoo.org>
#

set -e

test -x ./merge_ts_po || { echo "./merge_ts_po is not there. Please compile it: g++ -o merge_ts_po merge_ts_po.cpp -I /usr/include/qt4 -lQtCore."; exit 1; }

workdir="$(mktemp -d)"

TAG="4.6.0"
prefix="svn://anonsvn.kde.org/home/kde/tags/KDE/${TAG}/kde-l10n/"
# Translations can also be loaded from SVN trunk, uncomment below.
#prefix="svn://anonsvn.kde.org/home/kde/trunk/l10n-kde4"

echo "Generating translation template"
lupdate $(find ../../ -name "*.cpp" -o -name "*.h" -o -name "*.ui" | xargs echo) -ts "${workdir}/template.ts"

echo "Processing translations, please wait. This can take some time..."

svn -q export "${prefix}/subdirs" "${workdir}/subdirs"
for i in $(cat "${workdir}/subdirs")
do
	if svn -q export "${prefix}/${i}/messages/kdeedu/marble_qt.po" "${workdir}/marble_qt.po" 2>/dev/null
        then
          # Convert to binary .qm file format
          ./merge_ts_po "${workdir}/template.ts" "${workdir}/marble_qt.po" > "${workdir}/marble_qt.ts"
          sed -i 's@DownloadRegionDialog::Private@DownloadRegionDialog@' "${workdir}/marble_qt.ts"
          lconvert "${workdir}/marble_qt.ts" -o "marble_qt_${i}.qm"

	  # If you need the .ts file, uncomment below
          # cp "${workdir}/marble_qt.ts" "marble_qt_${i}.ts"
          mv "marble_qt_${i}.qm" "marble-${i}"
        fi
done

rm "${workdir}/template.ts"
rm "${workdir}/marble_qt.po"
rm "${workdir}/marble_qt.ts"
rm "${workdir}/subdirs"
rmdir "${workdir}"

echo "Done. The files marble-* contain the translations in .qm format"
