/* 
 * File:   IGRLVQ.h
 * Author: Flavia Araujo
 * Algorithm iGRLVQ-SNPi based in: Incremental GRLVQ: Learning relevant features for 3D object recognition
 *                                 Kietzmann et al. 2008. Neurocomputing, 71 (2008) 2868-2879.
 *                                 
 * Created on 21 de Setembro de 2016, 15:40
 */


#ifndef IGRLVQ_H
#define	IGRLVQ_H

#include "LVQ.h"
#include <map>
#include "DebugOut.h"
#include "NodeW.h"
#include "LVQ21.h"
#include "GLVQ.h"
#include "GRLVQ.h" 

#define distWeight(x,y)   distEuclidianaWeight((x),(y))
#define distDeriv(x,y,i)  distWeightDerivV((x),(y),(i))
#define distDerivP(x,y,i) distDerivParam((x),(y),(i))

class IGRLVQ: public GRLVQ {

public:

    TNumber thrRemoveNode; //Limite minimo para remocao do nodo
    TNumber thrInsereNode; //Limite minimo para inserir do nodo
    
    TNumber insertNodeStart;
    TNumber removeNodeStart;
    
    TNumber id; //Node id
    
    TNumber countProt0; //Contador do numero de prototipos da classe 0
    TNumber countProt1; //Contador do numero de prototipos da classe 1
    
    
    IGRLVQ(){
    }
    
    virtual ~IGRLVQ(){};
    
    //Inicialização com valores aleatorios
    virtual IGRLVQ& initialize(int nNodes, int ncls, int wsize){

        GRLVQ::initialize(nNodes, ncls, wsize);
        id = nNodes; //id
        countProt1 = 1;
        countProt0 = 1;
        
        return *this;
    }
    
    virtual IGRLVQ& insertNode(int cls, TVector v){
        
        LVQNode* node = new LVQNode(id, v, cls);
        node->winCount = 100;
        node->loseCount = 0;
        node->totalchange = 0;
        aloc_node++;
        meshNodeSet.insert(node);

        if(cls == 0){
            countProt0 =  countProt0 + 1;
        }
        else{
            countProt1 = countProt1 + 1;
        }
        id++;
        return *this;
    }
    
    void imprimeNode(LVQNode* node){
        
        dbgOut(0) << node->getId() << ": ";
        for(int i=0; i< node->w.size(); i++){
            dbgOut(0) << node->w[i] << " ";
        }
        dbgOut(0) << endl;
    }
    
    
    IGRLVQ& trainning(int N = 1) {
        TPNodeSet::iterator it;
        TNode *remove;
        TNode *add;
        int i = 0;

        do{
            
//          Training iGRLVQ
            total_change = 0;
            total_changeW = 0;

            trainningStep();
            
//          Remove e reseta os nodos perdedores
            if (i > removeNodeStart && i< N/2){

                for (it = Mesh<TNode>::meshNodeSet.begin(); it != Mesh<TNode>::meshNodeSet.end(); it++) {
                    
                    if((*it)->winCount <= thrRemoveNode && Mesh<TNode>::meshNodeSet.size() > 2){
                        if((*it)->cls == 0 && countProt0 > 1){
                            remove = (*it);
                            meshNodeSet.erase(remove);
                            countProt0 = countProt0 - 1;
                        }
                        else if ((*it)->cls == 1 && countProt1 > 1){
                            remove = (*it);
                            meshNodeSet.erase(remove);
                            countProt1 = countProt1 - 1;
                        }
                    }
                }
            }
            
            
//            Insere um novo nodo para o acumulado de erro 
            if (i > insertNodeStart && i< N/2){
                add = NULL;
                
                for (it = Mesh<TNode>::meshNodeSet.begin(); it != Mesh<TNode>::meshNodeSet.end(); it++) {
                    
                    if((*it)->loseCount > thrInsereNode && (add == NULL || ((*it)->loseCount > add->loseCount))){    
                        add = (*it);
                        
                    }
                }
                if(add != NULL){
                    dbgOut(2) << add->loseCount << "/"<< meshNodeSet.size() << endl;
                    if(add->cls == 0){
                        insertNode(1, add->w);
                    }
                    else{
                        insertNode(0, add->w);
                    }
                }
            }

            for (it = Mesh<TNode>::meshNodeSet.begin(); it != Mesh<TNode>::meshNodeSet.end(); it++) {
                (*it)->loseCount = 0;
                (*it)->winCount = 0;
            }
            
            total_changeW = total_changeW/data.rows();
            total_change = total_change/data.rows();
            
            if (i%100==0){
                
                dbgOut(1) << i << "\t" <<total_change << "\t" << total_changeW << "\t";
                for (it = Mesh<TNode>::meshNodeSet.begin(); it != Mesh<TNode>::meshNodeSet.end();it++) {
                    
                    dbgOut(1) << (*it)->cls << ":\t" << average((*it)->w, 0, 4) << "\t" << average((*it)->w, 5, (*it)->w.size()-1) << "\t";
                }
                dbgOut(1) << average(weight, 0, 4) << "\t" << average(weight, 5, weight.size()-1) << endl;
            }
            i++;
        } while (i < N && total_changeW > min_change); //(total_changeW > min_change);
        
        dbgOut(0) << "Ciclos: " << i << "\tChange: " << total_change << "\tChangeW: " << total_changeW << endl;
        
        
        //Calcula a Acuracia Geral
        TNumber tp = 0;
        
        for(int i=0; i<data.rows(); i++){
            TNode* winner; 
            TNode* runnerUp;
            TVector row;
            TNumber dR; 
            TNumber dW;
            
            data.getRow(i, row);
            getWinnerRunnerUpWeight(row, winner, runnerUp, vcls[i]);
            
            dR = distWeight(row, *runnerUp);
            dW = distWeight(row, *winner);
            
            if(dW < dR){
                tp++;
            }
        }
        dbgOut(0) << "Acuracia: " << tp/data.rows() << endl;
        return *this;
    }
    
    
    virtual IGRLVQ& trainningStep() {
        
        for(int i=0; i < data.rows(); i++){
            TVector v;
            int row = rand()%data.rows();
            //std::cout << "Row: " << row << std::endl;
            data.getRow(row, v);
            updateMap(v, vcls[row]);        
        }
        return *this;
    }
    
    virtual IGRLVQ& trainningEach(int N = 1) {
        MatVector<int> vindex(data.rows());
        vindex.range(0, vindex.size() - 1);
        
        int vSize=vindex.size();
        vindex.srandom(MatUtils::getCurrentCPUTimer());
        vindex.shuffler();
        
        TVector v;
        for (int n = 0; n < N; n++) {
            total_change = 0;            
            for (int l = 0; l < vindex.size(); l++) {
                data.getRow(vindex[l], v);
                updateMap(v, vcls[vindex[l]]);
            }
            dbgOut(0) << "It: " << n << "\t" << "Change Proto: " << total_change << endl;
        }       

        return *this;
    }

    
    float average(const TVector &w, int start, int end){
        float soma = 0;
        
        for (int i = start; i <= end; i++) {
            soma = soma + w[i];
        }
        return soma/(end-start+1);
    }
    
    
    
    virtual IGRLVQ& updateMap(const TVector &w, int cls) {
        TNode *winner, *runnerUp;
                
        //Pega os prototipos mais proximos de classes distintas
        getWinnerRunnerUpWeight(w, winner, runnerUp, cls);
        
        //Variavel que calcula o quanto o prototipo modificou
        TNumber proto_changeRunner = 0; 
        TNumber proto_changeWinner = 0;

        //Calcula as distancias com pesos da amostra para ambos os prototipos
        if(winner != NULL && runnerUp != NULL){
            TNumber dR, dW;
            dR = distWeight(w, *runnerUp);
            dW = distWeight(w, *winner);

//            dbgOut(2) << "dR:" << dR << "\tdW:" << dW << endl;
            
            if(fabs(dR) > 10e-10 && fabs(dW) > 10e-10){
                //Calcula passo a passo
                TNumber sqsum, xW, xR, sgd, factorW, factorR;
                sqsum = (dW + dR) * (dW + dR);
                xW = 2 * dR / sqsum; 
                xR = 2 * dW / sqsum;
                sgd = sgdprime((dW - dR) / (dW + dR)); //Calcula sigmoide
                factorW = alpha_tp * sgd * xW; 
                factorR = - alpha_tn * sgd * xR; 
                        
                for (int i = 0; i < w.size(); i++) {
                    TNumber updateW, updateR;
                    updateW = factorW * (distDeriv(w, *winner, i));
                    updateR = factorR * (distDeriv(w, *runnerUp, i));

                    //Atualiza os prototipos Winner e Runnerup
                    if(updateW && updateR == 0) continue;
                    winner->w[i] = winner->w[i] + updateW;
                    runnerUp->w[i] = runnerUp->w[i] + updateR;

                }

                updateDistanceMeasure(w, *winner, *runnerUp);

                //Calcula o erro de classificacao
                if (dW > dR) { //O Nodo Runner esta mais proximo da amostra que o Winner
                    //Guarda a amostra com a menor distância para cada classe
                    runnerUp->loseCount++;
                }
            }
        }
        learningDecay(); // Faz o decaimento dos valores dos parâmetros

    }
    
};

#endif	/* IGRLVQ_H */

