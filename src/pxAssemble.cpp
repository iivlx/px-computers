#include <string>
#include <iostream>
#include <fstream>

/* Compiles a `px assembly` input file to binary using the `px-assembler` */
std::pair<uint8_t*, size_t> pxAssemble(std::string input_file, std::string output_file) {

  // assemble file

  std::string command = "py tools/px-assembler/px-assembler.py " + input_file + " -o " + output_file + " 2>out/px-assembler.log";
  int result = std::system(command.c_str());

  // check errors

  std::ifstream error_file("out/px-assembler.log");
  std::string line;

  if (result != 0) {
    std::cerr << "Assembly failed:" << std::endl;
    if (error_file.peek() != std::ifstream::traits_type::eof()) {
      while (std::getline(error_file, line)) {
        std::cerr << "  " << line << std::endl;
      }
    }
    std::cerr << "Fatal!" << std::endl;
    exit(1);
  }

  // load assembly

  std::ifstream binary_file(output_file, std::ios::binary | std::ios::ate); // at the end, to get size
  if (!binary_file) { 
    std::cerr << "Failed to open binary file..." << std::endl;
    exit(1);
  }

  size_t code_size = binary_file.tellg(); // size
  uint8_t* machine_code = new uint8_t[code_size];

  binary_file.seekg(0);
  binary_file.read(reinterpret_cast<char*>(machine_code), code_size); // bytes
  binary_file.close();

  return { machine_code, code_size };

};
