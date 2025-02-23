from pprint import pprint
from collections import OrderedDict

from pxAssemblerFloat import encodeFloat

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


Or maybe we just add to the sections:
sections = {
  "name": [address, code, machine_code]
  "name2": [address2, code2, machine_code2]
  ...
  "nameN": [addressN, codeN, machine_codeN]
}

"""


#define IMM16 0b000
#define IMM8  0b001
#define DIR16 0b010
#define DIR8  0b011
#define IND16 0b100
#define IND8  0b101


STACK_MODES = {"^<>":0b111, "^<":0b110, "^>":0b101, "^":0b100, "<>":0b011, "<":0b010, ">":0b001, "":0b000}
OPERAND_MODES = {"immediate16":0b000,
                 "direct16":0b010,
                 "indirect16":0b100,
                 "immediate8":0b001,
                 "indirect8":0b101,
                 "direct8":0b011,
                 }


class Assembler:
    def __init__(self):
        self.sections = OrderedDict()
        self.symbols = OrderedDict()
        self.binary = OrderedDict()

        self.current_section = None
        self.current_address = None

    def assemble(self, lines):
        PRINT_COMMENTS = True
    
        self.parse_sections(lines)
        self.assemble_binary()

        #return

        # symbol table
        print("Symbol table:")
        for symbol, value in self.symbols.items():
          print("    ", f"{symbol} : {hex(value)}")
          
        # section table
        print("\n", "Section table:", sep='')
        for name, section in self.sections.items():
          if section[0] is None: print("    ", f"{name}: {section[0]}")
          else: print("    ", f"{name}: {hex(section[0])}")

          for instruction in section[1]:

            # label
            if instruction[0] in self.symbols.values():
              label = next((k for k, v in self.symbols.items() if v == instruction[0]), None)
              print('\t', end='')
              if PRINT_COMMENTS: print("/*", end='')
              print(label, ":", sep='', end='')
              if PRINT_COMMENTS: print(" */", end='')
              print('\n', end='')

            # instruction
            if type(instruction[1]) == list:
              print('\t', end='')
              if PRINT_COMMENTS: print("/*", end=' ')
              print(hex(instruction[0]), ":", sep='', end=' ')
              if PRINT_COMMENTS: print("*/", end=' ')

              # print the assembled binary
              for byte in instruction[2]:
                print(f"0x{byte:02X}", end=', ')

              # print the assembly code
              for i in range(6 - len(instruction[2])):
                print('     ', end='')
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
          for name, binary in self.binary.items():
            address = self.sections[name][0]
            if type(address) == int: address = hex(address)
            print("    ", f"{name} : {address}")
            print("\t", end='')
            for byte in binary:
              print(hex(byte), end=', ')
            print()



    def assemble_binary(self):
        """ Assemble instructions into binary"""
        # assemble each section
        for section_name, section in self.sections.items():
            for instruction in section[1]:
                # translate assembly code into machine code
                machine_code = self.assemble_instruction(section_name, instruction)

                # write machine code
                if machine_code:
                  self.binary[section_name].extend(machine_code)
                  instruction.append(machine_code) # keep track so we can print out listing...

     
    def assemble_instruction(self, name, instruction):
      """ Assemble specific instruction into machine code"""
      # raw bytes
      if type(instruction[1]) == int:
        self.binary[name].append(instruction[1])

      # operations
      if type(instruction[1]) == list:
        stack_modes = ["^<>", "^<", "^>", "^", "<>", "<", ">", ]
        machine_code = []

        operation = instruction[1][0]
        stack_mode = ""

        for mode in stack_modes:
          if mode in operation:
            operation, stack_mode = operation.split(mode, 1)
            stack_mode = mode

        operands = instruction[1][1:]
        resolved_operands = self.resolveOperands(operands)
        opcode = self.getOpcode(operation)

        # opcode
        machine_code.append(opcode)

        # stack mode
        machine_code.append(STACK_MODES[stack_mode])

        
        
        mode_byte = 0
        operand_bytes = []
        # operands
        for operand in resolved_operands:
            type_, value = operand

            # mode byte
            mode_byte <<= 4
            bits = OPERAND_MODES[type_]
            mode_byte += bits


            # operand bytes
            if type_ in ["immediate8", "direct8", "indirect8"]:
              operand_bytes.append(value)

            if type_ in ["immediate16", "direct16", "indirect16"]:
              byte_high = (value >> 8)
              byte_low = value & 0xFF

              operand_bytes.extend([byte_high, byte_low])

        if len(resolved_operands) < 2: mode_byte <<= 4
        machine_code.append(mode_byte)
        machine_code.extend(operand_bytes)

        return machine_code


    def tokenizeOperand(self, operand):
        operand = operand.replace('[[', '(').replace(']]', ')')
        operand += ' '  # force delimiter at end of string

        tokens = []
        current_token = ""

        for char in operand:
          # end token
          if char in "[]():! ":
              # current token
              if current_token: tokens.append(current_token)
              current_token = ""
              # delimiter token
              if not char.isspace(): tokens.append(char)
          # build token
          else: current_token += char

        return tokens


    def resolveOperand(self, operand):
        operand = operand.strip()

        tokens = self.tokenizeOperand(operand)

        value = 0
        value_type = "immediate"
        
        if tokens[0] == '[' and tokens[-1] == ']':
          value_type = "direct"
        if tokens[0] == '(' and tokens[-1] == ')':
          value_type = "indirect"

        # check each value / symobl
        for token in tokens:
            # symbol
            if token in self.symbols:
              value += self.symbols[token]

            # decimal
            elif token.isnumeric():
              value += int(token)

            # float
            elif '.' in token:
              value += encodeFloat(float(token))

            # hex
            elif token.startswith("0x"):
              value += int(token, 16)

        # Check for '#'
        if '#' in tokens:
          value_type += "8"
        else:
          value_type += "16"

        return [value_type, value]

    def resolveOperands(self, operands):
        """ Resolve each operand type based on its formatting..."""

        operand_types = []
        
        for operand in operands:
            operand_types.append(self.resolveOperand(operand))

        return operand_types

    def getOpcode(self, operation):

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
            
        if operation == "SQRT":
            return 0x58
        if operation == "HSQRT":
            return 0x59

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
        """ Break up lines into sections...
        - removes empty lines
        """
        self.current_section = None
        self.current_address = None

        for line in lines:
            line = line.strip()

            # empty lines
            if not line: continue

            # switch section
            elif line.startswith(".SECTION"):
                section_name = line.split()[1].upper()
                if section_name not in self.sections:
                    # enter new section  
                    self.current_section = section_name
                    self.current_address = None
                    self.sections[section_name] = [None, []]
                    self.binary[section_name] = []
                else:
                    raise ValueError(f"Section name already exists: {section_name}")

            # in a section
            elif self.current_section:
                # parse line
                parsed_line = self.parse_line(line, self.current_address) # []
                if parsed_line:
                  self.sections[self.current_section].append(parsed_line)

        return

    def tokenize_line(self, line):
        # remove comments
        line = line.split(";")[0].strip()
        line = line.replace(",", " ")
        tokens = line.split()
        return tokens

    def parse_line(self, line, address):
        """ Parse a line into a symbol or a section instruction"""
        # == Preprocess == #

        # skip empty lines and comments
        if not line or line.startswith(";"): return

        # break line into tokens
        tokens = self.tokenize_line(line)

        # check for labels first
        if tokens[0].endswith(":"):
            self.add_label(tokens[0], address)
            tokens = tokens[1:]
            if not tokens: return

        # directives

        if tokens[0] == ".ORG": # memory layout
            self.current_address = int(tokens[1], 16)
            self.sections[self.current_section][0] = self.current_address

        elif tokens[0] == ".ALIGN": # alignment
            adjust = self.current_address % int(tokens[1])
            for i in range(adjust):
              padding = 0
              self.sections[self.current_section][1].append([self.current_address, padding])
              self.current_address += 1

        elif tokens[0] == ".DEF": # defines
            self.symbols[tokens[1]] = int(tokens[2], 16) # we should check type here... we assume hex...

        elif tokens[0] == ".DB": # bytes
            for byte in tokens[1:]:
              byte = int(byte, 16)
              self.sections[self.current_section][1].append([self.current_address, byte])
              self.current_address += 1

        elif tokens[0] == ".DW": # words
            for word in tokens[1:]:
              byte_high = int(word, 16) >> 8
              byte_low = int(word, 16) & 0xFF
              self.sections[self.current_section][1].append([self.current_address, byte_high])
              self.sections[self.current_section][1].append([self.current_address+1, byte_low])
              self.current_address += 2

        elif tokens[0] == ".DH": pass # half precision IEEE 754-2008

         # instructions

        else: # add instruction to the appropriate section
            operands = tokens[1:]
            self.sections[self.current_section][1].append([self.current_address, tokens])
            self.current_address += 3 + 2*len(operands) # 1 byte instr + 2 bytes per operand
        

    def add_label(self, label, address):
        """Add a label to the symbol table"""
        if not address: raise ValueError(f"No address set in section {self.current_section}")
        self.symbols[label.replace(":","")] = address

if __name__=="__main__":

  with open("input.pa", "r") as assembly_file:
    asm = Assembler()
    assembly = assembly_file.readlines()
    binary = asm.assemble(assembly)

  with open("output.pbin", "w") as binary_file:
    #binary_file.write(binary)
    pass
