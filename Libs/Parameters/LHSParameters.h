/* 
 * File:   LHSParameters.h
 * Author: hans
 *
 * Created on 14 de Julho de 2014, 18:40
 */

#ifndef LHSPARAMETERS_H
#define	LHSPARAMETERS_H

#include "Parameters.h"
#include "MatMatrix.h"

template <class TNumber> class LHSParameter: public Parameter<TNumber> 
{
    void makeRandomValueList(int N) {
        
        double step = (this->maxValue - this->minValue)/(double)N;

        this->perm.resize(N);
        int i, k;
        // initialization
        for (i = 0; i < N; i++)
                this->perm[i] = (TNumber)(this->minValue + i*step + nextRand()*step);
        
        // permutation
        for (i = 0; i < N-1; i++)
        {
                 k = i + (int)(nextRand()*(N-i));
                 TNumber temp = this->perm[i];
                 this->perm[i] = this->perm[k];
                 this->perm[k] = temp;
        }
    }
    
    void selectRandomValue(int i) {
        this->value = this->perm[i];
    }
    
    double nextRand() {
        return (double)rand() / RAND_MAX;
    }
    
public:
    inline LHSParameter<TNumber>& operator=(const TNumber& value) {
        Parameter<TNumber>::operator =(value);
    }
};

class LHSParameters: public Parameters {
private:
    ParametersList lhsParams;
    int i;
    int N;
    
public:
    void addParameterToLHS(ParameterBase &parameter);
    void clearLHS();
    void initLHS(int N);
    void initLHS(int N, unsigned long seed);
    bool setNextValues();
    bool finished();
};

#endif	/* LHSPARAMETERS_H */

