// James Dixon & Fisher Williams
// CS 4301
// Stage0

#include <iostream>
#include <fstream>
#include <cwctype>
#include <cctype>
#include <vector>
#include <ctime>
#include <time.h>
#include <iomanip>
#include <sstream>
#include <string>

using namespace std;
/*
Stage 0
Read in the Pascallite language and detect errors
Output program to .lst with error messages
Fill .obj file with symbol table
*/

//Vars
const char END_OF_FILE = '$';                //arbitrary marker
const int MAX_SYMBOL_TABLE_SIZE = 256;       //max num symbols in table
enum storeType {PROG_NAME, INTEGER, BOOLEAN};//accepted storage types
enum allocation {NO,YES};                    //allocat addition memory (arrays)
enum modes {VARIABLE, CONSTANT};             //mode of storage

struct entry                                 //struct for symbol table entrys
{
   //entry vars
    string internalName;
    string externalName;
    storeType dataType;
    modes mode;
    string value;
    allocation alloc;
    int units;
    
    //entry constructor
    entry(string iN, string eN, storeType dT, modes m, string v, allocation a, int u) :
    internalName(iN),externalName(eN),dataType(dT), mode(m), value(v), alloc(a), units(u) {}
};

vector<entry> symbolTable;
vector<string> keywords = {"program","const","var","integer","boolean","begin","end","true","false","not"};
vector<char> SpecialChars = {':',',',';','=','+','-','.'};

fstream sourceFile;           //Input file, argv[1]
ofstream list;                //List file,   argv[2]
ofstream obj;                 //obj file,    argv[3]

char charac;                  //current character
string token;                 //current token
bool containsErrors;
int lineNum;                  //line counter
int intCount;                 //int counter
int boolCount;                //bool counter

//Prototypes
bool isSpecialChar(char);     //Helper functions, self explanitory
bool isAlphaNum(char);        //Is Alpha or Digit
bool isAlpha(char);           //Is Alpha
bool isDigit(char);           //Is Digit
bool isInteger(string);        //d, -d, +d
bool isNonKeyID(string);      //returns true if the string is not a keyword and is not in symbolTable
bool isDefined(string);       //returns true if string is in symbolTable
bool isKeyword(string);       //returns true if string is a keyword

void CreateListingHeader();   //Prints list files header
void CreateListingFooter();   //Prints list files footer
void LineNumber();            //Prints line numbers
void PrintSymbolTable();      //Prints Symbol Table to obj


char NextChar();              //Reads in the next character, sets charac
string NextToken();           //Pasers in the next token, sets token
void Parser();                //Reads in the first char and starts Prog  
void Prog();                  //ProgStmt(), Conts(), Vars(), BeginEndStmt()
void ProgStmt();              //'program', NON_KEY_ID, ";"
void Consts();                //'const' ConstStmts()
void ConstStmts();            //NON_KEY_ID, '=',(NON_KEY_ID|Literal),";"
void Vars();                  //'var', VarStmts()
void VarStmts();              //Ids(),':',storeType,";"
string Ids();                 //NON_KEY_ID,NON_KEY_ID,...
void BeginEndStmt();          //'begin','end','.'

void Insert(string,storeType,modes,string,allocation,int);  //insert new entry to symbolTable
string GenInternalName(storeType);                          //Generates internal storage name
storeType WhichType(string);                                //determines data type of string
string WhichValue(string);                                  //determines value of literal or constant
entry getEntry(string);                                     //returns entry with externalName string                              //prints 

int main(int argc, char **argv)
{
   //Open the input and output streams
   sourceFile.open(argv[1]);
   list.open(argv[2]);
   obj.open(argv[3]);
   
   //set initial values
   lineNum = 1;
   intCount = boolCount = 0;
   charac = '\n';
   token = "";
   containsErrors=false;
   
   //Print the header of the list file
   CreateListingHeader();
   try
   {
    Parser();
   }
   catch (int i)
   {
      containsErrors = true;
      list<<"\nError: Line "<<lineNum-1<<": ";
       switch(i)
       {
         case 1:
            list<< "keyword \"program\" expected"<<endl;
            break;
         case 2:
            list<<"unexpected end of file"<<endl;
            break;
         case 3:
            list<<"'}' cannot begin token"<<endl;
            break;
         case 4:
            list<<"'_' must be followed by an ALPHANUM"<<endl;
            break;
         case 5:
            list<<"'_' cannot end token"<<endl;
            break;
         case 6:
            list<<"illegal symbol"<<endl;
            break;
         case 7:
            list<<"program name expected"<<endl;
            break;
         case 8:
            list<<"semicolon expected"<<endl;
            break;
         case 9:
            list<<"keyword \"const\" expected"<<endl;
            break;
         case 10:
            list<<"non-keyword identifier must follow \"const\""<<endl;
            break;
         case 11:
            list<<"multiple name definition"<<endl;
            break;
         case 12:
            list<<"illegal use of keyword"<<endl;
            break;
         case 13:
            list<<"non-keyword identifier expected"<<endl;
            break;
         case 14:
            list<<"\"=\" expected"<<endl;
            break;
         case 15:
            list<<"token to right of \"=\" illegal"<<endl;
            break;
         case 16:
            list<<"reference to undefined constant"<<endl;
            break;
         case 17:
            list<<"integer expected after sign"<<endl;
            break;
         case 18:
            list<<"boolean expected after not"<<endl;
            break;
         case 19:
            list<<"non-keyword identifier, \"begin\", or \"var\" expected"<<endl;
            break;
         case 20:
            list<<"keyword \"var\" expected"<<endl;
            break;
         case 21:
            list<<"non-keyword identifier must follow \"var\""<<endl;
            break;
         case 22:
            list<<"\":\" expected"<<endl;
            break;
         case 23:
            list<<"illegal type follows \":\""<<endl;
            break;
         case 24:
            list<<"non-keyword identifier or \"begin\" expected"<<endl;
            break;
         case 25:
            list<<"keyword \"begin\" expected"<<endl;
            break;
         case 26:
            list<<"keyword \"end\" expected"<<endl;
            break;
         case 27:
            list<<"period expected after keyword \"end\""<<endl;
            break;
         case 28:
            list<<"no text may follow \"end\""<<endl;
            break;
         case 29:
            list<<"exceeds max number of symbols"<<endl;
            break;
       }
   }
  
   CreateListingFooter();
   PrintSymbolTable();

   sourceFile.close();
   list.close();
   obj.close();
   return 0;
}

void Parser()
{
   //set charac to the first char in the file
   if(NextToken() != "program") //returns the first token and sets token
   {
       //
      throw 1;
   }
   Prog();
}
void Prog()
{
   if(token != "program")
   {
       //
      throw 1;
   }
   ProgStmt();
   if(token == "const")
   {
       Consts();
   }
   if(token == "var")
   {
        Vars();
   }
   if(token != "begin")
   {
      throw 25;
   }
   BeginEndStmt();
   if(token[0] != END_OF_FILE)
   {
      throw 28;
   }
}
void ProgStmt()
{
   string x;
   if(token != "program")
   {
       //
      throw 1;
   }
   
   x = NextToken();
   //is string x a nonkeyid?
   if(!isNonKeyID(token)) //if token is not a non_key_id
   {
       //
       throw 7;
   }
   if(NextToken() != ";")
   {
       //
       throw 8;
   }
   NextToken();
   Insert(x,PROG_NAME,CONSTANT,x,NO,0);
}

void Consts()
{
    if(token != "const")
    {
        //
        throw 9;
    }
    if(!isNonKeyID(NextToken()))
    {
        //
        throw 10;
    }
    ConstStmts();
}

void ConstStmts()
{
   string x,y;
   if(!isNonKeyID(token))
   {
      throw 13;
   }
   x = token;
   if(NextToken() != "=")
   {
      throw 14;
   }
   y = NextToken();
   //(y != "+","-","not",NON_KEY_ID,"true","false",INTEGER)
   if(y!= "+" && y!="-" && y!="not" && !isNonKeyID(y)&& y!="true" && y!="false" && !isInteger(y))//fill in
   {
      throw 15;
   }
   if(y== "+" || y=="-")
   {
      if(!isInteger(NextToken())) //NextToken != INTEGER
      {
         throw 17;
      }
      y = y + token; //does this work right?
   }
   if(y == "not")
   {
      if(NextToken() != "true" && token != "false")
      {
       throw 18;  
      }
      if(token == "true")
      {
         y = "false";
      }
      else
      {
         y = "true";
      }
   }
   if(isDefined(y))
   {
      y = getEntry(y).value;
   }
   if(NextToken() != ";")
   {
      throw 8;
   }
   Insert(x,WhichType(y),CONSTANT,WhichValue(y),YES,1);
   if(NextToken() != "begin" && token != "var" && !isNonKeyID(token))
   {
      throw 19;
   }
   if(isNonKeyID(token))
   {
      ConstStmts();
   }
}

void Vars() //token shoud be "var"
{
   if(token != "var")
   {
      throw 20;
   }
   if(!isNonKeyID(NextToken()))
   {
      throw 21;
   }
   VarStmts();
}

void VarStmts() //token should be non_key_id
{
   string x;
   storeType t;
   if(!isNonKeyID(token))
   {
      throw 13;
   }
   x = Ids();
   if(token != ":")
   {
      throw 22;
   }
   if(NextToken() != "integer" && token != "boolean")
   {
      throw 23;
   }
   if(token == "integer")
   {
      t = INTEGER;
   }
   if(token == "boolean")
   {
      t = BOOLEAN;
   }
   if(NextToken() != ";")
   {
      throw 8;
   }
   
   Insert(x,t,VARIABLE,"",YES,1);
   if(NextToken() != "begin"&& !isNonKeyID(token))
   {
      throw 24;
   }
   if(isNonKeyID(token))
   {
      VarStmts();
   }
      
}

string Ids() //token should be non_key_id
{
   string temp, tempString;
   if(!isNonKeyID(token))
   {
      throw 13;
   }
   tempString = token;
   temp = token;
   if(NextToken() == ",")
   {
      if(!isNonKeyID(NextToken()))
      {
         throw 13;
      }
      tempString = temp + "," + Ids();
   }
   return tempString;
}

void BeginEndStmt() //token should be "begin"
{
   if(token != "begin")
   {
      throw 25;
   }
   if(NextToken() != "end")
   {
      throw 26;
   }
   if(NextToken() != ".")
   {
      throw 27;
   }
   NextToken();
}

//_________________________________________
void Insert(string externalName,storeType inType,modes inMode,string inValue,allocation inAlloc,int inUnits)
{
   istringstream listOfNames(externalName);
   string name;
   while(getline(listOfNames,name,','))
   {
      if(isDefined(name))
      {
         throw 11;
      }
      else if(isKeyword(name)){
         throw 12;
      }
      if(symbolTable.size()+1>256)
      {
         throw 29;
      }
      else
      {
         //entry(string iN, string eN, storeType dT, modes m, string v, allocation a, int u) :
         //fix by actually grabing the correct information
         entry e = entry(GenInternalName(inType),name,inType,inMode,inValue,inAlloc,inUnits);
         symbolTable.push_back(e);
      }
   }
   
}

storeType WhichType(string name) // tells which data type a name has
{
   storeType type;
   //if name is a literal
   if(name=="true"||name=="false") // if the string equals true or false then bool
   {
      type = BOOLEAN;
   }
   else if(isInteger(name)) 
   {
      type = INTEGER;
   }
   //if name is an identifier, get dataType
   else if(isDefined(name))
   {
      type = getEntry(name).dataType; // undefined constant error is built in to getEntry
   }
   else
   {
      throw 16;
   }
   return type;
}

string WhichValue (string name)  //tells which value a name has
{
   string value;
   //if name is a literal (bool or int)
   if(isInteger(name))
   {
      value = name;
   }
   else if(name == "true")
   {
      value = "1";
   }
   else if(name == "false")
   {
      value = "0";
   }
   else if(isDefined(name))
   {
      value = getEntry(name).value;// undefined constant error is built in to getEntry
   }
   return value;
}

string GenInternalName(storeType t)
{
   string name;
   switch(t)
   {
      case 0:
         name = "P0";
         break;
      case 1:
         name = "I"+ to_string(intCount++);
         break;
      case 2:
         name = "B"+ to_string(boolCount++);
         break;
   }
   return name;
}

//_________________________________________

bool isInteger(string s)
{
   return isDigit(s[0])||(s[0]=='-'&&isDigit(s[1]))||(s[0]=='+'&&isDigit(s[1]));
}
bool isKeyword(string s)
{
   for(uint i =0;i<keywords.size();i++)
	{
		if(s == keywords[i])
		{
			return true;
		}
	}
	return false;
}
entry getEntry(string name)
{
   if(!isDefined(name))
   {
      throw 16;
   }
   for(uint i =0;i<symbolTable.size();i++)
   {
     if(token == symbolTable[i].externalName)
      {
         return symbolTable[i];
      }
   }
   return symbolTable[0]; // this should never be reached but needs a return stmt
}
bool isDefined(string s)
{
   for(uint i =0;i<symbolTable.size();i++)
   {
      
     if(s == symbolTable[i].externalName)
      {
         return true;
      }
   }
   return false;
}
bool isNonKeyID(string name)  //determines if it is a valid name and if it is defined and if it is a keyword
{
   return (!isKeyword(name));
}
void CreateListingHeader()
{
   time_t rawtime;
   time (&rawtime);
   list<<setw(28)<<left<<"STAGE0: James Dixon & Fisher Williams "<< ctime (&rawtime)<<endl;
   list<<setw(21)<<left<<"LINE NO."<<" "<<"SOURCE STATEMENT"<<endl<<endl;
   //line numbers and source statements should be aligned under the headings
}
void CreateListingFooter()
{
   list<<"\nCOMPILATION TERMINATED      "<<(containsErrors)<<" ERRORS ENCOUNTERED"<<endl;
}
void LineNumber()          //prints line number
{
   list<< setw(5) << right << lineNum++<<'|';
}

void PrintSymbolTable()    //outputs symbol table to obj
{
   time_t rawtime;
   time (&rawtime);
   obj<<setw(28)<<left<<"STAGE0: James Dixon"<< ctime (&rawtime)<<endl;
   obj<<"Symbol Table"<<endl<<endl;
   for(uint i =0;i<symbolTable.size();i++)
   {
      string t,m,a;
      switch(symbolTable[i].dataType)
      {
         case 0:
            t = "PROG_NAME";
            break;
         case 1:
            t = "INTEGER";
            break;
         case 2:
            t = "BOOLEAN";
            break;  
      }
      m = (symbolTable[i].mode) ? "CONSTANT" : "VARIABLE";
      a = (symbolTable[i].alloc) ? "YES" : "NO";
     obj<<setw(17)<<left<<symbolTable[i].externalName<<setw(5)<<symbolTable[i].internalName<<setw(10)<<right<<t
     <<setw(10)<<m<<setw(17)<<symbolTable[i].value<<setw(5)<<a
     <<setw(3)<<symbolTable[i].units<<endl;
   }  
}

char NextChar() //returns the next character or end of file marker
{
   char NextChar;
      
   sourceFile>>std::noskipws;
   if(sourceFile>> NextChar)
   {
       if(charac == '\n')
       {
           LineNumber();
       }
       charac = NextChar;
       list<< charac;
   }
   else
   {
       charac = END_OF_FILE;
   }
   
   return charac;
}

string NextToken() //returns the next token or end of file marker
{
	token = "";
	while (token == "")
	{
	//Process Comments
		if(charac == '{')					//process comments
		{				
			while(charac != END_OF_FILE && charac != '}')		//loops through until it finds the end comment symbol or end of file
			{
				NextChar();
			}
			if(charac ==END_OF_FILE)					
			{
				//throw process error: unexpected end of file
                throw 2;
			}
			NextChar();
		}
		else if(charac == '}')
		{
			//throw process error: '}' cannot begin token
            throw 3;
		}
	//Process whitespace
		else if(iswspace(charac))
		{
				NextChar();
				//print the whitespace
		}
	//Process special characters
		else if(isSpecialChar(charac))
		{
			token = charac;			//set token to charac
			NextChar();				//grab next char for the next time through NextToken
		}
	//Process Alphas
		else if(isAlpha(charac))
		{
			token = charac; //set the start character then build out the rest of the string
			while(isAlphaNum(NextChar()) || charac == '_')
			{
				if(token[token.length()-1]=='_'&& charac == '_')
				{
					//error cannot have back to back underscores
					//as per Pascallite Lexicon production 6
                    throw 4;
				}
            else
            {
				token += charac;
            }
			}
			if(token[token.length()-1]=='_') //tokens cannot end in underscores
			{
				//process error: '_' cannot end token
                throw 5;
			}
         if(token.length()>15)
         {
            token = token.substr(0,15);
         }
		}
	//Process Numbers
		else if(isDigit(charac))
		{
			token = charac; //set the start character then build out the rest of the string
			while(isDigit(NextChar()))	//NextChar sets and returns charac
			{
				token += charac;
			}
		}
	//Set End of File
		else if(charac == END_OF_FILE)
		{
			token = charac;
		}
	//Default~anything else/illegal symbol
		else
		{
			//process error: illegal symbol
         throw 6;
         
		}

	}
	 return token;
}

bool isSpecialChar(char c)
{
	for(uint i =0;i<SpecialChars.size();i++)
	{
		if(c == SpecialChars[i])
		{
			return true;
		}
	}
	return false;
}

bool isAlphaNum(char c)
{
	return isAlpha(c) || isDigit(c);
}
bool isAlpha(char c)
{
	return isalpha(c)&&islower(c);
}

bool isDigit(char c)
{
	return isdigit(c);
}
