import sys

from pprint import pprint
from collections import OrderedDict

encodeFloat = getattr(__import__('px-assembler_float'), 'encodeFloat') # from px-assembler_float import encodeFloat

""" px-assembler - Assembler for converting px assembly code into machine code.

The Assembler class reads an input file and translates the directives and instructions into a binary listing,
it then converts the binary listing into a binary output file.

Usage:

    asm = Assembler()
    asm.load_source("input.pa")
    asm.assemble()
    asm.write_output("output.pbin")

"""

STACK_MODES = {"^<>":0b111, "^<":0b110, "^>":0b101, "^":0b100, "<>":0b011, "<":0b010, ">":0b001, " ":0b000}
OPERAND_MODES = {"immediate16":0b000,
                 "immediate8" :0b001,
                 "direct16"   :0b010,
                 "direct8"    :0b011,
                 "indirect16" :0b100,
                 "indirect8"  :0b101,
                 }

class Assembler:
    """
    The assembler creates both a symbol table, and a section table:

    symbols = {
      "name": value
      "name2": value2
      ...
      "nameN": valueN
    }

    sections = {
      "name": [address, code]
      "name2": [address2, code2]
      ...
      "nameN": [addressN, codeN]
    }

    where:
    code = [instr, instr2, ..., instrN]


    Then the assembler performs symbol substitution, and machine code translation
    for instructions in each section, using the specified base addresses:

    binary_sections = {
      "name" : [byte0, byte1, ..., byteN]
      "name2" : [byte0, byte1, ..., byteN]
      ...
      "nameN" : [byte0, byte1, ..., byteN]
    }

    # Also maybe we could just add to the sections:
    # sections = {
    #   "name": [address, code, machine_code]
    #   "name2": [address2, code2, machine_code2]
    #   ...
    #   "nameN": [addressN, codeN, machine_codeN]
    # }

    """

    def __init__(self):
        self.source = None

        self.symbol_table = OrderedDict()
        self.section_table = OrderedDict()
        self.binary_table = OrderedDict()

        self.current_section = None
        self.current_address = None

    def assemble(self):
        self.parse_sections(self.source)
        self.assemble_binary()

    def assemble_binary(self):
        """ Assemble section table into binary sections """

        # assemble each section
        for section_name, section in self.section_table.items():
            for instruction in section[1]:

                # raw bytes
                if type(instruction[1]) == int:
                    self.binary_table[section_name].append(instruction[1])

                # operations
                elif type(instruction[1]) == list:

                    # translate assembly code into machine code
                    machine_code = self.assemble_instruction(instruction)

                    # write machine code
                    self.binary_table[section_name].extend(machine_code)
                    instruction.append(machine_code) # keep track so we can print out listing...

    def assemble_instruction(self, instruction):
      """ Assemble a specific instruction into machine code"""
      machine_code = []

      operation, stack_mode = self.resolve_operation_and_mode(instruction[1][0])

      opcode = self.get_opcode(operation)
      operands = instruction[1][1:]
      operands = self.resolve_operands(operands)

      mode_byte = 0

      # operands
      operand_bytes = []
      for mode, value in operands:

          # mode byte
          mode_byte <<= 4

          bits = OPERAND_MODES[mode]
          mode_byte += bits

          # operand bytes
          if mode in ["immediate8", "direct8", "indirect8"]:
              operand_bytes.append(0x00) # zero pad
              operand_bytes.append(value)

          if mode in ["immediate16", "direct16", "indirect16"]:
              byte_high = (value >> 8)
              byte_low = value & 0xFF

              operand_bytes.extend([byte_high, byte_low])

      if len(operands) < 2: mode_byte <<= 4 # always shift left operand to left half of the byte

      machine_code.append(opcode)
      machine_code.append(STACK_MODES[stack_mode])
      machine_code.append(mode_byte)
      machine_code.extend(operand_bytes)

      return machine_code

    def tokenize_operand(self, operand):
        """ tokenize a single operand """

        operand = operand.replace('[[', '(').replace(']]', ')') # normalize notation: [[a]] -> (a)

        tokens = []
        current_token = ""

        for char in operand:
            if char in "[]():!":
                if current_token:
                    tokens.append(current_token) # add each token
                    current_token = ""
                tokens.append(char) # add delimiter as separate token
            else:
                current_token += char

        if current_token:
            tokens.append(current_token) # add last token

        return tokens

    def tokenize_line(self, line):
        """ removes any comments or commas, since both are just for readability """
        line = line.split(";")[0].strip()
        line = line.replace(",", " ")
        tokens = line.split()
        return tokens

    def resolve_mode(self, tokens):
        """ resolve mode type from operand tokens """

        mode = "immediate"
        width = "16"

        # width
        if '#' in tokens:
            width = "8"
            tokens.remove('#')

        # mode
        if tokens[0] == '[' and tokens[-1] == ']':
            mode = "direct"
        if tokens[0] == '(' and tokens[-1] == ')':
            mode = "indirect"

        return mode+width
        
    def resolve_operation_and_mode(self, operation):
      """ split operation and stack mode """

      stack_mode = " "
      for mode in STACK_MODES.keys():
          if mode in operation:
              operation, _ = operation.split(mode, 1)
              stack_mode = mode

      return [operation, stack_mode]

    def resolve_operand(self, operand):
        """ resolve an operand into mode and value """

        operand = operand.strip()
        tokens = self.tokenize_operand(operand)

        mode = self.resolve_mode(tokens)
        value = 0

        # check each value / symobl
        for token in tokens:

            # symbol
            if token in self.symbol_table:
              value += self.symbol_table[token]

            # decimal
            elif token.isnumeric():
              value += int(token)

            # float
            elif '.' in token:
              value += encodeFloat(float(token))

            # hex
            elif token.startswith("0x"):
              value += int(token, 16)

        return [mode, value]

    def resolve_operands(self, operands):
        """ Resolve each operand mode and value based on its formatting..."""

        resolved_operands = [] # modes, values
        
        for operand in operands:
            resolved_operands.append(self.resolve_operand(operand))

        return resolved_operands

    def get_opcode(self, operation):

        # NOP

        if operation == "NOP":
            return 0x00

        # DATA 

        if operation == "MOV":
            return 0x01

        # ARITHMETIC 
        
        if operation == "ADD":
            return 0x20
        if operation == "SUB":
            return 0x21
        if operation == "MUL":
            return 0x22
        if operation == "DIV":
            return 0x23
            
        if operation == "MULS":
            return 0x30
        if operation == "DIVS":
            return 0x31
            
        if operation == "INC":
            return 0x41
            
        if operation == "HALF":
            return 0x50
        if operation == "UHALF":
            return 0x51

        if operation == "HADD":
            return 0x52
        if operation == "HSUB":
            return 0x53
        if operation == "HMUL":
            return 0x54
        if operation == "HDIV":
            return 0x55
            
        if operation == "ISQRT":
            return 0x58
        if operation == "SQRT":
            return 0x59
        if operation == "ATAN2":
            return 0x5A
        if operation == "SIN":
            return 0x5B

        # COMPARISON / JUMP

        if operation == "CMP":
            return 0x60
            
        if operation == "JEQ" or operation == "JMP":
            return 0x70
        if operation == "JNE":
            return 0x71
        if operation == "JLT":
            return 0x72
        if operation == "JLE":
            return 0x73
        if operation == "JGT":
            return 0x74
        if operation == "JGE":
            return 0x75

        return 0xffffffff # unknown opcode...

    def parse_sections(self, lines):
        """ parse an assembly source file into a section table """
        """ - removes empty lines
            - creates empty binary table based on section names
        """

        self.current_section = None
        self.current_address = None

        # parse each line a section at a time
        for line in lines:
            line = line.strip()
            if not line: continue

            # new section  
            if line.startswith(".SECTION"):

                # check for duplicate section
                section_name = line.split()[1].upper()
                if section_name in self.section_table:
                    raise ValueError(f"Section name already exists: {section_name}")

                # create section
                self.current_section = section_name
                self.current_address = None
                self.section_table[section_name] = [None, []]
                self.binary_table[section_name] = []

            # current section
            elif self.current_section:
                parsed_line = self.parse_line(line, self.current_address)
                if parsed_line:
                  self.section_table[self.current_section].append(parsed_line)

            # no section
            else:
                raise ValueError(f"Instruction found outside of section: (line)")

    def parse_line(self, line, address):
        """ Parse a line into a symbol or a section instruction"""

        # skip empty lines and comments and tokenize
        if not line or line.startswith(";"): return
        tokens = self.tokenize_line(line)

        # labels

        if tokens[0].endswith(":"):
            self.add_label(tokens[0], address)
            tokens = tokens[1:]
            if not tokens: return

        # directives

        if tokens[0] == ".ORG": # memory layout
            self.current_address = int(tokens[1], 16)
            self.section_table[self.current_section][0] = self.current_address

        elif tokens[0] == ".ALIGN": # alignment
            adjust = self.current_address % int(tokens[1])
            for i in range(adjust):
              padding = 0
              self.section_table[self.current_section][1].append([self.current_address, padding])
              self.current_address += 1

        elif tokens[0] == ".DEF": # defines
            self.symbol_table[tokens[1]] = int(tokens[2], 16) # we should check type here... we assume hex...

        elif tokens[0] == ".DB": # bytes
            for byte in tokens[1:]:
              byte = int(byte, 16)
              self.section_table[self.current_section][1].append([self.current_address, byte])
              self.current_address += 1

        elif tokens[0] == ".DW": # words
            for word in tokens[1:]:
              byte_high = int(word, 16) >> 8
              byte_low = int(word, 16) & 0xFF
              self.section_table[self.current_section][1].append([self.current_address, byte_high])
              self.section_table[self.current_section][1].append([self.current_address+1, byte_low])
              self.current_address += 2

        elif tokens[0] == ".DH": # half precision IEEE 754-2008
            pass
            # for number in tokens[1:]:
              # word = encodeFloat(number)
              # byte_high = int(word, 16) >> 8
              # byte_low = int(word, 16) & 0xFF
              # self.section_table[self.current_section][1].append([self.current_address, byte_high])
              # self.section_table[self.current_section][1].append([self.current_address+1, byte_low])
              # self.current_address += 2

         # instructions

        else: # add instruction to the appropriate section
            operands = tokens[1:]
            self.section_table[self.current_section][1].append([self.current_address, tokens])
            self.current_address += 3 + 2*len(operands) # 1 byte instr + 2 bytes per operand

    def add_label(self, label, address):
        """Add a label to the symbol table"""
        if not address: raise ValueError(f"No address set in section {self.current_section}")
        self.symbol_table[label.replace(":","")] = address

    def print_bytes(self): pass

    def print_hex(self): pass
        
    def print_tables(self, comments=False):
        PRINT_COMMENTS = comments

        # symbol table
        print("Symbol table:")
        for symbol, value in self.symbol_table.items():
          print("    ", f"{symbol} : {hex(value)}")
          
        # section table
        print("\n", "Section table:", sep='')
        for name, section in self.section_table.items():
          if section[0] is None: print("    ", f"{name}: {section[0]}")
          else: print("    ", f"{name}: {hex(section[0])}")

          for instruction in section[1]:

            # label
            if instruction[0] in self.symbol_table.values():
                label = next((k for k, v in self.symbol_table.items() if v == instruction[0]), None)
                print('\t', end='')
                if PRINT_COMMENTS: print("/*", end='')
                print(label, ":", sep='', end='')
                if PRINT_COMMENTS: print(" */", end='')
                print('\n', end='')

            # instruction
            if type(instruction[1]) == list:

                # print the memory address
                print('\t', end='')
                if PRINT_COMMENTS: print("/*", end=' ')
                print(hex(instruction[0]), ":", sep='', end=' ')
                if PRINT_COMMENTS: print("*/", end=' ')

                # print the machine code
                for byte in instruction[2]:
                    print(f"0x{byte:02X}", end=' ')

                # print the assembly code
                for i in range(7 - len(instruction[2])): # sponge, random cols
                    print('\t', end='')
                print('\t', end='')
              
                if PRINT_COMMENTS: print("/*", end=' ')
                print(instruction[1][0], sep='', end=' ')
                print(*((instruction[1])[1:]), sep=', ', end=' ')
                if PRINT_COMMENTS: print("*/", end='')
                print()

            # raw bytes
            else:
              print('\t', end='')
              if PRINT_COMMENTS: print("/*", end=' ')
              print(hex(instruction[0]), ": ", hex(instruction[1]), sep='', end=' ')
              if PRINT_COMMENTS: print("*/", end='')
              print()


        # binary sections
        if False:
          print("\n", "Binary sections:", sep='')
          for name, binary in self.binary_table.items():
            address = self.section_table[name][0]
            if type(address) == int: address = hex(address)
            print("    ", f"{name} : {address}")
            print("\t", end='')
            for byte in binary:
              print(hex(byte), end=', ')
            print()

    def load_source(self, filename):
        """ load assembly source code from an input file """

        try:
            with open(filename, "r") as assembly_file:
                self.source = assembly_file.readlines()

        except FileNotFoundError:
            raise RuntimeError(f"Error: File `{filename}` not found.")

    def write_output(self, filename):
        """ write the machine code to an output file """

        with open(filename, "wb") as binary_file:
            last_address = None

            for name, binary in self.binary_table.items():

                address = self.section_table[name][0]
                if type(address) != int: continue

                if last_address is not None and address > last_address:
                    binary_file.write(bytes([0] * (address - last_address)))

                for byte in binary:
                    binary_file.write(bytes([byte]))

                last_address = address + len(binary)

def printUsageAndExit():
    print("Usage:")
    print("   px-assembler.py [-h | --help] [-v | --version] <input_file> [-o <output_file>] [-p[c] | [-b | --bytes] | [-x | --hex]]")
    print("")
    print("   Assembles the <input_file>, with specified options below.")
    print("")
    print("Options:                                                      ")
    print("   -o <output_file>  write binary to <output_file>            ")
    print("   -t                print tables                             ")
    print("   -tc               print tables with comments               ")
    print("   -b, --bytes       prints machine code as raw bytes         ")
    print("   -x, --hex         prints machine code as hexadecimal       ")
    print("   -h, --help        displays this help message and exits     ")
    print("   -v, --version     displays version information and exits   ")
    exit(1)

if __name__=="__main__":

    try:
      # input
      if len(sys.argv) < 2:
        print("Error: no input file specified")
        print("Try `px-assembler --help` for more info.")
        exit(1)
     
      if "-h" in sys.argv or "--help" in sys.argv:
        printUsageAndExit()
      if "-v" in sys.argv or "--version" in sys.argv:
        printUsageAndExit()

      input_file = sys.argv[1]

      print_tables = "-t" in sys.argv or "-tc" in sys.argv
      print_comments = "-tc" in sys.argv
      print_bytes = "-b" in sys.argv or "--bytes" in sys.argv
      print_hex = "-x" in sys.argv or "--hex" in sys.argv

      output_file = None
      if "-o" in sys.argv:
        index = sys.argv.index("-o") + 1
        if index < len(sys.argv):
          if sys.argv[index].startswith("-"):
            raise RuntimeError("Error: invalid filename specified after -o")
          output_file = sys.argv[index]
        else:
          raise RuntimeError("Error: no file specified after -o")

      # assemble
      asm = Assembler()
      asm.load_source(input_file)
      asm.assemble()

      # output
      if print_tables:
        asm.print_tables(print_comments)
      if output_file:
        asm.write_output(output_file)
      if print_bytes:
        raise RuntimeError("Error: NOT IMPLEMENTED... print_bytes()")
      if print_hex:
        raise RuntimeError("Error: NOT IMPLEMENTED... print_hex()")

    except RuntimeError as e:
      print(f"{e}", file=sys.stderr)
      exit(1)
