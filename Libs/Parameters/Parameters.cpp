#include "Parameters.h"
#include "StringHelper.h"

using namespace std;

const string CFGFile::commentStr = "#";
const string CFGFile::delimiterStr = "=";
const string CFGFile::rangeStartStr = "[";
const string CFGFile::rangeEndStr = "]";
const string CFGFile::startValueStr = "\"";
const string CFGFile::rangeSep = ",";
const string CFGFile::endValueStr = "\"";
const string CFGFile::sectionStartDel = "[";
const string CFGFile::sectionEndDel = "]";
const string CFGFile::sectionEnd = "[end]";

inline string replaceAll( const string& s, const string& f, const string& r ) {
	if ( s.empty() || f.empty() || f == r || s.find(f) == string::npos ) {
		return s;
	}
	ostringstream build_it;
	size_t i = 0;
	for ( size_t pos; ( pos = s.find( f, i ) ) != string::npos; ) {
		build_it.write( &s[i], pos - i );
		build_it << r;
		i = pos + f.size();
	}
	if ( i != s.size() ) {
		build_it.write( &s[i], s.size() - i );
	}
	return build_it.str();
}

bool getSectionFromLine(string const line, string &section)
{
	size_t delimiterStart = line.find(CFGFile::sectionStartDel);
	size_t delimiterEnd = line.find(CFGFile::sectionEndDel);
	if (delimiterStart < string::npos && delimiterEnd < string::npos)
	{
		section = line.substr(delimiterStart+1, delimiterEnd-1);
		trim(section);
		if (section.length()>0)
			return true;
	}

	return false;
}

bool getIdFromLine(string const line, string &id)
{
	size_t delimiterStart = line.find(CFGFile::delimiterStr);
	if (delimiterStart < string::npos)
	{
		id = line.substr( 0, delimiterStart );
		trim(id);
		if (id.length()>0)
			return true;
	}

	return false;
}

bool getRangeFromLine(string const line, string &range)
{
    size_t delimiterEnd = line.find(CFGFile::rangeStartStr) + CFGFile::rangeStartStr.length();
    size_t endValueStart = line.find(CFGFile::rangeEndStr, delimiterEnd);
    if (delimiterEnd < string::npos && endValueStart < string::npos)
    {
            range = line.substr(delimiterEnd, endValueStart-delimiterEnd);
            trim(range);
            if (range.length()>0)
                    return true;
    }

    return false;
}

bool getValueFromLine(string const line, string &value)
{
    size_t delimiterEnd = line.find(CFGFile::startValueStr) + CFGFile::startValueStr.length();
    size_t endValueStart = line.find(CFGFile::endValueStr, delimiterEnd);
    if (delimiterEnd < string::npos && endValueStart < string::npos)
    {
            value = line.substr(delimiterEnd, endValueStart-delimiterEnd);
            trim(value);
            if (value.length()>0)
                    return true;
    }

    return false;
}

bool getDescriptionFromLine(string const line, string &description)
{
	size_t commentStart = line.find(CFGFile::commentStr) + CFGFile::commentStr.length();
	if (commentStart < string::npos)
	{
		description = line.substr(commentStart);
		trim(description);
		if (description.length()>0)
			return true;
	}

	return false;
}

bool checkEndSection(string const line)
{
	size_t endSection = line.find(CFGFile::sectionEnd);
	return (endSection < string::npos);
}

bool CFGFileObject::findSectionStart(std::istream& in)
{
	string line;
	string currentSection;

	while (getline(in, line))
	{
		getSectionFromLine(line, currentSection);
		if (section.compare(currentSection) == 0)
		{
		    return true;
		}
	}

	return false;
}

bool CFGFileObject::readSectionComments(std::istream& in)
{
    string line;
    string secComments;
    size_t commentStart;
    bool commentsFound = false;

    secComments = "";

    while (getline(in, line))
    {
        if (line.length()>0)
        {
            commentStart = line.find(CFGFile::commentStr);

            if (commentStart==0) // Section comment must be at begining of line
            {
                commentsFound = true;

                string lineComment;
                if (commentStart< string::npos)
                    if (getDescriptionFromLine(line, lineComment))
                    {
                        if (secComments == "")
                            secComments = lineComment;
                        else
                            secComments += "\n" + lineComment;
                    }
            }
            else //Otherwise found variable comments: error in file
            {
              cerr << "Section comments not found on " << section << endl;
              return false;
            }
        }
        else
        {
          if (secComments.length()>0)
              comments = secComments;

          return commentsFound;
        }
    }

    return commentsFound;
}

std::ostream& operator << (std::ostream& out, const CFGFileObject &cfgFileObject)
{
    //Write section ID and comments
    string comments = replaceAll(cfgFileObject.comments, "\n", "\n"+ CFGFile::commentStr);
    out << CFGFile::sectionStartDel << cfgFileObject.section << CFGFile::sectionEndDel << endl;
    out << CFGFile::commentStr << comments << endl << endl;

    //Write object
    cfgFileObject.toStream(out);

    //Write end of section
    out << CFGFile::sectionEnd << endl << endl;

    return out;
}

std::istream& operator >> (std::istream& in, CFGFileObject &cfgFileObject)
{
    if (cfgFileObject.findSectionStart(in))
    {
        if (cfgFileObject.readSectionComments(in))
        {
            cfgFileObject.fromStream(in);
        }
    }
    else
		cerr << "section not found: " << cfgFileObject.section << endl;

    return in;
}

template <class T> std::ostream& operator << (std::ostream& out, const Parameter<T> &parameter)
{
	out << parameter.value;
	return out;
}

template <class T> std::istream& operator >> (std::istream& in,Parameter<T> &parameter)
{
	in >> parameter.value;
	return in;
}

void Parameters::addParameterN(ParameterBase &parameter, string name)
{
	parameter.name = name;
	persistentParams.push_back(&parameter);
	paramMap[name]=&parameter;
}

void Parameters::addParameterND(ParameterBase &parameter, string name, string description)
{
	parameter.name = name;
	parameter.description = description;
	persistentParams.push_back(&parameter);
	paramMap[name]=&parameter;
}

std::ostream& Parameters::toStream(std::ostream& out) const
{
	Parameters::ParametersList::iterator it;
        Parameters::ParametersList tempList = persistentParams;
        
	for (it = tempList.begin(); it!=tempList.end(); it++)
		out << (*it)->toString() << endl;

	return out;
}

std::istream& Parameters::fromStream(std::istream& in)
{
    Parameters::ParametersList::iterator it;
    string line;

    while (getline(in, line))
    {
        if (checkEndSection(line))
          break;

        if (line.length()>0)
        {
            string id;
            if (getIdFromLine(line, id))
            {
                Parameters::ParametersMap::iterator paramIt = paramMap.find(id);
                if (paramIt!=paramMap.end())
                    (*paramIt).second->fromString(line);
                else
                    cerr << "Parameter not defined: " << id << endl;
            }
        }
    }

    return in;
}

