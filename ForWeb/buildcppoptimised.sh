clang --std=c++17 -O2 --target=wasm32 --no-standard-libraries -fno-rtti -matomics -mbulk-memory -Wl,--max-memory=16777216 -Wl,--shared-memory -Wl,--import-memory -Wl,--export-all -Wl,--no-entry -o wwwroot/jynx-emulator.wasm ../Portable/WasmNeverFreeingMemoryAllocator.cpp ../Portable/Z80/JynxZ80.cpp ../Portable/Z80/JynxZ80Disassembler.cpp ../Portable/Z80/JynxZ80MainSet.cpp ../Portable/Z80/JynxZ80Shared.cpp ../Portable/Z80/JynxZ80Timings.cpp ../Portable/Z80/JynxZ80_CB.cpp ../Portable/Z80/JynxZ80_ED.cpp ../Portable/JynxFrameworkPanic.cpp ../Portable/JynxFramework.cpp JynxJavascriptInterfacing.cpp ../Portable/LynxHardware/LynxComputer.cpp ../Portable/LynxHardware/LynxAddressSpaceDecoder.cpp ../Portable/LynxHardware/LynxROMsAndRAMs.cpp ../Portable/LynxHardware/LynxScreen.cpp ../Roms/lynx48-1.cpp ../Roms/lynx48-2.cpp ../Roms/lynx96-1.cpp ../Roms/lynx96-2.cpp ../Roms/lynx96-3-scorpion.cpp ../Roms/lynx96-3.cpp