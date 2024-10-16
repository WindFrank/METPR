#include "Tool.h"
#include "UserDrive.h"
#include "ActionMutateLead/VarietyEvaluation.h"
#include "ActionMutateLead/Evaluate.h"

#include "lib/cmdline.h"

#include <iostream>
#include <string>

using namespace std;

#include <csignal>
#include <cstdlib>

void signalHandler( int signum ) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    exit(signum);  
}

int main(int argc, char *argv[]){

    signal(SIGINT, signalHandler);
    char* workPathDir = getenv("VGENERATOR_WORK_DIR");
    if(workPathDir == nullptr || workPathDir == ""){
        Tool::error("Error: VGENERATOR_WORK_DIR not found. Please check env.");
        return 0;
    }

    if(argc < 2){
        Tool::error("No command provided.\ncommand list: Vgenerator, VtrTest, VTRFeatureAnalyze, Arrange, VTRRandomTest, VTRRandomAnalyze, VTRCaseSizeGet");
        return 0;
    }
    string command = argv[1];
    if(command == "RandomGen"){
        cmdline::parser cmd;
        cmd.add<int>("num", 'n', "The number of the whole programs need generating.", true, 0);
        cmd.add<string>("set_path", 'p', "The path and name of the target program set. If the set is existing, its structure, the necessary content in it, will be checked. If valid, the path of the set will be saved directly. It is recommended to use at least one level content to save all various HDL set.", true, "");
        cmd.add<string>("write_type", 't', "if conserve the origin programs. If set 'overlap', only new programs will be saved.", false, "none", cmdline::oneof<string>("none", "overlap"));
        cmd.add<string>("generate_type", 'd', "The type of generated programs you want. Support: VTR FULL", false, "FULL");
        cmd.add<int>("state_iter", 'i', "The number of statements in each initial program. The higher the stateIter is, the larger the program is.", false, 20);
        cmd.add<int>("var_width", 'w', "The max width of each variable in HDL program.", false, 10);
        cmd.add<int>("deep_level", 'l', "The times of turns the larger HDL generated.", false, 2);
        cmd.add<int>("adjust_io_num", 'o', "Initial addition io numbers adjust. If it isn't 0, geneartion process will not adjust io and statements.", false, 0);
        cmd.add<int>("adjust_sequence_num", 'q', "Initial addition sequence numbers adjust. If it isn't 0, geneartion process will not adjust io and statements.", false, 0);
        cmd.add<double>("gen_ratio", 'r', "New percentage of the number of each new HDL generated to whole program.", false, 0.5);
        cmd.add<string>("active_check", 'c', "Use iverilog to do the active check for all verilog programs generated.", false, "off");
        cmd.add<int>("seed", 's', "The random seed.", false, 0);
        bool active_check = false;
        if(cmd.get<string>("active_check") == "on")
            active_check = true;
        
        cmd.parse_check(argc - 1, argv + 1);
        srand(cmd.get<int>("seed"));
        UserDrive ud;
        ud.setGenerateType(cmd.get<string>("generate_type"));
        ud.adjustSet(cmd.get<int>("adjust_io_num"), cmd.get<int>("adjust_sequence_num"));
        ud.loadHDLSet(cmd.get<int>("num"), cmd.get<int>("state_iter"), cmd.get<string>("set_path"), cmd.get<string>("generate_type"), cmd.get<string>("write_type"), cmd.get<int>("var_width"), cmd.get<int>("deep_level"), cmd.get<double>("gen_ratio"), active_check);
    } 
    else if(command == "VtrTest"){
        cmdline::parser cmd;
        cmd.add<string>("system_path", 'd', "The root content of vtr", true, "");
        cmd.add<string>("arch_path", 'a', "The path of arch files", true, "");
        cmd.add<string>("hdl_path", 'h', "the path of program set. You need to locate the path to the concrete dir whose content is all valid programs.", true, "");
        cmd.add<string>("MRs", 'm', "The list of MRs to used.", true, "");
        cmd.add<int>("turns", 't', "The times of testing turn.", false, 1000);
        cmd.add<int>("seed", 's', "The random seed.", false, 0);
        cmd.add<string>("randomLog", 'r', "Used to diversity-guided", false, "false");
        cmd.parse_check(argc - 1, argv + 1);
        UserDrive ud;
        ud.loadArchPath(cmd.get<string>("arch_path"));
        vector<string> MRs = Tool::split(cmd.get<string>("MRs"), '_');
        srand(cmd.get<int>("seed"));
        ud.VTRrunTesting(cmd.get<string>("system_path"), cmd.get<string>("hdl_path"), MRs, cmd.get<int>("turns"), cmd.get<string>("randomLog"));
    }
    else if(command == "VTRFeatureAnalyze"){
        cmdline::parser cmd;
        cmd.add<string>("treeHDLPath", 't', "the path of program set. You need to locate the path to the concrete dir whose content has \"Test\" dir.", true, "");
        cmd.add<string>("MRs", 'm', "The list of MRs to used. Please use \'_\' to split.", true, "");
        cmd.add<int>("judge_bound", 'j', "Model residual analysis bound.", false, 3);
        cmd.add<int>("high_leverage", 'l', "Feature analysis will remove high leverage points when the option is set 1.", false, 0);
        cmd.add<string>("extract", 'e', "If it set not empty str,  system will use assigned .csv file to do the back process without extract features again.", false, "");
        cmd.parse_check(argc - 1, argv + 1);
        UserDrive ud;
        vector<string> MRs = Tool::split(cmd.get<string>("MRs"), '_');
        ud.VTRFeatureExtract(cmd.get<string>("treeHDLPath"), MRs, cmd.get<int>("judge_bound"), cmd.get<int>("high_leverage"), true, cmd.get<string>("extract"));
    }
    else if(command == "Arrange"){
        cmdline::parser cmd;
        cmd.add<string>("treeHDLPath", 't', "the path of program set. You need to locate the path to the concrete dir whose content has \"Test\" dir.", true, "");
        cmd.add<string>("targetDir", 'd', "output path to save resource");
        cmd.add<string>("resources", 'r', "The list of resource to used. Please use \'_\' to split.\ntypes: singleVerilogError ErrorReports product Test setout DefectCases features performance", true, "");
        cmd.parse_check(argc - 1, argv + 1);
        vector<string> types = Tool::split(cmd.get<string>("resources"), '_');
        if(types.size() != 0 && types[0] == "")
            types = {};
        UserDrive ud;
        ud.arrangeData(cmd.get<string>("treeHDLPath"), cmd.get<string>("targetDir"), types);
    }
    else if(command == "VTRRandomTest"){
        cmdline::parser cmd;
        cmd.add<string>("treeHDLPath", 'h', "the path of program set. You need to locate the path to the concrete dir whose content is all .v file.", true, "");
        cmd.add<string>("rootPath", 'd', "VTR root path.", true, "");
        cmd.add<string>("archPath", 'a', "Arch file path.", true, "");
        cmd.add<string>("output", 'o', "Target directory to save output file.", true, "");
        cmd.add<int>("turns", 't', "The times of testing turn.", false, 1000);
        cmd.add<int>("seed", 's', "The seed you set to do the test", false, 0);
        cmd.parse_check(argc - 1, argv + 1);
        UserDrive ud;
        ud.loadArchPath(cmd.get<string>("archPath"));
        ud.VTRRTLRandomPerformTest(cmd.get<string>("treeHDLPath"), cmd.get<string>("rootPath"),
                                      cmd.get<string>("output"), cmd.get<int>("turns"), cmd.get<int>("seed"));
    }
    else if(command == "VTRRandomAnalyze"){
        cmdline::parser cmd;
        cmd.add<string>("randomTestPath", 'r', "the path of random test data set. You need to locate the path to the concrete dir whose content has \"product\" dir.", true, "");
        cmd.parse_check(argc - 1, argv + 1);
        UserDrive ud;
        ud.GetPerformData(cmd.get<string>("randomTestPath"));
    }
    else if(command == "VTRCaseSizeGet"){
        cmdline::parser cmd;
        cmd.add<string>("randomTestPath", 'r', "the path of random test data set. You need to locate the path to the concrete dir whose content has \"product\" dir.", true, "");
        cmd.add<string>("csv", 'c', "Random test csv generated by RandomAnalyze", true, "");
        cmd.parse_check(argc - 1, argv + 1);
        UserDrive ud;
        ud.VTRCaseSizeGet(cmd.get<string>("randomTestPath"), cmd.get<string>("csv"));
    }
    //test
    else if(command == "ged"){
        cmdline::parser cmd;
        cmd.add<string>("graph1", 'a', "Graph1", true, "");
        cmd.add<string>("graph2", 'b', "Graph2", true, "");
        cmd.parse_check(argc - 1, argv + 1);
        Evaluate e;
        Py_Initialize();
        cout << e.graphEditDistance(cmd.get<string>("graph1"), cmd.get<string>("graph2"), 30) << endl;
        Py_Finalize();
    }
    else {
        Tool::error("Unknown command: " + command + "\ncommand list: Vgenerator, VtrTest, VTRFeatureAnalyze, Arrange, VTRRandomTest, VTRRandomAnalyze, VTRCaseSizeGet");
    }
    return 0;
}