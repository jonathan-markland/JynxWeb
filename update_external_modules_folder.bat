@REM Update the emulator's ExternalModules folder by copying source files in.

@REM  TODO: delete the ExternalModules entire tree.

copy /Y ..\JynxFramework\JynxFramework\JynxFrameworkLibrary\* ExternalModules\JynxFrameworkLibrary\
copy /Y TapFileLibrary\* ExternalModules\TapFileLibrary\

@REM  Update the CassetteFPRefactor spike program's ExternalModules folder

@REM  TODO: delete the ExternalModules entire tree.

copy /Y ..\JynxFramework\JynxFramework\JynxFrameworkLibrary\* TechSpikes\CassetteFPRefactor\CassetteFPRefactor\ExternalModules\JynxFrameworkLibrary\
copy /Y TapFileLibrary\* TechSpikes\CassetteFPRefactor\CassetteFPRefactor\ExternalModules\TapFileLibrary\

