# Disassembler
Implementation:
  The Disassembler translate machine code into assembly code. In order to do this we used several if statements to differentiate between different formats. After that, 
  we differentiated between each format by the opcodes that resides with each one, in addition we then narrowed down each format with its corresponding instruction by
  using"funct 3" which is used to identify each instruction, and rare case we use an additionalif statement to  differentiate the instructions down further using "funct 7"
  for R types for instance.
Limitations and Known issues:

Contributions of each member:
Mokhtar:
  -
Ahmed:
  -
Mark:
  -
Sarah:
  - B type (32 bit)
  - S type (32 bit)
  - Jal (32 bit)
  - Swsp (CSS format) and Lwsp (16 bit)
