" Vim syntax file
" Language:	Pascal
" Version: 2.7
" Last Change:	2003 May 11
" Maintainer:  Xavier Crégut <xavier.cregut@enseeiht.fr>
" Previous Maintainer:	Mario Eusebio <bio@dq.fct.unl.pt>

" Contributors: Tim Chase <tchase@csc.com>, Stas Grabois <stsi@vtrails.com>,
"	Mazen NEIFER <mazen.neifer.2001@supaero.fr>,
"	Klaus Hast <Klaus.Hast@arcor.net>

" For version 5.x: Clear all syntax items
" For version 6.x: Quit when a syntax file was already loaded
if version < 600
  syntax clear
elseif exists("b:current_syntax")
  finish
endif

syn case match
syn sync lines=250

syn keyword gslOperator	mod not
syn keyword gslRepeat	do while
syn keyword gslStatement	declare external
syn keyword gslType		int float ptr string boolean false true
syn keyword gslTodo             TODO contained

highlight link gslType Type
highlight link gslStatement Statement
highlight link gslRepeat Repeat
highlight link gslOperator Operator
highlight link gslTodo Todo

" String
syn region  gslString        start='"' skip=/\\"/ end='"'
highlight link gslString     String

syn match   gslIdentifier    "\<[a-zA-Z_][a-zA-Z0-9_]*\>"
syn match   gslFunctionCall1  /[a-zA-Z_][a-zA-Z0-9_]*:/
syn match   gslFunctionCall2  /\[[a-zA-Z_][a-zA-Z0-9_]*\]/
syn match   gslFunctionCall3  /^\ *[a-zA-Z_][a-zA-Z0-9_]*$/
" highlight link gslIdentifier Identifier
highlight link gslFunctionCall1 Function
highlight link gslFunctionCall2 Function
highlight link gslFunctionCall3 Function

syn match   gslSymbolOperator      "[+\-/*=]"
syn match   gslSymbolOperator      "[<>]=\="
syn match   gslSymbolOperator      "<>"
syn match   gslSymbolOperator      "[()]"
syn match   gslSymbolOperator      "\.\."
syn match   gslSymbolOperator       "[\^.]"

syn match  gslNumber		"-\=\<\d\+\>"
syn match  gslFloat		"-\=\<\d\+\.\d\+\>"
syn match  gslFloat		"-\=\<\d\+\.\d\+[eE]-\=\d\+\>"
syn match  gslHexNumber	"\$[0-9a-fA-F]\+\>"

highlight link gslNumber Number
highlight link gslFloat Float
highlight link gslHexNumber Number

if exists("gsl_no_tabs")
  syn match gslShowTab "\t"
endif

syn match  gslLineComment /\/\/.*/ contains=gslTODO
syn region gslComment	start="/\*" end="\*/"  contains=gslTODO

highlight link gslComment Comment
highlight link gslLineComment Comment

syn match gslFunction /<.*>/ contains=gslType
highlight link gslFunction Statement

syn match  gslPreproc /\#.*/
highlight link gslPreproc Preproc

let b:current_syntax = "gsl"

" vim: ts=8 sw=2
