#!/bin/sh

tmpfile=tmp

echo "Ce script cree les Makefiles de tous les dossiers contenus dans 'data'."
echo "Tous les fichiers et dossiers doivent etre commites au prealable."

svn -R ls > ${tmpfile}

MAKEFILES=$(grep Makefile ${tmpfile} | grep "/")

for i in ${MAKEFILES}
do
  echo "Updating $i"
  
  FILES=$(grep -E '^'"${i%Makefile}"'[^/]+$' ${tmpfile}  | grep -v Makefile | sort)
  FI="FILES="
  for j in ${FILES}
  do
    FI="${FI} \"${j##*/}\""
  done
  echo "${FI}" > ${i}

  FOLDERS=$(grep -E '^'"${i%Makefile}"'[^/]+/$' ${tmpfile}  | grep -v Makefile | sort)
  FO="DIRECTORIES="
  for j in ${FOLDERS}
  do
    j=${j%/*}
    FO="${FO} \"${j##*/}\""
  done
  echo "${FO}" >> ${i}

  echo 'include ${SOURCE_DATADIR}/Datarules.mk' >> ${i}
    
done

rm -f tmp