/* 
 * File:   main.cpp
 * Author: Flavia Araujo
 * 
 * Created on 27 de Junho de 2014, 07:25
 */

#include <iomanip>
#include "LatinHypercubeSampling.h"
#include "IGRLVQ.h"
#include "ArffData.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <time.h>

#define RG_MIN 0 
#define RG_MAX 1 

#define NNODES 0
#define ATP 1
#define ATN 2
#define AW 3
#define TAU 4
#define EPOCHS 5
#define SEED 6
#define REMOVENODE 7
#define INSERENODE 8

using namespace std;
int classIndex = -1; // Class index: 0 firts column; -1 last column
int nCls = 2; //Number of classes in data
int topK = 6; //Numero das interacoes de SNPs avaliados atraves da acuracia
MatVector<int> colIndexesMax;
MatVector<float> colAccRand;
float AccMax;
clock_t tStart;
        
string filein;
//string filein = "../Data/Moore/threeway/20SNPs/800/best1.txt";

string fileout;
//string fileout = "output/Moore/threeway/20SNPs/800/relevances-20-best1-";


float printContigencyTable2(MatVector<int> &colIndexes, MatMatrix<float> &data, vector<int> &classes){
    MatVector<float> values(3);
    values[0] = 0; values[1] = 1; values[2] = 2;
    
    MatMatrix<int> result(pow(values.size(), colIndexes.size()), colIndexes.size() + 2);
    result.fill(0);
    
    //Fill first cols
    int alter=1;
    for (int j=0; j < result.cols()-2; j++) {
        int v=0;
        for (int i=0; i < result.rows(); i++) {
            result[i][j] = values[v];
            if ((i+1)%alter==0) v++;
            if (v>values.size()-1) v = 0;
        }
        alter = alter*values.size();
    }
    
    //Fill case/ctrl cols from data
    for (int r=0; r<data.rows(); r++) {
        for (int i=0; i < result.rows(); i++) {
            bool match=true;
            for (int j=0; j < result.cols()-2; j++) {
                if (data[r][colIndexes[j]]!=result[i][j]) {
                    match = false;
                    break;
                }
            }
            if (match) {
               if (classes[r]==0) {
                   result[i][result.cols()-2]++;
               }
               else {
                   result[i][result.cols()-1]++;
               }
               break;
            }
        }
    }
     
    //calculate confusion matrix
    MatMatrix<float> m_confusion(2,2);
    m_confusion.fill(0);
    int t_case = 0;
    int t_control = 0;
    
    for (int i=0; i < result.rows(); i++) {
        t_case = result[i][result.cols()-1];
        t_control = result[i][result.cols() - 2];
                
        if (t_case > t_control) {
            m_confusion[1][1] += t_case;
            m_confusion[1][0] += t_control;
        }
        if (t_control > t_case) {
            m_confusion[0][0] += t_control;
            m_confusion[0][1] += t_case;
        }
    }
    
    //calculate metrics
    float tp, tn, fp, fn = 0;
    tp = m_confusion[1][1];
    tn = m_confusion[0][0];
    fn = m_confusion[1][0];
    fp = m_confusion[0][1];
    
    //Accuracy
    float acc = 0;
    float accB = 0;
    acc = (tn + tp)/(tn + tp + fn + fp);
    accB = ((tp/(tp+fn))+(tn/(tn+tp)))/2;
    
    //Precision
    float precision = 0;
    precision = (tp)/(tp+fp);
    
    //Recall
    float recall = 0;
    recall = tp/(tp+fn);
    
    //F1
    float f1 = 0;
    f1 = (2*tp)/((2*tp) + fp + fn);
    
    //Fmeasure
    float odd = 0;
    odd = (tp/fp)/(fn/tn);
    
    float oddsq = 0;
    oddsq = sqrt((1/tp)+(1/fn)+(1/fp)+(1/tn));
    
    //print confusion matrix
    for (int i=0; i < m_confusion.rows(); i++) {
        for (int j=0; j < m_confusion.cols(); j++)  {
            dbgOut(0) << m_confusion[i][j] << "\t";
        }
        dbgOut(0) << endl;
    }
    dbgOut(2) << acc << "\t" << accB << endl; 
    dbgOut(2) << tp << "\t" << tn << "\t" << fp << "\t" << fn << "\t" << precision << "\t" << recall << "\t" << f1 <<  "\t";
    dbgOut(2) << endl;
    
    return acc;
}


float printContigencyTable(MatVector<int> &colIndexes, MatMatrix<float> &data, vector<int> &classes){
    MatVector<float> values(3);
    values[0] = 0; values[1] = 1; values[2] = 2;
    
    MatMatrix<int> result(pow(values.size(), colIndexes.size()), colIndexes.size() + 2);
    result.fill(0);
    
    //Fill first cols
    int alter=1;
    for (int j=0; j < result.cols()-2; j++) {
        int v=0;
        for (int i=0; i < result.rows(); i++) {
            result[i][j] = values[v];
            if ((i+1)%alter==0) v++;
            if (v>values.size()-1) v = 0;
        }
        alter = alter*values.size();
    }
    
    //Fill case/ctrl cols from data
    for (int r=0; r<data.rows(); r++) {
        for (int i=0; i < result.rows(); i++) {
            bool match=true;
            for (int j=0; j < result.cols()-2; j++) {
                if (data[r][colIndexes[j]]!=result[i][j]) {
                    match = false;
                    break;
                }
            }
            if (match) {
               if (classes[r]==0) {
                   result[i][result.cols()-2]++;
               }
               else {
                   result[i][result.cols()-1]++;
               }
               break;
            }
        }
    }
     
    //calculate confusion matrix
    MatMatrix<float> m_confusion(2,2);
    m_confusion.fill(0);
    int t_case = 0;
    int t_control = 0;
    
    for (int i=0; i < result.rows(); i++) {
        t_case = result[i][result.cols()-1];
        t_control = result[i][result.cols() - 2];
                
        if (t_case > t_control) {
            m_confusion[1][1] += t_case;
            m_confusion[1][0] += t_control;
        }
        if (t_control > t_case) {
            m_confusion[0][0] += t_control;
            m_confusion[0][1] += t_case;
        }
    }
    
    //calculate metrics
    float tp, tn, fp, fn = 0;
    tp = m_confusion[1][1];
    tn = m_confusion[0][0];
    fn = m_confusion[1][0];
    fp = m_confusion[0][1];
    
    //Accuracy
    float acc = 0;
    acc = (tn + tp)/(tn + tp + fn + fp);

    dbgOut(2) << acc << endl; 
    
    return acc;
}

void getRandomAccMean(int N, int K, GRLVQ &lvq, MatMatrix<float> &trainingData, float &randMean){
    
    MatVector<int> randIdxSel;
    std::string bitmask(K, 1); // K leading 1's
    bitmask.resize(N, 0); // N-K trailing 0's

    //Cria um vetor com indices das colunas aleatorizados
    MatVector<int> rindex(trainingData.cols());
    rindex.range(0, rindex.size() - 1);
    rindex.shuffler();
    
    // print integers and permute bitmask
    do {
        rindex.shuffler();
        for (int i = 0; i < N; ++i) // [0..N-1] integers
        {
            if (bitmask[i]){
                randIdxSel.append(rindex[i]);
                dbgOut(2) << "[" << rindex[i] << "] ";
            }
        }
        //Calcula a acuracia dos indices aleatorios
        colAccRand.append(printContigencyTable(randIdxSel, trainingData, lvq.vcls));
        randIdxSel.clear();
    } while (std::prev_permutation(bitmask.begin(), bitmask.end()));
    
    randMean = colAccRand.mean();
    dbgOut(2) << "Media dos Random: " << colAccRand.mean() << endl;
    colAccRand.clear();
    
}

void printInteractionsAccuracy(int N, int K, MatVector<int> &relIdx, MatMatrix<float> &trainingData, GRLVQ &lvq, float &randMean, float &diffMax)
{

    MatVector<int> colIndexesSelected;
    int count;
    std::string bitmask(K, 1); // K leading 1's
    bitmask.resize(N, 0); // N-K trailing 0's
    float meanRel;

//  print integers and permute bitmask
    do {
        for (int i = 0; i < N; ++i) // [0..N-1] integers
        {
            if (bitmask[i]){
                colIndexesSelected.append(relIdx[i]);
                dbgOut(2) << "[" << relIdx[i] << "] ";
            }
            dbgOut(2) << endl;
        }
        meanRel = printContigencyTable(colIndexesSelected, trainingData, lvq.vcls);
        
        //Verifica se a media obtida dos relevantes e maior que a media aleatoria
        if ((meanRel-randMean)>diffMax){
            
            for(int i=0; i < colIndexesSelected.size(); i++){
                dbgOut(2) << "["<< colIndexesSelected[i] << "]";
            }
            dbgOut(2) << ": " << meanRel << endl;
            colIndexesMax.clear();
            diffMax = meanRel-randMean;       
            colIndexesMax = colIndexesSelected;
            
            if(AccMax < meanRel){
                AccMax = meanRel;
            }
        }
        colIndexesSelected.clear();
    } while (std::prev_permutation(bitmask.begin(), bitmask.end()));
}


bool printNMax(IGRLVQ &lvq, int nNodes, int i, int epochs, unsigned int seed) {
    int n = lvq.weight.size();
    MatVector<float> relevances = lvq.weight;
    
    std::stringstream filenameout;
    filenameout << fileout << i << ".txt";
    
    ofstream file(filenameout.str().c_str());
    if(!file.is_open()){
        dbgOut(0) << "Error openning training file" << endl;
        return false;
    }
    dbgOut(0) << "Relevancias em: " << filenameout.str().c_str() << endl;
    
    //Print parameters in file
    file << std::fixed << std::setprecision(7);
    file << "Numero de Nodos:\t" << lvq.meshNodeSet.size() << endl;
    file << "T.A Positiva:\t" << lvq.alpha_tp0 << endl;
    file << "T.A Negativa:\t" << lvq.alpha_tn0 << endl;
    file << "T.A Peso:\t" << lvq.alpha_w0 << endl;
    file << "T. Decaimento:\t" << lvq.tau << endl;
    file << "Numero de Epocas:\t" << epochs << endl;
    file << "T. Remove Nodo:\t" << lvq.thrRemoveNode << endl;
    file << "T. Insere Nodo:\t" << lvq.thrInsereNode << endl;
    file << "Semente:\t" << seed << endl << endl;
    
    //Print n max relevances
    float max = -1;
    int index = 0;
    for (int i=0; i<lvq.data.cols(); i++) {
        for (int c=0; c<relevances.size(); c++) {
            if (relevances[c]>max) {
                max = relevances[c];
                index = c;
            }
        }
        
        dbgOut(2) << index << ": " << max << "; ";
        file << index << "\t" << max << endl;
        relevances[index] = -1;
        max = -1;
    }
    dbgOut(0) << endl;
    
    file << "Interacao mais relevante: ";
    dbgOut(0) << "Interacao mais relevante: ";
    for(int i=0; i< colIndexesMax.size(); i++){
        file << "[" << colIndexesMax[i] << "]";
        dbgOut(0) << "[" << colIndexesMax[i] << "]";
    }
    colIndexesMax.clear();
    file << ": " << AccMax << endl << endl;
    dbgOut(0) << ": " << AccMax << endl << endl;
    
    
    file.close();
    
    return true;
}


bool openFile(MatMatrix<float> &trainingData, vector<int> &classes, map<int,int> &labels){

    //Read data
    if (!ArffData::readArffBD(filein, trainingData, labels, classes, classIndex)) {
        dbgOut(0) << "Error openning training file" << endl;
        return false;
    }
    
    dbgOut(0) << "Pacientes: " << trainingData.rows() << endl;
    dbgOut(0) << "SNPs: " << trainingData.cols() << endl;
          
    return true;
}

void getRelIdx(int n, MatVector<float> rel, MatVector<int> &relIdx){
    relIdx.clear();
       
    for(int i=0; i<n; i++){
        float vmax = 0;
        int count = 0;
        
        for(int j=0; j<rel.size(); j++){   
            if(vmax < rel[j]){
                vmax = rel[j];
                count = j;
            }
        }
        relIdx.append(count);
        dbgOut(0) << "[" << i << "]: " << relIdx[i] << "\t" << rel[count] << endl;
        rel[count] = 0;
    }
}

void printRank(int n, MatVector<float> rel){
    
    int count;
    int countSoma;
    
    countSoma = 0;
    for(int i=0; i<n; i++){
        count = 0;
        for(int j=0; j<rel.size(); j++){
            if(rel[i] < rel[j]){
                count++;
            }
        }
        dbgOut(2) << "[" << i << "]: " << rel[i] << " " << "[" << count << "]" << endl;
        countSoma = countSoma + count;
    }
    dbgOut(2) << countSoma << endl;
    dbgOut(2) << endl;
}

bool pretrainLVQ(IGRLVQ &lvq, int nNodes, int epochs, unsigned int seed, MatMatrix<float> &trainingData, vector<int> &classes, map<int,int> &labels, int pretrainEpochs) {
      
    lvq.data = trainingData;
    lvq.vcls = classes;
    
    //Initialize LVQ
    srand(seed);
    lvq.initialize(nNodes, nCls, trainingData.cols());
    //lvq.saveLVQ("teste.lvq");
    
    //Print parameters
    dbgOut(2) << std::fixed << std::setprecision(15);
    dbgOut(2) << "Numero de Nodos: " << nNodes << endl;
    dbgOut(2) << "T.A Positiva: " << lvq.alpha_tp0 << endl;
    dbgOut(2) << "T.A Negativa: " << lvq.alpha_tn0 << endl;
    dbgOut(2) << "T.A Peso: " << lvq.alpha_w0 << endl;
    dbgOut(2) << "T. Decaimento: " << lvq.tau << endl;
    dbgOut(2) << "Numero de Epocas: " << epochs << endl;
    dbgOut(2) << "Semente: " << seed << endl;
    dbgOut(2) << "T. Remove Nodo:\t" << lvq.thrRemoveNode << endl;
    dbgOut(2) << "T. Insere Nodo:\t" << lvq.thrInsereNode << endl;
    
    //Train LVQ
    lvq.tmax = pretrainEpochs;
    lvq.trainning();

    return true;
}



bool trainLVQ(IGRLVQ &lvq, int nNodes, int epochs, unsigned int seed, MatMatrix<float> &trainingData, vector<int> &classes, map<int,int> &labels) {
   
    lvq.data = trainingData;
    lvq.vcls = classes;
    
    //Initialize LVQ
    srand(seed);
    lvq.initialize(nNodes, nCls, trainingData.cols());
    //lvq.saveLVQ("teste.lvq");
    
    //Print parameters
    dbgOut(2) << std::fixed << std::setprecision(2);
    dbgOut(2) << "Numero de Nodos: " << nNodes << endl;
    dbgOut(2) << "T.A Positiva: " << lvq.alpha_tp0 << endl;
    dbgOut(2) << "T.A Negativa: " << lvq.alpha_tn0 << endl;
    dbgOut(2) << "T.A Peso: " << lvq.alpha_w0 << endl;
    dbgOut(2) << "T. Decaimento: " << lvq.tau << endl;
    dbgOut(2) << "Numero de Epocas: " << epochs << endl;
    dbgOut(2) << "T. Remove Nodo:\t" << lvq.thrRemoveNode << endl;
    dbgOut(2) << "T. Insere Nodo:\t" << lvq.thrInsereNode << endl << endl;
    dbgOut(2) << "Semente: " << seed << endl;
    
    
    //Train LVQ
    lvq.tmax = epochs;
    lvq.trainning(lvq.tmax);
    
    IGRLVQ::TPNodeSet::iterator it;
    
    dbgOut(0) << "Nodos: " << nNodes << " -> " << lvq.meshNodeSet.size() << "\tTotal: " << lvq.id << endl;
    dbgOut(2) << "Ciclos: " << epochs << endl;
    
    return true;
}

double round(double n, unsigned d){
    return floor(n * pow(10., d) + .5) / pow(10., d);
} 

void runLHS(){
        MatMatrix<float> trainingData;
        vector<int> classes;
        map<int,int> labels;
        MatVector<int> relIdx;
        float randMean;
        
        //Open File
        openFile(trainingData, classes, labels);
        float datarowsmin = trainingData.rows()*0.01;
        float datarowsmax = trainingData.rows()*0.1;
        
        MatMatrix<double> ranges(9, 2);
        ranges[NNODES][RG_MIN] = 10; ranges[NNODES][RG_MAX] = 30; //2 a 10
	ranges[ATP][RG_MIN] = 0.8; ranges[ATP][RG_MAX] = 0.9; //ranges[ATP][RG_MIN] = 0.4; ranges[ATP][RG_MAX] = 0.5;      
	ranges[ATN][RG_MIN] = 0.01; ranges[ATN][RG_MAX] = 0.06; //Valores Percentuais de ATP   
	ranges[AW][RG_MIN] = 0.2; ranges[AW][RG_MAX] = 0.3; 
	ranges[TAU][RG_MIN] = 0.000005; ranges[TAU][RG_MAX] = 0.00001; //ideal 000001 to 0.000005
	ranges[EPOCHS][RG_MIN] = 10000; ranges[EPOCHS][RG_MAX] = 30000; //10000 a 25000
        ranges[SEED][RG_MIN] = 0; ranges[SEED][RG_MAX] = 10000;
        ranges[REMOVENODE][RG_MIN] = datarowsmin; ranges[REMOVENODE][RG_MAX] = datarowsmax;
        ranges[INSERENODE][RG_MIN] = datarowsmin; ranges[INSERENODE][RG_MAX] = datarowsmax;
        
	int L = 100; //Number of parameters sorted
	MatMatrix<double> lhs;
	LHS::getLHS(ranges, lhs, L); //Create matrix lhs with parameters
        
	//GRLVQ lvq;
	for (int i=0;i<100;i++) { // For each set of parameters
                tStart = clock();
                IGRLVQ lvq;
                lvq.insertNodeStart = 20; //ciclo para iniciar a insercao de prototipos
                lvq.removeNodeStart = 50; // ciclo para iniciar a remocao de prototipos
                AccMax = 0;
                lvq.min_change = -1;
                int nNodes = labels.size(); //Number of initial nodes created in map
		lvq.alpha_tp0 = round(lhs[i][ATP], 7); 
                lvq.alpha_tn0 = round(lhs[i][ATN], 7); 
		lvq.alpha_w0 = round(lhs[i][AW], 7); 
		lvq.tau = round(lhs[i][TAU], 7);
		int epochs = lhs[i][EPOCHS];
                lvq.thrRemoveNode = trainingData.rows()*0.01;
                lvq.thrInsereNode = trainingData.rows()*0.1;
                lvq.thrRemoveNode = round(lhs[i][REMOVENODE],7); //numero minino de vitorias de um nodo para nao ser removido
                lvq.thrInsereNode = round(lhs[i][INSERENODE],7);
                unsigned int seed = (unsigned int) lhs[i][SEED];
                               
                //Train LVQ
		trainLVQ(lvq, nNodes, epochs, seed, trainingData, classes, labels);
                            
                //Print Relevance and Index of Revelant SNPs
                printRank(topK, lvq.weight);
                
                getRelIdx(topK, lvq.weight, relIdx);
                //Print all possible interactions of K top SNPs
                float diffMax = 0;
                for(int j=1; j<topK; j++){
                    getRandomAccMean(topK, j, lvq, trainingData, randMean);
                    printInteractionsAccuracy(topK, j, relIdx, trainingData, lvq, randMean, diffMax); //Parametro numero de SNPs no topo que serao avaliados
                }
                
                //Print Results
                printNMax(lvq, nNodes, i, epochs, seed);
                
                dbgOut(0) << "Execution time: " << (double)(clock() - tStart)/CLOCKS_PER_SEC << "s" << endl;
	}
}

int main(int argc, char** argv) {
    
    dbgOut(0) << std::fixed << std::setprecision(2);
    
    dbgThreshold(0);
        
    if(argc < 2){
        dbgOut(0) << "Use: lvqlhs filein fileout" << endl;
        return -1;
    }
    filein = argv[1];
    fileout = argv[2];
    
    //dbgToLogFile("groupSNPsParametrico.log", DebugOut::erase);
    srand(0);
    runLHS();
    
    return 0;
}