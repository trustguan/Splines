@IF [%1] EQU [] (SET YEAR=2013) else (SET YEAR=%1)
@IF [%2] EQU [] (SET BITS=x64)  else (SET BITS=%2)

@echo.
@powershell -command write-host -foreground "red" -background "yellow" -nonewline "Select Compiler Visual Studio %YEAR% "
@echo.

@IF %YEAR% == 2010 (
  @set STR="Visual Studio 10 2010"
) ELSE IF %YEAR% == 2012 (
  @set STR=Visual Studio 11 2012
) ELSE IF %YEAR% == 2013 (
  @set STR=Visual Studio 12 2013
) ELSE IF %YEAR% == 2015 (
  @set STR=Visual Studio 14 2015
) ELSE IF %YEAR% == 2017 (
  @set STR=Visual Studio 15 2017
) ELSE (
  @echo.
  @powershell -command write-host -foreground "red" -background "yellow" -nonewline "Unsupported %YEAR%"
  @echo.
  GOTO:eof
)

@IF "%BITS%" NEQ "x86" IF "%BITS%" NEQ "x64" (
  @echo.
  powershell -command write-host -foreground "red" -background "yellow" -nonewline "Unsupported ARCH %BITS%"
  @echo.
  GOTO:eof
)

@IF "%BITS%" == "x64" (@set STR=%STR% Win64)


@SET COMPILE="YES"
@IF EXIST lib\Debug\Splines.lib (
  @IF EXIST lib\Release\Splines.lib (
    @IF EXIST lib\include\Splines.hh (
      @SET COMPILE="NO"
	)
  )
)

@SET VSDIR=vs%YEAR%_%BITS%

@IF %COMPILE% == "YES" (

  @IF NOT EXIST GC (
    @echo.
    @powershell -command write-host -foreground "red" -background "yellow" -nonewline "Download GenericContainer"
    @echo.
    @rmdir /S GC
    @git clone --depth 1 git@github.com:ebertolazzi/GenericContainer.git GC
  )

  @RMDIR /S /Q %VSDIR%
  @mkdir %VSDIR%
  @cd %VSDIR%
  @cmake -G "%STR%" -D%LAPACK%=1 -DYEAR=%YEAR% -DBITS=%BITS% -DCMAKE_INSTALL_PREFIX:PATH=..\lib ..
  @cmake --build . --config Release --target Install
  @cmake --build . --config Debug --target Install
  @cd ..
) else (
  @echo.
  @powershell -command write-host -foreground "red" -background "yellow" -nonewline "Splines already compiled"
  @echo.
)

@echo.
@powershell -command write-host -foreground "red" -background "yellow" -nonewline "Splines all done!"
@echo.