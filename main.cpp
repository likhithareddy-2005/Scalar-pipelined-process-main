#include <bits/stdc++.h>
using namespace std;
unsigned int Inst_Reg[4] ={0x00};
unsigned int PC = 0x00;
unsigned int R[16];

unsigned int I_Cache[256];
unsigned int D_Cache[256];

unsigned int opcode = 0, imm_4 = 0, imm_8 = 0, src1 = 0, src2 = 0, src = 0, dest = 0, output=0;

  int total_instr = 0;
  int Arithm = 0;
  int Logical = 0;
  int Shift = 0;
  int Memory = 0;
  int Control = 0;
  int Halt = 1;
  int Data_hazard = 0;
  int Control_hazard = 0;
  int Load_Imm = 0;
  int Control_timer = 0;

int mag_reg(unsigned int &A)
{
  if(A>=128)
    A=A-256;
  return A;
}

int twos_comp_reg(int &A)
{
  if(A<0)
    A=A+256;
  return A;
}

int mag_imm(unsigned int &A)
{
  if(A>=7)
    A=A-16;
  return A;
}

//int twos_comp_imm(int &A)
// {
//   if(A<0)
//     A=A+16;
//   return A;
// }

void Instr_Fetch() //having instruction decode here only
{
  // cout<<"pc:"<<PC<<endl;
  //and instruction decode
  Inst_Reg[0] = (I_Cache[PC]/16); 
  Inst_Reg[1] = (I_Cache[PC]%16); 
  Inst_Reg[2] = (I_Cache[PC+1]/16);
  Inst_Reg[3] = (I_Cache[PC+1]%16);

  PC+=2;
}

void Instr_Decode()
{
  opcode = Inst_Reg[0];
  //cout<<"opcode in decode is: "<<opcode<<endl;

  if(opcode == 0||opcode == 1||opcode == 2||opcode == 4||opcode == 5||opcode == 6)//add,sub,mul,and,or,xor
  {
    dest  = Inst_Reg[1];
    src1  = Inst_Reg[2];
    src2  = Inst_Reg[3]; 
  }

  else if(opcode == 3)//increment
  {
    dest = Inst_Reg[1];
  }

  else if(opcode == 7)//not
  {
    dest = Inst_Reg[1];
    src = Inst_Reg[2];
  }

  else if(opcode == 8 || opcode == 9 || opcode == 11 || opcode == 12) //slri, slli and ld and st
  {
    dest = Inst_Reg[1];
    src1 = Inst_Reg[2];
    imm_4  = Inst_Reg[3];
  }

  else if(opcode == 10 || opcode == 14) //load immediate or branch
  {
    dest = Inst_Reg[1];
    imm_8  = Inst_Reg[2]*16 + Inst_Reg[3];
  }

  else if(opcode == 13) //load immediate or branch
  {
    imm_8  = Inst_Reg[1]*16 + Inst_Reg[2];
  }

}

void ALU() // always do ALU before next instruction fetch.
{
  if(opcode == 0||opcode == 1||opcode == 2) //add,sub,mul 
  {
    unsigned int A_temp = R[src1], B_temp = R[src2];
    int output_temp = 0;

    int A = mag_reg(A_temp);
    int B = mag_reg(B_temp);

    if(opcode == 0)
    {
      output_temp = A + B;
      output = twos_comp_reg(output_temp);
      Arithm++;
    }  

    else if(opcode == 1)
    {
      output_temp = A - B;
      output = twos_comp_reg(output_temp);  
      Arithm++;
    }

    else if(opcode == 2)
    {
      output_temp = A * B;
      output = twos_comp_reg(output_temp);
      Arithm++;
    }
  }

  if(opcode == 4||opcode == 5||opcode == 6) //and, or, xor
  {
    unsigned int A = R[src1], B = R[src2];
    if(opcode == 4)
      {
        output = A & B;
        Logical++;
      }

    else if(opcode == 5)
      {
        output = A | B;
        Logical++;
      }
    
    else if(opcode == 6)
    {
      output = A ^ B;
      Logical++;
    }
  }

  else if(opcode == 3)//increment // parameters are dest only
  {
    unsigned int A_temp = R[dest];
    int A = mag_reg(A_temp);

    int output_temp = A + 1;
    output = twos_comp_reg(output_temp);

    Arithm++;
  }

  else if(opcode == 7)//not function  // parameters are dest, src
  {
    unsigned int A = R[src];
    output = ~A;
    Logical++;
  }

  else if(opcode == 8 || opcode == 9 ) //slli// parameters are dest, src1, imm_4
  {
    unsigned int A = R[src1];
    if (opcode==8)
      output = A >> imm_4;

    else if (opcode==9)
      output =  A << imm_4;
    Shift++;
  }
  
  else if(opcode == 11 || opcode ==12) //ld and st // parameters are dest, src1, imm_4
  {
    unsigned int A_temp = R[src1];
    int A = mag_reg(A_temp);

    int imm_4_neg = mag_imm(imm_4);
    output = A + imm_4_neg;
    Memory++;
  }

  else if(opcode == 10) //load immediate or branch // parameters are dest and imm_8
  {
    output = imm_8;
    Load_Imm++;
  }

  else if( opcode == 14)
  {
    int imm_8_neg = mag_reg(imm_8);
    if(R[dest]==0)
      PC+=(imm_8_neg)*2;
    
    Control++;
    Control_hazard+=2;
  }

  else if(opcode == 13) //jump //parameters used are imm_8
  {  
    int imm_8_neg = mag_reg(imm_8);
    PC+=(imm_8_neg)*2;
    Control++;
    Control_hazard+=2;
  }
}

void Mem()
{
  if(opcode == 11)
  {
    output = D_Cache[output];
  }

  if(opcode == 12)
  {
    D_Cache[output] = R[dest];
  }
}

void Wrt_Back()
{
  if(opcode==0||opcode==1||opcode==2||opcode==3||opcode==4||opcode==5||opcode==6||
  opcode==7||opcode==8||opcode==9||opcode==10||opcode==11)
  {
    R[dest]=output;
  }
}

int main(int argc, char* argv[]){
  ifstream  Inst_Cache (argv[1]);
  fstream Data_Cache(argv[2]);
  ifstream Reg_File(argv[3]);
  ofstream ofile ("Output.txt", ios::out);
 // intialise data, registers and instruction arrays

  for(int i=0;i<16;i++)
  {
    Reg_File>>hex>>R[i];
  }

  for(int i=0;i<256;i++)
  {
    Inst_Cache>>hex>>I_Cache[i];
    Data_Cache>>hex>>D_Cache[i];
  }
  //Initialisation completed

 Instr_Fetch();

  // set to keep track of registers in use (registers that aren't updated yet)
  unordered_set<int> register_in_use;
  // counter stores the stage that it is currently in (if counter = 1 -> exec, 2 -> mem, 3-> sets to 0)
  vector<int> register_counter(16,0);
    
  while(Inst_Reg[0]!=15)
  {
    Instr_Decode();

    // cout<<"opcode: "<<opcode<<" dest: "<<dest<<" src1: "<<src1<<" src2: "<<src2<<" Data Hazard: "<<Data_hazard<<endl;
    bool flag = false;

    // checking for dependencies and updating Data_hazard accordingly
    if(opcode != 3 && opcode != 10 && opcode != 13 && opcode != 14 && opcode != 15)
    {
      for(auto i : register_in_use)
      {
        if(opcode != 7 && opcode != 11 && opcode != 12)
        {
          if (i == src1 || i == src2) 
          {
            Data_hazard++;
            flag = true;
          }
        }
        else
        {
          if(i == src1)
          {
            Data_hazard++;
            flag = true;
          }
        }
      }
    }

    else
    {
      if(opcode == 3 || opcode == 14)
      {
        for(auto i : register_in_use)
        {
          if(i == dest)
          {
            Data_hazard++;
            flag = true;
          }
        }
      }
    }
      
    // to see if the counter value equals 3 and marking the register that has to be deleted from the set
    vector<bool> mark(16,false);

    for(auto i : register_in_use)
    {
      register_counter[i]++;
      if(register_counter[i] == 3) 
      {
        mark[i] = true;
        register_counter[i] = 0;
      }
    }

    // deleting the register from the set (only if it is marked as true)
    for(int i = 0;i<16;i++)
    {
      if(mark[i]) 
      {
        register_in_use.erase(i);
        mark[i] = false;
      }
    }

    if (flag)
    {
      continue;
    }
  
    ALU();

    // adding the register to the set and updating the counter accordingly
    total_instr++;
    register_in_use.insert(dest);
    register_counter[dest]++;
    if(register_counter[dest]==3) register_counter[dest] = 1;

    if(opcode==13||opcode==14)
    {
      for(int i=0;i<16;i++)
      {
        register_counter[i]=0;
      }
      register_in_use.clear();
    }

    Mem();

    Wrt_Back();

    Instr_Fetch();
    // cout<<Inst_Reg[0]<<"\t"<<Inst_Reg[1]<<"\t"<<Inst_Reg[2]<<"\t"<<Inst_Reg[3]<<"\t"<<endl;

    // cout<<"opcode: "<<opcode<<" src: "<<src<<" src1: "<<src1<<" src2: "<<src2
    // <<" dest:"<<dest<<" imm_4: "<<imm_4<<" imm_8: "<<imm_8<<endl;

    // for(int i=0;i<16;i++)
    // {
    //   cout<<R[i]<<" ";
    // }
    // cout<<endl<<endl;
  }


  Data_Cache.seekg(0, ios::beg);

  for(int i=0;i<256;i++)
  {
    if(D_Cache[i]<16)
      Data_Cache<<hex<<0<<D_Cache[i]<<endl;
    else
      Data_Cache<<hex<<D_Cache[i]<<endl;
  }

  int total_hazard = Data_hazard + Control_hazard;
  total_instr +=1;

  ofile << "Total number of instructions executed        : " << total_instr << endl;
  ofile << "Number of instructions in each class\n";
  ofile << "Arithmetic instructions                      : " << Arithm << endl;
  ofile << "Logical instructions                         : " << Logical << endl;
  ofile << "Shift instructions                           : " << Shift << endl;
  ofile << "Memory instructions                          : " << Memory << endl;
  ofile << "Load immediate instructions                  : " << Load_Imm << endl;
  ofile << "Control instructions                         : " << Control << endl;
  ofile << "Halt instructions                            : " << Halt << endl;
  ofile << "Cycles Per Instruction                       : " << (double)(total_instr + total_hazard +4)/total_instr << endl;
  ofile << "Total number of stalls                       : " << total_hazard <<endl;
  ofile << "Data stalls                                  : " << Data_hazard << endl;
  ofile << "Controlrol stalls                            : " << Control_hazard << endl;
}
