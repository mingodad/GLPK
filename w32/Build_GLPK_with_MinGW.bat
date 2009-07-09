rem Build GLPK with MinGW

rem NOTE: Make sure that HOME variable specifies correct path.
set HOME=C:\MinGW

set PATH=%HOME%\bin;%HOME%\libexec\gcc\mingw32\3.4.5;%PATH%
copy config_MinGW config.h
%HOME%\bin\mingw32-make.exe -f Makefile_MinGW
%HOME%\bin\mingw32-make.exe -f Makefile_MinGW check

pause
