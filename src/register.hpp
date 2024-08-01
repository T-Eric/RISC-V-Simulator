#ifndef REGISTER_HPP
#define REGISTER_HPP

class Registers{
  public:
    unsigned int reg[32][2];
    int occupied_by[32][2];// 被RoB
};

class RegisterFile {
  public:
    Registers reg;
    unsigned int pc[2];
    RegisterFile(){
      for(int i=0;i<32;++i){
        for(int j=0;j<2;++j){
          reg.reg[i][j]=0;
          reg.occupied_by[i][j]=-1;
        }
      }
      pc[0]=pc[1]=0;
    }
    // reg是原装，teg是重命名
    void update(bool clk){
      for(int i=0;i<32;++i){
        reg.reg[i][!clk]=reg.reg[i][clk];
        reg.occupied_by[i][!clk]=reg.occupied_by[i][clk];
      }
      pc[!clk]=pc[clk];
      reg.reg[0][!clk]=0;//r0=0
    }
    void clear(bool clk){
      for(int i=0;i<32;++i){
        reg.occupied_by[i][clk]=-1;
      }
    }
    void release(unsigned int r,unsigned int rob_tag,bool clk){
      reg.occupied_by[r][clk]=-1;
    }
};

#endif // !REGISTER_HPP
