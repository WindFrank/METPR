#include "Executer.h"
#include "../Tool.h"

#define CMD_RESULT_BUF_SIZE 1024

/*
Generate testbench from params automatically and Verify the program
The params should contain all the params used or assigned in the program.
*/
int Executer::generateTbandVerify(string mName, vector<Attribute> params, string path){
    int outputNum = 0;
    int inputNum = 0;
    string text = "`timescale 1ps/1ps\n";
    string message = "";

    text += "module tb;\n";
    for(auto it = params.begin(); it < params.end(); it++){
        string pName = it->getName();
        int pEnd = it->getEnd();
        string laterPart = " ";
        if(pEnd != 0)
            laterPart += "[" + to_string(pEnd) + ":0] " + pName + ";\n";
        else
            laterPart += pName + ";\n";
        if(it->getType() == "output" || it->getType() == "input"){
            if(it->getType() == "output"){
                outputNum++;
                text += "output";
            }
            else{
                inputNum++;
                text += "reg";
            }
            text += laterPart;
        }
    }
    text += "reg clk_random;\n";

    text += "\n";

    text += mName + " dut(\n";
    for(auto it = params.begin(); it < params.end(); it++){
        if(it != params.begin())
            text += ",\n";
        text += "." + it->getName() + "(" + it->getName() + ")";
    }
    text += "\n);\n";

    if(inputNum > 0){
        text += "initial clk_random = 1;\n";
        text += "always #4000 clk_random = ~clk_random;\n";
        text += "always@(clk_random) begin\n";
        for(int i = outputNum; i < outputNum + inputNum; i++){
            Attribute recentParam = params[i];
            int len = recentParam.getEnd() + 1;
            string randomValue = "";
            int rightClosed = pow(2, len) - 1;
            randomValue += "$urandom_range(" + to_string(rightClosed) + ", 0)";
            text += recentParam.getName() + " = " + randomValue + ";\n";
        }
        text += "end\n";
    }

    text += "endmodule";

    if(Tool::fileWrite("tb.v", text)){
        Tool::error("Error: Write tb.v wrong.");
    }

    // make sure no test file exist
    if(access("test", F_OK) == 0){
        string cmd = "rm -rf test";
        system(cmd.c_str());
    }

    string iverilogtestcmd = "iverilog -o test " + path + "/" + mName + ".v tb.v";
    int state = system(iverilogtestcmd.c_str());
    // bool isSucc = executeCMD_Cpp("iverilog -o test " + mName + ".v tb.v", message);

    if(access("test", F_OK) == 0 && state == 0){
        Tool::logMessage("iverilog compiled successfully.");
        string cmd = "rm -rf test";
        system(cmd.c_str());
        return 0;
    }
    else{
        Tool::logMessage("Error: iverilog compiled fault.");
        return 1;
    }
}