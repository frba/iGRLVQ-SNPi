/* 
 * File:   LHSParameters.cpp
 * Author: hans
 * 
 * Created on 14 de Julho de 2014, 18:40
 */

#include "LHSParameters.h"

void LHSParameters::addParameterToLHS(ParameterBase &parameter) {
     lhsParams.push_back(&parameter);
}

void LHSParameters::clearLHS() {
    lhsParams.clear();
}

void LHSParameters::initLHS(int N, unsigned long seed) {
    srand(seed);
    initLHS(N);
}

void LHSParameters::initLHS(int N) {
    i = 0;
    this->N = N;
    
    ParametersList::iterator it;
    for (it = lhsParams.begin(); it!=lhsParams.end(); it++) {
        (*it)->makeRandomValueList(N);
        (*it)->selectRandomValue(i);
    }
}

bool LHSParameters::setNextValues() {
    
    if (finished()) return false;    
    i++;
    
    ParametersList::iterator it;
    for (it = lhsParams.begin(); it!=lhsParams.end(); it++) {
        (*it)->selectRandomValue(i);
    }
    
    return !finished();
}

bool LHSParameters::finished() {
    return (i>=N);
}