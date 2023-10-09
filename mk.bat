REM /O1 /GR- /GX-
if "%CC%"=="" set CC=cl
%CC% fart.cpp fart_shared.c wildmat.c %*
