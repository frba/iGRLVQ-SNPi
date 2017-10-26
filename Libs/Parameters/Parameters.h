#ifndef PARAMETERS_H_
#define PARAMETERS_H_

#include <list>
#include <vector>
#include <map>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <typeinfo>

class Parameters;
template <class T> class Parameter;


class CFGFileObject {

    protected:
	/*
	 * Sessão destes parâmetros no arquivo
	 */
	std::string section;

	/*
	 * Comentários de começo de arquivo
	 */
	std::string comments;

    virtual std::ostream& toStream(std::ostream& out) const = 0;
    virtual std::istream& fromStream(std::istream& in) = 0;

    friend std::ostream& operator << (std::ostream& out, const CFGFileObject &cfgFileObject);
    friend std::istream& operator >> (std::istream& in, CFGFileObject &cfgFileObject);

    bool findSectionStart(std::istream& in);
    bool readSectionComments(std::istream& in);
};

std::ostream& operator << (std::ostream& out, const CFGFileObject &cfgFileObject);
std::istream& operator >> (std::istream& in, CFGFileObject &cfgFileObject);

bool getSectionFromLine(std::string const line, std::string &section);
bool getIdFromLine(std::string const line, std::string &id);
bool getValueFromLine(std::string const line, std::string &value);
bool getRangeFromLine(std::string const line, std::string &range);
bool getDescriptionFromLine(std::string const line, std::string &description);
bool checkEndSection(std::string const line);


class ParameterBase {

public:
	std::string name;
	std::string description;

	virtual std::string toString() const = 0;
	virtual void fromString(std::string&) = 0;
        virtual void makeRandomValueList(int N){};
        virtual void selectRandomValue(int i){};
};

class Parameters:public CFGFileObject {
public:

	//Declarações de tipos
	typedef std::list<ParameterBase *> ParametersList;
	typedef std::map<std::string, ParameterBase *> ParametersMap;

	//Listas e mapas
	ParametersList persistentParams;
	ParametersMap paramMap;

	/*
	 * Construtor padrão
	 */
	Parameters() {
		section = typeid(this).name();
	}

	/*
	 * Construtor que carrega parâmetros de um arquivo
	 */
	Parameters(const std::string fileName) {
		load(fileName);
	}

	/*
	 * Construtor que seta a sessão e carrega parâmetros de um arquivo
	 */
	Parameters(const std::string fileName, const std::string section) {
		this->section = section;
		load(fileName);
	}

	/*
	 * Adiciona à lista um parâmetro sem informar sua descrição
	 * @param parameter Parametro
	 */
	void addParameterN(ParameterBase &parameter, std::string name);

	/*
	 * Adiciona à lista um parâmetro setando seu nome e descrição
	 * @param parameter Parametro
	 * @param name Nome
	 * @param description Descrição
	 */
	void addParameterND(ParameterBase &parameter, std::string name, std::string description);


	/*
	 *Esta macro pode ser chamada dentro de classes derivadas de ParametersBase
	 *Com objetivo de adicionar um parâmetro à lista de parametros com nome identico ao do identificador
	 * passado no primeiro parâmetro
	 * @param parameter Parametro
	 */
	#define addParameter(parameter) addParameterN(parameter, #parameter)

	/*
	 *Esta macro pode ser chamada dentro de classes derivadas de ParametersBase
	 *Com objetivo de adicionar um parâmetro à lista de parametros com nome identico ao do identificador
	 * passado no primeiro parâmetro
	 * @param parameter Parametro
	 * @param description Descrição
	 */
	#define addParameterD(parameter, description) addParameterND(parameter, #parameter, description)


	/*
	 * Salva os parâmetros em um arquivo
	 * @param fileName Nome do arquivo
	 */
	void save(std::string fileName) {
		std::ofstream outfile (fileName.c_str());
		outfile << (*this);
		outfile.close();
	}

	/*
	 * Lê os parâmetros de um arquivo
	 * @param fileName Nome do arquivo
	 */
	void load(std::string fileName) {
		std::ifstream infile;
		infile.open(fileName.c_str(), std::ifstream::in);
		if (infile.good())
			infile >> (*this);
		else
			std::cerr << "could not open the file:" << fileName << std::endl;
		infile.close();
	}


    std::ostream& toStream(std::ostream& out) const;
    std::istream& fromStream(std::istream& in);

};

class CFGFile
{
private:
	std::string fileName;
	std::ofstream fout;
	std::ifstream fin;

public:

	//Configurações de formatação do arquivo
	static const std::string delimiterStr;  // separator between key and value
        static const std::string rangeStartStr; //separador inicial de range
        static const std::string rangeEndStr; //separador inicial de range
        static const std::string rangeSep; //separador de range
	static const std::string commentStr;    // separator between value and comments
        static const std::string startValueStr;   // separator between value and comments
	static const std::string endValueStr;   // separator between value and comments
	static const std::string sectionStartDel; // string that defines the first string of section start
	static const std::string sectionEndDel;   // string that defines the last string of section start
	static const std::string sectionEnd;      // string that defines the section end delimiter


	CFGFile(std::string fileName) {
		this->fileName = fileName;
	}

        bool exists() {
            if (FILE * file = fopen(fileName.c_str(), "r")) {
                fclose(file);
                return true;
            } else {
                return false;
            }   
        }
	/*
	 * Limpa arquivo de configuração
	 */
	CFGFile& erase()
	{
		fout.open(fileName.c_str());
		fout << "";
		fout.close();
		return *this;
	}
	/*
	 * Write config file
	 */
	CFGFile& operator << (CFGFileObject& is) {
		fout.open(fileName.c_str(), std::ofstream::app);
		fout << is;
		fout.close();
		return *this;
	}
/*
	template<class T> CFGFile& operator << (T is) {
		fout.open(fileName.c_str(), std::ofstream::app);
		fout << is;
		fout.close();
		return *this;
	}
*/
	/*
	 * Read config file
	 */
	CFGFile& operator >> (CFGFileObject& os) {

		fin.open(fileName.c_str());
		fin >> os;
		fin.close();
		return *this;
	}
/*
	template<class T> CFGFile& operator >> (T os) {

		fin.open(fileName.c_str());
		fin >> os;
		fin.close();
		return *this;
	}
*/
};

/*
 * Classe de parâmetros para tipos que suportam básicos (os operadores da iostream << e >>)
 */
template <class T> class Parameter: public ParameterBase
{
public:
        typedef T ParameterType;
	T value;
        T minValue;
        T maxValue;
        std::vector<T> perm;
        bool rangeSet;

        Parameter() {
            rangeSet = false;
        }
        
	inline Parameter<T>& operator=(const T& value)
	{
		this->value = value;
                
                if (rangeSet && (value<minValue || value>maxValue)) {
                    std::cerr << "Parameter '" << name << "' is out of range: " << value << " is not in [" << minValue << "," << maxValue << "]." << std::endl;
                }
                
		return *this;
	}
        
        inline Parameter<T>& setRange(const T &vmin, const T &vmax) {
            minValue = vmin;
            maxValue = vmax;
            rangeSet = true;
            return *this;
        }

	inline operator T () const
	{
		return this->value;
	}

	std::string toString() const {

		std::ostringstream ost;
		ost << name << CFGFile::delimiterStr << CFGFile::startValueStr << value << CFGFile::endValueStr;
                
                if (rangeSet) {
                    ost << "\t" << CFGFile::rangeStartStr << minValue << CFGFile::rangeSep << maxValue << CFGFile::rangeEndStr;
                }
                
                ost << "\t" << CFGFile::commentStr << description;
		return ost.str();
	}

	void fromString(std::string& str) {

		if (!getIdFromLine(str, name))
			std::cerr << "Error reading parameter name " << std::endl;
                
                bool gotValue = false;

		std::string valueStr;
    		if (getValueFromLine(str, valueStr))
		{
			std::istringstream ist(valueStr);
			ist >> value;
                        gotValue = true;
		}
		
                std::string vrange;
                if (getRangeFromLine(str, vrange)) {
                    std::istringstream ist(vrange);
                    char coma;
                    ist >> minValue;
                    
                    while (!ist.eof()) {
                        ist >> coma;
                        ist >> maxValue;
                        if (!ist.bad() || !ist.fail()) break;
                    }
                    
                    if (!gotValue) value = minValue;
                    gotValue = true;
                    rangeSet = true;
                }

                if (!gotValue) 
                    std::cerr << "Error reading value for parameter " << name << std::endl;
                else
                if (rangeSet && (value<minValue || value > maxValue)) //value is out of range
                    std::cerr << "Value is out or range for parameter " << name << std::endl;
                
		getDescriptionFromLine(str, description);
	}
};

#endif /*PARAMETERS_H_*/
