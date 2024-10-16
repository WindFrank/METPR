#include "BlifAnalyzer.h"
#include "../Tool.h"

BlifAnalyzer::BlifAnalyzer(string filePath){
    string content = Tool::readFile(filePath);
    mainString = Tool::findFilefromPath(filePath);
    mainString = mainString.substr(0, mainString.find("."));
    loadBlif(content);
}

void BlifAnalyzer::loadBlif(string content){
    vector<string> lines = Tool::split(content, '\n');
    bool insideModule = false;
    for(int i = 0; i < lines.size(); i++){
        string recentLine = lines[i];
        if(recentLine.size() == 0)  //empty line
            continue;
        while(recentLine.back() == '\\'){   //long line load
            recentLine.pop_back();
            recentLine.pop_back();
            recentLine += lines[++i];
        }
        vector<string> items = Tool::split(recentLine, ' ');
        string recentType = items[0];
        if(recentType == ".model" && items[1] == mainString)
            insideModule = true;
        else if(insideModule){
            if(recentType == ".inputs"){    // inpads
                for(int k = 1; k < items.size(); k++){
                    string recentItem = items[k];
                    if(recentItem != ""){
                        inputs.push_back(recentItem);
                        BlifGraphNode bgn;
                        bgn.type = "input";
                        bgn.name = recentItem;
                        blifNodes[recentItem] = bgn;
                    }
                }
            }
            else if(recentType == ".outputs"){  // outpads
                for(int k = 1; k < items.size(); k++){
                    string recentItem = items[k];
                    if(recentItem != ""){
                        outputs.push_back(recentItem);
                        BlifGraphNode bgn;
                        bgn.type = "output";
                        bgn.name = "out__" + recentItem;
                        blifNodes[bgn.name] = bgn;
                    }
                }
            }
            else if(recentType == ".latch"){    // latch
                Latch l(items[1], items[2], items[3], items[4], stoi(items[5]));
                latchs.push_back(l);
                BlifGraphNode bgn;
                bgn.type = "latch";
                bgn.name = l.output;
                bgn.inputs.push_back(l.input);
                bgn.inputs.push_back(l.control);
                bgn.outputs.push_back(l.output);
                blifNodes[bgn.name] = bgn;
            }
            else if(recentType == ".subckt"){   // subckt
                string subcktType = items[1];
                Subckt s;
                BlifGraphNode bgn;
                s.name = subcktType;
                for(int k = 2; k < items.size(); k++){
                    string recentPortandSema = items[k];
                    if(recentPortandSema == "")
                        continue;
                    vector<string> portandSema = Tool::split(recentPortandSema, '=');
                    string port = portandSema[0];
                    string portName = port.substr(0, port.find("["));
                    string semaphore = portandSema[1];
                    s.ports.push_back(pair<string, string>(port, semaphore));
                    if(subcktType == "multiply" && portName == "out")
                        bgn.outputs.push_back(semaphore);
                    else if(subcktType == "adder" && (portName == "cout" || portName == "sumout"))
                        bgn.outputs.push_back(semaphore);
                    else
                        bgn.inputs.push_back(semaphore);
                }
                subckts.push_back(s);
                bgn.type = "subckt";
                bgn.name = mainString + "_subckt^" + to_string(subckts.size());
                blifNodes[bgn.name] = bgn;
            }
            else if(recentType == ".names"){    // names function
                NamesFunction nf;
                BlifGraphNode bgn;
                bgn.type = "names";
                if(items.back() == "")
                    items.pop_back();
                nf.funcOutput = items.back();
                items.pop_back();
                for(int k = 1; k < items.size(); k++)
                    nf.funcInputs.push_back(items[k]);
                char first = lines[++i][0];
                while(first == '0' || first == '1' || first == '-'){
                    nf.mapRules.push_back(lines[i]);
                    first = lines[++i][0];
                }
                bgn.name = nf.funcOutput;
                bgn.inputs = nf.funcInputs;
                names.push_back(nf);
                blifNodes[bgn.name] = bgn;
            }
            else if(recentType == ".end"){
                insideModule = false;
            }
        }
        else if(recentType == ".model"){
            BlifModel bm;
            bm.name = items[1];
            string subRecentLine = lines[++i];
            while(subRecentLine != ".end " && subRecentLine != ".end"){
                if(subRecentLine == ""){
                    if(i + 1 < lines.size())
                        subRecentLine = lines[++i];
                    continue;
                }
                else{
                    while(subRecentLine.back() == '\\'){
                        subRecentLine.pop_back();
                        subRecentLine.pop_back();
                        subRecentLine += lines[++i];
                    }
                    vector<string> subItems = Tool::split(subRecentLine, ' ');
                    string subType = subItems[0];
                    if(subType == ".inputs"){
                        for(int k = 1; k < subItems.size(); k++){
                            string recentItem = subItems[k];
                            bm.inpads.push_back(recentItem);
                        }
                    }
                    else if(subType == ".outputs"){
                        for(int k = 1; k < subItems.size(); k++){
                            string recentItem = subItems[k];
                            bm.outpads.push_back(recentItem);
                        }
                    }
                    // Don't forget to output .blackbox
                    if(i + 1 < lines.size())
                        subRecentLine = lines[++i];
                }
            }
            models.push_back(bm);
        }
    }
}

string BlifAnalyzer::getBlifContent(){
    string result = "";

    /*main model*/
    string mainModelStr = ".model " + mainString + "\n\n";

    //inputs and outputs
    string inoutStr = ".inputs ";
    for(int i = 0; i < inputs.size(); i++){
        inoutStr += inputs[i] + " ";
        if(i != 0 && i % 5 == 0 && i != inputs.size() - 1)
            inoutStr += "\\\n ";
    }
    inoutStr += "\n\n.outputs ";
    for(int i = 0; i < outputs.size(); i++){
        inoutStr += outputs[i] + " ";
        if(i != 0 && i % 5 == 0 && i != outputs.size() - 1)
            inoutStr += "\\\n ";
    }

    //latch
    string latchStr = "";
    for(Latch l : latchs)
        latchStr += l.toString() + "\n\n";

    //subckt
    string subcktStr = "";
    for(Subckt s : subckts)
        subcktStr += s.toString() + "\n\n";

    //names
    string namesFunctionStr = "";
    for(NamesFunction nf : names)
        namesFunctionStr += nf.toString() + "\n\n";

    //main model str addition
    mainModelStr += inoutStr + "\n\n" + latchStr + "\n\n" + subcktStr + "\n\n" + namesFunctionStr + "\n\n" + ".end";

    /*other model*/
    string otherModels = "";
    for(BlifModel bm : models)
        otherModels += bm.toString() + "\n\n";

    //All str addition
    result += mainModelStr + "\n\n" + otherModels;
    return result;
}

bool BlifAnalyzer::addModuleTag()
{
    //subckt
    for(int li = 0; li < latchs.size(); li++){
        if(isVarModuleTag(latchs[li].input))
            latchs[li].input = mainString + "^" + latchs[li].input;
        if(isVarModuleTag(latchs[li].output))
            latchs[li].output = mainString + "^" + latchs[li].output;
        if(isVarModuleTag(latchs[li].control))
            latchs[li].control = mainString + "^" + latchs[li].control;
    }

    //subckt
    for(int si = 0; si < subckts.size(); si++){
        for(int portI = 0; portI < subckts[si].ports.size(); portI++){
            if(isVarModuleTag(subckts[si].ports[portI].second))
                subckts[si].ports[portI].second = mainString + "^" + subckts[si].ports[portI].second;
        }
    }

    //names
    for(int ni = 0; ni < names.size(); ni++){
        for(int nfInputI = 0; nfInputI < names[ni].funcInputs.size(); nfInputI++){
            if(isVarModuleTag(names[ni].funcInputs[nfInputI]))
                names[ni].funcInputs[nfInputI] = mainString + "^" +names[ni].funcInputs[nfInputI];
        }
        if(isVarModuleTag(names[ni].funcOutput))
            names[ni].funcOutput = mainString + "^" + names[ni].funcOutput;
    }

    return true;
}

bool BlifAnalyzer::isVarModuleTag(string var)
{
    return (var.find(mainString + "^") == string::npos && var != "gnd" && var != "vcc" && var != "unconn");
}

bool BlifAnalyzer::connectCrit(string firstOutput, string secondInput){
    //remove old output
    for(auto it = inputs.begin(); it < inputs.end(); it++){
        if(*it== secondInput){
            inputs.erase(it);
            break;
        }
        if(it + 1 == inputs.end())
            return false;
    }

    //connect firstOutput to secondInput

    //.latchs substitute
    bool findPin = false;
    for(int i = 0; i < latchs.size(); i++){
        if(latchs[i].input == secondInput){
            latchs[i].input = firstOutput;
            findPin = true;
            break;
        }
        else if(latchs[i].control == secondInput){
            latchs[i].control = firstOutput;
            findPin = true;
            break;   
        }
    }

    //.subckts substitute
    for(int i = 0; i < subckts.size(); i++){
        for(int j = 0; j < subckts[i].ports.size(); j++){
            if(subckts[i].ports[j].second == secondInput){
                subckts[i].ports[j].second = firstOutput;
                findPin = true;
                break;
            }
        }
    }

    //.names substitute
    for(int i = 0; i < names.size(); i++){
        for(int j = 0; j < names[i].funcInputs.size(); j++){
            if(names[i].funcInputs[j] == secondInput){
                names[i].funcInputs[j] = firstOutput;
                findPin = true;
                break;
            }
        }

        if(!findPin && i + 1 == names.size())
            return false;
    }

    return true;
}

string BlifAnalyzer::toGraph(bool reset){
    if(mainString == "")
        Tool::error("Error: toGraph no loaded BlifAnalyzer.");
    else if(graphResult != "" && !reset)
        return graphResult;

    string result = "";

    for(auto bgn = blifNodes.begin(); bgn != blifNodes.end(); bgn++){
        if(find(outputs.begin(), outputs.end(), bgn->second.name) != outputs.end()) //next output
            bgn->second.nextNodes.push_back("out__" + bgn->second.name);
        for(string input : bgn->second.inputs){
            if(input == "vcc" || input == "gnd" || input == "unconn")
                continue;
            else if(blifNodes.count(input) > 0)                             //direct connect
                blifNodes[input].nextNodes.push_back(bgn->second.name);
            else{   //.subckt
                for(auto subBGN = blifNodes.begin(); subBGN != blifNodes.end(); subBGN++){
                    string subName = subBGN->first;
                    if(subBGN->second.type != "subckt")
                        continue;
                    else if(subName == bgn->second.name)
                        continue;
                    for(string subOutput : subBGN->second.outputs){
                        if(subOutput == input)
                            subBGN->second.nextNodes.push_back(bgn->second.name);
                    }
                }

                for(string output : bgn->second.outputs){
                    if(find(outputs.begin(), outputs.end(), output) != outputs.end())
                        bgn->second.nextNodes.push_back("out__" + bgn->second.name);
                }
            }
        }
    }
    for(pair<string, BlifGraphNode> sab : blifNodes){
        BlifGraphNode bgn = sab.second;
        if(bgn.nextNodes.size() != 0 || bgn.type == "output")
            result += bgn.toString() + "\n";
    }

    result.pop_back();
    return result;
}

bool BlifAnalyzer::commonGraphChange(string commonGraph){
    vector<string> graphLines = Tool::split(commonGraph, '\n');
    GraphMaxSub g = Tool::parseGraph(graphLines);
    
    //remove all nodes not referred in g
    for(auto it = blifNodes.begin(); it != blifNodes.end(); it++){
        bool isContain = false;
        for(const auto& node : g.type){
            if(it->second.name == node.first){
                isContain = true;
                break;
            }
        }
        if(!isContain)
            blifNodes.erase(it);
        else{
            for(auto nextIt = it->second.nextNodes.begin(); nextIt != it->second.nextNodes.end(); nextIt++){
                bool isNextContain = false;
                for(const auto& node : g.type){
                    if(*nextIt == node.first){
                        isNextContain = true;
                        break;
                    }
                }
                if(!isNextContain)
                    it->second.nextNodes.erase(nextIt);
            }
        }
    }
    

    //ports collect
    vector<string> remainPorts;
    for(auto blifnode : blifNodes){
        string nodename = blifnode.first;
        for(const string& portName : g.adj.at(nodename)){
            remainPorts.push_back(portName);
        }
    }
    
    //ports remove
    for(auto it = inputs.end() - 1; it >= inputs.begin(); it--){
        bool isContain = false;
        for(const auto& node : g.type){
            string nodename = node.first;
            string nodetype = node.second;
            if(nodename == *it && nodetype == "input"){
                isContain = true;
                break;
            }
        }
        if(!isContain){
            inputs.erase(it);
        }
    }

    for(auto it = outputs.end() - 1; it >= outputs.begin(); it--){
        bool isContain = false;
        for(const auto& node : g.type){
            string nodename = node.first;
            string nodetype = node.second;
            if(nodename == *it && nodetype == "output"){
                isContain = true;
                break;
            }
        }
        if(!isContain){
            outputs.erase(it);
        }
    }

    for(auto it = latchs.end() - 1; it >= latchs.begin(); it--){
        string latchName = it->output;
        bool isContain = false;
        for(const auto& node : g.type){
            string nodename = node.first;
            string nodetype = node.second;
            if(nodename == latchName && nodetype == "latch"){
                isContain = true;
                break;
            }
        }
        if(!isContain){
            latchs.erase(it);
            for(auto item = blifNodes.begin(); item != blifNodes.end(); item++){
                if(item->first == latchName){
                    blifNodes.erase(item);
                    break;
                }
            }
        }
        else{
            for(auto item = latchs.begin(); item < latchs.end(); item++){
                if(item->output == latchName){
                    if(find(remainPorts.begin(), remainPorts.end(), item->input) == remainPorts.end() 
                    || find(remainPorts.begin(), remainPorts.end(), item->control) == remainPorts.end())
                        return false;
                }
            }
        }
    }

    for(auto it = subckts.end() - 1; it >= subckts.begin(); it--){
        string subcktName = it->name;
        bool isContain = false;
        for(const auto& node : g.type){
            string nodename = node.first;
            string nodetype = node.second;
            if(nodename == subcktName && nodetype == "subckt"){
                isContain = true;
                break;
            }
        }
        if(!isContain){
            subckts.erase(it);
            for(auto item = blifNodes.begin(); item != blifNodes.end(); item++){
                if(item->first == subcktName){
                    blifNodes.erase(item);
                    break;
                }
            }
        }
        else{
            for(auto item = subckts.begin(); item < subckts.end(); item++){
                if(item->name == subcktName){
                    for(auto itport = item->ports.begin(); itport < item->ports.end(); itport++)
                        if(find(remainPorts.begin(), remainPorts.end(), itport->second) == remainPorts.end())
                            itport->second = "unconn";
                }
            }
        }
    }

    for(auto it = names.end() - 1; it >= names.begin(); it--){
        string funcname = it->funcOutput;
        bool isContain = false;
        for(const auto& node : g.type){
            string nodename = node.first;
            string nodetype = node.second;
            if(nodename == funcname && nodetype == "names"){
                isContain = true;
                break;
            }
        }
        if(!isContain){
            names.erase(it);
            for(auto item = blifNodes.begin(); item != blifNodes.end(); item++){
                if(item->first == funcname){
                    blifNodes.erase(item);
                    break;
                }
            }
        }
        else{
            for(auto item = names.begin(); item < names.end(); item++)
                if(funcname == item->funcOutput)
                    for(int k = item->funcInputs.size() - 1; k >= 0 ; k--)
                        if(find(remainPorts.begin(), remainPorts.end(), item->funcInputs[k]) == remainPorts.end()){
                            item->funcInputs.erase(item->funcInputs.begin() + k);
                            for(auto itMapRule = item->mapRules.begin(); itMapRule < item->mapRules.end(); it++)
                                itMapRule->erase(k, 1);
                        }
        }
    }

    return true;
}