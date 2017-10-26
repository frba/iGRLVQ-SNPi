#include "LHSParameters.h"
/* 
 * File:   main.cpp
 * Author: hans
 *
 * Created on 14 de Julho de 2014, 17:03
 */

#include <cstdlib>
#include "ParametersTest.h"
#include "DebugOut.h"
using namespace std;


class MyParameters: public LHSParameters {
public:
        
    Parameter<int> N;
    LHSParameter<double> p1;
    LHSParameter<float> p2;
    LHSParameter<int> p3;

    MyParameters() {
        comments = "Test float Parameters";
        section = "Teste";

        //make parameters persistent:
        addParameterD(N, "Number of samples");
        addParameterD(p1, "Parametro 1");
        addParameterD(p2, "Parametro 2");
        addParameterD(p3, "Parametro 3");

        //Set default ranges and values
        N = 50;
        p1.setRange(5, 15) = 10;
        p2.setRange(50, 150) = 100;
        p3.setRange(500, 1500) = 1000;
        
        //Add parameters to latin hypercube sampling:
        addParameterToLHS(p1);
        addParameterToLHS(p2);
        addParameterToLHS(p3);
    }
};


/*
 * 
 */
int main(int argc, char** argv) {

    CFGFile cfgFile("test.ini");
    MyParameters params;
        
    if (cfgFile.exists())
        cfgFile >> params;
    else 
        cfgFile << params;
    
    dbgOut() << params;    
    dbgOut() << params.p1.name << "\t" << params.p2.name << "\t" << params.p3.name << endl;

    for (params.initLHS(params.N); !params.finished(); params.setNextValues()) {
        dbgOut() << params.p1 << "\t" << params.p2 << "\t" << params.p3 << endl;
    }


    params.p3 = -15;
    
    return 0;
}


