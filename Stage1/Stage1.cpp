// James Dixon & Fisher Williams
// CS 4301
// Stage1

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
#include <stack>

using namespace std;
/*
Stage1
Good as of 4pm 11/26/2018
Updated morning 11/27/2018 
*/

//Vars
const char END_OF_FILE = '$';                		   	//arbitrary marker
const int MAX_SYMBOL_TABLE_SIZE = 256;       		   	//max num symbols in table
enum storeType {PROG_NAME, INTEGER, BOOLEAN, UNKNOWN};  //accepted storage types
enum allocation {NO,YES};                    			//allocat addition memory (arrays)
enum modes {VARIABLE, CONSTANT};             			//mode of storage

struct entry                                 			//struct for symbol table entrys
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
vector<string> keywords = {"program","const","var","integer","boolean","begin","end","true","false","not","mod","div","and","or","read","write"};
vector<string> SpecialChars = {":",",",";","=","+","-",".",":=","*","(",")","<>","<","<=",">=",">"};

fstream sourceFile;           //Input file, argv[1]
ofstream list;                //List file,   argv[2]
ofstream obj;                 //obj file,    argv[3]

char charac;                  //current character
string token;                 //current token
bool containsErrors;
int lineNum;                  //line counter
int intCount;                 //int counter
int boolCount;                //bool counter
int currentTempNo;			  //
int maxTempNo;
int labelCount;
string curA;
stack <string> operandStack;
stack <string> operatorStack;

//Prototypes
bool isSpecialChar(string);     //Helper functions, self explanitory
bool isAlphaNum(char);        //Is Alpha or Digit
bool isAlpha(char);           //Is Alpha
bool isDigit(char);           //Is Digit
bool isIntegerLit(string);        //d, -d, +d
bool isNonKeyID(string);      //returns true if the string is not a keyword and is not in symbolTable
bool isDefined(string);       //returns true if string is in symbolTable
bool isKeyword(string);       //returns true if string is a keyword
bool isBooleanLit(string);

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
void ExecStmts();
void ExecStmt();
void AssignStmt();
void ReadStmt();
void WriteStmt();
void ReadList();
void WriteList();
void Express();
void Expresses();
void Term();
void Terms();
void Factor();
void Factors();
void Part();
void RelOp();
void AddLevelOp();
void MultLevelOp();

void Insert(string,storeType,modes,string,allocation,int);  //insert new entry to symbolTable
string GenInternalName(storeType);                          //Generates internal storage name
storeType WhichType(string);                                //determines data type of string
string WhichValue(string);                                  //determines value of literal or constant
entry getEntry(string);                                     //returns entry with externalName string                              
string getExternalName(string);

void Code(string,string,string);
void EmitProgramCode();
void EmitEndCode();
void EmitReadCode(string,string);
void EmitWriteCode(string, string);
void EmitAddCode(string, string);
void EmitSubCode(string, string);
void EmitNegateCode(string);
void EmitNotCode(string);
void EmitMultCode(string, string);
void EmitDivCode(string, string);
void EmitModCode(string, string);
void EmitAndCode(string, string);
void EmitOrCode(string, string);
void EmitEqualsCode(string, string);
void EmitNotEqualsCode(string, string);
void EmitLessThanCode(string, string);
void EmitLessThanEqualToCode(string, string);
void EmitGreaterThanCode(string, string);
void EmitGreaterThanEqualToCode(string, string);
void EmitAssignCode(string, string);

void FreeTemp();
string GetTemp();
bool isTemp(string);

string PopOperator();
string PopOperand();
void PushOperator(string);
void PushOperand(string);
void PrintStks();
string getExistingConstant(string);


int main(int argc, char **argv)
{
   //Open the input and output streams
   sourceFile.open(argv[1]);
   list.open(argv[2]);
   obj.open(argv[3]);
   
   //set initial values
   lineNum = 1;
   intCount = boolCount= labelCount = 0;
   charac = '\n';       //current character (lex)
   token = "";          //current token
   curA = "";           //current A-register
   containsErrors=false;
   maxTempNo = -1;
   currentTempNo = -1;
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
            list<<"digit expected after (+/-)sign"<<endl;
            break;
         case 18:
            list<<"boolean expected after keyword not"<<endl;
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
            list<<"\":\" or \",\" expected"<<endl;
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
            list<<"non-key id, \"read\", \"write\" or \"end\" expected"<<endl;
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
         case 30:
            list<<"may not assign to type PROG_NAME"<<endl;
            break;
         case 31:
            list<<"\"(\" expected"<<endl;
            break;
         case 32:
            list<<"\")\" or \",\" expected"<<endl;
            break;
		 case 33:
			list<<" \":=\" expected"<<endl;
			break;
		 case 34:
			list<<"\"(\", BOOLEAN, or NON_KEY_ID expected"<<endl;
			break;
		 case 35:
			list<<"\"(\", INTEGER, or NON_KEY_ID expected"<<endl;
			break;
		 case 36:
			list<<"relational operator expected"<<endl;
			break;
		 case 37:
			list<<"add level operator expected"<<endl;
			break;
		 case 38:
			list<<"multiplication level operator expected"<<endl;
			break;
         case 39:
			list<<"undefined operation encountered"<<endl;
			break;
         case 40:
			list<<"illegal type"<<endl;
			break;
         case 41:
            list<<"variable name is undefined"<<endl;
            break;
         case 42:
            list<<"cannot read in CONSTANTs"<<endl;
            break;
		 case 43:
			list<<"operand stack underflow"<<endl;
			break;
		 case 44:
			list<<"operator stack underflow"<<endl;
			break;
		 case 45:
			list<<"compiler error, currentTempNo should be >= -1"<<endl;
			break;
		 case 46:
			list<<"symbol on left-hand side of assignment must have a storage mode of VARIABLE"<<endl;
			break;
		 case 47:
			list<<"illegal type; operator requires operands of matching type"<<endl;
			break;
         case 48:
            list<<"illegal type; operator requires boolean operand(s)"<<endl;
            break;
         case 49:
            list<<"illegal type; operator requires integer operand(s)"<<endl;
            break;
       }
   }
  
   CreateListingFooter();
   //PrintSymbolTable();
   //EmitEndCode();
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
       //keyword \"program\" expected
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
   Code("program","","");
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
   if(y!= "+" && y!="-" && y!="not" && !isNonKeyID(y)&& y!="true" && y!="false" && !isIntegerLit(y))//fill in
   {
      throw 15;
   }
   if(y== "+" || y=="-")
   {
      if(!isIntegerLit(NextToken())) //NextToken != INTEGER
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
   
   if(NextToken() != ";")
   {
      throw 8;
   }
   if(isDefined(y))
   {
      if(WhichType(y)==PROG_NAME)
      {
        throw 30;
      }
      y = getEntry(y).externalName;
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

string Ids() //token should be first non_key_id in IDS list
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
   else
   {
       throw 32;
   }
   return tempString;
}

void BeginEndStmt() //token should be "begin"
{
   if(token != "begin")
   {
      throw 25;
   }
   if(isNonKeyID(NextToken())||token == "read"||token == "write")
   {
       ExecStmts();
       //need to call next token for "end" part below in exec stmts
   }
   if(token != "end")
   {
      throw 26;
   }
   if(NextToken() != ".")
   {
      throw 27;
   }
   Code("end","","");
   NextToken();
}

void ExecStmts()
{
    if(!isNonKeyID(token)&&token != "read"&&token != "write")
    { 
        throw 26; //non key id, read, write or end expected
    }
    ExecStmt(); // do the stmt
    if(isNonKeyID(NextToken())||token == "read"||token == "write")
    {
        ExecStmts();  //check for more stmts and do them
    }
}

void ExecStmt()
{
    if(isNonKeyID(token))    //nonkeyid
    {
        AssignStmt();
    }
    else if(token == "read")//keyword read
    {
        ReadStmt();
    }
    else if(token == "write")//keyword write
    {
        WriteStmt();
    }
    else     //error
    {
        throw 26;
    }
}

void AssignStmt()	//token keyid
{
    if(!isNonKeyID(token))
	{
		
		throw 13; // expected non key id
	}
	PushOperand(token);
	if(NextToken() != ":=")
	{
		throw 33;
	}
	PushOperator(token);
    NextToken();
	//EXPRESS
	Express();
	
	if(token!= ";")
	{
		throw 8;
	}
    
    string op1 = PopOperator();
	string op2 = PopOperand();
	string op3 = PopOperand();
	Code(op1,op2,op3);
}


void ReadStmt()
{
    if(token != "read")
    {
        throw 26;
    }
    ReadList();
    if(NextToken() != ";")
    {
        throw 8;
    }
}

void ReadList()
{
    string x;
    if(NextToken() != "(")
    {
        throw 31;
    }
    NextToken();
    x = Ids();
    if(token != ")")
    {
        throw 32;
    }
    
    Code("read",x,"");
}

void WriteStmt()
{
    if(token != "write")
    {
        throw 26;
    }
    WriteList();
    if(NextToken() != ";")
    {
        throw 8;
    }
    
}
void WriteList()
{
    string x;
    if(NextToken() != "(")
    {
        throw 31;
    }
    NextToken();
    x = Ids();
    if(token != ")")
    {
        throw 32;
    }
    Code("write",x,"");
}

void Express() 
{
	Term();
	Expresses();
}
void Expresses()
{
	if(token == "<>" || token == "=" || token == "<=" ||token == ">=" ||token == "<" ||token == ">")
	{
		RelOp(); // token should be the RelOp after this call
        PushOperator(token);
        NextToken();
		Term();
        string op1 = PopOperator();
		string op2 = PopOperand();
		string op3 = PopOperand();
		Code(op1,op2,op3);
		Expresses();
	}
	else if (token == ")" || token == ";")
	{
		return;
	}
	else
	{
		throw 6; // illegal symbol
	}
}
void Term()
{
	if (token == "not" || token == "true" || token == "false" || token == "(" || token == "+" || token == "-"
		|| isIntegerLit(token) || isNonKeyID(token))
	{
		Factor();
		Terms();
	}
	else
	{
		throw 6;
	}
	
}
void Terms()
{
	if (token == "-" || token == "+" || token == "or")
	{
		AddLevelOp();//token should have to AddLevelOp after this call
        PushOperator(token);
        NextToken();
		Factor();
		string op1 = PopOperator();
		string op2 = PopOperand();
		string op3 = PopOperand();
		Code(op1,op2,op3);
		Terms();
	}
	else if (token == "<>" || token == "=" || token == "<=" ||token == ">=" ||token == "<" ||token == ">" 
				||token == ")" ||token == ";")
	{
		return;
	}
	else
	{
		throw 6;
	}
}
void Factor()
{
	if (token == "not" || token == "true" || token == "false" || token == "(" || token == "+" || token == "-"
		|| isIntegerLit(token) || isNonKeyID(token))
	{
		Part();
		Factors();
	}
	else
	{
		throw 6;
	}
}
void Factors()
{
	if (token == "*" || token == "div" || token == "mod" ||token == "and")
	{
		MultLevelOp();//token should have MultLevelOp after this call
        PushOperator(token);
        NextToken();
		Part();
		string op1 = PopOperator();
		string op2 = PopOperand();
		string op3 = PopOperand();
		Code(op1,op2,op3);
        //Code(PopOperator(),PopOperand(),PopOperand());        
		Factors();
	}
	else if (token == "<>" || token == "=" || token == "<=" ||token == ">=" ||token == "<" ||token == ">" 
			||token == ")" ||token == ";" ||token == "-" ||token == "+" ||token == "or")
	{
		return;
	}
	else
	{
		throw 6;
	}
}
void Part()
{
	if (token == "not")
	{
		if (NextToken() == "(")
		{
			NextToken();
			Express();
			
			if(token != ")") //may need to be NextToken
			{
				throw 32;
			}
            string op1 = PopOperand();
			Code("not",op1,"");            
            NextToken();
		}
		else if (isBooleanLit(token))
		{
            //Maybe broken, not sure if the Operand Stack can hold strings
            PushOperand(token=="true"?"false":"true"); //flip the value and push
			NextToken();
		}
		else if (isNonKeyID(token))
		{
            Code("not", getEntry(token).internalName,"");
			NextToken();
		}
		else
		{
			throw 34;
		}
	}
	else if (token == "+")
	{
		
		if (NextToken() == "(")
		{
			NextToken();
			Express();
			
			if(token != ")") //may need to be NextToken
			{
				throw 32;
			}
            NextToken();
		}
		else if (isIntegerLit(token))
		{
            PushOperand(token);
			NextToken();
		}
		else if (isNonKeyID(token))
		{
            PushOperand(token);
			NextToken();
		}
		else
		{
			throw 35;
		}
	}
	else if (token == "-")
	{
		if (NextToken() == "(")
		{
			NextToken();
			Express();
			
			if(token != ")") //may need to be NextToken
			{
				throw 32;
			}

            string op1 = PopOperand();
			Code("neg",op1,"");   
            NextToken();
		}
		else if (isIntegerLit(token))
		{
            PushOperand('-'+token);
			NextToken();
		}
		else if (isNonKeyID(token))
		{
            
            Code("neg",getEntry(token).internalName,"");
			NextToken();
		}
		else
		{
			throw 35;
		}
	}
	else if (token == "(")
	{
        NextToken();
		Express();
			
		if(token != ")") //may need to be NextToken
		{
			throw 32;
		}
        NextToken();
	}
	else if (isIntegerLit(token))
	{
        
		PushOperand(token);
		NextToken();
	}
	else if (isBooleanLit(token))
	{
		PushOperand(token);
		NextToken();
	}
	else if (isNonKeyID(token))
	{
        
		PushOperand(token);
		NextToken();
	}
	else
	{
		throw 6;
	}
}
void RelOp()
{
	if(token == "<>")
	{
	}
	else if(token == "=")
	{
	}
	else if(token == "<=")
	{
	}
	else if(token == ">=")
	{
	}
	else if(token == "<")
	{
	}
	else if(token == ">")
	{
	}
	else 
	{
		throw 36;
	}
}
void AddLevelOp()
{
	if(token == "+")
	{
	}
	else if(token == "-")
	{
	}
	else if(token == "or")
	{
	}
	else
	{
		throw 37;
	}
}
void MultLevelOp()
{
	if(token == "*")
	{
	}
	else if(token == "div")
	{
	}
	else if(token == "mod")
	{
	}
	else if(token == "and")
	{
	}
	else
	{
		throw 38;
	}
}

//Action Routines
//_________________________________________
void Code(string operation, string op1, string op2)
{
			
    if(operation == "program")
    {
        EmitProgramCode();
    }
    else if(operation == "end")
    {
        EmitEndCode();
    }
    else if(operation == "read")
    {
        istringstream listOfNames(op1);
        string name;
        while(getline(listOfNames,name,','))
        {
           if(!isDefined(name))
           {
              throw 41;//variable is not defined
           }
           else if(isKeyword(name)){
              throw 12;
           }
           else if(getEntry(name).mode != VARIABLE)
           {
               throw 42;
           }
           else
           {
              EmitReadCode(getEntry(name).internalName, name);
           }
        }
        
    }
    else if(operation == "write")
    {
        istringstream listOfNames(op1);
        string name;
        while(getline(listOfNames,name,','))
        {
           if(!isDefined(name))
           {
              throw 41;//variable is not defined
           }
           else if(isKeyword(name)){
              throw 12;
           }
           else
           {
              EmitWriteCode(getEntry(name).internalName, name);
           }
        }
    }
    else if(operation == "+")
    {               //rhs, lhs ?
        EmitAddCode(op1,op2);
    }
    else if(operation == "-")
    {
        EmitSubCode(op1, op2);
    }
    else if(operation == "neg")
    {
        EmitNegateCode(op1);
    }
    else if(operation == "not")
    {
        EmitNotCode(op1);
    }
    else if(operation == "*")
    {
        EmitMultCode(op1, op2);
    }
    else if(operation == "div")
    {
        EmitDivCode(op1, op2);
    }
    else if(operation == "mod")
    {
        EmitModCode(op1, op2);
    }
    else if(operation == "and")
    {
        EmitAndCode(op1, op2);
    }
    else if(operation == "or")
    {
        EmitOrCode(op1, op2);
    }
    else if(operation == "=")
    {
        EmitEqualsCode(op1, op2);
    }
    else if(operation == "<>")
    {
        EmitNotEqualsCode(op1, op2);
    }
    else if(operation == "<")
    {
        EmitLessThanCode(op1, op2);
    }
    else if(operation == "<=")
    {
        EmitLessThanEqualToCode(op1, op2);
    }
    else if(operation == ">")
    {
        EmitGreaterThanCode(op1, op2);
    }
    else if(operation == ">=")
    {
        EmitGreaterThanEqualToCode(op1, op2);
    }
    else if(operation == ":=")
    {
        EmitAssignCode(op1, op2);
    }
    else
    {
        //undefined operation
        throw 39;
    }
}

void EmitProgramCode()
{
    //easy copypaste for all formating issues
    string location,opCode,address,signNumber,comments;
    location=opCode=address=signNumber=comments ="";
    location = "STRT";
    opCode = "NOP";
    comments = getExternalName("P0")+" - James Dixon & Fisher Williams";
    obj<<left<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
}
void EmitEndCode()
{
    string location,opCode,address,signNumber,comments;
    location=opCode=address=signNumber=comments ="";
    
	opCode = "HLT";
	obj<<left<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	
    //print symbolTable
    for(uint i =0;i<symbolTable.size();i++)
    {
       if(symbolTable[i].alloc == YES)
       {
           location = symbolTable[i].internalName;
           opCode = symbolTable[i].mode == CONSTANT ? "DEC":"BSS";
           address = symbolTable[i].mode == CONSTANT ? symbolTable[i].value:"1";
           comments = symbolTable[i].externalName;
           if(address[0]=='-')
           {
               address = address.substr(1);
               obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" -"<<setw(3)<<setfill('0')<<address<<setfill(' ')<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;

           }
           else
           {
               obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<setfill('0')<<address<<setfill(' ')<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
           }
       }
    }  
    

    //print END op
    opCode = "END";
    address = "STRT";
    comments = "end of program";
	location = "";
    obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;

}
void EmitReadCode(string adr, string name) 
{
    string location,opCode,address,signNumber,comments;
    location=opCode=address=signNumber=comments ="";
    opCode = "RDI";
    address = adr;
    comments = "read("+name+")";
    obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
    //Read does NOT change A-register
}
void EmitWriteCode(string adr, string name)
{
    string location,opCode,address,signNumber,comments;
    location=opCode=address=signNumber=comments ="";
    opCode = "PRI";
    address = adr;
    comments = "write("+name+")";
    obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
    //Write does NOT change A-register

}
void EmitAddCode(string op1, string op2)
{
	string location,opCode,address,signNumber,comments;
    location=opCode=address=signNumber=comments ="";

    if (getEntry(getExternalName(op1)).dataType != INTEGER || getEntry(getExternalName(op2)).dataType != INTEGER)
    {
       throw 49;
    }
    if(isTemp(curA) && curA != op1 && curA != op2) //if curA is a temp that isn't used in this addition
    {
		 //store the temp in curA into memory
		 opCode = "STA";
		address = curA;
		comments = "store "+curA+" into memeory";
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
		 
		  for(uint i =0;i<symbolTable.size();i++)
		   {
			 if(curA == symbolTable[i].internalName)
			  {
				 symbolTable[i].alloc = YES;
				 symbolTable[i].dataType = INTEGER;
			  }
		   }
		//getEntry(curA).type = WhichType(curA);
		curA = "";
		
    }
	if(!(isTemp(curA)) && curA != op1 && curA != op2) 
	{
		curA = "";
	}
	if(curA != op1 && curA != op2)
	{
		 
		opCode = "LDA";
		address = op2;
		comments = "load "+ getExternalName(op2)+" into A-reg";
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
		curA = op2;
	}
	
	//do the math
	if(curA!=op2) 	//op1 is in A-reg
	{
		opCode = "IAD";
		address = op2;
		comments = getExternalName(op2) + " + " + getExternalName(op1);
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
	}
	else			//op2 is in A-reg
	{
		opCode = "IAD";
		address = op1;
		comments = getExternalName(op2) + " + " + getExternalName(op1);
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
	}
	if(isTemp(op1))
	{
		FreeTemp();
	}
	if(isTemp(op2))
	{
		FreeTemp();
	}
	curA = GetTemp();
	  for(uint i =0;i<symbolTable.size();i++)
	   {
		 if(curA == symbolTable[i].internalName)
		  {
			 symbolTable[i].dataType = INTEGER;
		  }
	   }
	PushOperand(curA);
	
	
    /*   
      if(curA[0]=='T' && curA != op1 && curA != op2))
     if A-register holds a temp not op1 nor op2
            deassign temp
            emit code to store temp into memory
            change the allocate entry for the temp in the symbol table to yes 
     if A-register holds a non-temp not op1 nor op2
            deassign it
     if neither operand is in the A-register
            emit code to load op2 into A-reg
     emit code to perform register-memory addition
     deassign all temporaries involved in the addition and free those names for reuse
     A-register = next available temporary name and change type of its symbol table entry to integer
     push the name of the result onto operandStk
    */
}
void EmitSubCode(string op1, string op2)
{
	string location,opCode,address,signNumber,comments;
    location=opCode=address=signNumber=comments ="";

    if (getEntry(getExternalName(op1)).dataType != INTEGER || getEntry(getExternalName(op2)).dataType != INTEGER)
    {
       throw 49;
    }
    if(isTemp(curA) && curA != op2) //if curA is a temp that isn't used in this subtraction
    {
		 //store the temp in curA into memory
		 opCode = "STA";
		address = curA;
		comments = "store "+curA+" into memeory";
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
		 
		  for(uint i =0;i<symbolTable.size();i++)
		   {
			 if(curA == symbolTable[i].internalName)
			  {
				 symbolTable[i].alloc = YES;
			  }
		   }
		//getEntry(curA).type = WhichType(curA);
		curA = "";
		
    }
	if(!(isTemp(curA)) && curA != op2) 
	{
		curA = "";
	}
	if(curA != op2)
	{
		 
		opCode = "LDA";
		address = op2;
		comments = "load "+ getExternalName(op2)+" into A-reg";
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
		curA = op2;
	}
	
	//do the math

	opCode = "ISB";
	address = op1;
	comments = getExternalName(op2) + " - " + getExternalName(op1);
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	comments = "";

	
	if(isTemp(op1))
	{
		FreeTemp();
	}
	if(isTemp(op2))
	{
		FreeTemp();
	}
	curA = GetTemp();
	  for(uint i =0;i<symbolTable.size();i++)
	   {
		 if(curA == symbolTable[i].internalName)
		  {
			 symbolTable[i].dataType = INTEGER;
		  }
	   }
	PushOperand(curA);
}
void EmitNegateCode(string op1)
{
	if (getEntry(getExternalName(op1)).dataType != INTEGER)
    {
       throw 49;
    }
	
	string location,opCode,address,signNumber,comments;
    location=opCode=address=signNumber=comments ="";

    if(isTemp(curA)) //if curA is a temp that isn't used in this subtraction
    {
		 //store the temp in curA into memory
		 opCode = "STA";
		address = curA;
		comments = "store "+curA+" into memeory";
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
		 
		  for(uint i =0;i<symbolTable.size();i++)
		   {
			 if(curA == symbolTable[i].internalName)
			  {
				 symbolTable[i].alloc = YES;
			  }
		   }
		//getEntry(curA).type = WhichType(curA);
		curA = "";
		
    }
	
	//load false
    if(getExistingConstant("ZERO")!="")
    {
        address = getExistingConstant("ZERO");
    }
	else if (!isDefined("ZERO"))
	{
		Insert("ZERO",INTEGER,CONSTANT,"0",YES,1);
        address = "ZERO";
	}
	if(curA!=address)
    {
        opCode = "LDA";
        comments = "load "+getExternalName(address)+" into A-reg";
        obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
        signNumber = "";
        location = "";
    }
	
	opCode = "ISB";
	address = op1;
	comments = "-( "+getExternalName(op1)+" )";
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	signNumber = "";
	location = "";
	
	if(isTemp(op1))
	{
		FreeTemp();
	}
    curA = GetTemp();
	for(uint i =0;i<symbolTable.size();i++)
   {
	 if(curA == symbolTable[i].internalName)
	  {
		 symbolTable[i].dataType = INTEGER;
	  }
   }
	PushOperand(curA);
}
void EmitNotCode(string op1)
{
	if (getEntry(getExternalName(op1)).dataType != BOOLEAN)
    {
       throw 48;
    }
	
	string location,opCode,address,signNumber,comments;
    location=opCode=address=signNumber=comments ="";

    if(isTemp(curA) && curA != op1) //if curA is a temp 
    {
		 //store the temp in curA into memory
		 opCode = "STA";
		address = curA;
		comments = "store "+curA+" into memeory";
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
		 
		  for(uint i =0;i<symbolTable.size();i++)
		   {
			 if(curA == symbolTable[i].internalName)
			  {
				 symbolTable[i].alloc = YES;
			  }
		   }
		//getEntry(curA).type = WhichType(curA);
		curA = "";
		
    }
	if(!(isTemp(curA)) && curA != op1) 
	{
		curA = "";
	}
	if(curA != op1)
	{
		 
		opCode = "LDA";
		address = op1;
		comments = "load "+ getExternalName(op1)+" into A-reg";
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
		curA = op1;
	}
	

	//jump
	opCode = "AZJ";
	address = "L" + to_string(labelCount);
	comments = "not( "+getExternalName(op1)+" )";
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	signNumber = "";
	
	//load false
    if(getExistingConstant("FALS")!="")
    {
        address = getExistingConstant("FALS");
    }
	else if (!isDefined("FALS"))
	{
		Insert("FALS",BOOLEAN,CONSTANT,"0",YES,1);
        address = "FALS";
	}
	
	opCode = "LDA";
	comments = "";
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	signNumber = "";
	location = "";
	
	//unj
	opCode = "UNJ";
	address = "L" + to_string(labelCount);
	comments = "";
	signNumber = "+1";
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	signNumber = "";
	
	//load true
    if(getExistingConstant("TRUE")!="")
    {
        address = getExistingConstant("TRUE");
    }
	else if (!isDefined("TRUE"))
	{
		Insert("TRUE",BOOLEAN,CONSTANT,"1",YES,1);
        address = "TRUE";
	}
	
	opCode = "LDA";
	comments = "";
	location = "L" + to_string(labelCount);
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	signNumber = "";
	location = "";
	
	if(isTemp(op1))
	{
		FreeTemp();
	}

	curA = GetTemp();
	  for(uint i =0;i<symbolTable.size();i++)
	   {
		 if(curA == symbolTable[i].internalName)
		  {
			 symbolTable[i].dataType = BOOLEAN;
		  }
	   }
	labelCount++;
	PushOperand(curA);
}
void EmitMultCode(string op1, string op2)
{
	string location,opCode,address,signNumber,comments;
    location=opCode=address=signNumber=comments ="";

    if (getEntry(getExternalName(op1)).dataType != INTEGER || getEntry(getExternalName(op2)).dataType != INTEGER)
    {
       throw 49;
    }
    if(isTemp(curA) && curA != op1 && curA != op2) //if curA is a temp that isn't used in this addition
    {
		 //store the temp in curA into memory
		 opCode = "STA";
		address = curA;
		comments = "store "+curA+" into memeory";
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
		 
		  for(uint i =0;i<symbolTable.size();i++)
		   {
			 if(curA == symbolTable[i].internalName)
			  {
				 symbolTable[i].alloc = YES;
			  }
		   }
		//getEntry(curA).type = WhichType(curA);
		curA = "";
		
    }
	if(!(isTemp(curA)) && curA != op1 && curA != op2) 
	{
		curA = "";
	}
	if(curA != op1 && curA != op2)
	{
		 
		opCode = "LDA";
		address = op2;
		comments = "load "+ getExternalName(op2)+" into A-reg";
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
		curA = op2;
	}
	
	//do the math
	if(curA!=op2) 	//op1 is in A-reg
	{
		opCode = "IMU";
		address = op2;
		comments = getExternalName(op2) + " * " + getExternalName(op1);
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
	}
	else			//op2 is in A-reg
	{
		opCode = "IMU";
		address = op1;
		comments = getExternalName(op2) + " * " + getExternalName(op1);
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
	}
	if(isTemp(op1))
	{
		FreeTemp();
	}
	if(isTemp(op2))
	{
		FreeTemp();
	}
	curA = GetTemp();
	  for(uint i =0;i<symbolTable.size();i++)
	   {
		 if(curA == symbolTable[i].internalName)
		  {
			 symbolTable[i].dataType = INTEGER;
		  }
	   }
	PushOperand(curA);
	
    /*
    if A-register holds a temp not op1 nor op2 then 
        deassign it
        emit code to store that temp into memory
        change the allocate entry for it in the symbol table to yes 
    if A-register holds a non-temp not op2 nor op1 
        then deassign it
    if neither operand is in A-register then
        emit code to load op2 into the A-register;
    emit code to perform A-register-memory multiplication with A-register holding the result;
    deassign all temporaries involved and free those names for reuse;
    A-register = next available temporary name and change type of its symbol table entry to integer
    push name of result onto operandStk;
    */
}
void EmitDivCode(string op1, string op2)
{
    string location,opCode,address,signNumber,comments;
    location=opCode=address=signNumber=comments ="";

    if (getEntry(getExternalName(op1)).dataType != INTEGER || getEntry(getExternalName(op2)).dataType != INTEGER)
    {
       throw 49;
    }
    if(isTemp(curA) && curA != op2) //if curA is a temp that isn't used in this addition
    {
		 //store the temp in curA into memory
		 opCode = "STA";
		address = curA;
		comments = "store "+curA+" into memeory";
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
		 
		  for(uint i =0;i<symbolTable.size();i++)
		   {
			 if(curA == symbolTable[i].internalName)
			  {
				 symbolTable[i].alloc = YES;
			  }
		   }
		//getEntry(curA).type = WhichType(curA);
		curA = "";
		
    }
	if(!(isTemp(curA)) && curA != op2) 
	{
		curA = "";
	}
	if(curA != op2)
	{
		 
		opCode = "LDA";
		address = op2;
		comments = "load "+ getExternalName(op2)+" into A-reg";
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
		curA = op2;
	}
	
	//do the math

	opCode = "IDV";
	address = op1;
	comments = getExternalName(op2) + " div " + getExternalName(op1);
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	comments = "";

	//curA = GetTemp();
	if(isTemp(op1))
	{
		FreeTemp();
	}
	if(isTemp(op2))
	{
		FreeTemp();
	}
	curA = GetTemp();
	for(uint i =0;i<symbolTable.size();i++)
	{
		 if(curA == symbolTable[i].internalName)
		  {
			 symbolTable[i].dataType = INTEGER;
		  }
	}
	PushOperand(curA);
}
void EmitModCode(string op1, string op2)
{
	    string location,opCode,address,signNumber,comments;
    location=opCode=address=signNumber=comments ="";

    if (getEntry(getExternalName(op1)).dataType != INTEGER || getEntry(getExternalName(op2)).dataType != INTEGER)
    {
       throw 49;
    }
    if(isTemp(curA) && curA != op2) //if curA is a temp that isn't used in this addition
    {
		 //store the temp in curA into memory
		 opCode = "STA";
		address = curA;
		comments = "store "+curA+" into memeory";
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
		 
		  for(uint i =0;i<symbolTable.size();i++)
		   {
			 if(curA == symbolTable[i].internalName)
			  {
				 symbolTable[i].alloc = YES;
			  }
		   }
		//getEntry(curA).type = WhichType(curA);
		curA = "";
		
    }
	if(!(isTemp(curA)) && curA != op2) 
	{
		curA = "";
	}
	if(curA != op2)
	{
		 
		opCode = "LDA";
		address = op2;
		comments = "load "+ getExternalName(op2)+" into A-reg";
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
		curA = op2;
	}
	
	//do the math

	opCode = "IDV";
	address = op1;
	comments = getExternalName(op2) + " mod " + getExternalName(op1);
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	comments = "";
	
	address = GetTemp();
	
	for(uint i =0;i<symbolTable.size();i++)
	   {
		 if(address == symbolTable[i].internalName)
		  {
			 symbolTable[i].alloc = YES;
             symbolTable[i].dataType = INTEGER; //this one belongs here
		  }
	   }
	   
	opCode = "STQ";
	
	comments = "store "+address+" into memeory";
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	comments = "";
	
	
	
	opCode = "LDA";
	comments = "";
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	comments = "";

	
	if(isTemp(op1))
	{
		FreeTemp();
	}
	if(isTemp(op2))
	{
		FreeTemp();
	}
	FreeTemp();
	curA = GetTemp();
	for(uint i =0;i<symbolTable.size();i++)
	{
		 if(curA == symbolTable[i].internalName)
		  {
			 symbolTable[i].dataType = INTEGER;
		  }
	}
	PushOperand(curA);

}
void EmitAndCode(string op1, string op2)
{
    string location,opCode,address,signNumber,comments;
    location=opCode=address=signNumber=comments ="";

    if (getEntry(getExternalName(op1)).dataType != BOOLEAN || getEntry(getExternalName(op2)).dataType != BOOLEAN)
    {
       throw 48;
    }
    if(isTemp(curA) && curA != op1 && curA != op2) //if curA is a temp that isn't used in this addition
    {
		 //store the temp in curA into memory
		 opCode = "STA";
		address = curA;
		comments = "store "+curA+" into memeory";
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
		 
		  for(uint i =0;i<symbolTable.size();i++)
		   {
			 if(curA == symbolTable[i].internalName)
			  {
				 symbolTable[i].alloc = YES;
			  }
		   }
		//getEntry(curA).type = WhichType(curA);
		curA = "";
		
    }
	if(!(isTemp(curA)) && curA != op1 && curA != op2) 
	{
		curA = "";
	}
	if(curA != op1 && curA != op2)
	{
		 
		opCode = "LDA";
		address = op2;
		comments = "load "+ getExternalName(op2)+" into A-reg";
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
		curA = op2;
	}
	
	//do the math
	if(curA!=op2) 	//op1 is in A-reg
	{
		opCode = "IMU";
		address = op2;
		comments = getExternalName(op2) + " and " + getExternalName(op1);
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
	}
	else			//op2 is in A-reg
	{
		opCode = "IMU";
		address = op1;
		comments = getExternalName(op2) + " and " + getExternalName(op1);
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
	}
	if(isTemp(op1))
	{
		FreeTemp();
	}
	if(isTemp(op2))
	{
		FreeTemp();
	}
	curA = GetTemp();
	  for(uint i =0;i<symbolTable.size();i++)
	   {
		 if(curA == symbolTable[i].internalName)
		  {
			 symbolTable[i].dataType = BOOLEAN;
		  }
	   }
	PushOperand(curA);
    /*
    if A-register holds a temp not op1 nor op2 then 
        deassign it
        emit code to store that temp into memory
        change the allocate entry for it in the symbol table to yes 
    if A-register holds a non-temp not op2 nor op1 
        then deassign it
    if neither operand is in A-register then
        emit code to load op2 into the A-register;
    emit code to perform A-register-memory multiplication with A-register holding the result;
    deassign all temporaries involved and free those names for reuse;
    A-register = next available temporary name and change type of its symbol table entry to boolean
    push name of result onto operandStk;
    */
}
void EmitOrCode(string op1, string op2)
{	
	string location,opCode,address,signNumber,comments;
    location=opCode=address=signNumber=comments ="";

    if (getEntry(getExternalName(op1)).dataType != BOOLEAN || getEntry(getExternalName(op2)).dataType != BOOLEAN)
    {
       throw 48;
    }
    if(isTemp(curA) && curA != op1 && curA != op2) //if curA is a temp that isn't used in this addition
    {
		//store the temp in curA into memory
		opCode = "STA";
		address = curA;
		comments = "store "+curA+" into memeory";
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
		 
		  for(uint i =0;i<symbolTable.size();i++)
		   {
			 if(curA == symbolTable[i].internalName)
			  {
				 symbolTable[i].alloc = YES;
			  }
		   }
		//getEntry(curA).type = WhichType(curA);
		curA = "";
		
    }
	if(!(isTemp(curA)) && curA != op1 && curA != op2) 
	{
		curA = "";
	}
	if(curA != op1 && curA != op2)
	{
		 
		opCode = "LDA";
		address = op2;
		comments = "load "+ getExternalName(op2)+" into A-reg";
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
		curA = op2;
	}
	
	//do the math
	if(curA!=op2) 	//op1 is in A-reg
	{
		opCode = "IAD";
		address = op2;
		comments = getExternalName(op2) + " or " + getExternalName(op1);
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
	}
	else			//op2 is in A-reg
	{
		opCode = "IAD";
		address = op1;
		comments = getExternalName(op2) + " or " + getExternalName(op1);
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
	}
	
	//jump
	opCode = "AZJ";
	address = "L" + to_string(labelCount);
	comments = "";
	signNumber = "+1";
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	comments = "";
	signNumber = "";
	
	//load true
    if(getExistingConstant("TRUE")!="")
    {
        address = getExistingConstant("TRUE");
    }
	else if (!isDefined("TRUE"))
	{
		Insert("TRUE",BOOLEAN,CONSTANT,"1",YES,1);
        address = "TRUE";
	}
	opCode = "LDA";
	comments = "";
	location = "L" + to_string(labelCount);
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	comments = "";
	location = "";
	
	if(isTemp(op1))
	{
		FreeTemp();
	}
	if(isTemp(op2))
	{
		FreeTemp();
	}
	curA = GetTemp();
	for(uint i =0;i<symbolTable.size();i++)
	{
		if(curA == symbolTable[i].internalName)
	    {
	   	 symbolTable[i].dataType = BOOLEAN;
		}
	}
	PushOperand(curA);
	labelCount++;
    /*
    if A-register holds a temp not op1 nor op2 then 
        deassign it
        emit code to store that temp into memory
        change the allocate entry for it in the symbol table to yes 
    if A-register holds a non-temp not op2 nor op1 
        then deassign it
    if neither operand is in A-register then
        emit code to load op2 into the A-register;
    emit code to perform A-register-memory addition with A-register holding the result;
    emit code to perform an AZJ to the next available Ln + 1
    emit code to label the next instruction with that label and do A-register-memory load TRUE
    insert TRUE in symbol table with value 1 and external name true
    deassign all temporaries involved and free those names for reuse;
    A-register = next available temporary name and change type of its symbol table entry to boolean       
    push name of result onto operandStk;
    */
}
void EmitEqualsCode(string op1, string op2)
{
     if (getEntry(getExternalName(op1)).dataType != getEntry(getExternalName(op2)).dataType)
    {
       throw 47;
    }
	
	string location,opCode,address,signNumber,comments;
    location=opCode=address=signNumber=comments ="";

    if(isTemp(curA) && (curA != op2 || curA != op1)) //if curA is a temp that isn't used in this subtraction
    {
		 //store the temp in curA into memory
		 opCode = "STA";
		address = curA;
		comments = "store "+curA+" into memeory";
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
		 
		  for(uint i =0;i<symbolTable.size();i++)
		   {
			 if(curA == symbolTable[i].internalName)
			  {
				 symbolTable[i].alloc = YES;
			  }
		   }
		//getEntry(curA).type = WhichType(curA);
		curA = "";
		
    }
	if(!(isTemp(curA)) && (curA != op2 && curA != op1)) 
	{
		curA = "";
	}
	if(curA != op2 && curA != op1)
	{
		 
		opCode = "LDA";
		address = op2;
		comments = "load "+ getExternalName(op2)+" into A-reg";
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
		curA = op2;
	}
	
	//do the math
   if(curA==op2)
   {
      opCode = "ISB";
      address = op1;
      comments = getExternalName(op2) + " = " + getExternalName(op1);
      obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
      comments = "";
   }
   else
   {
      opCode = "ISB";
      address = op2;
      comments = getExternalName(op2) + " = " + getExternalName(op1);
      obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
      comments = "";
   }

	//jump
	opCode = "AZJ";
	address = "L" + to_string(labelCount);
	comments = "";
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	signNumber = "";
	
	//load false
	if(getExistingConstant("FALS")!="")
    {
        address = getExistingConstant("FALS");
    }
	else if (!isDefined("FALS"))
	{
		Insert("FALS",BOOLEAN,CONSTANT,"0",YES,1);
        address = "FALS";
	}
	
	opCode = "LDA";
	comments = "";
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	signNumber = "";
	location = "";
	
	//unj
	opCode = "UNJ";
	address = "L" + to_string(labelCount);
	comments = "";
	signNumber = "+1";
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	signNumber = "";
	
	//load true
    if(getExistingConstant("TRUE")!="")
    {
        address = getExistingConstant("TRUE");
    }
	else if (!isDefined("TRUE"))
	{
		Insert("TRUE",BOOLEAN,CONSTANT,"1",YES,1);
        address = "TRUE";
	}
	
	opCode = "LDA";
	
	comments = "";
	location = "L" + to_string(labelCount);
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	signNumber = "";
	location = "";
	
	if(isTemp(op1))
	{
		FreeTemp();
	}
	if(isTemp(op2))
	{
		FreeTemp();
	}
	curA = GetTemp();
	  for(uint i =0;i<symbolTable.size();i++)
	   {
		 if(curA == symbolTable[i].internalName)
		  {
			 symbolTable[i].dataType = BOOLEAN;
		  }
	   }
	labelCount++;
	PushOperand(curA);
	
    /*
    if A-register holds a temp not op1 nor op2 then 
        deassign it
        emit code to store that temp into memory
        change the allocate entry for it in the symbol table to yes 
    if A-register holds a non-temp not op2 nor op1 
        then deassign it
    if neither operand is in A-register then
        emit code to load op2 into the A-register;
    emit code to perform A-register-memory subtraction with A-register holding the result;
    emit code to perform an AZJ to the next available Ln
    emit code to do A-register-memory load FALS 
    insert FALS in symbol table with value 0 and external name false
    emit code to perform a UNJ to the acquired label Ln + 1
    emit code to label the next instruction with the acquired label Ln and do A-register-memory load TRUE
    insert TRUE in symbol table with value 1 and external name true
    deassign all temporaries involved and free those names for reuse;
    A-register = next available temporary name and change type of its symbol table entry to boolean       
    push name of result onto operandStk;
    */
}
void EmitNotEqualsCode(string op1, string op2)
{
	string location,opCode,address,signNumber,comments;
    location=opCode=address=signNumber=comments ="";

    if (getEntry(getExternalName(op1)).dataType != getEntry(getExternalName(op2)).dataType)
    {
       throw 47;
    }
    if(isTemp(curA) && curA != op1 && curA != op2) //if curA is a temp that isn't used in this addition
    {
		//store the temp in curA into memory
		opCode = "STA";
		address = curA;
		comments = "store "+curA+" into memeory";
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
		 
		  for(uint i =0;i<symbolTable.size();i++)
		   {
			 if(curA == symbolTable[i].internalName)
			  {
				 symbolTable[i].alloc = YES;
			  }
		   }
		//getEntry(curA).type = WhichType(curA);
		curA = "";
		
    }
	if(!(isTemp(curA)) && curA != op1 && curA != op2) 
	{
		curA = "";
	}
	if(curA != op1 && curA != op2)
	{
		 
		opCode = "LDA";
		address = op2;
		comments = "load "+ getExternalName(op2)+" into A-reg";
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
		curA = op2;
	}
	
	//do the math
	if(curA!=op2) 	//op1 is in A-reg
	{
		opCode = "ISB";
		address = op2;
		comments = getExternalName(op2) + " <> " + getExternalName(op1);
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
	}
	else			//op2 is in A-reg
	{
		opCode = "ISB";
		address = op1;
		comments = getExternalName(op2) + " <> " + getExternalName(op1);
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
	}
	
	//jump
	opCode = "AZJ";
	address = "L" + to_string(labelCount);
	comments = "";
	signNumber = "+1";
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	signNumber = "";
	
	//load true
    if(getExistingConstant("TRUE")!="")
    {
        address = getExistingConstant("TRUE");
    }
	else if (!isDefined("TRUE"))
	{
		Insert("TRUE",BOOLEAN,CONSTANT,"1",YES,1);
        address = "TRUE";
	}
	opCode = "LDA";
	comments = "";
	location = "L" + to_string(labelCount);
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	signNumber = "";
	location = "";
	
	if(isTemp(op1))
	{
		FreeTemp();
	}
	if(isTemp(op2))
	{
		FreeTemp();
	}
	curA = GetTemp();
	for(uint i =0;i<symbolTable.size();i++)
	{
		if(curA == symbolTable[i].internalName)
	    {
	   	 symbolTable[i].dataType = BOOLEAN;
		}
	}
	labelCount++;
	PushOperand(curA);
}
void EmitLessThanCode(string op1, string op2)
{
	string location,opCode,address,signNumber,comments;
    location=opCode=address=signNumber=comments ="";

    if (getEntry(getExternalName(op1)).dataType != INTEGER || getEntry(getExternalName(op2)).dataType != INTEGER)
    {
       throw 49;
    }
    if(isTemp(curA) && curA != op2) //if curA is a temp that isn't used in this subtraction
    {
		 //store the temp in curA into memory
		 opCode = "STA";
		address = curA;
		comments = "store "+curA+" into memeory";
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
		 
		  for(uint i =0;i<symbolTable.size();i++)
		   {
			 if(curA == symbolTable[i].internalName)
			  {
				 symbolTable[i].alloc = YES;
			  }
		   }
		//getEntry(curA).type = WhichType(curA);
		curA = "";
		
    }
	if(!(isTemp(curA)) && curA != op2) 
	{
		curA = "";
	}
	if(curA != op2)
	{
		 
		opCode = "LDA";
		address = op2;
		comments = "load "+ getExternalName(op2)+" into A-reg";
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
		curA = op2;
	}
	
	//do the math

	opCode = "ISB";
	address = op1;
	comments = getExternalName(op2) + " < " + getExternalName(op1);
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	comments = "";

	//jump
	opCode = "AMJ";
	address = "L" + to_string(labelCount);
	comments = "";
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	signNumber = "";
	
	//load false
    if(getExistingConstant("FALS")!="")
    {
        address = getExistingConstant("FALS");
    }
	else if (!isDefined("FALS"))
	{
		Insert("FALS",BOOLEAN,CONSTANT,"0",YES,1);
        address = "FALS";
	}
	
	opCode = "LDA";
	comments = "";
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	signNumber = "";
	location = "";
	
	//unj
	opCode = "UNJ";
	address = "L" + to_string(labelCount);
	comments = "";
	signNumber = "+1";
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	signNumber = "";
	
	//load true
    if(getExistingConstant("TRUE")!="")
    {
        address = getExistingConstant("TRUE");
    }
	else if (!isDefined("TRUE"))
	{
		Insert("TRUE",BOOLEAN,CONSTANT,"1",YES,1);
        address = "TRUE";
	}
	
	opCode = "LDA";
	comments = "";
	location = "L" + to_string(labelCount);
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	signNumber = "";
	location = "";
	
	if(isTemp(op1))
	{
		FreeTemp();
	}
	if(isTemp(op2))
	{
		FreeTemp();
	}
	curA = GetTemp();
	  for(uint i =0;i<symbolTable.size();i++)
	   {
		 if(curA == symbolTable[i].internalName)
		  {
			 symbolTable[i].dataType = BOOLEAN;
		  }
	   }
	PushOperand(curA);
	labelCount++;
}
void EmitLessThanEqualToCode(string op1, string op2)
{
	string location,opCode,address,signNumber,comments;
    location=opCode=address=signNumber=comments ="";

    if (getEntry(getExternalName(op1)).dataType != INTEGER || getEntry(getExternalName(op2)).dataType != INTEGER)
    {
       throw 49;
    }
    if(isTemp(curA) && curA != op2) //if curA is a temp that isn't used in this subtraction
    {
		 //store the temp in curA into memory
		 opCode = "STA";
		address = curA;
		comments = "store "+curA+" into memeory";
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
		 
		  for(uint i =0;i<symbolTable.size();i++)
		   {
			 if(curA == symbolTable[i].internalName)
			  {
				 symbolTable[i].alloc = YES;
			  }
		   }
		//getEntry(curA).type = WhichType(curA);
		curA = "";
		
    }
	if(!(isTemp(curA)) && curA != op2) 
	{
		curA = "";
	}
	if(curA != op2)
	{
		 
		opCode = "LDA";
		address = op2;
		comments = "load "+ getExternalName(op2)+" into A-reg";
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
		curA = op2;
	}
	
	//do the math

	opCode = "ISB";
	address = op1;
	comments = getExternalName(op2) + " <= " + getExternalName(op1);
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	comments = "";

	//jump
	opCode = "AZJ";
	address = "L" + to_string(labelCount);
	comments = "";
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	signNumber = "";
	
	opCode = "AMJ";
	address = "L" + to_string(labelCount);
	comments = "";
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	signNumber = "";
	
	//load true
    if(getExistingConstant("FALS")!="")
    {
        address = getExistingConstant("FALS");
    }
	else if (!isDefined("FALS"))
	{
		Insert("FALS",BOOLEAN,CONSTANT,"0",YES,1);
        address = "FALS";
	}
	
	opCode = "LDA";
	comments = "";
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	signNumber = "";
	location = "";
	
	//unj
	opCode = "UNJ";
	address = "L" + to_string(labelCount);
	comments = "";
	signNumber = "+1";
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	signNumber = "";
	
	//load true
    if(getExistingConstant("TRUE")!="")
    {
        address = getExistingConstant("TRUE");
    }
	else if (!isDefined("TRUE"))
	{
		Insert("TRUE",BOOLEAN,CONSTANT,"1",YES,1);
        address = "TRUE";
	}
	
	opCode = "LDA";
	
	comments = "";
	location = "L" + to_string(labelCount);
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	signNumber = "";
	location = "";
	
	if(isTemp(op1))
	{
		FreeTemp();
	}
	if(isTemp(op2))
	{
		FreeTemp();
	}
	curA = GetTemp();
	  for(uint i =0;i<symbolTable.size();i++)
	   {
		 if(curA == symbolTable[i].internalName)
		  {
			 symbolTable[i].dataType = BOOLEAN;
		  }
	   }
	labelCount++;
	PushOperand(curA);
}
void EmitGreaterThanCode(string op1, string op2)
{
	string location,opCode,address,signNumber,comments;
    location=opCode=address=signNumber=comments ="";

    if (getEntry(getExternalName(op1)).dataType != INTEGER || getEntry(getExternalName(op2)).dataType != INTEGER)
    {
       throw 49;
    }
    if(isTemp(curA) && curA != op2) //if curA is a temp that isn't used in this subtraction
    {
		 //store the temp in curA into memory
		 opCode = "STA";
		address = curA;
		comments = "store "+curA+" into memeory";
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
		 
		  for(uint i =0;i<symbolTable.size();i++)
		   {
			 if(curA == symbolTable[i].internalName)
			  {
				 symbolTable[i].alloc = YES;
			  }
		   }
		//getEntry(curA).type = WhichType(curA);
		curA = "";
		
    }
	if(!(isTemp(curA)) && curA != op2) 
	{
		curA = "";
	}
	if(curA != op2)
	{
		 
		opCode = "LDA";
		address = op2;
		comments = "load "+ getExternalName(op2)+" into A-reg";
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
		curA = op2;
	}
	
	//do the math

	opCode = "ISB";
	address = op1;
	comments = getExternalName(op2) + " > " + getExternalName(op1);
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	comments = "";

	//jump
	opCode = "AZJ";
	address = "L" + to_string(labelCount);
	comments = "";
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	signNumber = "";
	
	opCode = "AMJ";
	address = "L" + to_string(labelCount);
	comments = "";
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	signNumber = "";
	
	//load true
    if(getExistingConstant("TRUE")!="")
    {
        address = getExistingConstant("TRUE");
    }
	else if (!isDefined("TRUE"))
	{
		Insert("TRUE",BOOLEAN,CONSTANT,"1",YES,1);
        address = "TRUE";
	}
	
	opCode = "LDA";
	
	comments = "";
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	signNumber = "";
	location = "";
	
	//unj
	opCode = "UNJ";
	address = "L" + to_string(labelCount);
	comments = "";
	signNumber = "+1";
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	signNumber = "";
	
	//load false
    if(getExistingConstant("FALS")!="")
    {
        address = getExistingConstant("FALS");
    }
	else if (!isDefined("FALS"))
	{
		Insert("FALS",BOOLEAN,CONSTANT,"0",YES,1);
        address = "FALS";
	}
	
	opCode = "LDA";
	comments = "";
	location = "L" + to_string(labelCount);
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	signNumber = "";
	location = "";
	
	if(isTemp(op1))
	{
		FreeTemp();
	}
	if(isTemp(op2))
	{
		FreeTemp();
	}
	curA = GetTemp();
	  for(uint i =0;i<symbolTable.size();i++)
	   {
		 if(curA == symbolTable[i].internalName)
		  {
			 symbolTable[i].dataType = BOOLEAN;
		  }
	   }
	PushOperand(curA);
	labelCount++;
}
void EmitGreaterThanEqualToCode(string op1, string op2)
{
	string location,opCode,address,signNumber,comments;
    location=opCode=address=signNumber=comments ="";

    if (getEntry(getExternalName(op1)).dataType != INTEGER || getEntry(getExternalName(op2)).dataType != INTEGER)
    {
       throw 49;
    }
    if(isTemp(curA) && curA != op2) //if curA is a temp that isn't op2
    {
		 //store the temp in curA into memory
		 opCode = "STA";
		address = curA;
		comments = "store "+curA+" into memeory";
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
		 
		  for(uint i =0;i<symbolTable.size();i++)
		   {
			 if(curA == symbolTable[i].internalName)
			  {
				 symbolTable[i].alloc = YES;
			  }
		   }
		//getEntry(curA).type = WhichType(curA);
		curA = "";
		
    }
	if(!(isTemp(curA)) && curA != op2) 
	{
		curA = "";
	}
	if(curA != op2)
	{
		 
		opCode = "LDA";
		address = op2;
		comments = "load "+ getExternalName(op2)+" into A-reg";
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
		curA = op2;
	}
	
	//do the math

	opCode = "ISB";
	address = op1;
	comments = getExternalName(op2) + " >= " + getExternalName(op1);
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	comments = "";

	//jump
	opCode = "AMJ";
	address = "L" + to_string(labelCount);
	comments = "";
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	signNumber = "";
	
	//load true
    if(getExistingConstant("TRUE")!="")
    {
        address = getExistingConstant("TRUE");
    }
	else if (!isDefined("TRUE"))
	{
		Insert("TRUE",BOOLEAN,CONSTANT,"1",YES,1);
        address = "TRUE";
	}
	
	opCode = "LDA";
	
	comments = "";
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	signNumber = "";
	location = "";
	
	//unj
	opCode = "UNJ";
	address = "L" + to_string(labelCount);
	comments = "";
	signNumber = "+1";
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	signNumber = "";
	
	//load false
    if(getExistingConstant("FALS")!="")
    {
        address = getExistingConstant("FALS");
    }
	else if (!isDefined("FALS"))
	{
		Insert("FALS",BOOLEAN,CONSTANT,"0",YES,1);
        address = "FALS";
	}
	
	opCode = "LDA";
	comments = "";
	location = "L" + to_string(labelCount);
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	signNumber = "";
	location = "";
	
	if(isTemp(op1))
	{
		FreeTemp();
	}
	if(isTemp(op2))
	{
		FreeTemp();
	}
	curA = GetTemp();
	  for(uint i =0;i<symbolTable.size();i++)
	   {
		 if(curA == symbolTable[i].internalName)
		  {
			 symbolTable[i].dataType = BOOLEAN;
		  }
	   }
	PushOperand(curA);
	labelCount++;
}

void EmitAssignCode(string op1, string op2)
{
	string location,opCode,address,signNumber,comments;
    location=opCode=address=signNumber=comments ="";
	//op1 & op2 must be of the same type
	if (getEntry(getExternalName(op1)).dataType != getEntry(getExternalName(op2)).dataType)
    {
       throw 47;
    }
	
	if(getEntry(getExternalName(op2)).mode != VARIABLE)
	{
		throw 46;
	}
	
    // if the address are already equal, emit not code
	if(op1 == op2)
	{
		return;
	}
	
    //if op1 is not in curA
	if(op1 != curA)
	{
        //LDA op1
		opCode = "LDA";
		address = op1;
		comments = "load "+ getExternalName(op1)+" into A-reg";
		obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
		comments = "";
	}
	
    //STA op2
	opCode = "STA";
	address = op2;
	comments = getExternalName(op2) +" := "+getExternalName(op1);
	obj<<setw(4)<<left<<location<<right<<setw(5)<<opCode<<" "<<setw(4)<<left<<address<<right<<setw(2)<<signNumber<<setw(3)<<""<<comments<<endl;
	comments = "";
	
	//if op1 was a temp, free it
	if(isTemp(op1))
	{
		FreeTemp();
	}
    
    //set current A-reg to op2
    curA = op2;
}

void PushOperator(string name)
{
	operatorStack.push(name);
}
void PushOperand(string name)
{
    string internalName ="";
    //if the name is not defined already and we have and existing constant
    
    //if we are pushing a boolean literal or 0
    //change the symbol to match the symbol used in Emits to prevent redundancy
	if(name == "true")
	{
		name = "TRUE";
	}
	else if(name == "false")
	{
		name = "FALS";
	}
    else if(name == "0")
    {
        
        name = "ZERO";
    }
    
    //if the symbol is already defined
    if(isDefined(name))
    {
        internalName = getEntry(name).internalName;
    }
    //if there is an existing constant with the same type and value
    else if(getExistingConstant(name)!="")
    {
       internalName = getExistingConstant(name);
    }
    //if we need to add an integer constant (not 0)
	else if(isIntegerLit(name))
	{
		Insert(name,INTEGER,CONSTANT,WhichValue(name),YES,1);
		internalName = getEntry(name).internalName;
	}
    //if we need to add a boolean constant
	else if(name =="TRUE"||name =="FALS")
	{
		Insert(name,BOOLEAN,CONSTANT,name=="TRUE"?"1":"0",YES,1);
        internalName = getEntry(name).internalName;
	}
    //if we need to add ZERO
    else if(name =="ZERO")
    {
        Insert(name,INTEGER,CONSTANT,"0",YES,1);
        internalName = getEntry(name).internalName;
    }
    else
    {
        throw 40; 
    }
    
	operandStack.push(internalName);
}

string PopOperator()//returns the element on top on the stack and removes it
{
	if(operatorStack.empty())
	{
		throw 44;
	}
	string s = operatorStack.top();
	operatorStack.pop();
	return s;
}

string PopOperand() //returns the element on top on the stack and removes it
{
	if(operandStack.empty())
	{
		throw 43;
	}
	string s = operandStack.top();
	operandStack.pop();
	return s;
}

void FreeTemp()
{
	currentTempNo -= 1;
	if (currentTempNo < -1)
	{
		throw 45;
	}
}
string GetTemp()
{
	string temp;
	currentTempNo += 1;
	temp = "T"+ to_string(currentTempNo);
	if (currentTempNo > maxTempNo)
	{
		Insert(temp, UNKNOWN, VARIABLE ,"",NO,1);
		maxTempNo += 1;
	}
	
	return temp;
}


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
      //if we are inserting a Temp, true, false or ZERO make the internal and external name the same
	  if(inType == UNKNOWN||name =="TRUE"||name =="FALS"||name =="ZERO")
	  {
		 entry e = entry(name,name,inType,inMode,inValue,inAlloc,inUnits);
         symbolTable.push_back(e);
	  }
      else
      {
         //generate the internal name based on the number of that type in the symbol table
         entry e = entry(GenInternalName(inType),name,inType,inMode,inValue,inAlloc,inUnits);
         symbolTable.push_back(e);
      }
   }
   
}

storeType WhichType(string name) // tells which data type a name has
{
   storeType type;
   //if name is a literal
   if(isBooleanLit(name)) // if the string equals true or false then bool
   {
      type = BOOLEAN;
   }
   else if(isIntegerLit(name)) 
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
   if(isIntegerLit(name))
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
	default:
		break;
   }
   return name;
}

//_________________________________________

bool isIntegerLit(string s) //
{
   return isDigit(s[0])||(s[0]=='-'&&isDigit(s[1]))||(s[0]=='+'&&isDigit(s[1]));
}

bool isBooleanLit(string s)
{
	return (s == "true" || s == "false");
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

entry getEntry(string name) //input externalname
{
   if(!isDefined(name))
   {
	   //cout<<"getEntry "<<name<<endl;
      throw 16;
   }
   for(uint i =0;i<symbolTable.size();i++)
   {
     if(name == symbolTable[i].externalName)
      {
         return symbolTable[i];
      }
   }
   return symbolTable[0]; // this should never be reached but needs a return stmt
}
string getExternalName(string name)
{
	string ext = "";
   for(uint i =0;i<symbolTable.size();i++)
   {
     if(name == symbolTable[i].internalName)
      {
         ext = symbolTable[i].externalName;
      }
   }
   return ext;
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
bool isNonKeyID(string name)  //determines if it is a valid name and if it is a keyword
{
   return (!isKeyword(name)&&!isSpecialChar(name));
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
   obj<<setw(28)<<left<<"STAGE0: James Dixon & Fisher Williams "<< ctime (&rawtime)<<endl;
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
		default:
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
				//Error: unexpected end of file
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
		else if(isSpecialChar(string(1,charac)))
		{
			token = string(1,charac);			//set token to charac
            NextChar();				//grab next char in charac
            if(token == ":")
            {
                if(charac == '=')
                {
                    token = ":=";
                    NextChar();
                }
            }
            else if(token == "<")
            {
                if(charac == '=')
                {
                    token = "<=";
                    NextChar();
                }
                else if (charac == '>')
                {
                    token = "<>";
                    NextChar();
                }
            }
            else if(token == ">")
            {
                if(charac == '=')
                {
                    token = ">=";
                    NextChar();
                }
            }
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

bool isSpecialChar(string c)
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

string getExistingConstant(string value) //return the internal name of an existing constant with the same value and type
{
    
    if(value == "TRUE")     
    {
        for(uint i =0;i<symbolTable.size();i++)
        {
          
         if(symbolTable[i].mode == CONSTANT && symbolTable[i].dataType == BOOLEAN && symbolTable[i].value == "1")
          {
             return symbolTable[i].internalName;
          }
        }

    }
    else if(value == "FALS")
    {
        for(uint i =0;i<symbolTable.size();i++)
        {
          
         if(symbolTable[i].mode == CONSTANT && symbolTable[i].dataType == BOOLEAN && symbolTable[i].value == "0")
          {
             return symbolTable[i].internalName;
          }
        }
    }
    else if(value == "ZERO")
    {
        for(uint i =0;i<symbolTable.size();i++)
        {
          
         if(symbolTable[i].mode == CONSTANT && symbolTable[i].dataType == INTEGER && symbolTable[i].value == "0")
          {
             return symbolTable[i].internalName;
          }
        }
    }
    for(uint i =0;i<symbolTable.size();i++)
    {
      
     if(symbolTable[i].mode == CONSTANT && symbolTable[i].dataType == INTEGER && symbolTable[i].value == value)
      {
         return symbolTable[i].internalName;
      }
    }
    return "";
}

bool isTemp(string name)    //checks to see if the name is a temp
{
    return name[0]=='T'&&isDigit(name[1]);
}

void PrintStks()            //prints the contents of operand and operator stacks for debugging
{
	stack<string> temp;
	cout<<"Operand"<<endl;
	while(!operandStack.empty())
	{
		cout<<operandStack.top()<<endl;
		temp.push(operandStack.top());
		operandStack.pop();
	}
	while(!temp.empty())
	{
		operandStack.push(temp.top());
		temp.pop();
	}
		cout<<"Operator"<<endl;
	while(!operatorStack.empty())
	{
		cout<<operatorStack.top()<<endl;
		temp.push(operatorStack.top());
		operatorStack.pop();
	}
	while(!temp.empty())
	{
		operatorStack.push(temp.top());
		temp.pop();
	}
	cout<<"_____________________"<<endl;
}