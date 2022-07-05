#include <iostream>
#include <unordered_map>
#include <fstream>
#include "stdlib.h"
#include <iomanip>
#include <iostream>

using namespace std;

unsigned int pc = 0x0;

char memory[8*1024];	// only 8KB of memory located at address 0



string convert3BitToABIName  (unsigned int binary) // a utility function that takes number of a regsiter and returns its ABI name, EX : 000 => s0
{
    unordered_map<unsigned int, string> map;
    map[0]="s0";    map[1]="s1";  map[2]="a0";  map[3]="a1";
    map[4]="a2";    map[5]="a3";  map[6]="a4";  map[3]="a5";
    return map[binary];
}
string convert5bitToABIName (unsigned int binary){

    unordered_map<unsigned int ,string> map;
    map[0]= "zero"; map[1]= "ra";  map[2]= "sp"; map[3] = "gp";
    map[4]= "tp"; map[5]= "t0";  map[6]= "t1"; map[7] = "t2";
    map[8]= "s0"; map[9]= "s1";  map[10]= "a0"; map[11] = "a1";
    map[12]= "a2"; map[13]= "a3";  map[14]= "a4"; map[15] = "a5";
    map[16]= "a6"; map[17]= "a7";  map[18]= "s2"; map[19] = "s3";
    map[20]= "a4"; map[21]= "s5";  map[22]= "s6"; map[23] = "s7";
    map[24]= "s8"; map[25]= "s9";  map[26]= "s10"; map[27] = "s11";
    map[28]= "t3"; map[29]= "t4";  map[30]= "t5"; map[31] = "t6";

    return map[binary];
}

void emitError(char *s)
{
    cout << s;
    exit(0);
}

void printPrefix(unsigned int instA, unsigned int instW){
    cout << "0x" << hex << std::setfill('0') << std::setw(8) << instA << "\t0x" << std::setw(8) << instW;
}

void instDecExec(unsigned int instWord)
{
    int tmp = ((int) instWord) ;
    unsigned int rd, rs1, rs2,rs2_C, funct3, funct7, opcode;
    unsigned int I_imm, B_imm, S_imm, U_imm, J_imm;
    unsigned int address;

    unsigned int instPC = pc - 4;
    unsigned int shamt;
    opcode = instWord & 0x0000007F;
    rd = (instWord >> 7) & 0x0000001F;
    funct3 = (instWord >> 12) & 0x00000007;
    funct7 = (instWord >> 25) & 0x00007F;
    rs1 = (instWord >> 15) & 0x0000001F;
    rs2 = (instWord >> 20) & 0x0000001F;
    rs2_C =(instWord >> 2) & 0x0000001F;
    shamt = (instWord >> 20) & 0x0000001F;


 I_imm = ((instWord >> 20) & 0x7FF) | (((instWord >> 31) ? 0xFFFFF800 : 0x0));
 U_imm = ((instWord >> 12) &0x7FFFF ) |( ((instWord >>31) &1)?0xfff80000:0x0 );  ;
B_imm =((((instWord>>31)&1) ? 0xfffff800:0x0) | (((instWord >> 7) & 1)  <<10) | (((instWord>>8) &15)) | (((instWord >>25)&63)<<4)  ) <<1 ;
 S_imm = ((((instWord>>31)&1)?0xfffff800 : 0x0 ) | (((instWord>>25)&63) <<5 ) |(((instWord>>7)&31)));

 J_imm = ((((instWord>>31)&1)?0xfff80000:0x0) |(((instWord >>12)&255)<<11) | (((instWord>>20)&1) <<10) | (((instWord>>21)&1023))  )<<1 ;
//    printPrefix(instPC, instWord);
        int instructionType = opcode & 3;   // opcode & .b11

        if (instructionType!=3) { // 16 bit instruction
            unsigned int func4_extraBitForCR = (instWord>>12) &1;
            unsigned int func3_16bit = (instWord >>13) &  7;

            unsigned int func2 = (instWord >> 10)  &3;
             unsigned int ciImm = ((( (( (signed)instWord >> 12) & 1)) ? 0xFFFFFFE0 : 0x0   ) | (( (signed)instWord >> 2) & 31)  )  ;
             unsigned int lwsp_imm=  ( (((instWord>>3)&1)? 0xffffffe0:0x0) | ((instWord>>12)&1)<<3) | (((instWord >>2)&1)<<4) | (((instWord>>4)&7));
             unsigned int swsp_imm=( (((instWord>>8)&1)?0xffffffe0:0x0) |(((instWord >> 7) & 3) << 4) | (((instWord >> 9) & 7))) <<2;
             unsigned int immCJ =  ((((instWord >>12)&1)?0xFFFFFC00:0x0 )|(((instWord>>2) &1) <<4 ) | (((instWord >>3) &3)) | (((instWord >>6) &1) <<6) | (((instWord >>7)&1) <<5) | (((instWord >>8)&1) <<9) | (((instWord >>9) &3)<<7) |(((instWord >>11 )&1)<<3) ) <<1;
             unsigned int CBrs1 = (instWord  >>7) &7;
             unsigned int CBimm =   (((((instWord >>12)&1 )?0xffffff80:0x0)|((instWord >>2)&1) <<4) | (((instWord >>3)&3)) | (((instWord >>5)&3)<<5) | (((instWord>>10)&3)<<2)) <<1;
            if (instructionType==2){
                if (func3_16bit == 2 )
                    cout << "C.LWSP \t" <<convert5bitToABIName(rd)<< ",  " <<(int) (lwsp_imm *4)<< " (sp)" << "\n";

                else if (func3_16bit ==  6) //sp
                    cout << "C.SWSP \t" << convert5bitToABIName(((instWord >>2)&31)) << " ,   " <<   (int)swsp_imm << "(sp) \n";


                else if (func3_16bit== 4) {                // CR format
                    unsigned int CrRS1 = (instWord >>7) & 0x0000001F ;  // still need to decode this
                    // C.jr
                    if (!func4_extraBitForCR)

                    {
                        if (!rs2_C)
                            cout << "jr \tx0, " << convert5bitToABIName(CrRS1) << "\t ,   0" ;
                        else{
                            cout << "MV    " << convert5bitToABIName(rd) << " ,     "<< convert5bitToABIName(((instWord >> 2) & 31) ) << "\n";
                    }}

                    else  { //C.jalr
                        if ( !rs2_C && !rd ) cout<<"EBREAK\n";
                        else if (!rs2_C)
                             cout << "jalr \tx1, " <<convert5bitToABIName(CrRS1) << "\t ,   0" ;
                        else cout << "c.add \t " <<convert5bitToABIName(rd) << "\t ,   " <<convert5bitToABIName(rs2_C) << "\n";
                    }


                }
                else if (func3_16bit==0){
                    // slli
                    unsigned int dest = (instWord >>7) & 31;
                    cout << "C.SLLI\t" << convert5bitToABIName(dest) << " ,    " << (int)ciImm << "\n";
                }
        }

        else if (instructionType ==0){  //Register-Based Loads and Stores format

            unsigned int rd16bit = (instWord >>2) & 7;
            unsigned int rs16bit = (instWord >>7) & 7;
            unsigned int offset = ( ((((signed)instWord >>5) & 1 ) ? 0xfffffff0:0x0)|((((signed)instWord >>10) &7) <<1) | (((signed)instWord >>6) & 1)) <<2 ;
            unsigned int imm_CIW =   ((instWord>>10)&1)?0xFFFFFFEF:0x0 | (((instWord>>5)&1)<<1) | (((instWord>>6)&1)) | (((instWord>>7)&7)<<4) | (((instWord>>10)&3)<<2);

            if (func3_16bit==2) cout << "C.LW\t" << convert3BitToABIName(rd16bit) << "\t" << (int) offset +pc << "(" << convert3BitToABIName(rs16bit) << ")\n";
            else if (func3_16bit==6) cout << "C.SW\t" << convert3BitToABIName(rd16bit) << "\t" << (int) offset + pc << "(" << convert3BitToABIName(rs16bit) << ")\n";

            else if (func3_16bit == 0) // CIW format
                cout << "C.ADDI4SPN\t" << convert3BitToABIName(rd16bit) << ",\t" <<(int) imm_CIW << ")\n";
            else cout << "not supported register based load/store format-RVC";
        }
        else { //instructionType ==1  // control instructions

             if (func3_16bit==0)
             {
                 cout << ((signed int)instWord ) ;
                 ciImm = ((( (( (signed)instWord >> 12) & 1)) ? 0xFFFFFFE0 : 0x0   ) | (( (signed)instWord >> 2) & 31)  )  ;
                 cout << "c.addi\t " << convert5bitToABIName((instWord >> 7) & 31) << ",    " << ((int)ciImm) << "\n" ;

             }



           else if (func3_16bit==5){ // CJ format
                // C.J
                cout << "C.j\t " <<  (int)immCJ+pc << "\n";
            }
            else if (func3_16bit==1) { // CJ format
                // c.jal // saves pc+2 to ra =>x1
                cout << "c.jal , " <<  (int) immCJ +pc << "\n";
            }
            else if (func3_16bit==6) { //CB format
                // C.BEQZ
//                cout << "beq\t" << convert3BitToABIName(CBrs1) << ",  x0 , " << CBimm << "\n";
                cout << "c.beqz\t" << convert3BitToABIName(CBrs1) << ",  " << (int) (CBimm ) + pc<< "\n";

            }
            else if (func3_16bit==7) { // CB format
                // C.bnez
                cout << "c.bnez\t" << convert3BitToABIName(CBrs1) << " , " << (int)(CBimm) + pc<< "\n";
//                cout << "bne\t" << convert3BitToABIName(CBrs1) << ",  " << CBimm << "\n";
            }
            else if (func3_16bit ==2){  //CI
                // C.LI
                unsigned  int rdcli = (instWord >> 7) & 31;
                cout << "C.LI\t " << convert5bitToABIName(rdcli) << ",    " << (int)ciImm << "\n" ;
            }
            else if (func3_16bit == 3){
                cout << "c.lui\t" <<convert5bitToABIName((instWord >> 7) & 31)  << " ,    " << (int)ciImm;
            }
            else if (func3_16bit==4){ // checking for func 2
                CBrs1 = CBrs1 & 7;  // removing the most two significant bits coz they are for func2

                if (func2==0) { // CI
                    //SRLI
                    cout << "C.SRLI\t" << convert3BitToABIName(CBrs1) << ",   " << (int) (ciImm ) << "\n";
                }
                else if (func2==1){ // CI
                    // SRAI
                    cout << "C.SRAI\t" << convert3BitToABIName(CBrs1) << ",   " << (int)(ciImm) << "\n";
                }
                else if (func2==2){ //CI
                    //ANDI
                    cout << "C.ANDI\t" << convert3BitToABIName(CBrs1) << ",   " << (int)(ciImm) << "\n";
                }
                // CR
                else if (func2==3 ){ // func2 = 3
                    int CRfunc22 = (instWord >> 5)&3;
                    //unsigned int func4_extraBitForCR = (instWord>>12) &1;
                    if (!func4_extraBitForCR)
                    {
                        unsigned  int rd_CR = (instWord>>7)&7;
                        unsigned  int rs2_CR = (instWord>>2)&7;
                        if (CRfunc22==0){
                            cout << "C.SUB\t" << convert3BitToABIName(rd_CR) << ",   " << convert3BitToABIName(rs2_CR) << "\n";
                        }
                        else if (CRfunc22==1){
                            cout << "C.XOR\t" << convert3BitToABIName(rd_CR) << ",   " << convert3BitToABIName(rs2_CR) << "\n";
                        }
                        else if (CRfunc22==2){
                            cout << "C.OR\t" << convert3BitToABIName(rd_CR) << ",   " << convert3BitToABIName(rs2_CR) << "\n";
                        }
                        else if (CRfunc22==3){
                            cout << "C.AND\t" << convert3BitToABIName(rd_CR) << ",   " << convert3BitToABIName(rs2_CR) << "\n";
                        }

                    }
                    else{

                        unsigned  int rd_CR = (instWord>>7)&7;
                        unsigned  int rs2_CR = (instWord>>2)&7;
                        if (CRfunc22==0){
                            cout << "C.SUBW\t" << convert3BitToABIName(rd_CR) << ",   " << convert3BitToABIName(rs2_CR) << "\n";
                        }
                        else if (CRfunc22==1){
                            cout << "C.ADDW\t" << convert3BitToABIName(rd_CR) << ",   " << convert3BitToABIName(rs2_CR) << "\n";
                        }


                    }

                }

            }


        }



    }
    else {  // 32 bit instruction
            if(opcode == 0x33){		// R Instructions

                switch(funct3){
                    case 0:
                        {
                            if(funct7 == 32) {
                                cout << "\tSUB\t" << convert5bitToABIName(rd) << ", " << convert5bitToABIName(rs1) << ", " << convert5bitToABIName(rs2) << "\n";
                            }
                            else {
                                cout << "\tADD\t" << convert5bitToABIName(rd) << ", " << convert5bitToABIName(rs1) << ", " << convert5bitToABIName(rs2) << "\n";
                            }
                            break;

                        }
                    case 1:
                        cout << "\tSLL\t" << convert5bitToABIName(rd) << ", " << convert5bitToABIName(rs1) << ", " << convert5bitToABIName(rs2) << "\n";
                        break;
                    case 2:
                        cout << "\tSLT\t" << convert5bitToABIName(rd) << ", " << convert5bitToABIName(rs1) << ", " << convert5bitToABIName(rs2) << "\n";
                        break;
                    case 3:
                        cout << "\tSLTU\t" << convert5bitToABIName(rd) << ", " << convert5bitToABIName(rs1) << ", " << convert5bitToABIName(rs2) << "\n";
                        break;
                    case 4:
                        cout << "\tXOR\t" << convert5bitToABIName(rd) << ", " << convert5bitToABIName(rs1) << ", " << convert5bitToABIName(rs2) << "\n";
                        break;
                    case 5:
                        if(funct7 == 32) {
                            cout << "\tSRA\t" << convert5bitToABIName(rd) << ", " << convert5bitToABIName(rs1) << ", " << convert5bitToABIName(rs2) << "\n";
                        }
                        else {
                            cout << "\tSRL\t" << convert5bitToABIName(rd) << ", " << convert5bitToABIName(rs1) << ", " << convert5bitToABIName(rs2) << "\n";
                        }
                        break;
                    case 6:
                        cout << "\tOR\t" << convert5bitToABIName(rd) << ", " << convert5bitToABIName(rs1) << ", " << convert5bitToABIName(rs2) << "\n";
                        break;
                    case 7:
                        cout << "\tAND\t" << convert5bitToABIName(rd) << ", " << convert5bitToABIName(rs1) << ", " << convert5bitToABIName(rs2) << "\n";
                        break;
                    default:
                        cout << "\tUnkown Instruction \n";
                }
            }
        else if (opcode == 0x13) {    // I instructions

            switch (funct3) {
                case 0:    cout << "\tADDI\t " << convert5bitToABIName(rd) << ", " << convert5bitToABIName(rs1) << ", " << (int) (I_imm) << "\n";
                break;
                case 1:    cout << "\tSLLI\t " << convert5bitToABIName(rd) << ", " << convert5bitToABIName(rs1) << ", " <<  (int)(shamt) << "\n";
                break;
                case 2:    cout << "\tSLTI\t " << convert5bitToABIName(rd) << ", " << convert5bitToABIName(rs1) << ", " <<  (int)(I_imm )<< "\n";
                break;
                case 3:    cout << "\tSLTIU\t " << convert5bitToABIName(rd) << ", " << convert5bitToABIName(rs1) << ", " << (int)(I_imm) << "\n";
                break;
                case 4:    cout << "\tXORI\t " << convert5bitToABIName(rd) << ", " << convert5bitToABIName(rs1) << ", " << (int)(I_imm )<< "\n";
                break;
            case 5: {
                if (((instWord >> 30) & 1) == 1) {
                    cout << "\tSRAI\t " << convert5bitToABIName(rd) << ", " << convert5bitToABIName(rs1) << ", " << (int)(shamt) << "\n";
                }
                else {
                    cout << "\tSRLI\t " << convert5bitToABIName(rd) << ", " << convert5bitToABIName(rs1) << ", " <<(int)(shamt )<< "\n";
                }

            }
                  break;
            case 6:    cout << "\tORI\t " << convert5bitToABIName(rd) << ", " << convert5bitToABIName(rs1) << ", " <<(int)(I_imm) << "\n";
                break;
                case 7:    cout << "\tANDI\t " << convert5bitToABIName(rd) << ", " << convert5bitToABIName(rs1) << ", " << (int)(I_imm )<< "\n";
                break;
            default:
                cout << "\tUnkown I Instruction \n";
            }
        }
        else if (opcode == 0x3) { // I-Type
            switch (funct3) {
                case 0:    cout << "\tLB\t " << convert5bitToABIName(rd) << ", " << (int)(I_imm ) << "(" << convert5bitToABIName(rs1) << ")" << "\n";
                break;
                case 1:    cout << "\tLH\t " << convert5bitToABIName(rd) << ", " << (int)(I_imm ) << "(" << convert5bitToABIName(rs1) << ")" << "\n";
                break;
                case 2:    cout << "\tLW\t " << convert5bitToABIName(rd) << ", " << (int)(I_imm ) << "(" << convert5bitToABIName(rs1) << ")" << "\n";
                break;
                case 4:    cout << "\tLBU\t " << convert5bitToABIName(rd) << ", " << (int)(I_imm ) << "(" << convert5bitToABIName(rs1) << ")" << "\n";
                break;
                case 5:    cout << "\tLHU\t " << convert5bitToABIName(rd) << ", " << (int)(I_imm ) << "(" << convert5bitToABIName(rs1) << ")" << "\n";
                break;
            }
        }
        else if (opcode == 0x67) { // JALR I-Type
            cout << "JALR\t " << convert5bitToABIName(rd) << ", " << hex << "0x" << ",    " << convert5bitToABIName(rs1) << " ,  " << (int)I_imm  << "\n";
        }
        else if (opcode == 0x37) { // LUI U-Type
            cout << "\tLUI\t " << convert5bitToABIName(rd) << ", " << (int)(U_imm) << "\n";
        }
        else if (opcode == 0x17) { // AUIPC - U-Type
            cout << "\tAUIPC\t " << convert5bitToABIName(rd) << ", " << (int)(U_imm )<< "\n";
        }

        else if (opcode == 0x63) {    // B instructions
            switch (funct3) {
                case 0:    cout << "BEQ\t" << convert5bitToABIName(rs1) << ", " << convert5bitToABIName(rs2) << ",  " << (int)(B_imm) +pc << "\n";
                break;
                case 1:    cout << "BNE\t" << convert5bitToABIName(rs1) << ", " << convert5bitToABIName(rs2) << ",  " <<(int)(B_imm) +pc << "\n";
                break;
                case 4:    cout << "BLT\t" << convert5bitToABIName(rs1) << ", " << convert5bitToABIName(rs2) << ",  " <<(int)(B_imm) +pc<< "\n";
                break;
                case 5:    cout << "BGE\t" << convert5bitToABIName(rs1) << ", " << convert5bitToABIName(rs2) << ",  " << (int)(B_imm) +pc << "\n";
                break;
                case 6:    cout << "BLTU\t" << convert5bitToABIName(rs1) << ", " << convert5bitToABIName(rs2) << ",  " << (int)(B_imm) +pc << "\n";
                break;
                case 7:   cout << "BGEU\t" << convert5bitToABIName(rs1) << ", " << convert5bitToABIName(rs2) << ",  " << (int)(B_imm) +pc << "\n";
                break;
            default:
                cout << "\tUnkown Instruction \n";
             }
         }
        else if  (opcode == 0x23) {    // S instructions
            switch (funct3) {
            case 0:    cout << "\tSB\t" << convert5bitToABIName(rs2) << hex << ", " << (int)S_imm << "  (" << convert5bitToABIName(rs1) << ")" << "\n";
                break;
            case 1:    cout << "\tSH\t" << convert5bitToABIName(rs2) << hex << ", " << (int)S_imm << ", "  << "  (" << convert5bitToABIName(rs1) << ")\n";
                break;
            case 2:    cout << "\tSW\t" << convert5bitToABIName(rs2) << hex << ", " << (int)S_imm << ","  << "  (" << convert5bitToABIName(rs1) << ")\n";
                break;
            default:
                cout << "\tUnkown Instruction \n";
            }
        }
        else if (opcode == 0x6F) //jal
            {
                int x  = ((int)J_imm)+ pc;
                cout << "JAL\t" << convert5bitToABIName(rd) << ",   " << x  << "\n";
            }

        else if ( opcode== 0x73){

            unsigned int rest = (instWord>>7);
            if (!rest) cout<<"\tecall\n";
            else cout<< "\tebreak\n";

        }
    }

}

int main (int argc, char *argv [] ){
    instDecExec(0x1151);
   unsigned  int input = 23912;
    unsigned int instWord=0;
    ifstream inFile;
    ofstream outFile;
    argv[1]= "../samples_2/parr_comp.bin";
//    cout << argc << "\n";
//    if(argc<2) {
//        emitError("use: rvcdiss <machine_code_file_name>\n");
//    }

    inFile.open(argv[1], ios::in | ios::binary | ios::ate);
    if(inFile.is_open())
    {
        int fsize = inFile.tellg();
        inFile.seekg (0, inFile.beg);
        if(!inFile.read((char *)memory, fsize)) emitError("Cannot read from input file\n");

        while(pc<fsize){
            // 2 pm
            instWord = 	(unsigned char)memory[pc] |
                    (((unsigned char)memory[pc+1])<<8) ;

            if (((int)instWord & 3 )!= 3) {
                cout <<( int)instWord << "  \t";
                pc+=2;
                instDecExec( (int)instWord);

            }
            else {
                instWord = (unsigned char)memory[pc] |
                        (((unsigned char)memory[pc+1])<<8) |(((unsigned char)memory[pc+2])<<16) |
                        (((unsigned char)memory[pc+3])<<24);

                cout <<(int)instWord << "  \t";

                instDecExec((int)instWord);
                pc += 4;
            }



//            // remove the following line once you have a complete simulator

//            if(pc==40) {
//                cout <<"break\n";
//                break;			// stop when PC reached address 32
//            }
        }
    }
    else emitError("Cannot access input file\n");

    return 0;
}

