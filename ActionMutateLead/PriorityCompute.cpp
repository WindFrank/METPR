#include "PriorityCompute.h"
#include "../Tool.h"

vector<string> PriorityCompute::prioritySequence(vector<string> programPaths, string lastProgramPath){
    vector<double> allDistance;
    string targetGraph = VarietyEvaluation::fromVtoFormularG(lastProgramPath, Tool::findFilefromPath(lastProgramPath));
    Py_Initialize();
    for(int i = 0; i < programPaths.size(); i++){
        string recentProgramPath = programPaths[i];
        string recentGraph = VarietyEvaluation::fromVtoFormularG(recentProgramPath, Tool::findFilefromPath(recentProgramPath));
        //Get GED
        double ged = Evaluate::graphEditDistance(recentGraph, targetGraph, 30);
        double jaccard = Evaluate::getJaccard(recentProgramPath, lastProgramPath, 1);
        double distance = ged + jaccard;
        allDistance.push_back(distance);
    }
    Py_Finalize();
    quickSort(allDistance, programPaths, 0, allDistance.size());
    return programPaths;
}

 
void PriorityCompute::quickSort(vector<double>& ary, vector<string>& programPaths, int left, int right) {
	if (left >= right)
        return;
    int i, j;
    double temp, base;
    string tempStr = "";
    string baseStr = "";
    i = left, j = right;
    base = ary[left];
    baseStr = programPaths[left];
    while (i < j)
    {
        while (ary[j] <= base && i < j)
            j--;
        while (ary[i] >= base && i < j)
            i++;
        if (i < j)
        {
            temp = ary[i];
            ary[i] = ary[j];
            ary[j] = temp;
            tempStr = programPaths[i];
            programPaths[i] = programPaths[j];
            programPaths[j] = tempStr;
        }
    }

    ary[left] = ary[i];
    programPaths[left] = programPaths[i];
    ary[i] = base;
    programPaths[i] = baseStr;
    quickSort(ary, programPaths, left, i - 1);
    quickSort(ary, programPaths, i + 1, right);
}
