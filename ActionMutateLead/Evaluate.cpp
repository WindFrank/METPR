#include "Evaluate.h"
#include "../Tool.h"


VerilogGraph Evaluate::graphConstruct(string formalDescripe) {
    VerilogGraph g;
    vector<string> lines = Tool::split(formalDescripe, ';');
    vector<string> elements;
    vector<pair<string, string>> renameMap;
    int index = 1;
    for (auto& line : lines) {
        elements = Tool::split(line, ' ');
        pair<string, string> p(elements[0], to_string(index));
        g.addNode(to_string(index));
        renameMap.push_back(p);
        index++;
    }
    for (auto& p : renameMap) {
        size_t startPos = formalDescripe.find(p.first);
        while (startPos != std::string::npos) {
            char prevChar = (startPos > 0) ? formalDescripe[startPos - 1] : ' ';
            char nextChar = (startPos + p.first.length() < formalDescripe.length()) ? formalDescripe[startPos + p.first.length()] : ' ';
            if ((isspace(prevChar) || prevChar == ';') && (isspace(nextChar) || nextChar == ';')) {
                formalDescripe.replace(startPos, p.first.length(), p.second);
            }
            startPos = formalDescripe.find(p.first, startPos + p.second.length());
        }
    }
    vector<string> newlines = Tool::split(formalDescripe, ';');
    for (auto& line : newlines) {
        elements = Tool::split(line, ' ');
        for (int i = 1; i < elements.size(); i++) {
            g.addEdge(elements[i], elements[0]);
        }
    }
    return g;
}

double Evaluate::graphEditDistance(string formulaGraph1, string formulaGraph2, double timeout) {
    VerilogGraph g1 = graphConstruct(formulaGraph1);
    VerilogGraph g2 = graphConstruct(formulaGraph2);
    vector<string> elements1;
    vector<string> elements2;
    for (auto& p : g1.nodes) {
        Node node = p.second;
        elements1.push_back(node.id);
        if (!node.neighbors.empty()) {
            for (auto& n : node.neighbors) {
                elements1.push_back(node.id + "NEXT" + n);
            }
        }
    }
    for (auto& p : g2.nodes) {
        Node node = p.second;
        elements2.push_back(node.id);
        if (!node.neighbors.empty()) {
            for (auto& n : node.neighbors) {
                elements2.push_back(node.id + "NEXT" + n);
            }
        }
    }
    if (!Py_IsInitialized()) {
        cout << "python initialize failed!" << endl;
        return 0;
    }
    //run python file
    string pythonCode = string("import sys\nsys.path.append('") + getenv("VGENERATOR_WORK_DIR") + "/lib')";
    PyRun_SimpleString(pythonCode.c_str());
    PyObject* pModule = PyImport_ImportModule("GED");
    if (pModule == nullptr) {
        cout << "module not found!" << endl;
        return 0;
    }
    PyObject* pFunc = PyObject_GetAttrString(pModule, "GED");
    if (pFunc == nullptr || !PyCallable_Check(pFunc)) {
        cout << "function not found!" << endl;
        return 0;
    }
    PyObject* arg1 = vectorToTuple(elements1);
    PyObject* arg2 = vectorToTuple(elements2);
    PyObject* arg3 = PyFloat_FromDouble(timeout); 
    PyObject* pyValue = PyObject_CallFunctionObjArgs(pFunc, arg1, arg2, arg3, NULL);
    Py_DECREF(arg1);
    Py_DECREF(arg2);
    Py_DECREF(arg3);
    if (pyValue == nullptr) {
        PyErr_Print();
        std::cout << "Error calling the Python function." << std::endl;
        Py_DECREF(pFunc);
        Py_DECREF(pModule);
        Py_Finalize();
        return 0;
    }
    double res;
    if (!PyArg_Parse(pyValue, "d", &res)) {
        PyErr_Print();
    }
    Py_DECREF(pModule);
    Py_DECREF(pFunc);
    Py_DECREF(pyValue);
    return res;
}

PyObject* Evaluate::vectorToTuple(const vector<string>& data) {
    PyObject* tuple = PyTuple_New(data.size());
    for (size_t i = 0; i < data.size(); i++) {
        PyObject* str = PyUnicode_FromString(data[i].c_str());
        PyTuple_SetItem(tuple, i, str);
    }
    return tuple;
}

double Evaluate::getJaccard(string filePath1, string filePath2, double adjustRatio = 1) {
    string filename1 = Tool::findFilefromPath(filePath1);
    string filename2 = Tool::findFilefromPath(filePath2);
    ifstream file1(filePath1);
    if (!file1.is_open()) Tool::error("Error opening file " + filename1);
    stringstream ss1;
    ss1 << file1.rdbuf();
    file1.close();

    ifstream file2(filePath2);
    if (!file2.is_open()) Tool::error("Error opening file " + filename2);
    stringstream ss2;
    ss2 << file2.rdbuf();
    file2.close();

    string line;
    vector<string> statement1;
    while (getline(ss1, line)) {
        line.erase(remove(line.begin(), line.end(), ' '), line.end());
        line.erase(remove(line.begin(), line.end(), '\n'), line.end());
        line.erase(remove(line.begin(), line.end(), '\t'), line.end());
        if (!line.empty()) statement1.push_back(line);
    }
    vector<string> statement2;
    while (getline(ss2, line)) {
        line.erase(remove(line.begin(), line.end(), ' '), line.end());
        line.erase(remove(line.begin(), line.end(), '\n'), line.end());
        line.erase(remove(line.begin(), line.end(), '\t'), line.end());
        if (!line.empty()) statement2.push_back(line);
    }

    unordered_set<string> unionSet;
    unordered_set<string> intersection;
    for (auto& s1 : statement1) {
        unionSet.insert(s1);
        for (auto& s2 : statement2) {
            unionSet.insert(s2);
            if (s1.compare(s2) == 0) intersection.insert(s1);
        }
    }

    double numerator = intersection.size();
    double denomitor = unionSet.size();
    return (1 - (numerator / denomitor)) * adjustRatio;
}