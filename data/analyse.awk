#!/usr/bin/awk -f analyse.awk .svn/entries

BEGIN {
  FS="="
  name=""
  files=""
  directories=""
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
  if ( $0 ~ "name=" )
  {
    split($2,tmp,"\"")
    name=tmp[2]
    #print "-->" name
  }
  if ( $0 ~ "kind=" )
  {
    split($2,tmp,"\"")
    if  ( tmp[2] == "dir" )
    {
      if ( name != "" )
      {
        #print "Should enter " whereami "/" name
        directories=directories "\"" name "\" "
        system("/usr/bin/awk -f \"" whereisorignalscript "\" -v whereisorignalscript=\"" whereisorignalscript "\" -v whereami=\"" whereami name "/\" \"" whereami name "/.svn/entries\"")
      }
    }
    else
    {
      if ( tmp[2] == "file" && name != "" && name != "Makefile" && name != "Datarules.mk" && name != "analyse.awk" )
      {
        files=files "\"" name "\" "
        #print name
      }
      else
      {
        #print "Kind " tmp[2] " unkown !"
        #print $0
      }
    }
    name=""
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
