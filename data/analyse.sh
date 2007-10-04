#!/bin/sh

# Fichier temporaire
tmpfile=tmp

# Pour avoir un sort qui trie pareil sur toutes les machines
LANG=C

# Warning...
echo "Ce script cree les Makefiles de tous les dossiers contenus dans 'data'."
echo "Tous les fichiers et dossiers doivent etre commites au prealable."
echo "Ce script ne gere pas les chemins contenant des espaces."

# Recuperation des fichiers sur le svn
svn -R ls > ${tmpfile}

# Extraction des Makefiles
MAKEFILES=$(grep Makefile ${tmpfile} | grep "/")

# Verification qu'il existe un Makefiles par dossier
FOLDERSTOCHECK=$(grep -E '^.*/$' ${tmpfile}  | grep -v Makefile | sort -u)
for i in ${FOLDERSTOCHECK}
do
  found=0
  k="${i%/*}/Makefile"
  for j in ${MAKEFILES}
  do
    if [ "${k}" == "${j}" ]
    then
      found=1
    fi
  done
    if [ ! ${found} -eq 1 ]
    then
      echo "WARNING: Le dossier ${i} dans le svn ne contient pas de Makefile."
      echo "Creation et ajout dans le svn."
      touch "$k"
      svn add ${k}
      MAKEFILES="${MAKEFILES} ${k}"
    fi
done

# Pour chaque makefile
for i in ${MAKEFILES}
do
  echo "Updating $i"
  
  # Ajouter les fichiers
  FILES=$(grep -E '^'"${i%Makefile}"'[^/]+$' ${tmpfile}  | grep -v Makefile | sort)
  FI="FILES="
  for j in ${FILES}
  do
    FI="${FI} \"${j##*/}\""
  done
  echo "${FI}" > ${i}

  # Ajouter les dossiers
  FOLDERS=$(grep -E '^'"${i%Makefile}"'[^/]+/$' ${tmpfile}  | grep -v Makefile | sort)
  FO="DIRECTORIES="
  for j in ${FOLDERS}
  do
    j=${j%/*}
    FO="${FO} \"${j##*/}\""
  done
  echo "${FO}" >> ${i}

  # Inserer les regles
  echo 'include ${SOURCE_DATADIR}/Datarules.mk' >> ${i}

done

# Nettoyage
rm -f tmp

# Commit
svn commit -m "Mise a jour des Makefiles des donnees" ${MAKEFILES}
