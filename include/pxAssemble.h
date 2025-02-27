#include <string>

/* Assembles an input string into px machine code... */
std::pair<uint8_t*, size_t> pxAssemble(std::string input_file, std::string output_file="out/output.pbin");
