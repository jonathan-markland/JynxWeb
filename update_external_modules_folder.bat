@REM Update the emulator's ExternalModules folder by copying source files in.

rd /S /Q ExternalModules
xcopy /E /Y ..\JynxFramework\JynxFramework\JynxFrameworkLibrary\ ExternalModules\JynxFrameworkLibrary\
xcopy /E /Y TapFileLibrary\* ExternalModules\TapFileLibrary\
attrib ExternalModules\JynxFrameworkLibrary\*.* +r
attrib ExternalModules\TapFileLibrary\*.* +r

@REM  Update the CassetteFPRefactor spike program's ExternalModules folder

rd /S /Q TechSpikes\CassetteFPRefactor\CassetteFPRefactor\ExternalModules
xcopy /E /Y ..\JynxFramework\JynxFramework\JynxFrameworkLibrary\ TechSpikes\CassetteFPRefactor\CassetteFPRefactor\ExternalModules\JynxFrameworkLibrary\
xcopy /E /Y TapFileLibrary\* TechSpikes\CassetteFPRefactor\CassetteFPRefactor\ExternalModules\TapFileLibrary\
attrib TechSpikes\CassetteFPRefactor\CassetteFPRefactor\ExternalModules\JynxFrameworkLibrary\*.* +r
attrib TechSpikes\CassetteFPRefactor\CassetteFPRefactor\ExternalModules\TapFileLibrary\*.* +r

