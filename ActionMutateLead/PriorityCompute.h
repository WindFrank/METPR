/*
    PriorityCompute: accpet programs, get their variety by GED and Jaccard, generate a sequence of theire priority.
*/

#include "Evaluate.h"
#include "VarietyEvaluation.h"

class PriorityCompute{
public:
    static vector<string> prioritySequence(vector<string> program, string lastProgramPath);
    static void quickSort(vector<double>& arr, vector<string>& programPaths, int strat, int end);
};
