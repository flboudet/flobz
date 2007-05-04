#!/usr/bin/awk -f analyse.awk .svn/entries

BEGIN {
  FS="="
  name=""
  files=""
  directories=""
  deleted=""
  entry=0
  kind=""
  if ( whereisorignalscript == "" )
  {
    whereisorignalscript=ENVIRON["PWD"] "/analyse.awk"
  }
  iamoriginal="no"
  if ( whereami == "" )
  {
    whereami=ENVIRON["PWD"] "/"
    iamoriginal="yes"
  }
  #print "My source script is there : " whereisorignalscript
}

{
  #On verifie les balises d'entrees
  if ( $0 ~ "<entry" )
  {
    entry = 1
  }
  

  # Test du parametre name
  if ( $0 ~ "name=" && entry == 1 )
  {
    split($2,tmp,"\"")
    name=tmp[2]
    #print "-->" name
  }

  # Test du parametre kind
  if ( $0 ~ "kind=" )
  {
    split($2,tmp,"\"")
    kind=tmp[2]
  }
  
  # Test du parametre deleted
  if ( $0 ~ "deleted=" )
  {
    split($2,tmp,"\"")
    deleted=tmp[2]
  }
  
  
  # On verifie les balises de sortie
  if ( $0 ~ "/>" )
  {
    # Si on etait dans une entry avec un nom et non effacee
    if ( entry == 1 && name != "" && deleted != "true" )
    {
      # Si c'Žtait un dossier, on le parse recursivement
      if  ( kind == "dir" )
      {
        #print "Should enter " whereami "/" name
        directories=directories "\"" name "\" "
        system("/usr/bin/awk -f \"" whereisorignalscript "\" -v whereisorignalscript=\"" whereisorignalscript "\" -v whereami=\"" whereami name "/\" \"" whereami name "/.svn/entries\"")
      }
      # Sinon on traite le fichier, sauf exception
      else
      {
        if ( kind == "file" && name != "Makefile" && name != "Datarules.mk" && name != "analyse.awk" )
        {
          files=files "\"" name "\" "
          #print name
        }
      }
    }
    name = ""
    entry = 0
    deleted = ""
    kind= ""
  }
}

END {
#  print "I am here : " whereami
#  print "Makefile states : "
#  system("/usr/bin/grep FILES \"" whereami "\"/Makefile")
#  system("/usr/bin/grep DIRECTORIES \"" whereami "\"/Makefile")
#  print "I FOUND :"
#  print "FILES=" files
#  print "DIRECTORIES=" directories
  destfile=whereami "/Makefile"
  print "FILES=" files >destfile
  print "" >destfile
  print "DIRECTORIES=" directories >destfile
  print "" >destfile
  if ( iamoriginal == "yes" )
  {
    print "SOURCE_DATADIR:=$(shell pwd)" >destfile
    print "" >destfile
    print "include Datarules.mk" >destfile
  }
  else
  {
    print "include ${SOURCE_DATADIR}/Datarules.mk" >destfile
  }
}
