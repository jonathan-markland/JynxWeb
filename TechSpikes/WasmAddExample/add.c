// add.c

// clang --target=wasm32 --no-standard-libraries -Wl,--export-all -Wl,--no-entry -o add.wasm add.c

int add (int first, int second)
{
  return first + second;
}