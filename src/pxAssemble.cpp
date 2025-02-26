#include <string>

/* This should call the assemlber, but for now just return hardcoded machine instructions... */
/* This should take the input string and pass it to the px-assembler python program, and then
 * return the machine code bytes and size...
 */
std::pair<uint8_t*, size_t> pxAssemble(std::string code) {
  // returns some assembly code...
  uint8_t test_code[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  };

  size_t code_size = sizeof(test_code);
  uint8_t* machine_code = new uint8_t[code_size];
  for (size_t i = 0; i < code_size; ++i) {
    machine_code[i] = test_code[i];
  }
  return { machine_code, code_size };
};
