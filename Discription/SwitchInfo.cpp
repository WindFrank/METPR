#include "SwitchInfo.h"

SwitchInfo::SwitchInfo()
{
}

SwitchInfo::SwitchInfo(int id, string type, string name, double R, double Cout, double Cin, double Tdel, double mux_trans_size, double buf_size)
{
    this->id = id;
    this->type = type;
    this->name = name;
    this->R = R;
    this->Cout = Cout;
    this->Cin = Cin;
    this->Tdel = Tdel;
    this->mux_trans_size = mux_trans_size;
    this->buf_size = buf_size;
}
