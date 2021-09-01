#include <iostream>
#include <fstream> 
#include <cstdio> // fscanf
#include <string.h>
#include <bitset>
#include <stdlib.h> 
#include <stdio.h> 
#include <sstream>
#include <vector>
#include <stack>
#include <queue>
using namespace std ;

struct Line{ // �C�@�� 
	bool error ; // �Y��k���~ �h��true 
	string errormsg ; // ���~�T�� 
	string str ; // ���statement
	int line ; // �b�ĴX�� (��X�ݭn)  
	int entry7 ; // ���Vtable7���ĴX��entry  (CALL�������i��|�Ψ� �]���i�HCALL �P��Fn�ܦh��) 
	bool assignment ;
}; 
struct Table6{
	int n1 ;
	int n2 ;
	int n3 ;
	int n4 ;
	int n5 ;
	int n6 ;
	int n7 ;
	int n8 ;
	string str ; 
};
struct Table5{
	string str ; // identifier
	int subroutine ;
	string type ;
	int pointerTable ; // ���V�ĴX��TABLE 
	int pointerIndex ; // ���V��table���ĴX��(table�ҥ�1��_) �ثe�S�� 
};
struct Label{
	string str ;
	int line ; // �btable6���ĴX�� 
};
struct Token{ // �C�@��Token����T 	
	string str ;
	int tableType ;
	int tableValue ;
	int level ; // �M����delimeter�Ӥ�j�p�Ϊ� 
	bool isArrayOne ; // �O�_���@���}�C 
	bool isArrayTwo ; // �O�_���G���}�C 
	int pointAT ; // ���VarrayTable���Ӱ}�C��entry 
};
struct ArrayTable{
	Token array ; // �}�C
	Token index1 ; // ��index 
	Token index2 ;  // �ĤG��index (�����G���}�C) 
	bool two_dimention ; // �O�_���G���}�C 
};
struct ERROR{
	string errormsg ;
	int line = -1 ;
};
static string fileName ; // �ɮצW�� 
static vector<Token> token ; // ���n��token�� vector 
static vector<Line> all; // �h�P�_Syntax��vector
static vector<string> variable ; // �ΨӧP�_���L"�|���w�q"���ܼ� 
static vector<string> label ; // LABEL&GTO �P�_���Lundefined symbol
static vector<int> table7 ; // imformation table 
static vector<Table6> table6 ; // �����X 
static vector<Label> gto ; // ��GTO��Label�btable6�����ĴX�� 
static vector<Token> table0 ; // ��Ȯ��ܼ�T1....
static vector<Token> condi ; // ��condition &statement
static stack<Token> op1 ; 
static stack<Token> op2 ; 
static Table5 table5[1000] ; // identifier table
static vector <ArrayTable> arrayTable ; // �ȦsARRAY��T 
static string table3[100] ; // integer table
//static vector<Token> labelTable ; // �w��address��Label 
static vector<Label> labelTable ; // �w��address��Label 
static queue<ERROR> err ;
// =================================================================================================================
class Syntax{ // start �T�{��k�O�_���T  =============================================================================================================
public:	
	void GetLine() ; // �C�����@��h�P�_��k 
	void SyntaxAnalysis(int index, int startLine, int endLine) ; // // �Ǧ���Ҧb(all��)��index �Φ���Ĥ@��/�̫�@��token�Ҧbtoken��(vector)����} 
    bool IsDataType ( string str ) ; // �P�_str�O�_��'type'�pARRAY, BOOLEAN...�� 
	void Convert2BigString( string in , string &out ) ; // �ন�j�g 
	int CheckDataType( string dataType) ; // �^�Ǩ�dataType�s�� 
	void GenerateIntermediate() ; // ���ͤ����X 
	bool CheckCondition( ) ; // �P�_condi�O�_���T 
	bool CheckStatement( int index ) ; // �P�_statemet�O�_���T 		
	void GenerateCode( Token operator1, Token operand1, Token operand2 ) ; // ���ͫ��woperand/operator�������X 
	void GenerateCondition() ; // ����condi�������X 	
	void GenerateCALL( int statementStart, int a ) ; // ����CALL�������X 
	void GenerateGTO( int statementStart ) ; // ����GTO�������X 
	void GenerateAssignment( int statementStart ) ; // ����assignment�������X i���]token��index 
	void CheckLevel( Token &t ) ; // �P�_delimeter���j�p �|�s�Jlevel�� �Ʀr�V�j�N���ŶV��(�V�j) 
	void GenerateStatement( int statementStart, int statementEnd, int a, bool &isGTO ) ; // ����statement�������X 
	void GenerateArray(Token operator1, Token operand1, Token operand2 ) ; // ���ͦ���array��assignment�������X 
	void WriteOutput ( ) ;
};

void Syntax::GetLine() { // ���@��h�P�_��k 
	int i = 0 ; // �]token�� 
	int index = 0 ; // �]all�� 
	int startLine = 0 ; // �O�����檺�Ĥ@��token�Ҧb(token����)��} 
	int endLine = 0 ; // �O�����檺�̫�@��token(�]�N�O����)�Ҧb(token����)��} 
	bool hasSemicolon = false ; // �O�_�J����� 
	Line temp ;
	
	while ( i < token.size() ) {
		temp.str = temp.str + token.at(i).str ;
		if ( token.at(i).str == "\n" ) { // �J�짹��@�檺�̫�@token 
			all.push_back(temp) ;
			if ( hasSemicolon == false ) { // ���statement�S������ 
				all.at(index).error = true ;
				all.at(index).errormsg = "�̫�r���D�������O ';'" ;
			} // if 
			else { // �O����statement �n�i�J��L��k�P�_ 
				endLine = i ;
				SyntaxAnalysis(index, startLine, endLine) ; // �Ǧ���Ҧb(all��)��index �Φ���Ĥ@��/�̫�@��token�Ҧbtoken��(vector)����} 
			} // else 
			hasSemicolon = false ; // init
			index++ ; // ���U�@���o 
			temp.str = "" ; // init
			startLine = i + 1 ; // �U�@��Ĥ@��Token�Ҧbtoken(vector)������} 
		} // if 
		else if ( token.at(i).str == ";" ) { // �J����� (�S������������error) 
			hasSemicolon = true ;
		} // else if 
		
		i++ ;
	} // while 
} // end GetLine()���@��h�P�_��k 
void Syntax::SyntaxAnalysis(int index, int startLine, int endLine ) { 
// index����statement�Ҧball(vector)������} 
// startLine �� �o���Ĥ@��token�btoken(vector)������}
// endLine���̫�@�� �]�N�O"����"�Ҧb��} 
	bool comma = false ; // �P�_�O�_�J��r�� 
    int i = startLine ;
    string dataType = "" ; // �sdataType  
    int countDimention = 1 ; // ��array�O�X���� 
    int countArray = 1 ; // �p�⦹�榳�h�֭�array 
    all.at(index).error = false ; // init
    all.at(index).assignment = false ; // init ����assignment 
    while ( i < endLine ) { // endLine ������Ҧb��} 
    	while ( token.at(i).str == " " || token.at(i).str == "\t" ) {
    		i++ ;
		} // while 
    	if ( token.at(i).str == ";" ) { // �ˬd�����᭱�O�_���F�� 
    		i++ ;
    		while ( token.at(i).str == " " ) { // �ťո��� 
				i++ ;
			} // while
		
    		if ( token.at(i).str != "\n" ) {
				all.at(index).error = true ;
				all.at(index).errormsg = "';'�᭱�������F��" ;
				break ;
			} // if
			break ;
    	} // if 
    	else if ( token.at(i).tableType == 5 ) {
    		int q = 0 ;
    		bool isLab = false ;
    		while ( q < label.size() ) {
    			if ( label[q] == token.at(i).str ) {
    				isLab = true ;
				} // if 
				q++ ;
			} // while 
			if ( isLab ) {
				int tempForI = i ;
	    		i++ ;
	    		while( token.at(i).str == " " ) {
	    			i++ ;
				} // while 
			} // if 
    		
		} // else if 
    	if ( token.at(i).tableType == 2 ) { // reserve word table
    		 
			if ( token.at(i).tableValue == 21 ) { // PROGRAM------------------------------------------(PROGRAM)
			// PROGRAM<identifier>;
				i++ ;
				while ( token.at(i).str == " " ) { // �ťո��� 
					i++ ;
				} // while
		
				if ( token.at(i).tableType != 5 ) { // �᭱�Did 
					all.at(index).error = true ;
					all.at(index).errormsg = "PROGRAM�᭱�Didentifier" ;
					break ;
				} // if 
				i++ ;
				while ( token.at(i).str == " " ) { // �ťո��� 
					i++ ;
				} // while
				if ( token.at(i).str != ";" ) {
					all.at(index).error = true ;
					all.at(index).errormsg = "PROGRAM�᭱���~(���Ӧh�F��)" ;
					break ; 
				} // if 
				i++ ;
				while ( token.at(i).str == " " ) { // �ťո��� 
					i++ ;
				} // while
				if ( token.at(i).str != "\n" ) {
					all.at(index).error = true ;
					all.at(index).errormsg = "';'�᭱�������F��" ;
					break ;
				} // if 
				break ;
			} // if (PROGRAM)-------------------------------------------------------------------------(PROGRAM)
			else if (token.at(i).tableValue == 23 ) { // SUBROUTINE
			// SUBROUTINE<identifier>(<parameter group>{,<parameter group>});
				i++ ;
				while ( token.at(i).str == " " ) { // �ťո��� 
					i++ ;
				} // while
				if ( token.at(i).tableType != 5 ) { // �᭱�Did 
					all.at(index).error = true ;
					all.at(index).errormsg = "SUBROUTINE�᭱�Didentifier";
					break ;
				} // if 
				i++ ;
				while ( token.at(i).str == " " ) { // �ťո��� 
					i++ ;
				} // while
				if (token.at(i).str != "(" ) {
					all.at(index).error = true ;
					all.at(index).errormsg = "SUBROUTINE<identifier>�᭱����'('" ;
					break ;
				} // if 
				i++ ;
				while ( token.at(i).str == " " ) { // �ťո��� 
					i++ ;
				} // while
				if ( !IsDataType(token.at(i).str) ) { // <parameter group>:=<type>:<parameter>{,<parameter>}
					all.at(index).error = true ;
					all.at(index).errormsg = "SUBROUTINE<identifier>( �᭱����'type'" ;
					break ;
				} // if 
				i++ ;
				while ( token.at(i).str == " " ) { // �ťո��� 
					i++ ;
				} // while
				if ( token.at(i).str != ":" ) {
					all.at(index).error = true ;
					all.at(index).errormsg = "SUBROUTINE<identifier>(<type> ������':'" ;
					break ;
				} // if 
				i++ ;
				while ( token.at(i).str == " " ) { // �ťո��� 
					i++ ;
				} // while
				if ( token.at(i).tableType != 5 ) { // �Did �h�P�_�@�� �קK�_��:�᭱�����O����; 
		        	all.at(index).error = true ;
					all.at(index).errormsg = "SUBROTINE��<parameter group>����<parameter>���~" ;
					break ;
				} // if 
				while ( token.at(i).str != ";" ) { // �P�_id,id,id,id... ;
					while ( token.at(i).str == " " ) { // �ťո��� 
						i++ ;
					} // while
			        if ( token.at(i).tableType != 5 ) { // �Did 
			        	all.at(index).error = true ;
						all.at(index).errormsg = "SUBROTINE��<parameter group>����<parameter>���~" ;
						break ;
					} // if 
					else { // ��id 
					    comma = false ; // init
						i++ ;
						while ( token.at(i).str == " " ) { // �ťո��� 
							i++ ;
						} // while
						if ( token.at(i).str == ")" ) {
							i++ ;
							while ( token.at(i).str == " " ) { // �ťո��� 
								i++ ;
							} // while
							if ( token.at(i).str != ";") {
								all.at(index).error = true ;
								all.at(index).errormsg = "SUBROTINE��')'�᭱����';'" ;
								break ;
							} // if 
							else {
								i++ ;
								while ( token.at(i).str == " " ) { // �ťո��� 
									i++ ;
								} // while
								if ( token.at(i).str != "\n" ) {
									all.at(index).error = true ;
									all.at(index).errormsg = "';'�᭱�������F��" ;
									break ;
								} // if 
								break ;
							} // else 
						} // if 
						else if ( token.at(i).str == "," ) {
							comma = true ;
							i++ ;
						} // else if 	 					
						else {
							all.at(index).error = true ;
							all.at(index).errormsg = "SUBROTINE��<parameter group>���A<parameter>������','��')'" ;
							break ;
						} // else 
						if ( comma == true && token.at(i).str == ";" ) { // �r���᭱�����J����� 
							all.at(index).error = true ;
							all.at(index).errormsg = "SUBROTINE���r���᭱����������'" ;
							break ;
						} //  if
					} // else 
				} // while 
				
				// �P�_����k �N�Ҧ�variable��Jvariable��vector��(���e���Ҧ�variable�n�M��)
				variable.clear() ;
				i = startLine ;
				while ( token.at(i).str != ":" ) { 
					i++ ;
				} // while  
				i++ ; // id �� " " 
				while ( token.at(i).str != ";" ) {
					while ( token.at(i).str == " " || token.at(i).str == ","|| token.at(i).str == ")" ) {
						i++ ;	
					} // while 
					if ( token.at(i).str == ";" ) break ;
					variable.push_back(token.at(i).str) ;
					i++ ;
				}  // while 
				break ;				
			} // else if (SUBROUTINE)-------------------------------------------------------------------------(SUBROUTINE)
			else if (token.at(i).tableValue == 25 ) { // VARIABLE
			// VARIABLE <type>:<identifier>{,<identifier>}
				i++ ;
				while ( token.at(i).str == " " ) { // �ťո��� 
					i++ ;
				} // while
				if ( !IsDataType(token.at(i).str) ) { //<type>
					all.at(index).error = true ;
					all.at(index).errormsg = "VARIABLE �᭱����'type'" ;
					break ;
				} // if 
				i++ ;
				while ( token.at(i).str == " " ) { // �ťո��� 
					i++ ;
				} // while
				if ( token.at(i).str != ":" ) { // :
					all.at(index).error = true ;
					all.at(index).errormsg = "VARIABLE<type> �᭱����':'" ;
					break ;
				} // if 
				i++ ;
				while ( token.at(i).str == " " ) { // �ťո��� 
					i++ ;
				} // while
				if ( token.at(i).tableType != 5 ) { // �Did �h�P�_�@�� �קK�_��:�᭱�����O����; 
			        	all.at(index).error = true ;
						all.at(index).errormsg = "VARIABLE<type>:�᭱����<identifier>" ;
						break ;
					} // if 
				while ( token.at(i).str != ";" ) { // �P�_id,id,id,id... ;
					while ( token.at(i).str == " " ) { // �ťո��� 
						i++ ;
					} // while
			        if ( token.at(i).tableType != 5 ) { // �Did 
			        	all.at(index).error = true ;
						all.at(index).errormsg = "VARIABLE<type>:�᭱����<identifier>" ;
						break ;
					} // if 
					else { // ��id 
						comma = false ; // init
						i++ ;
						while ( token.at(i).str == " " ) { // �ťո��� 
							i++ ;
						} // while
						if ( token.at(i).str == "," ) {
							comma = true ;
							i++ ;
						} // if 
						else if (token.at(i).str == ";" ) { // �J������ˬd�᭱���L�F�� 
							i++ ;
							while ( token.at(i).str == " " ) { // �ťո��� 
								i++ ;
							} // while
							if ( token.at(i).str != "\n" ) {
								all.at(index).error = true ;
								all.at(index).errormsg = "';'�᭱�������F��"; 
								break ;
							} // if 
							break ;
						} // else if 			
						else {
							all.at(index).error = true ;
							all.at(index).errormsg = "VARIABLE<type>:<identifier>������','��';'" ;
							break ;
						} // else 
						if ( comma == true && token.at(i).str == ";" ) { // �r���᭱�����J����� 
							all.at(index).error = true ;
							all.at(index).errormsg = "VARIABLE<type>:<identifier>,������'<identifier>'" ;
							break ;
						} //  if						 
					} // else 
				} // while 
				// �P�_��VARIABLE��(�T�{��k���T) �N�ܼƩ�Jvariable�o��vactor��
				i = startLine ;
				while ( i < endLine ) {
					while ( token.at(i).str != ":" ) {
						i++ ;
					} // while 
					i++ ;  // id �� " "
					while ( token.at(i).str != ";" ) { // ���ܼƩ|����Jvector�� 
						while ( token.at(i).str == " " || token.at(i).str == "\t" || token.at(i).str == "," ) {
						    i++ ;
						} // while  
						if ( token.at(i).str == ";" ) break ;
						variable.push_back(token.at(i).str ) ;
						i++ ;
					} // while 
					break ; 
				} // while 
				break ;
			} // else if (VARIABLE)-------------------------------------------------------------------------(VARIABLE)
			else if (token.at(i).tableValue == 15 ) { // LABEL
			// LABEL<label>{,<label>};
			// <label>:=<identifier>
				i++ ;
				while ( token.at(i).str == " " ) { // �ťո��� 
					i++ ;
				} // while
				if ( token.at(i).tableType != 5 ) { // �Did �h�P�_�@�� �קKLABEL�᭱�����O����; 
		        	all.at(index).error = true ;
					all.at(index).errormsg = "LABEL�᭱����<identifier>" ;
					break ;
				} // if 
				while ( token.at(i).str != ";" ) { // �P�_id,id,id,id... ;
					while ( token.at(i).str == " " ) { // �ťո��� 
						i++ ;
					} // while
			        if ( token.at(i).tableType != 5 ) { // �Did 
			        	all.at(index).error = true ;
						all.at(index).errormsg = "LABEL�᭱����<identifier>" ;
						break ;
					} // if 
					else { // ��id 
						comma = false ; // init
						i++ ;
						while ( token.at(i).str == " " ) { // �ťո��� 
							i++ ;
						} // while
						if ( token.at(i).str == "," ) {
							comma = true ;
							i++ ;
						} // if 
						else if (token.at(i).str == ";" ) { // �J������ˬd�᭱���L�F�� 
							i++ ;
							while ( token.at(i).str == " " ) { // �ťո��� 
								i++ ;
							} // while
							if ( token.at(i).str != "\n" ) {
								all.at(index).error = true ;
								all.at(index).errormsg = "';'�᭱�������F��" ;
								break ;
							} // if 
							break ;
						} // else if 			
						else {
							all.at(index).error = true ;
							all.at(index).errormsg = "LABEL<identifier>������','��';'" ;
							break ;
						} // else 
						if ( comma == true && token.at(i).str == ";" ) { // �r���᭱�����J����� 
							all.at(index).error = true ;
							all.at(index).errormsg = "LABEL<identifier>,������'<identifier>'" ;
							break ;
						} //  if 
					} // else 
				} // while 
				// �P�_����k �N�Ҧ�label��Jlabel��vector��
				i = startLine ;
				while ( token.at(i).tableValue != 15 ) { // LABEL
					i++ ;
				} // while  
				i++ ; // label �� " " 
				while ( token.at(i).str != ";" ) {
					while ( token.at(i).str == " "|| token.at(i).str == "\t" || token.at(i).str == ",") {
						i++ ;	
					} // while 
					if ( token.at(i).str == ";" ) break ;
					label.push_back(token.at(i).str) ;
					i++ ;
				}  // while 
				break ;
			} // else if (LABEL)-------------------------------------------------------------------------(LABEL)
			else if (token.at(i).tableValue == 4 ) { // DIMENSION
				i++ ;
				while ( token.at(i).str == " " ) { // �ťո��� 
					i++ ;
				} // while
				if ( !IsDataType(token.at(i).str) ) { // <parameter group>:=<type>:<parameter>{,<parameter>}
					all.at(index).error = true ;
					all.at(index).errormsg = "DIMENSION�᭱����'type'" ;
					break ;
				} // if 
				dataType = token.at(i).str ; // �Ȧs ��table7�ɷ|�Ψ� 
				i++ ;
				while ( token.at(i).str == " " ) { // �ťո��� 
					i++ ;
				} // while
				if ( token.at(i).str != ":" ) {
					all.at(index).error = true ;
					all.at(index).errormsg = "DIMENSION<type> �᭱����':'" ;
					break ;
				} // if 
				i++ ;
				while ( token.at(i).str == " " ) { // �ťո��� 
					i++ ;
				} // while
				if ( token.at(i).tableType != 5 ) { // �Did
		        	all.at(index).error = true ;
					all.at(index).errormsg = "DIMENSION<type>: �᭱����<identifier>";
					break ;
				} // if 	
				string arrayName = token.at(i).str ;	
				while ( token.at(i).str != "\n" ) { // �P�_A(), B() ;
					while ( token.at(i).str == " " ) { // �����ť� 
						i++ ;
					} // while
					if ( token.at(i).tableType != 5 ) { // �Did
			        	all.at(index).error = true ;
						all.at(index).errormsg = "DIMENSION<type>: �᭱����<identifier>";
						break ;
					} // if 
					arrayName = token.at(i).str ;
					i++ ;
					while ( token.at(i).str == " " ) { // �����ť� 
						i++ ;
					} // while 
					if ( token.at(i).str != "(" ) {
						all.at(index).error = true ;
						all.at(index).errormsg = "DIMENSION<type>:<identifier> �᭱����'('" ;
						break ;
					} // if 
					else { // �� ( �P�_�A���̭��F�観�L���T 
					    comma = false ; // init
						i++ ;
						while ( token.at(i).str == " " ) { // �ťո��� 
							i++ ;
						} // while
						if ( token.at(i).tableType != 3 ) { // �Dinteger �h�P�_�@�� �קK�A��(�᭱�����O����; 
				        	all.at(index).error = true ;
							all.at(index).errormsg = "DIMENSION<type>:<identifier>( �᭱����<unsigned integer>" ;
							break ;
						} // if  
						while ( token.at(i).str != ";" ) {
							while ( token.at(i).str == " " ) { // �ťո��� 
								i++ ;
							} // while
							if ( token.at(i).tableType != 3 ) { // �Dinteger
					        	all.at(index).error = true ;
								all.at(index).errormsg = "DIMENSION<type>:<identifier>( �᭱����<unsigned integer>" ;
								break ;
							} // if
							else { // �Ointeger �n�P�_�᭱�O���A���٬O�r�� 
								i++ ;
								while ( token.at(i).str == " " ) { // �ťո��� 
									i++ ;
								} // while
								if ( token.at(i).str != ")" && token.at(i).str != "," ) {
									all.at(index).error = true ;
									all.at(index).errormsg = "DIMENSION��integer�᭱����')'��','" ;
									break ;
								} // if 
								else {
									if ( token.at(i).str == ")" ) {
										break ; // �n�X�h�~���P�_�٦��S���U��array 
									} // if 
									else if ( token.at(i).str == "," ) {
										comma = true ;
										i++ ;
									} // else if 	 					
									else {
										all.at(index).error = true ;
										all.at(index).errormsg = "DIMENSION<array declaration>�A<unsigned integer>������','��')'" ;
										break ;
									} // else 
									if ( comma == true && token.at(i).tableType != 3 ) { // �r���᭱���O�Ʀr 
										all.at(index).error = true ;
										all.at(index).errormsg = "DIMENSION���r���᭱����integer'" ;
										break ;
									} //  if
								} // else 
							} // else integer �n�P�_�᭱�O���A���٬O�r�� 							
						} // while �P�_�A���̭�
						i++ ;
						while ( token.at(i).str == " " ) { // �����ť� 
							i++ ;
						} // while 
						if ( token.at(i).str != "," && token.at(i).str != ";" ) {
							all.at(index).error = true ;
							all.at(index).errormsg = "DIMENSION��)�᭱�����r���Τ���'" ;
							break ;
						} // if 
						else if ( token.at(i).str == "," ) {
							countArray = countArray + 1 ;
							i++ ;
						} // else if
						else if ( token.at(i).str == ";" ) {
							i++ ;
							while ( token.at(i).str == " " ) {
								i++ ;
							} // while 
							if ( token.at(i).str != "\n" ) {
								all.at(index).error = true ;
								all.at(index).errormsg = "�����᭱�������F��" ;
							} // if 
							
						} // else if 
					} // else  													
				} // while �P�_���X��array(A(),B(), C() ....) 
				// �P�_���� �Narray��Jtable7(vector)
						
				
				dataType = "" ; // init
				i = startLine ;
				int tempi = i ; // �Ȧsi���� 
				while ( token.at(i).str != ":" ) {
					i++ ; 
				} // while
				i++ ;
				while ( countArray > 0 ) {					
					
					while ( token.at(i).str == " " ) { // �����ť� 
						i++ ;
					} // while 
					while ( token.at(i).tableType != 5 ) {
						i++ ;
					} // while 
					arrayName = token.at(i).str ;
					
					int w = 0 ;
					while( w < 1000 ) {
						if ( table5[w].str == arrayName ) {
							table5[w].pointerIndex = table7.size() ;
							break ;
						} // if 
						w++ ;
					} // while
					
					int dataTypeNumber = CheckDataType( dataType ) ;
					table7.push_back(dataTypeNumber) ;
					tempi = i ; // ���Ȧsi���ȡA�ثe��(��Ĥ@�ӼƦr 
					while( token.at(i).str != ")" ) { // ���p�⦳�h��dimention 
						if ( token.at(i).str == "," ) countDimention++ ;
						i++ ;
					} // while 
					table7.push_back(countDimention) ;
					i = tempi ; // �^�Y 
					while( token.at(i).str != ")" ) { // �A��size(int) 
						if ( token.at(i).tableType == 3 ) {
							int size = atoi(token.at(i).str.c_str()) ;
							table7.push_back(size) ;
						} // if 
						i++ ;
					} // while 
					countArray = countArray - 1 ;
				} // while 	
				while( token.at(i).str != "\n" ) {
					i++ ;
				} // while 
				break ;	
			} // else if (DIMENTION)-------------------------------------------------------------------------(DIMENTION)			
			else if (token.at(i).tableValue == 11 ) { // GTO
			// GTO<label>; 
				i++ ;
				while ( token.at(i).str == " " ) { // �ťո��� 
					i++ ;
				} // while
		
				if ( token.at(i).tableType != 5 ) { // �᭱�Did 
					all.at(index).error = true ;
					all.at(index).errormsg = "GTO�᭱�Didentifier" ;
					break ;
				} // if 
				string token_label = token.at(i).str ; // ���Ȧslabel�A�ΨӧP�_�O�_��undefined symbol  
				i++ ;
				while ( token.at(i).str == " " ) { // �ťո��� 
					i++ ;
				} // while
				if ( token.at(i).str != ";" ) {
					all.at(index).error = true ;
					all.at(index).errormsg = "GTO�᭱���~(���Ӧh�F��)" ;
					break ; 
				} // if 
				i++ ;
				while ( token.at(i).str == " " ) { // �ťո��� 
					i++ ;
				} // while
				if ( token.at(i).str != "\n" ) {
					all.at(index).error = true ;
					all.at(index).errormsg = "';'�᭱�������F��" ;
					break ;
				} // if 
				
				// �P�_����k ���U�ӧP�_�O�_��undefined symbol
				int num = 0 ;
				bool exist = false ;
				while ( num < label.size() ) {
					if ( token_label == label.at(num) ) {
						exist = true ;
					} // if 
					num++ ;
				} // while 
				if ( exist == false ) { // ���s�b��label 
					all.at(index).error = true ;
					all.at(index).errormsg = "Undefined Label" ;
				} // if 
				break ;
			} // else if (GTO)-------------------------------------------------------------------------(GTO)
			else if (token.at(i).tableValue == 3 ) { // CALL
			// CALL<id> (id {,id}) ;
				i++ ;
				while ( token.at(i).str == " " ) { // �ťո��� 
					i++ ;
				} // while		
				if ( token.at(i).tableType != 5 ) { // �᭱�Did 
					all.at(index).error = true ;
					all.at(index).errormsg = "CALL�᭱�Didentifier" ;
					break ;
				} // if 
				i++ ;
				while ( token.at(i).str == " " ) { // �ťո��� 
					i++ ;
				} // while
				if (token.at(i).str != "(" ) {
					all.at(index).error = true ;
					all.at(index).errormsg = "CALL<identifier>�᭱����'('" ;
					break ;
				} // if 
				i++ ;
				while ( token.at(i).str == " " ) { // �ťո��� 
					i++ ;
				} // while
				if ( token.at(i).tableType != 5 ) { // �᭱�Did �h�P�_�@�����O�I 
					all.at(index).error = true ;
					all.at(index).errormsg = "CALL<identifier>( �᭱�Didentifier" ;
					break ;
				} // if 
				while ( token.at(i).str != ";" ) { // �P�_id,id,id,id... ;
					while ( token.at(i).str == " " ) { // �ťո��� 
						i++ ;
					} // while
			        if ( token.at(i).tableType != 5 ) { // �Did 
			        	all.at(index).error = true ;
						all.at(index).errormsg = "CALL�A�������F��D<identifier>" ;
						break ;
					} // if 
					else { // ��id 
					    comma = false ; // init
						i++ ;
						while ( token.at(i).str == " " ) { // �ťո��� 
							i++ ;
						} // while
						if ( token.at(i).str == ")" ) {
							i++ ;
							while ( token.at(i).str == " " ) { // �ťո��� 
								i++ ;
							} // while
							if ( token.at(i).str != ";") {
								all.at(index).error = true ;
								all.at(index).errormsg = "CALL��')'�᭱����';'" ;
								break ;
							} // if 
							else {
								i++ ;
								while ( token.at(i).str == " " ) { // �ťո��� 
									i++ ;
								} // while
								if ( token.at(i).str != "\n" ) {
									all.at(index).error = true ;
									all.at(index).errormsg = "';'�᭱�������F��" ;
									break ;
								} // if 
								break ;
							} // else 
						} // if 
						else if ( token.at(i).str == "," ) {
							comma = true ;
							i++ ;
						} // else if 	 					
						else {
							all.at(index).error = true ;
							all.at(index).errormsg = "CALL�A�����A<identifier>������','��')'" ;
							break ;
						} // else 
						if ( comma == true && token.at(i).str == ";" ) { // �r���᭱�����J����� 
							all.at(index).error = true ;
							all.at(index).errormsg = "CALL���r���᭱����������'" ;
							break ;
						} //  if
					} // else 
				} // while 
				// �P�_����k ���U�ӧP�_�O�_��undefined symbol
				i = startLine ;
				while ( token.at(i).str != "(" ) {
					i++ ;
				} // while
				i++ ;
				while ( i < endLine ) {	 
					if ( token.at(i).str == ";" ) {
						break ;
					} // if 
					while ( token.at(i).str == " " ) { // �����ť� 
						i++ ;
					} // while 
					int num = 0 ;
					bool exist = false ;
					if ( token.at(i).str  == " " || token.at(i).str == "," || token.at(i).str == ")" ) {
						i++ ;
					} // if 
					else { // ��id 
						while ( num < variable.size() ) { // �䦳�S���bvector�� 
							if ( token.at(i).str == variable.at(num) ) {
								exist = true ;
							} // if 
							num++ ;
						} // while 
						if ( exist == false ) { // ���s�b��label 
							all.at(index).error = true ;
							all.at(index).errormsg = "Undefined Variable" ;
							break ;
						} // if 
						i++ ;				
					} // else 										
				} // while 
				// �P�_����k�N�n�N�ܼƩ��table7
				int countVariable = 1 ;
				i = startLine ;
				while ( token.at(i).str != "(" ) {
					i++ ;
				} // while
				i++ ;
				while ( token.at(i).str == " " ) { // �����ť� 
					i++ ;
				} // while 
				int tempii = i ; // �����A����Ĥ@���ܼ� 
				while ( token.at(i).str != ";" ) {
					while ( token.at(i).str != ")" ) { // �p�⦳�h���ܼ� 
						if ( token.at(i).str == "," ) {
							countVariable++ ;
						} // if 						
						i++ ;
					} // while
					all.at(index).entry7 = table7.size() ; // ������O½�����X�� (table6).n8 = entry7 
					table7.push_back(countVariable) ;//�@�X��variables										
					i = tempii ; // �^�Y�A��N�ܼƩ�Jtable7��
					while ( token.at(i).str != ")" ) {
						if ( token.at(i).tableType == 5 ) {
							table7.push_back(token.at(i).tableType) ;
							table7.push_back(token.at(i).tableValue) ;
						} // if 
						i++ ;
					}  // while 
					i++ ;
				} // while	
				break ;			
			} // else if (CALL)-------------------------------------------------------------------------(CALL)
			else if (token.at(i).tableValue == 12 ) { // IF
			    i++ ;
			    while ( token.at(i).str == " " ) { // ���L�ť� 
			    	i++ ;
				} // while 
				Token condition ;
				condi.clear() ; // ���M�ŦA�� 
				
				while ( token.at(i).str != "THEN" && token.at(i).str != ";" ) {
					condition.str = token.at(i).str ;
					condition.tableType = token.at(i).tableType ;
					condition.tableValue = token.at(i).tableValue ;
					condi.push_back(condition) ;
					i++ ;
					
				} // while 
				bool condiCorrect = CheckCondition() ;
				if ( token.at(i).str == ";") {
					all.at(index).error = true ;
					all.at(index).errormsg = "IF��'THEN'���~" ;
					break ;
				} // if 
				if ( condiCorrect == false ) {
					all.at(index).error = true ;
					all.at(index).errormsg = "IF��'condition'���~" ;
					break ;
				} // if 
				i++ ;
				condi.clear() ; // ���M�ŦA��
				while ( token.at(i).str != "ELSE" && token.at(i).str != ";" ) {
					condition.str = token.at(i).str ;
					condition.tableType = token.at(i).tableType ;
					condition.tableValue = token.at(i).tableValue ;
					condi.push_back(condition) ;
					i++ ;
				} // while 
				;
				 condiCorrect = CheckStatement( index ) ;
				 if ( token.at(i).str == ";") {
					all.at(index).error = true ;
					all.at(index).errormsg = "IF��'ELSE'���~" ;
					break ;
				} // if 
				if ( condiCorrect == false ) {
					all.at(index).error = true ;
					all.at(index).errormsg = "IF��'statement'���~" ;
					break ;
				} // if 
				i++ ;
				condi.clear() ;
				while ( token.at(i).str != "ELSE" && token.at(i).str != ";" ) {
					condition.str = token.at(i).str ;
					condition.tableType = token.at(i).tableType ;
					condition.tableValue = token.at(i).tableValue ;
					condi.push_back(condition) ;
					i++ ;
				} // while 
				 condiCorrect = CheckStatement( index ) ;
				 if ( condiCorrect == false ) {
					all.at(index).error = true ;
					all.at(index).errormsg = "IF��'statement'���~" ;
					break ;
				} // if 
				break ;
			} // else if (IF)-------------------------------------------------------------------------(IF)
			else { // ��L���� �ڭ̥u�n�B�z�H�W�X�ӴN�n 
				break;
			} // else  
		} // else if (reserve word)
		else { // �i�ରassignment  
			int countLeft = 0 ; // �p�⥪�k�A���ƶq 
			int countRight = 0 ;
			all.at(index).assignment = true ;
			if ( token.at(i).tableType != 5 ) {
				all.at(index).error = true ;
				all.at(index).errormsg = "assignment�Ĥ@��token����identifier" ;
				break ;
			} // if 
			i++ ;
			while( token.at(i).str == " " ) { // ���L�Ů� 
				i++ ; 
			} // while
			if ( token.at(i).str == "(" ) { // �P�_�A���̭� 
				while ( token.at(i).str != ")" ) {
					i++ ;
					while( token.at(i).str == " " ) { // ���L�Ů� 
						i++ ; 
					} // while
					if ( token.at(i).tableType != 3 && token.at(i).tableType != 5 ) {
						all.at(index).error = true ;
						all.at(index).errormsg = "assignment�A��������identifier��unsigned-integer" ;
						break ;
					} // if 
					i++ ;
					while( token.at(i).str == " " ) { // ���L�Ů� 
						i++ ; 
					} // while
					if ( token.at(i).str == ")" ) {
						i++ ;
						break ;
					} // if 
					if ( token.at(i).str != "," ) {
						all.at(index).error = true ;
						all.at(index).errormsg = "assignment�A�������X��k" ;
						break ;
					} // if 
					else {
						i++ ;
						while( token.at(i).str == " " ) { // ���L�Ů� 
							i++ ; 
						} // while
						if ( token.at(i).tableType != 3 && token.at(i).tableType != 5 ) {
							all.at(index).error = true ;
							all.at(index).errormsg = "assignment�A��������identifier��unsigned-integer" ;
							break ;
						} // if 
						i++ ;
						while( token.at(i).str == " " ) { // ���L�Ů� 
							i++ ; 
						} // while
						if ( token.at(i).str != ")" ) {
							all.at(index).error = true ;
							all.at(index).errormsg = "assignment�֤F�k�A��" ;
							break ;
						} // if 
					} // else  
				} // while  				
			} // if
			if ( all.at(index).error == false ) { //�����e�ŦX��k
				
				while( token.at(i).str == " " ) { // ���L�Ů� 
					i++ ; 
				} // while
				if ( token.at(i).str != "=" ) {
					all.at(index).error = true ;
					all.at(index).errormsg = "assignment�֤F����" ;
					break ;
				} // if 
				i++ ;
				while( token.at(i).str == " " ) { // ���L�Ů� 
					i++ ; 
				} // while
				// �P�_�����᭱!!!
				bool ope1 = false ; // operand
				bool ope2 = true ; // operator
				bool left = false ; // ���A�� 
				bool right = false ; // �k�A�� 
				
				// ���P�_�Ĥ@�� 
				if ( token.at(i).str == ")" ) {
					all.at(index).error = true ;
					all.at(index).errormsg = "assignment�����k�䤣��H)�}�Y" ;
					break ;
				} // else if
				else if ( token.at(i).str != "(" && token.at(i).tableType != 5 &&token.at(i).tableType != 3 &&token.at(i).tableType != 4 ) {
					all.at(index).error = true ;
					all.at(index).errormsg = "assignment�����k�䤣��Hdelimeter�}�Y(�����A���~)" ;
					break ;
				} // else if 
				while( token.at(i).str != ";" ) {
					if ( token.at(i).str == ";" ) {
						break ;
					} // if 
					if ( token.at(i).str == "(" ){
						ope2 = true ;
						countLeft++ ;
						left = true ;
					} // if 
					else if (token.at(i).str == ")" ) {
						if ( ope2 == true ) {
							all.at(index).error = true ;
							all.at(index).errormsg = "assignment���A��(��m)����" ;
							break ;
						} // if 
						ope1 = true ;
						countRight++ ;
						right = true ;
					} // else if
					else if ( token.at(i).tableType == 5 || token.at(i).tableType == 3 || token.at(i).tableType == 4 ) {
						ope1 = true ;
						if ( ope2 == false ) {
							all.at(index).error = true ;
							all.at(index).errormsg = "assignment��operand/operator�ƶq����" ;
							break ;
						} // if 
						ope2 = false ;
					} // else if 
					else if ( token.at(i).tableType == 1 ) {
						ope2 = true ;
						if ( ope1 == false ) {
							all.at(index).error = true ;
							all.at(index).errormsg = "assignment��operand/operator�ƶq����" ;
							break ;
						} // if 
						ope1 = false ;
					} // else if 
					i++ ;
				} // while 
				if ( all.at(index).error == false && ope2 == true ) {
					all.at(index).error = true ;
					all.at(index).errormsg = "assignment�������HDelimeter����(�k�A�����~)" ;
					break ;
				} // if 
			} // if //�����e�ŦX��k
			if ( all.at(index).error == false && countLeft != countRight ) {
				all.at(index).error = true ;
				all.at(index).errormsg = "assignment���A���ƶq�����T" ;
				break ;
			} // if 
			break ;
			 // �P�_����k�n�A�P�_�O�_��undefined symbol(��variable��vector�t�d�s�Ҧ����ܼ�) 
		} // else 
	} // while 
} // end SyntaxAnalysis() �P�_�����k 
bool Syntax::IsDataType( string str ) { // �P�_�O�_��'type' �pARRAY, BOOLEAN..... 
	string out = "" ;
	Convert2BigString(str, out) ;
	if ( out == "ARRAY" || out == "BOOLEAN" || out == "CHARACTER" || out == "INTEGER" || out == "LABEL" || out == "REAL" ) {
		return true ;
	} // if 
	return false ;
} // end IsDataType() �P�_�O�_��'type' �pARRAY, BOOLEAN..... 
void Syntax::Convert2BigString( string in , string &out ) {
	int i = 0 ;
	out = in ;
	while ( i < in.length() ) {
		if ( in[i] >= 'a'&& in[i] <= 'z' ) {
			out[i] = in[i] - 32 ;
		} // if 
		i++ ;
	} // while
} // �ন�j�g 
int Syntax::CheckDataType( string str ) { // �^�Ǩ�dataType���X 
	if ( str == "ARRAY" ) return 1 ;
	else if ( str == "BOOLEAN" ) return 2 ;
	else if ( str == "CHARACTER" ) return 3 ;
	else if ( str == "INTEGER" ) return 4 ;
	else if ( str == "LABEL" ) return 5 ;
	else if ( str == "REAL" ) return 6 ;
	else return -1 ; 	
} // CheckDataType()

void Syntax::GenerateIntermediate() { // ���ͤ����X 
	int i = 0 ; // �] token (vector) 
	int a = 0 ; // �] all (vector) 
	Table6 temp ; 
	int statementStart = 0 ; // ����statement���_�l��} 
	int statementEnd = 0 ; // statement������m 
	bool firstLine = true ;
	bool hasLabel = false ;
	string tempLabel = "" ;
	while ( a < all.size() ) {
		firstLine = true ;
		hasLabel = false ;
		tempLabel = "" ;
		while ( token.at(i).str != "\n" ) { // �P�_�@��� 
			if ( all.at(a).error ) { // �����~��½�����X 
				ERROR e ;
				char ca[100] ;
				itoa(a+1, ca, 10) ;
				string caa = ca ;
				e.errormsg = "line " + caa + ":	" + all.at(a).errormsg ;
				e.line = table6.size() ; 
				err.push(e) ;
				break ;
			} // if
			else { // ��k�L�~ �n½�����X 
				string str = "" ;
				if ( firstLine && token.at(i).tableType == 5 ) { // �J��Label 
					hasLabel = true ;
					Label l ;
					l.str = token.at(i).str ;
					tempLabel = l.str ;
					l.line = table6.size() ;
					labelTable.push_back(l) ;
					i++ ;
				} // if 
				else if ( token.at(i).tableType == 2 ) {
					if ( token.at(i).tableValue == 21 ) { // PROGRAM
						break;
					} // if PROGRAM
					else if ( token.at(i).tableValue == 3 ) { // CALL ---------------------------------------------(CALL) 
						//�|�� CALL SI(A, B, C, D) ;
						statementStart = i ;
						while ( token.at(i).str != ";" ) {
							i++ ;
						} // while  
						GenerateCALL(statementStart, a) ; // ����CALL�������X 
			            break ; // ����CALL 
					} // else if CALL -----------------------------------------------------------------------------(CALL) 
					else if ( token.at(i).tableValue == 25 ) { // VARIABLE---------------------------------------------(VARIABLE)
					 	while ( token.at(i).str != ";" ) {
					 		
					 		if ( token.at(i).tableType == 5 ) {
					 			temp.n1 = 5 ; 
								temp.n2 = token.at(i).tableValue ;
								temp.n3 = -1 ;
								temp.n4 = -1 ;
								temp.n5 = -1 ;
								temp.n6 = -1 ;
								temp.n7 = -1 ;
								temp.n8 = -1 ;
								temp.str = token.at(i).str ;
								table6.push_back(temp) ;
								str = "" ;
								temp.str = "" ;
							} // if 
							i++ ;
						} // while 
					} // else if VARIABLE---------------------------------------------(VARIABLE) 
					else if ( token.at(i).tableValue == 4 ) {// DIMENSION---------------------------------------------(DIMENSION)
						while ( token.at(i).str != ":" ) {
							i++ ;
						} // while
						i++ ;
						while ( token.at(i).str != ";" ) {
							
							if ( token.at(i).tableType == 5 ) {
					 			temp.n1 = 5 ; // CALL
								temp.n2 = token.at(i).tableValue ;
								temp.n3 = -1 ;
								temp.n4 = -1 ;
								temp.n5 = -1 ;
								temp.n6 = -1 ;
								temp.n7 = -1 ;
								temp.n8 = -1 ;
								temp.str = token.at(i).str ;
								table6.push_back(temp) ;
								str = "" ;
								temp.str = "" ;
							} // if 
							i++ ;
						} // while 
						break ;
					} // else if DIMENSION---------------------------------------------(DIMENSION)
					else if ( token.at(i).tableValue == 23 ) { // SUBROUTINE------------------------------------------(SUBROUTINE)
						while ( token.at(i).str != ":" ) {
							i++ ;
						} // while
						while ( token.at(i).str != ";" ) {
							
							if ( token.at(i).tableType == 5 ) {
					 			temp.n1 = 5 ;
								temp.n2 = token.at(i).tableValue ;
								temp.n3 = -1 ;
								temp.n4 = -1 ;
								temp.n5 = -1 ;
								temp.n6 = -1 ;
								temp.n7 = -1 ;
								temp.n8 = -1 ;
								temp.str = token.at(i).str ;
								table6.push_back(temp) ;
								str = "" ;
								temp.str = "" ;
							} // if 
							i++ ;
						} // while 
						break ;
					} // else if SUBROUTINE---------------------------------------------(SUBROUTINE)
					else if ( token.at(i).tableValue == 6 || token.at(i).tableValue == 7 ) { // ENP/ENS ---------------(ENP/ENS)
						temp.n1 = 2 ; 
						temp.n2 = token.at(i).tableValue ;
						temp.n3 = -1 ;
						temp.n4 = -1 ;
						temp.n5 = -1 ;
						temp.n6 = -1 ;
						temp.n7 = -1 ;
						temp.n8 = -1 ;
						temp.str = token.at(i).str ;
						table6.push_back(temp) ;
						temp.str = "" ;
						while ( token.at(i).str != ";" ) {
							i++ ;
						} // while 
						break ;
					} // else if ENP/ENS -----------------------------------------------------(ENP/ENS)
					else if ( token.at(i).tableValue == 15 ) { // LABEL----------------------------------------------(LABEL)						
						Label templ ; 
						while ( token.at(i).str != ";" ) {
							if ( token.at(i).tableType == 5 ) {
								
								temp.n1 = 5 ; 
								temp.n2 = token.at(i).tableValue ;
								temp.n3 = -1 ;
								temp.n4 = -1 ;
								temp.n5 = -1 ;
								temp.n6 = -1 ;
								temp.n7 = -1 ;
								temp.n8 = -1 ;
								temp.str = token.at(i).str ;
								table6.push_back(temp) ;
								temp.str = "" ;
								templ.str = token.at(i).str ;
								templ.line = table6.size() ;
								//gto.push_back(templ) ;
							} // if 
							i++ ;
						} // while 
						break ;
					} // else if LABEL----------------------------------------------(LABEL)
					else if ( token.at(i).tableValue == 11 ) { // GTO----------------------------------------------(GTO)
						
						statementStart = i ;
						
						/*while ( token.at(i).str != ";" ) {
							if ( token.at(i).tableType == 5 ) {
								
								temp.n1 = 5 ; 
								temp.n2 = token.at(i).tableValue ;
								temp.n3 = -1 ;
								temp.n4 = -1 ;
								temp.n5 = -1 ;
								temp.n6 = -1 ;
								temp.n7 = -1 ;
								temp.n8 = -1 ;
								temp.str = token.at(i).str ;
								table6.push_back(temp) ;
								temp.str = "" ;
								templ.str = token.at(i).str ;
								templ.line = table6.size() ;
								gto.push_back(templ) ;
							} // if 
							i++ ;
						} // while */
						GenerateGTO( statementStart ) ; // ����GTO�������X 
						
						break ;
					} // else if GTO----------------------------------------------(GTO)
					else if ( token.at(i).tableValue == 12) { // IF --------------------------------------------(IF)
						string ifStatement= "" ;
						i++ ;
						int thenNum ;
						while ( token.at(i).str == " " ) { // �����ť� 
							i++ ;
						} // while 
						Token condition ;
						condi.clear() ;
						string tempStr = "" ;
						while ( token.at(i).str != "THEN" ) { // condition
							tempStr = tempStr + token.at(i).str ;
							condition.str = token.at(i).str ;
							condition.tableType = token.at(i).tableType;			
							condition.tableValue = token.at(i).tableValue ;
							condi.push_back(condition) ;
							i++ ;
						} // while 
						tempStr = tempStr + token.at(i).str ;
						GenerateCondition() ; // ����condition�����X 
						temp.n1 = 2 ; // if
						temp.n2 = 12 ;
						temp.n3 = 0 ; // T1
						temp.n4 = table0.size()-1 ;
						temp.n5 = 6 ;
						temp.n6 = table6.size()+2 ;
						temp.n7 = 6 ;
						temp.n8 = -1 ;
						temp.str = "IF " + tempStr ;
						ifStatement = temp.str ;
						if ( hasLabel ) { // �Y�e����label 
							labelTable.at(labelTable.size()-1).line = table6.size() ;
						} // if 
						table6.push_back(temp) ;
						int ifNum = table6.size() ; // ��ӭn�^�Ӷ� 
						tempStr = "" ;
						i++ ;
						while ( token.at(i).str == " " ) { // �����ť� 
							tempStr = tempStr + token.at(i).str ;
							i++ ;
						} // while 
						
						
						Token t ; // �Ȧs�ܼƥN�� 
						char ch[10] = {'\0'} ;
						// Condition
						statementStart = i ; // statement�}�l��m 
						while( token.at(i).str != "ELSE" ) {
							tempStr = tempStr + token.at(i).str ;
							i++ ;
						} // while
						tempStr = tempStr + token.at(i).str ;
						statementEnd = i ; // statement������m 
						bool isGTO = false ;
						GenerateStatement( statementStart, statementEnd, a, isGTO ) ; // ����statement�������X 
						if ( isGTO ) { // THEN����GTO �N���β��ͲĤG��GTO�F 
							;
						} // if 
						else {
							temp.n1 = 2 ; // GTO
							temp.n2 = 11 ;
							temp.n3 = -1 ;
							temp.n4 = -1 ;
							temp.n5 = -1 ;
							temp.n6 = -1 ;
							temp.n7 = 6 ;
							temp.n8 = -1 ;
							temp.str = "GTO" ; 
							thenNum =table6.size(); // �n�^�Ӷ�
							table6.push_back(temp) ;
						} // else 
												
						// �^�h��IF��ELSE����m						
						table6.at(ifNum-1).n8 = table6.size()+1;
											
						i++ ;
						while ( token.at(i).str == " " ) { // �����ť� 
							tempStr = tempStr + token.at(i).str ;
							i++ ;
						} // while																		
						// ELSE STATEMENT
						statementStart = i ;
						while ( token.at(i).str != ";" ) {
							tempStr = tempStr + token.at(i).str ;
							i++ ;
						} // while
						char c[100] ;
						itoa(table6.at(ifNum-1).n4, c, 10);
						string n4 = c ; 
						n4 = "T" + n4 ;
						itoa(table6.at(ifNum-1).n6, c, 10);
						string n6 = c ; 
						itoa(table6.at(ifNum-1).n8, c, 10);
						string n8 = c ; 
						table6.at(ifNum-1).str = "IF " + n4 + " GO TO " + n6 + " ELSE GO TO " + n8 ;
						
						statementEnd = i ;
						bool tempISGTO = isGTO ;
						isGTO = false ;
						GenerateStatement( statementStart, statementEnd, a, isGTO ) ;
						if ( tempISGTO ) {
							;
						} // if 
						else {
							table6.at(thenNum).n8 = table6.size()+1;
						} // else 
						
						
						char v[100] ;
						itoa(table6.size()+1, v, 10) ;
						string vv = v ;
						if ( tempISGTO ) {
							;
						} // if 
						else {
							table6.at(thenNum).str = table6.at(thenNum).str + " " + vv ;
						} // else 
						break ;
				    } // else if IF --------------------------------------------(IF)
				} // if  table2
				else if ( !hasLabel && all.at(a).assignment == true ) { // ��assignment 
					statementStart = i ;
					while ( token.at(i).str != ";" ) {
						i++ ;
					} // while 
					
					GenerateAssignment( statementStart ) ; // ����assignment�������X  
				} // else if ��assignment 
			} // else // ��k�L�~ �n½�����X
				
			i++ ;
			while ( token.at(i).str == " " ) { // �קK�����ᦳ�ťզӤ��O���� 
				i++ ; 
			} // while 
		} // while 
		a++ ;
		while( token.at(i).str != "\n" ) {
			i++ ;
		} // while 
		i++ ;
	} // while 
	
	int g = 0 ;
	int l = 0 ;
	while ( g < gto.size() ) {
		l = 0 ;
		while ( l < labelTable.size() ) {
			if (labelTable.at(l).str == gto.at(g).str ) {
				table6.at(gto.at(g).line).n8 = labelTable.at(l).line+1 ;
				break ;
			} // if 
			l++ ;
		} // while 
		g++ ;
	} // while 
	
} // ���ͤ����X 
bool Syntax::CheckCondition() { // �P�_condition�O�_���T 
	int i = 0 ;
	bool isID = false ;
	bool isTWO = true ;
	while( i < condi.size() ) {
		if( condi.at(i).str == " " ) {
			;
		} // if
		else {
			if ( condi.at(i).tableType == 5 || condi.at(i).tableType == 3 || condi.at(i).tableType == 4 ) { // id or �Ʀr 
				isID = true ;
				if ( isTWO == false ) {
					return false ;
				} // if 
				isTWO = false ;
			} // if 
			else {
				if( isID == false ) {
					return false ;
				} // if 
				else {
					isID = false ;
					if ( condi.at(i).tableType != 2 ) {
						return false ;
					} // if 
					isTWO = true ;
				} // else 
			} // else 
		} // else
		i++ ; 
	} // while 
	if ( isTWO == true ) {
		return false ;
	} // if
	return true ;
} // CheckCondition()�P�_condition�O�_���T 
bool Syntax::CheckStatement( int index ) { // �P�_Statement�O�_���T 
	int i = 0 ;
	int startLine = i ;
	bool isID = false ;
	bool isONE = true ;
	bool isGTO = false ;
	bool isCALL = false ;
	Token end ;
	end.str = ";" ;
	end.tableType = 1 ;
	end.tableValue = 1 ; 
	condi.push_back(end) ;
	while( i < condi.size() ) {
		if( condi.at(i).str == " " ) {
			;
		} // if
		else {
			if ( condi.at(i).tableType == 2 ) { // GTO��CALL 
				if (condi.at(i).tableValue == 11 ) { // GTO
					// GTO<label>; 
					isGTO = true ;
					i++ ;
					while ( condi.at(i).str == " " ) { // �ťո��� 
						i++ ;
					} // while
			
					if ( condi.at(i).tableType != 5 ) { // �᭱�Did 
						return false ;
					} // if 
					string token_label = condi.at(i).str ; // ���Ȧslabel�A�ΨӧP�_�O�_��undefined symbol  
					i++ ;
					while ( condi.at(i).str == " " ) { // �ťո��� 
						i++ ;
					} // while
					if ( condi.at(i).str != ";" ) {
						return false ;
					} // if 					
					// �P�_����k ���U�ӧP�_�O�_��undefined symbol
					int num = 0 ;
					bool exist = false ;
					while ( num < label.size() ) {
						if ( token_label == label.at(num) ) {
							exist = true ;
						} // if 
						num++ ;
					} // while 
					if ( exist == false ) { // ���s�b��label 
						return false ;
					} // if 
					break ;
				} // else if (GTO)-------------------------------------------------------------------------(GTO)
				else if (condi.at(i).tableValue == 3 ) { // CALL
				// CALL<id> (id {,id}) ;
				    isCALL = true ;
					i++ ;
					while ( condi.at(i).str == " " ) { // �ťո��� 
						i++ ;
					} // while		
					if ( condi.at(i).tableType != 5 ) { // �᭱�Did 
						return false ;
					} // if 
					i++ ;
					while ( condi.at(i).str == " " ) { // �ťո��� 
						i++ ;
					} // while
					if (condi.at(i).str != "(" ) {
						return false ;
					} // if 
					i++ ;
					while ( condi.at(i).str == " " ) { // �ťո��� 
						i++ ;
					} // while
					if ( condi.at(i).tableType != 5 ) { // �᭱�Did �h�P�_�@�����O�I 
						return false ;
					} // if 
					while ( condi.at(i).str != ";" ) { // �P�_id,id,id,id... ;
						while ( condi.at(i).str == " " ) { // �ťո��� 
							i++ ;
						} // while
				        if ( condi.at(i).tableType != 5 ) { // �Did 
				        	return false ;
						} // if 
						else { // ��id 
						    bool comma = false ; // init
							i++ ;
							while ( condi.at(i).str == " " ) { // �ťո��� 
								i++ ;
							} // while
							if ( condi.at(i).str == ")" ) {
								i++ ;
								while ( condi.at(i).str == " " ) { // �ťո��� 
									i++ ;
								} // while
								if ( condi.at(i).str != ";") {
									return false ;
								} // if 
								
							} // if 
							else if ( condi.at(i).str == "," ) {
								comma = true ;
								i++ ;
							} // else if 	 					
							else {
								return false ;
							} // else 
							if ( comma == true && condi.at(i).str == ";" ) { // �r���᭱�����J����� 
								return false ;
							} //  if
						} // else 
					} // while 
					// �P�_����k ���U�ӧP�_�O�_��undefined symbol
					i = startLine ;
					while ( condi.at(i).str != "(" ) {
						i++ ;
					} // while
					i++ ;
					while ( condi.at(i).str != ";" ) {	 
						if ( condi.at(i).str == ";" ) {
							break ;
						} // if 
						while ( condi.at(i).str == " " ) { // �����ť� 
							i++ ;
						} // while 
						int num = 0 ;
						bool exist = false ;
						if ( condi.at(i).str  == " " || condi.at(i).str == "," || condi.at(i).str == ")" ) {
							i++ ;
						} // if 
						else { // ��id 
							while ( num < variable.size() ) { // �䦳�S���bvector�� 
								if ( condi.at(i).str == variable.at(num) ) {
									exist = true ;
								} // if 
								num++ ;
							} // while 
							if ( exist == false ) { // ���s�b��label 
								return false ;
							} // if 
							i++ ;				
						} // else 										
					} // while 
					// �P�_����k�N�n�N�ܼƩ��table7
					int countVariable = 1 ;
					i = startLine ;
					while ( condi.at(i).str != "(" ) {
						i++ ;
					} // while
					i++ ;
					while ( condi.at(i).str == " " ) { // �����ť� 
						i++ ;
					} // while 
					int tempii = i ; // �����A����Ĥ@���ܼ� 
					while ( condi.at(i).str != ";" ) {
						while ( condi.at(i).str != ")" ) { // �p�⦳�h���ܼ� 
							if ( condi.at(i).str == "," ) {
								countVariable++ ;
							} // if 						
							i++ ;
						} // while
						all.at(index).entry7 = table7.size() ; // ������O½�����X�� (table6).n8 = entry7 
						table7.push_back(countVariable) ;//�@�X��variables										
						i = tempii ; // �^�Y�A��N�ܼƩ�Jtable7��
						while ( condi.at(i).str != ")" ) {
							if ( condi.at(i).tableType == 5 ) {
								table7.push_back(condi.at(i).tableType) ;
								table7.push_back(condi.at(i).tableValue) ;
							} // if 
							i++ ;
						}  // while 
						i++ ;
					} // while	
					break ;	
				} // else if CALL
			} // if GTO��CALL 
			else { // assignment
				if ( condi.at(i).tableType == 5 || condi.at(i).tableType == 3 || condi.at(i).tableType == 4 ) { // id or �Ʀr 
					isID = true ;
					if ( isONE == false ) {
						return false ;
					} // if 
					isONE = false ;
				} // if 
				else if ( condi.at(i).str != ";" ){
					if( isID == false ) {
						return false ;
					} // if 
					else {
						
						if ( condi.at(i).tableType != 1 ) {
							return false ;
						} // if 
						isONE = true ;
						isID = false ;
					} // else 
				} // else 
			} // else assignment			
		} // else
		i++ ; 
	} // while 
	if ( !isGTO && !isCALL && isONE == true ) {
		return false ;
	} // if 
	return true ;
} // CheckStatement()�P�_Statement�O�_���T
void Syntax::GenerateCondition() { // ����condi�������X 
	int i = 0 ;
	string str = "" ;
	string tempS = "" ;
	while ( i < condi.size() ) {
		if ( condi.at(i).tableType == 2 && op2.size() == 1 ) {
			tempS = condi.at(i).str ;
		} // if 
		else {
			str = str + condi.at(i).str ;
		} // else 
		if ( condi.at(i).str == " " ) {
			;
		} // if
		else if ( condi.at(i).tableType == 5 ) { // X
			op1.push(condi.at(i) ) ;
		} // else if 		
		if ( condi.at(i).tableType == 2 && op2.size() == 1 ) { // pop()
			Table6 temp ;
			Token t ;
			t = op1.top() ; // ��Y 
			temp.n5 = 5 ;
			temp.n6 = t.tableValue ;
			op1.pop() ; // ��Y 
			t = op1.top() ; // ��X 
			temp.n3 = 5 ;
			temp.n4 = t.tableValue ;
			op1.pop() ; // ��X 
			t = op2.top() ; // ��GT
			temp.n1 = 2 ;
			temp.n2 = t.tableValue ;
			op2.pop() ; // ��GT 
			t.str = "T" ;
			t.tableType = 0 ;
			t.tableValue = table0.size() ;
			char ch[10] = {'\0'} ;
			itoa(t.tableValue, ch, 10) ;
			string s = ch ;
			t.str = t.str + s ;
			temp.n7 = 0 ;
			temp.n8 = table0.size() ;
			
			temp.str = t.str + " = " + str ; // T1 = X GT Y
			table6.push_back(temp) ;
			str = "" ;
			temp.str = "" ;
			table0.push_back(t) ;
			op1.push(t) ; // ��T1
			op2.push(condi.at(i) ) ; // ��AND 
			str = t.str + " " + tempS ;
		} // if 
		else if ( condi.at(i).tableType == 2 ){ 
			op2.push(condi.at(i) ) ;
		} // else 
		i++ ;
	} // while 
	if ( op2.empty() == false ) {
		Token t ;
		Table6 temp ;
		t = op1.top() ; // ��Y 
		temp.n5 = 5 ;
		temp.n6 = t.tableValue ;
		op1.pop() ; // ��Y 
		t = op1.top() ; // ��X 
		temp.n3 = t.tableType ;
		temp.n4 = t.tableValue ;
		op1.pop() ; // ��X 
		t = op2.top() ; // ��GT
		temp.n1 = 2 ;
		temp.n2 = t.tableValue ;
		op2.pop() ; // ��GT 
		t.str = "T" ;
		t.tableType = 0 ;
		t.tableValue = table0.size() ;
		char ch[10] = {'\0'} ;
		itoa(t.tableValue, ch, 10) ;
		string s = ch ;
		t.str = t.str + s ;
		temp.n7 = 0 ;
		temp.n8 = table0.size() ;
		temp.str = t.str + " = " + str ; // T1 = X GT Y
		table6.push_back(temp) ;
		str = "" ;
		temp.str = "" ;
		table0.push_back(t) ;

	} // if  
} //  ����condi�������X 
void Syntax::GenerateAssignment( int statementStart ) { // ����assignment�������X 
	stack<Token> op1 ; // operand
	stack<Token> op2 ; // operator
	int i = statementStart ;
	Token temp ;
	Token operand1 ; // operand 1
	Token operand2 ; // operand 2 
	Token operator1 ; // operator
	bool isOp1 = false ;
	bool isArray = false ; // �W�@�ӬO���A�� 
	while ( token.at(i).str != ";" ) {
		token.at(i).isArrayTwo = false ; // init
		token.at(i).isArrayOne = false ;
		while ( token.at(i).str == " " ) {
			i++ ;
			token.at(i).isArrayTwo = false ; // init
		    token.at(i).isArrayOne = false ;
		} // while 
		if ( token.at(i).str == ";" ) {
			break ;
		} // if 
		if ( token.at(i).tableType != 1 ) { // operand ������ 
		    isOp1 = true ;
			op1.push( token.at(i) ) ;
		} // if 
		else { // delimeter 
			if ( op2.empty() ) {
				CheckLevel( token.at(i) ) ;
				op2.push( token.at(i) ) ;
				isArray = false  ;					
				if ( token.at(i).str == "(" ) { // ���A����stack���K�̤p 
					token.at(i).level = 0 ;
					if ( isOp1 ) {
						isArray = true ;
					} // if 
				} // if 
			} // if 
			else { // ��j�p�M�w�n���npop() 
				CheckLevel( token.at(i) ) ;
				temp = op2.top() ; // ��top�ǳƤ���j�p
				if ( temp.level < token.at(i).level ) { // �n��stack  (���k�A���~) 
					if ( token.at(i).str == ")" ) {
						operand2 = op1.top() ;
						op1.pop() ;
						operand1 = op1.top() ;
						op1.pop() ;
						operator1 = op2.top() ;
						op2.pop() ;
						if ( operator1.str == "(" ) { // ���i��Oarray 
							if ( !isArray) { // ���Oarray 
								op1.push(operand1) ;
							    op1.push(operand2) ;
							} // if 
							else { // �Oarray 
								token.at(i).isArrayOne = true ;
								Token AT ;
								AT.str = "at" ;
								AT.tableType = 8 ; // �N��array
								AT.tableValue = arrayTable.size() ; // tableValue ���VarrayTable��entry 
								AT.pointAT = arrayTable.size() ;
								ArrayTable at ;
								at.array = operand1 ;
								at.index1 = operand2 ;
								at.two_dimention = false ;
								arrayTable.push_back(at) ;
								AT.isArrayTwo = false ;
								AT.isArrayOne = true ;
								op1.push(AT) ; // ����NARRAY��Jstack 
							} // else �Oarray 

						} // if 
						else if ( operator1.str == "," ) { // �O�G���}�C 
							token.at(i).isArrayTwo = true ;
							op2.pop() ; // pop���A��
							Token operand0 = op1.top() ; // ��ARRAY NAME
							op1.pop() ;
							Token AT ;
							AT.str = "at" ;
							AT.tableType = 8 ; // �N��array
							AT.tableValue = arrayTable.size() ; // tableValue ���VarrayTable��entry 
							AT.pointAT = arrayTable.size() ;
							ArrayTable at ;
							at.array = operand0 ;
							at.index1 = operand1 ;
							at.index2 = operand2 ;
							at.two_dimention = true ;
							arrayTable.push_back(at) ;
							AT.isArrayTwo = true ;
							AT.isArrayOne = false ;
							op1.push(AT) ; // ����NARRAY��Jstack 
						} // else if  �O�G���}�C 
						else {							
							op2.pop() ; // pop ���A��
							GenerateCode( operator1, operand1, operand2 ) ;
							// �A�N�Ȯ��ܼ�T1��istack 
							op1.push(table0.at(table0.size()-1) ); 
						} // else 
							
					} // if 
					else {	
						isArray = false  ;					
						if ( token.at(i).str == "(" ) { // ���A����stack���K�̤p 
							token.at(i).level = 0 ;
							if ( isOp1 ) {
								isArray = true ;
							} // if 
						} // if 
						op2.push(token.at(i)) ;
					} // else 
				} // if // �n��stack  (���k�A���~)
				else if ( temp.level == token.at(i).level || temp.level > token.at(i).level ){ // �npop() ���ͤ����X  (�Y������ �h����Jstack)
					if ( token.at(i).str == "^" ) {
						op2.push(token.at(i)) ;
					} // if 
					else { // �u���nPOP()
						if ( !op2.empty() ) {
							// �qop1������Ӽ� �⧹��J�Ȯ��ܼƦApush�^�h
							if ( token.at(i).str == ";" ) {
								break;
							} // if 
							if ( temp.level == token.at(i).level || temp.level > token.at(i).level ) {	// �npop()
														
								if ( token.at(i).str == ")" ) {
									operand2 = op1.top() ;
									op1.pop() ;
									operand1 = op1.top() ;
									op1.pop() ;
									operator1 = op2.top() ;
									op2.pop() ;
									if ( operator1.str == "("  ) { // ���i��Oarray 
									    if ( !isArray   ) { // ���Oarray 
								            op1.push(operand1) ;
							                op1.push(operand2) ;
						            	} // if 
							            else if( isOp1 == true ) { // �Oarray 
							            	token.at(i).isArrayOne = true ;
							            	Token AT ;
							            	AT.str = "at" ;
							            	AT.tableType = 8 ; // �N��array
							            	AT.tableValue = arrayTable.size() ; // tableValue ���VarrayTable��entry 
							            	AT.pointAT = arrayTable.size() ;
							            	AT.isArrayOne = true ;
							            	ArrayTable at ;
							            	at.array = operand1 ;
							            	at.index1 = operand2 ;
							            	at.two_dimention = false ;
							            	arrayTable.push_back(at) ;
							            	AT.isArrayTwo = false ;
								            AT.isArrayOne = true ;
							            	op1.push(AT) ; // ����NARRAY��Jstack 
						            	} // if �Oarray 

						            } // if 
						            else if ( operator1.str == "," ) { // �O�G���}�C 
							            token.at(i).isArrayTwo = true ;
						            	op2.pop() ; // pop���A��
							            Token operand0 = op1.top() ; // ��ARRAY NAME
							            op1.pop() ;
							            Token AT ;
							            AT.str = "at" ;
						            	AT.tableType = 8 ; // �N��array
						            	AT.isArrayTwo = true ;
						            	AT.tableValue = arrayTable.size() ; // tableValue ���VarrayTable��entry 
							            AT.pointAT = arrayTable.size() ;
						            	ArrayTable at ;
						            	at.array = operand0 ;
						            	at.index1 = operand1 ;
						            	at.index2 = operand2 ;
						            	at.two_dimention = true ;
						            	arrayTable.push_back(at) ;
						            	AT.isArrayTwo = true ;
								        AT.isArrayOne = false ;
						            	op1.push(AT) ; // ����NARRAY��Jstack 
					            	} // else if  �O�G���}�C 
									else {	
									    if ( operand1.tableType != 8 && operand2.tableType != 8 ) {
									    	GenerateCode( operator1, operand1, operand2 ) ;
										    // �A�N�Ȯ��ܼ�T1��istack 
										    op1.push(table0.at(table0.size()-1) ); 
										    i-- ;
										} // if 
										else { // ��array 
											GenerateArray( operator1, operand1, operand2 ) ; // ���ͦ���array��assignment�������X 
										
										}	// else ��array														
										
									} // else 
																		
								} // if �k�A�� 
								else {
									operand2 = op1.top() ;
									op1.pop() ;
									operand1 = op1.top() ;
									op1.pop() ;
									operator1 = op2.top() ;
									op2.pop() ;
									GenerateCode( operator1, operand1, operand2 ) ;
									// �A�N�Ȯ��ܼ�T1��istack 
									op1.push(table0.at(table0.size()-1)) ; 
									i-- ;
								} // else 														
							} // if �npop() 
							else { // push!
								op2.push(token.at(i)) ; 
							} // else 																													
						} // if
					} // else �u���nPOP() 
				} // else if // �npop() ���ͤ����X  (�Y������ �h����Jstack)				
			} // else 
			isOp1 = false ;
		} // else Delimiter 
		i++ ;
		token.at(i).isArrayTwo = false ; // init
		token.at(i).isArrayOne = false ;
	} // while 
	
	
	while ( !op2.empty() ) {
		operand2 = op1.top() ; // T1
		op1.pop() ;
		operand1 = op1.top() ; // X 
		op1.pop() ;
		operator1 = op2.top() ; // = || +-*/
		op2.pop() ;
		if ( operator1.str == "=" ) {
			
			if ( operand1.tableType != 8 && operand2.tableType != 8 ) {
		    	Table6 tempp ;
				tempp.n1 = operator1.tableType ;
				tempp.n2 = operator1.tableValue ;
				tempp.n3 = operand2.tableType ;
				tempp.n4 = operand2.tableValue ;
				tempp.n5 = -1 ;
				tempp.n6 = -1 ;
				tempp.n7 = operand1.tableType ;
				tempp.n8 = operand1.tableValue ;
				string statement = operand1.str + " " + operator1.str + " " +operand2.str ;
				tempp.str = statement ;
				table6.push_back(tempp) ; 
			} // if 
			else { // ��array 
				GenerateArray( operator1, operand1, operand2 ) ; // ���ͦ���array��assignment�������X 
			
			}	// else ��array				
		} // if 
		else {
			if ( operand1.tableType != 8 && operand2.tableType != 8 ) {
				GenerateCode( operator1, operand1, operand2 ) ;
				// �A�N�Ȯ��ܼ�T1��istack 
				op1.push(table0.at(table0.size()-1)) ; 
			} // if 
			else {
				GenerateArray( operator1, operand1, operand2 ) ; // ���ͦ���array��assignment�������X 
				op1.push(table0.at(table0.size()-1)) ; 
			} // else 
		} // else 
	} // while
		
} // ����assignment�������X 
void Syntax::GenerateArray( Token operator1, Token operand1, Token operand2 ) { // ���ͦ���array��assignment�������X 
	Table6 temp ;
	Token t ; // �Ȯ��ܼ� 
	char ch[100] ;
	string s ;
	Token n34;
	Token n56 ;
	int i = 0 ;
	Token n78 ; 
	if ( operator1.str == "=" ) { // ���󪺱��p
		if ( operand2.tableType == 8 ) { // �ݥͦ�T1�Ȯ��ܼ�  ���󻡳B�z�����ᵥ���k�䤣�|�Oarray�F 
			if ( operand2.isArrayTwo ) {// �G���}�C
				temp.n1 = 1 ; // - 
				temp.n2 = 6 ;
				n34 = arrayTable.at(operand2.pointAT).index2 ;
				temp.n3 = n34.tableType ;
				temp.n4 = n34.tableValue ;
				temp.n5 = 3 ; // 1
				temp.n6 = 49 ; 
				t.str = "T" ;
				t.tableType = 0 ;
				temp.n7 = 0 ;
				t.tableValue = table0.size() ;
				temp.n8 = t.tableValue ;
				itoa(table0.size(), ch, 10) ;
				s = ch ;
				t.str = t.str + s ;
				temp.str = t.str + "=" + n34.str + "-1" ;
				t.isArrayOne = false ;
				t.isArrayTwo = false ;
				table0.push_back(t) ;
				table6.push_back(temp) ;
				//----------------------------------------------------------
				temp.n1 = 1 ; // *
				temp.n2 = 7 ;
				n34 = t ;
				temp.n3 = t.tableType ;
				temp.n4 = t.tableValue ;
				n56 = arrayTable.at(operand2.pointAT).array ;
				i = 0 ;
				int index = 0 ;
				while ( i < 1000 ) { // table5
					if ( table5[i].str == n56.str ) {
						
						index = table5[i].pointerIndex ; // ArrayName
						break ;
					} // if 
					i++ ;
				} // while 

				index = index + 2 ;
				char number[100] ;
				itoa( table7.at(index), number, 10);
				string numm = number ;
				int j = 0 ;
				while ( j < 100 ) {
					if ( numm == table3[j] ) {
						temp.n6 = j ;
						break ;
					} // if 
					j++ ;
				} // while  
				temp.n5 = 3 ;
				
				t.str = "T" ;
				t.tableType = 0 ;
				temp.n7 = 0 ;
				t.tableValue = table0.size() ;
				temp.n8 = t.tableValue ;
				itoa(table0.size(), ch, 10) ;
				s = ch ;
				t.str = t.str + s ;
				itoa(temp.n6, ch, 10) ;
				s = ch ;
				temp.str = t.str + "=" + n34.str + "*" + s ;
				t.isArrayOne = false ;
				t.isArrayTwo = false ;
				table0.push_back(t) ;
				table6.push_back(temp) ;
				//----------------------------------------------------
				
				temp.n1 = 1 ;// + 
				temp.n2 = 5 ;
				n34 = t ;
				temp.n3 = n34.tableType ;
				temp.n4 = n34.tableValue ;
				n56 = arrayTable.at(operand2.pointAT).index1 ;
				temp.n5 = n56.tableType ;
				temp.n6 = n56.tableValue ;
				t.str = "T" ;
				t.tableType = 0 ;
				temp.n7 = 0 ;
				t.tableValue = table0.size() ;
				temp.n8 = t.tableValue ;
				itoa(table0.size(), ch, 10) ;
				s = ch ;
				t.str = t.str + s ;
				itoa(temp.n6, ch, 10) ;
				s = ch ;
				temp.str = t.str + "=" + n34.str + "+" + n56.str ;
				t.isArrayOne = false ;
				t.isArrayTwo = false ;
				table0.push_back(t) ;
				table6.push_back(temp) ;
				//----------------------------------------------------
				
				temp.n1 = 1 ; // =
				temp.n2 = 4 ;
				n34 = arrayTable.at(operand2.pointAT).array ;
				temp.n3 = n34.tableType ;
				temp.n4 = n34.tableValue ;
				n56 = t ;
				temp.n5 = n56.tableType ;
				temp.n6 = n56.tableValue ;
				t.str = "T" ;
				t.tableType = 0 ;
				temp.n7 = 0 ;
				t.tableValue = table0.size() ;
				temp.n8 = t.tableValue ;
				itoa(table0.size(), ch, 10) ;
				s = ch ;
				t.str = t.str + s ;
				itoa(temp.n6, ch, 10) ;
				s = ch ;
				temp.str = t.str + "=" + n34.str + "(" + n56.str + ")" ;
				t.isArrayOne = false ;
				t.isArrayTwo = false ;
				table0.push_back(t) ;
				table6.push_back(temp) ;
				
			} // if  �G���}�C 
			else { // �@���}�C 
				temp.n1 = 1 ; // =
				temp.n2 = 4 ;
				n34 = arrayTable.at(operand2.pointAT).array ;
				temp.n3 = n34.tableType ;
				temp.n4 = n34.tableValue ;
				n56 = arrayTable.at(operand2.pointAT).index1 ;
				temp.n5 = n56.tableType ;
				temp.n6 = n56.tableValue ;
				temp.n7 = 0 ;
				temp.n8 = table0.size() ;
				t.str = "T" ;
				t.tableType = 0 ;
				t.tableValue = table0.size() ;
				itoa(table0.size(), ch, 10) ;
				s = ch ;
				t.str = t.str + s ;
				temp.str = t.str + "=" + n34.str + "(" + n56.str + ")" ;
				t.isArrayOne = false ;
				t.isArrayTwo = false ;
				table0.push_back(t) ;
				table6.push_back(temp) ;
			} // else �@���}�C 
			operand2 = t ;
			 
		} // if  �ݥͦ�T1�Ȯ��ܼ�  ���󻡳B�z�����ᵥ���k�䤣�|�Oarray�F 
		
		if ( operand1.tableType == 8 ) {
			if ( operand1.isArrayTwo ) { // �G���}�C 
				
				
				temp.n1 = 1 ; // - 
				temp.n2 = 6 ;
				n34 = arrayTable.at(operand1.pointAT).index2 ;
				temp.n3 = n34.tableType ;
				temp.n4 = n34.tableValue ;
				temp.n5 = 3 ; // 1
				temp.n6 = 49 ; 
				t.str = "T" ;
				t.tableType = 0 ;
				temp.n7 = 0 ;
				t.tableValue = table0.size() ;
				temp.n8 = t.tableValue ;
				itoa(table0.size(), ch, 10) ;
				s = ch ;
				t.str = t.str + s ;
				temp.str = t.str + "=" + n34.str + "-1" ;
				t.isArrayOne = false ;
				t.isArrayTwo = false ;
				table0.push_back(t) ;
				table6.push_back(temp) ;
				//----------------------------------------------------------
				temp.n1 = 1 ; // *
				temp.n2 = 7 ;
				n34 = t ;
				temp.n3 = t.tableType ;
				temp.n4 = t.tableValue ;
				n56 = arrayTable.at(operand1.pointAT).array ;
				i = 0 ;
				int index = 0 ;
			while ( i < 1000 ) { // table5
					if ( table5[i].str == n56.str ) {
						
						index = table5[i].pointerIndex ; // ArrayName
						break ;
					} // if 
					i++ ;
				} // while 

				index = index + 2 ;
				char number[100] ;
				itoa( table7.at(index), number, 10);
				string numm = number ;
				int j = 0 ;
				while ( j < 100 ) {
					if ( numm == table3[j] ) {
						temp.n6 = j ;
						break ;
					} // if 
					j++ ;
				} // while  
				temp.n5 = 3 ;
				
				t.str = "T" ;
				t.tableType = 0 ;
				temp.n7 = 0 ;
				t.tableValue = table0.size() ;
				temp.n8 = t.tableValue ;
				itoa(table0.size(), ch, 10) ;
				s = ch ;
				t.str = t.str + s ;
				itoa(temp.n6, ch, 10) ;
				s = ch ;
				temp.str = t.str + "=" + n34.str + "*" + s ;
				t.isArrayOne = false ;
				t.isArrayTwo = false ;
				table0.push_back(t) ;
				table6.push_back(temp) ;
				//----------------------------------------------------
				
				temp.n1 = 1 ;// + 
				temp.n2 = 5 ;
				n34 = t ;
				temp.n3 = n34.tableType ;
				temp.n4 = n34.tableValue ;
				n56 = arrayTable.at(operand1.pointAT).index1 ;
				temp.n5 = n56.tableType ;
				temp.n6 = n56.tableValue ;
				t.str = "T" ;
				t.tableType = 0 ;
				temp.n7 = 0 ;
				t.tableValue = table0.size() ;
				temp.n8 = t.tableValue ;
				itoa(table0.size(), ch, 10) ;
				s = ch ;
				t.str = t.str + s ;
				itoa(temp.n6, ch, 10) ;
				s = ch ;
				temp.str = t.str + "=" + n34.str + "+" + n56.str ;
				t.isArrayOne = false ;
				t.isArrayTwo = false ;
				table0.push_back(t) ;
				table6.push_back(temp) ;
				// �����@���F				
			} // if 
			n78 = t ;
			if ( operand1.isArrayOne ) { // ��n78 
				n78 = arrayTable.at(operand1.pointAT).index1 ;
			} // if 
			temp.n1 = 1 ; // =
			temp.n2 = 4 ;
			n34 = operand2 ;
			temp.n3 = n34.tableType ;
			temp.n4 = n34.tableValue ;
			n56 = arrayTable.at(operand1.pointAT).array ;
			temp.n5 = n56.tableType ;
			temp.n6 = n56.tableValue ;
			temp.n7 = n78.tableType ;
			temp.n8 = n78.tableValue ;
			temp.str = n56.str + "(" + n78.str + ") = " + n34.str ;
			table6.push_back(temp) ;
		} // if 
		else { // operand1 != array 
			temp.n1 = 1 ;
			temp.n2 = 4 ;
			n34 = operand2 ;
			temp.n3 = n34.tableType ;
			temp.n4 = n34.tableValue ;
			temp.n5 = -1 ;
			temp.n6 = -1 ;
			n78 = operand1 ;
			temp.n7 = n78.tableType ;
			temp.n8 = n78.tableValue ;
			temp.str = n78.str + "=" + n34.str ;
			table6.push_back(temp);
		} // else 		
	} // if  ���󪺱��p
	else { // +-*/�����p 
		if ( operand2.tableType == 8 ) { // �ݥͦ�T1�Ȯ��ܼ�  ���󻡳B�z�����ᵥ���k�䤣�|�Oarray�F 
			if ( operand2.isArrayTwo ) {// �G���}�C
				temp.n1 = 1 ; // - 
				temp.n2 = 6 ;
				n34 = arrayTable.at(operand2.pointAT).index2 ;
				temp.n3 = n34.tableType ;
				temp.n4 = n34.tableValue ;
				temp.n5 = 3 ; // 1
				temp.n6 = 49 ; 
				t.str = "T" ;
				t.tableType = 0 ;
				temp.n7 = 0 ;
				t.tableValue = table0.size() ;
				temp.n8 = t.tableValue ;
				itoa(table0.size(), ch, 10) ;
				s = ch ;
				t.str = t.str + s ;
				temp.str = t.str + "=" + n34.str + "-1" ;
				t.isArrayOne = false ;
				t.isArrayTwo = false ;
				table0.push_back(t) ;
				table6.push_back(temp) ;
				//----------------------------------------------------------
				temp.n1 = 1 ; // *
				temp.n2 = 7 ;
				n34 = t ;
				temp.n3 = t.tableType ;
				temp.n4 = t.tableValue ;
				n56 = arrayTable.at(operand2.pointAT).array ;
				i = 0 ;
				int index = 0 ;
				while ( i < 1000 ) { // table5
					if ( table5[i].str == n56.str ) {
						
						index = table5[i].pointerIndex ; // ArrayName
						break ;
					} // if 
					i++ ;
				} // while 

				index = index + 2 ;
				char number[100] ;
				itoa( table7.at(index), number, 10);
				string numm = number ;
				int j = 0 ;
				while ( j < 100 ) {
					if ( numm == table3[j] ) {
						temp.n6 = j ;
						break ;
					} // if 
					j++ ;
				} // while  
				temp.n5 = 3 ;
				
				t.str = "T" ;
				t.tableType = 0 ;
				temp.n7 = 0 ;
				t.tableValue = table0.size() ;
				temp.n8 = t.tableValue ;
				itoa(table0.size(), ch, 10) ;
				s = ch ;
				t.str = t.str + s ;
				itoa(temp.n6, ch, 10) ;
				s = ch ;
				temp.str = t.str + "=" + n34.str + "*" + s ;
				t.isArrayOne = false ;
				t.isArrayTwo = false ;
				table0.push_back(t) ;
				table6.push_back(temp) ;
				//----------------------------------------------------
				
				temp.n1 = 1 ;// + 
				temp.n2 = 5 ;
				n34 = t ;
				temp.n3 = n34.tableType ;
				temp.n4 = n34.tableValue ;
				n56 = arrayTable.at(operand2.pointAT).index1 ;
				temp.n5 = n56.tableType ;
				temp.n6 = n56.tableValue ;
				t.str = "T" ;
				t.tableType = 0 ;
				temp.n7 = 0 ;
				t.tableValue = table0.size() ;
				temp.n8 = t.tableValue ;
				itoa(table0.size(), ch, 10) ;
				s = ch ;
				t.str = t.str + s ;
				itoa(temp.n6, ch, 10) ;
				s = ch ;
				temp.str = t.str + "=" + n34.str + "+" + n56.str ;
				t.isArrayOne = false ;
				t.isArrayTwo = false ;
				table0.push_back(t) ;
				table6.push_back(temp) ;
				//----------------------------------------------------
				
				temp.n1 = 1 ; // =
				temp.n2 = 4 ;
				n34 = arrayTable.at(operand2.pointAT).array ;
				temp.n3 = n34.tableType ;
				temp.n4 = n34.tableValue ;
				n56 = t ;
				temp.n5 = n56.tableType ;
				temp.n6 = n56.tableValue ;
				t.str = "T" ;
				t.tableType = 0 ;
				temp.n7 = 0 ;
				t.tableValue = table0.size() ;
				temp.n8 = t.tableValue ;
				itoa(table0.size(), ch, 10) ;
				s = ch ;
				t.str = t.str + s ;
				itoa(temp.n6, ch, 10) ;
				s = ch ;
				temp.str = t.str + "=" + n34.str + "(" + n56.str + ")" ;
				t.isArrayOne = false ;
				t.isArrayTwo = false ;
				table0.push_back(t) ;
				table6.push_back(temp) ;
				
			} // if  �G���}�C 
			else { // �@���}�C 
				temp.n1 = 1 ; // =
				temp.n2 = 4 ;
				n34 = arrayTable.at(operand2.pointAT).array ;
				temp.n3 = n34.tableType ;
				temp.n4 = n34.tableValue ;
				n56 = arrayTable.at(operand2.pointAT).index1 ;
				temp.n5 = n56.tableType ;
				temp.n6 = n56.tableValue ;
				temp.n7 = 0 ;
				temp.n8 = table0.size() ;
				t.str = "T" ;
				t.tableType = 0 ;
				t.tableValue = table0.size() ;
				itoa(table0.size(), ch, 10) ;
				s = ch ;
				t.str = t.str + s ;
				temp.str = t.str + "=" + n34.str + "(" + n56.str + ")" ;
				t.isArrayOne = false ;
				t.isArrayTwo = false ;
				table0.push_back(t) ;
				table6.push_back(temp) ;
			} // else �@���}�C 
			operand2 = t ;
			 
		} // if  �ݥͦ�T1�Ȯ��ܼ�  ���󻡳B�z�����ᵥ���k�䤣�|�Oarray�F 
		if ( operand1.tableType == 8 ) { // �ݥͦ�T1�Ȯ��ܼ�
			if ( operand1.isArrayTwo ) {// �G���}�C
				temp.n1 = 1 ; // - 
				temp.n2 = 6 ;
				n34 = arrayTable.at(operand1.pointAT).index2 ;
				temp.n3 = n34.tableType ;
				temp.n4 = n34.tableValue ;
				temp.n5 = 3 ; // 1
				temp.n6 = 49 ; 
				t.str = "T" ;
				t.tableType = 0 ;
				temp.n7 = 0 ;
				t.tableValue = table0.size() ;
				temp.n8 = t.tableValue ;
				itoa(table0.size(), ch, 10) ;
				s = ch ;
				t.str = t.str + s ;
				temp.str = t.str + "=" + n34.str + "-1" ;
				t.isArrayOne = false ;
				t.isArrayTwo = false ;
				table0.push_back(t) ;
				table6.push_back(temp) ;
				//----------------------------------------------------------
				temp.n1 = 1 ; // *
				temp.n2 = 7 ;
				n34 = t ;
				temp.n3 = t.tableType ;
				temp.n4 = t.tableValue ;
				n56 = arrayTable.at(operand1.pointAT).array ;
				i = 0 ;
				int index = 0 ;
				while ( i < 1000 ) { // table5
					if ( table5[i].str == n56.str ) {
						
						index = table5[i].pointerIndex ; // ArrayName
						break ;
					} // if 
					i++ ;
				} // while 

				index = index + 2 ;
				char number[100] ;
				itoa( table7.at(index), number, 10);
				string numm = number ;
				int j = 0 ;
				while ( j < 100 ) {
					if ( numm == table3[j] ) {
						temp.n6 = j ;
						break ;
					} // if 
					j++ ;
				} // while  
				temp.n5 = 3 ;

				
				t.str = "T" ;
				t.tableType = 0 ;
				temp.n7 = 0 ;
				t.tableValue = table0.size() ;
				temp.n8 = t.tableValue ;
				itoa(table0.size(), ch, 10) ;
				s = ch ;
				t.str = t.str + s ;
				itoa(temp.n6, ch, 10) ;
				s = ch ;
				temp.str = t.str + "=" + n34.str + "*" + table3[j] ;
				t.isArrayOne = false ;
				t.isArrayTwo = false ;
				table0.push_back(t) ;
				table6.push_back(temp) ;
				//----------------------------------------------------
				
				temp.n1 = 1 ;// + 
				temp.n2 = 5 ;
				n34 = t ;
				temp.n3 = n34.tableType ;
				temp.n4 = n34.tableValue ;
				n56 = arrayTable.at(operand1.pointAT).index1 ;
				temp.n5 = n56.tableType ;
				temp.n6 = n56.tableValue ;
				t.str = "T" ;
				t.tableType = 0 ;
				temp.n7 = 0 ;
				t.tableValue = table0.size() ;
				temp.n8 = t.tableValue ;
				itoa(table0.size(), ch, 10) ;
				s = ch ;
				t.str = t.str + s ;
				itoa(temp.n6, ch, 10) ;
				s = ch ;
				temp.str = t.str + "=" + n34.str + "+" + n56.str ;
				t.isArrayOne = false ;
				t.isArrayTwo = false ;
				table0.push_back(t) ;
				table6.push_back(temp) ;
				//----------------------------------------------------
				
				temp.n1 = 1 ; // =
				temp.n2 = 4 ;
				n34 = arrayTable.at(operand1.pointAT).array ;
				temp.n3 = n34.tableType ;
				temp.n4 = n34.tableValue ;
				n56 = t ;
				temp.n5 = n56.tableType ;
				temp.n6 = n56.tableValue ;
				t.str = "T" ;
				t.tableType = 0 ;
				temp.n7 = 0 ;
				t.tableValue = table0.size() ;
				temp.n8 = t.tableValue ;
				itoa(table0.size(), ch, 10) ;
				s = ch ;
				t.str = t.str + s ;
				itoa(temp.n6, ch, 10) ;
				s = ch ;
				temp.str = t.str + "=" + n34.str + "(" + n56.str + ")" ;
				t.isArrayOne = false ;
				t.isArrayTwo = false ;
				table0.push_back(t) ;
				table6.push_back(temp) ;
				
			} // if  �G���}�C 
			else { // �@���}�C 
				temp.n1 = 1 ; // =
				temp.n2 = 4 ;
				n34 = arrayTable.at(operand1.pointAT).array ;
				temp.n3 = n34.tableType ;
				temp.n4 = n34.tableValue ;
				n56 = arrayTable.at(operand1.pointAT).index1 ;
				temp.n5 = n56.tableType ;
				temp.n6 = n56.tableValue ;
				temp.n7 = 0 ;
				temp.n8 = table0.size() ;
				t.str = "T" ;
				t.tableType = 0 ;
				t.tableValue = table0.size() ;
				itoa(table0.size(), ch, 10) ;
				s = ch ;
				t.str = t.str + s ;
				temp.str = t.str + "=" + n34.str + "(" + n56.str + ")" ;
				t.isArrayOne = false ;
				t.isArrayTwo = false ;
				table0.push_back(t) ;
				table6.push_back(temp) ;
			} // else �@���}�C 
			operand1 = t ;			 
		} // if  �ݥͦ�T1�Ȯ��ܼ�  
		// ���`�[��� 
		temp.n1 = operator1.tableType ;
		temp.n2 = operator1.tableValue ;
		n34 = operand1 ;
		temp.n3 = operand1.tableType ;
		temp.n4 = operand1.tableValue ;
		n56 = operand2 ;
		temp.n5 = operand2.tableType ;
		temp.n6 = operand2.tableValue ;
		t.str = "T" ;
		t.tableType = 0 ;
		temp.n7 = 0 ;
		t.tableValue = table0.size() ;
		temp.n8 = t.tableValue ;
		itoa(table0.size(), ch, 10) ;
		s = ch ;
		t.str = t.str + s ;
		temp.str = t.str + "=" + n34.str + " + " + n56.str ;
		t.isArrayOne = false ;
		t.isArrayTwo = false ;
		table0.push_back(t) ;
		table6.push_back(temp) ;		
	} // else 

} // ���ͦ�array��assignment�������X 

void Syntax::CheckLevel( Token &t ) { // �T�{delimeter���j�p  
	if ( t.str == "(" ) t.level = 6 ;
	else if ( t.str == "^" ) t.level = 5 ;
	else if ( t.str == "*" ) t.level = 4 ;
	else if ( t.str == "/" ) t.level = 4 ;
	else if ( t.str == "+" ) t.level = 3 ;
	else if ( t.str == "-" ) t.level = 3 ;
	else if ( t.str == ")" ) t.level = 2 ;
	else if ( t.str == "=" ) t.level = 1 ;
} // �T�{delimeter���j�p 
void Syntax::GenerateCode( Token operator1, Token operand1, Token operand2 ) {  // ���ͫ��woperand/operator�������X 
	Table6 temp ;
	Token var ; // �Ȯ��ܼ� 
	char ch[100] ;
	string statement = "" ;
	temp.n1 = operator1.tableType ;
	temp.n2 = operator1.tableValue ;
	temp.n3 = operand1.tableType ;
	temp.n4 = operand1.tableValue ;
	temp.n5 = operand2.tableType ;
	temp.n6 = operand2.tableValue ;
	var.str = "T" ;
	var.tableType = 0 ;
	var.tableValue = table0.size() ;
	temp.n7 = 0 ;
	temp.n8 =  var.tableValue ;
	itoa( var.tableValue, ch,  10 ) ;
	string s = ch ;
	var.str = var.str + s ;
	statement = operand1.str + operator1.str + operand2.str ;

	
	temp.str =  var.str + "=" + statement ;
	table6.push_back(temp) ;
	table0.push_back(var) ;
} // ���ͫ��woperand/operator�������X 
void Syntax::GenerateCALL( int statementStart, int a ) { // ����CALL�������X 
	Table6 temp ;
	string str = "" ;
	int i = statementStart ; // statement�_�l��m 
	
	str = str + token.at(i).str ;
	temp.n1 = 2 ; // CALL
	temp.n2 = 3 ;
	i++ ;
	while ( token.at(i).str == " " ) { // �����ť� 
		str = str + token.at(i).str ;
		i++ ;
	} // while 
	str = str + token.at(i).str ;
	temp.n3 = token.at(i).tableType ; // SI
	temp.n4 = token.at(i).tableValue ;
	i++ ;
	temp.n5 = -1 ; // �� 
	temp.n6 = -1 ; // �� 
	temp.n7 = 7 ; // ���Vtable7 
	temp.n8 = all.at(a).entry7 ;
	
	while ( token.at(i).str != ";" ) {
		str = str + token.at(i).str ;
		i++ ;
	} // while 
	i++ ;
	while ( token.at(i).str != "\n" ) {
		i++ ;
	} // while
	temp.str = str ;
	table6.push_back(temp) ; 
} // ���� CALL�������X 
void Syntax::GenerateGTO( int statementStart ) { // ����GTO�������X

	Table6 temp ;
	Label l ;
	string str = "" ;
	int i = statementStart ; // statement�_�l��m 
    string labelName = "" ;
	while( token.at(i).tableType != 5 ) {
		str = str + token.at(i).str ;
		i++ ;
	} // while 
	labelName = token.at(i).str ;
	str = str + token.at(i).str ;
	temp.n1 = 2 ; 
	temp.n2 = 11 ;
	temp.n3 = -1 ;
	temp.n4 = -1 ;
	temp.n5 = -1 ;
	temp.n6 = -1 ;
	temp.n7 = 6 ;
	temp.n8 = -1 ;
	temp.str = str ;
	l.line = table6.size() ;
	l.str = labelName ;
	gto.push_back(l) ;
	table6.push_back(temp) ;
	str = "" ;
	temp.str = "" ;
				
} // ����GTO�������X 
void Syntax::GenerateStatement( int statementStart, int statementEnd, int a, bool &isGTO  ) { // ����statement�������X  �I�sgenerateGTO/ CALL / assignment()
	Token end ; // ���� 
	end.str = ";" ;
	end.tableType = 1 ;
	end.tableValue = 1 ; 
	bool isCALL = false ;
	int i = statementStart ;
	while( token.at(i).str != "ELSE" && token.at(i).str != ";" ) {
		if ( token.at(i).tableType == 2 ) {
			if ( token.at(i).tableValue == 11) { // GTO
				isGTO = true ;
			} // if 
			else if ( token.at(i).tableValue == 3 ) { // CALL
				isCALL = true ;
			} // else if 
		} // if 
		i++ ;
	} // while
	token.at(i) = end ; // �����������A�h����
	if ( isGTO ) {
		GenerateGTO( statementStart ) ;
	} // if 
	else if ( isCALL ) {
		GenerateCALL( statementStart, a ) ;
	} // else if 
	else {
		GenerateAssignment( statementStart ) ;
	} // else 
	 
} // ����Statement�������X 
// end class Syntax �P�_��k======================================================================================================================
class Lex{ // start ��Token ==========================================================================================================
	string table1[12] ; // delimiter table
	string table2[25] ; // reserve word table
	
	string table4[100] ; // real number table
	
	vector<string> inputFile ; // Ū�J���ɮ� 
	 
public:
	bool ReadFile() ; // Ūinput��
	void ReadTable(string number) ; 
	void GetLine() ; // �C�����@��h��Token <<�I�s LexicalAnalysis()>>
	void LexicalAnalysis(string line, int &subroutine ) ; // ���o�檺Token 
	bool IsDelimeter( char ch ) ; // �P�_�O�_��delimeter 
	void CheckWhichTable ( Token &temp, int &subroutine ) ; // �P�_�b����table���Y���ݭn�h��J�۹�����table��
	void Convert2BigString( string in , string &out ) ; // �ন�j�g 
}; 
void Lex::Convert2BigString( string in , string &out ) {
	int i = 0 ;
	out = in ;
	while ( i < in.length() ) {
		if ( in[i] >= 'a'&& in[i] <= 'z' ) {
			out[i] = in[i] - 32 ;
		} // if 
		i++ ;
	} // while
} // �ন�j�g 
bool Lex::ReadFile() { // Ūinput�� 
	FILE *file = NULL ; 
	cout << "�п�J�ɮצW��: \n" ; 
	cin >> fileName ;
	
	string oriFileName = fileName + ".txt" ;

    file = fopen( oriFileName.c_str(), "r" ) ; // open the file
	if ( file == NULL ) {
		cout << "�ɮפ��s�b!\n" ;
		fclose( file ) ; 
		return false ;
	}
	else {
		bool done = false ; // �O�_�w��Jvector 
		char ch = '\0' ;
		string temp = "" ; // �ȦsŪ�J���F��	
		while ( fscanf(file, "%c", &ch ) != EOF ) {
			done = false ;
			if ( ch != '\n' ){
				temp = temp + ch ;
			} // if 
			else {
				inputFile.push_back(temp) ;
				done = true ;
				temp = "" ; 
			} // else 			
		} // while  
		if ( !done ) {
		   inputFile.push_back(temp) ; 	
		} // if	
		fclose( file ) ; 	
		return true ;
	} // else 		
} // ReadFile Ū�� 
void Lex::ReadTable( string number ) { // Ūtable 
	FILE *file = NULL ; 
	
	string oriFileName = "table" + number + ".table" ;

    file = fopen( oriFileName.c_str(), "r" ) ; // open the file
	if ( file == NULL ) {
		cout << "�ɮפ��s�b!\n" ;
		fclose( file ) ; 
	} // if
	else {	
		bool done = false ;
		int i = 0 ; // �]table��index	
		char ch = '\0' ;
		string temp = "" ; // �ȦsŪ�J���F��	
		while ( fscanf(file, "%c", &ch ) != EOF ) { // Ūchar����Ū���F 
		    done = false ;
			if ( ch != '\n' ){
				temp = temp + ch ;
			} // if 
			else {
				done = true ;
				if ( number == "1" ) { // table1���F�� 
					table1[i] = temp ;
				} // if 
				else if ( number == "2" ) { // table2���F�� 
					table2[i] = temp ;
				} // else if 
				temp = "" ; 
				i++ ;
			} // else 			
		} // while  
		if ( !done ) {
		    if ( number == "1" ) { // table1���F�� 
				table1[i] = temp ;
			} // if 
			else if ( number == "2" ) { // table2���F�� 
				table2[i] = temp ;
			} // else if 
		} // if			
		fclose( file ) ; 	
	} // else 		
} // ReadTable()Ūtable1��table2 
void Lex::GetLine() { // �C�����@��h��token  <<�I�s LexicalAnalysis()>>
	int i = 0 ; // �]inputFile��index
	Token temp ;
	temp.str = "\n" ;
	temp.tableType = -1 ;
	temp.tableValue = -1 ;
	int subroutine = -1 ;
	while( i < inputFile.size() ) {
		LexicalAnalysis( inputFile[i], subroutine ) ;
		token.push_back(temp) ; // �񴫦�i�h 
		i++ ;
	} // while 
} // GetLine() �C�����@��h��token  <<�I�s LexicalAnalysis()>>
void Lex::LexicalAnalysis( string line, int &subroutine ) { // ���o�檺Token 
	int i = 0 ; // �Ψӷ�line��index
	Token temp ;// �ΨӼȦstoken
	Token a ;
	a.str = "" ;
	a.tableType = -1 ;
	a.tableValue = -1 ;

	temp = a ;// ��temp �^�k�Ū� 
	while ( i < line.size() ) {
		if ( line[i] == ' ' || line[i] == '\t' || line[i] == '\n' ) { // �J��White Space 
			 if ( temp.str != "" ) { // temp���F��  
			     CheckWhichTable( temp, subroutine ) ;  // �P�_�b����table���Y���ݭn�h��J�۹�����table�� 
			     token.push_back(temp) ; // ��JToken��vector�� 
			     temp = a ;
			 } // if 
			 if ( line[i] != '\n' ) {
		         temp.str = " " ;
				 temp.tableType = -1 ;
				 temp.tableValue = -1 ;
				 token.push_back(temp) ; // �N"�ť�"��JToken��vector�� 
				 temp = a ;
			 } // if 
				 
		} // if 
		else if ( IsDelimeter( line[i] ) ) { // �Y�Odelineter 
			if ( temp.str != "" ) { // temp���F��  
			     CheckWhichTable( temp, subroutine ) ;  // �P�_�b����table���Y���ݭn�h��J�۹�����table��
			     token.push_back(temp) ; // ��JToken��vector�� 
			     temp = a ; // ��temp �^�k�Ū� 
			} // if
			
			temp.str = temp.str + line[i] ; // ��delimeter��Jtable�� 
			CheckWhichTable( temp, subroutine ) ;
			token.push_back(temp) ; // ��JToken��vector�� 
			temp = a ;
		} // else if 
		else { // ��L���p�h��Jtemp�� 
			temp.str = temp.str + line[i] ;			
		} // else 
		i++ ;
	} // while 
	if ( temp.str != "" ) { // temp���F��  
	     CheckWhichTable( temp, subroutine ) ;  // �P�_�b����table���Y���ݭn�h��J�۹�����table�� 
	     token.push_back(temp) ; // ��JToken��vector�� 
	     temp = a ;
	 } // if  
} // LexicalAnalysis() 
bool Lex::IsDelimeter( char ch ) { // �P�_�O�_��Delimeter 
	int i = 0 ; 
	string str = "" ;
	str = str + ch ; 
	while ( i < 12 ) {
		if ( str == table1[i] ) {
			return true ;
		} // if 
	    i++ ;
	} // while
	return false ; 
} // IsDelimeter
void Lex::CheckWhichTable( Token &temp, int &subroutine ) { // �P�_�b����table���Y���ݭn�h��J�۹�����table��
	int i = 0 ;
	bool done = false ; // �O�_�P�_���� 
	string bigStr = "" ;
	Convert2BigString( temp.str, bigStr ) ;
	while ( i < 12 ) { // �P�_table1 
		if ( bigStr == table1[i] ) {
			temp.tableType = 1 ;
			temp.tableValue = i+1 ;
			done = true ;
			break ;
		}  // if 
		i++ ;
	} // while 
	if ( done == false ) {
		i = 0 ;
		while ( i < 25 ) { // �P�_table2 
			if ( bigStr == table2[i] ) {
				temp.tableType = 2 ;
				temp.tableValue = i+1 ;
				done = true ;
				if ( bigStr == "PROGRAM" ) {
					subroutine = i ;
				} // if 
				else if ( bigStr == "SUBROUTINE" ) {
					subroutine = i ;
				} // else if 
				break ;
			}  // if 
			i++ ;
		} // while 
	} // if 
	if ( done == false ) {
		i = 0 ;
		int ascii = 0 ;
		bool isNum = true ;
		bool isInteger = true ;
		while ( i < temp.str.size() ) { // ���j��t�d�P�_�O�_��integer��real number (���p���I) 
			ascii = ascii + (int) temp.str[i] ;
			if ( temp.str[i] == '.' ) {
				isInteger = false ;
			} // if 
			else if ( temp.str[i] >= '0' && temp.str[i] <= '9' ) {
				;
			} // else if 
			else {
				isNum = false ;
				isInteger = false ;
			} // else 
			i++ ;
		} // while 
		
		int index = ascii % 100 ;
		if ( isNum && isInteger ) { // �OTable3 integer
			temp.tableType = 3 ;
			while ( table3[index] != "" ) {
				if ( table3[index] == temp.str ) {
					break ;
				} // if
				index++ ;
			} // while
			table3[index] = temp.str ;
			temp.tableValue = index ;
		} // if Table3
		else if ( isNum ) { // �OTable4 real
			temp.tableType = 4 ;
			while ( table4[index] != "" ) {
				if ( table4[index] == temp.str ) {
					break ;
				} // if 
				index++ ;
			} // while
			table4[index] = temp.str ;
			temp.tableValue = index ;
		} // else if Table4
		else if ( !isNum && !isInteger ) {// �OTable5 identifier
			temp.tableType = 5 ;
			
			while ( table5[index].str != "" ) {
				if ( table5[index].str == temp.str && table5[index].subroutine == subroutine ) { // �J��ۦP���N���L���\�i�h subroutine���ۦP 
					break ;
				} // if
				index++ ;
			} // while
			table5[index].str = temp.str ;
			table5[index].subroutine = subroutine ;
			temp.tableValue = index ;
		} // else if Table5 
	} // if  
} // CheckWhichTable
// end class Lex ��Token======================================================================================================================
int main(int argc, char** argv) {
	Lex l ; 
	cout << "*********************************\n" ;
	cout << "*********************************\n" ;
	cout << "*****       �w����{      *******\n" ;
	cout << "*****   FRANCIS COMPILER  *******\n" ;
	cout << "*********************************\n" ;
	cout << "*********************************\n" ;
	cout << "*********************************\n\n" ;
	
	while ( !l.ReadFile() ) ;// Ū�� 
	l.ReadTable("1") ; // Ūtable1 
	l.ReadTable("2") ; // Ūtable2 
	l.GetLine() ;
	
	//�P�_Token��Vector���S�����T 
	/*int i = 0 ;
	while ( i < token.size() ) {
		if ( token[i].str == "\n" ) {
			cout << "\n" ;
		}
		cout << token[i].str << "( " << token.at(i).tableType << ", " << token[i].tableValue << ")	\n" ;
		i++ ;
	} // while */
	table7.push_back(-1) ; // �]���q1�}�l��A�ҥH�@�}�l0����m����-1 
	Token z ;
	table0.push_back(z) ; // �]���q1�}�l��
	Syntax s ;
	s.GetLine() ;
	// �P�_SYNTAX���L���T 
	/*int j = 0 ;
	while (j < all.size()) {
		if ( all.at(j).error == true ) {
			cout << all.at(j).str << "	" << all.at(j).errormsg << "\n" ;
		} // if 
		else {
			cout << all.at(j).str << "\n" ;
		} // else 
		j++ ;
	} // while */
	// �P�_table7���L���T
	/*int k = 0 ;
	while ( k < table7.size()) {
		cout << table7.at(k) << "\n" ;
		k++ ;
	} // while*/
	s.GenerateIntermediate() ; // ���ͤ����X 
	// �P�_�����X���S���T
	/*int a = 0 ;
	while ( a < table6.size() ) {
		cout << "(" << "(" << table6.at(a).n1 << "," << table6.at(a).n2 
			 << ")" << "," << "(" << table6.at(a).n3 << "," << table6.at(a).n4 
		     << ")" << ",""(" << table6.at(a).n5 << "," << table6.at(a).n6 << ")" 
		     << ",""(" << table6.at(a).n7 << "," << table6.at(a).n8 << ")" << ")	" << table6.at(a).str << "\n" ;
		a++ ;
	}  // while */
	s.WriteOutput() ;
} // main()



void Syntax::WriteOutput ( ) {
	fstream file1 ;
	string fileOutput1 = fileName + "_output.txt" ; 
	file1.open( fileOutput1.c_str(), ios::app ) ; 
	file1.close() ; 
	
	fstream file ;
	string fileOutput = fileName + "_IntermediateCode.txt" ;  
	file.open( fileOutput.c_str(), ios::app ) ;
	int i = 0 ;
	ERROR er ;
	if ( !err.empty() ) {
		er = err.front() ;
	} // if 
	while ( i < table6.size() ) {
		
		if ( i == er.line ) { // ��error 
			file << er.errormsg << "\n" ;
			err.pop() ;
			i-- ;
			if ( !err.empty() ) {
				er = err.front() ;
				//err.pop() ;
			} // if 
			else {
				er.line = -1 ;
			} // else
		} // if 
		else {
			
			
			file << i+1 <<"	(" ;
			if ( table6.at(i).n1 == -1 ) {
				file << "	," ;
			} // if 
			else {
				file << "(" << table6.at(i).n1 << "," << table6.at(i).n2 << ")	" << "," ;
			} // else 
			if ( table6.at(i).n3 == -1 ) {
				file << "	," ;
			} // if 
			else {
				file << "(" << table6.at(i).n3 << "," << table6.at(i).n4 << ")	" << "," ;
			} // else 
			if ( table6.at(i).n5 == -1 ) {
				file << "	," ;
			} // if 
			else {
				file << "(" << table6.at(i).n5 << "," << table6.at(i).n6 << ")	" << "," ;
			} // else 
			if ( table6.at(i).n7 == -1 ) {
				file << "	)" ;
			} // if 
			else {
				file << "(" << table6.at(i).n7 << "," << table6.at(i).n8 << ")" << "	)"  ;
			} // else 
			
			file << "	" << table6.at(i).str << "\n" ;		
		} // else 
		i++ ;		
	} // while 
	while ( !err.empty() ) {
		er = err.front() ;
		file << er.errormsg << "\n" ;
		err.pop() ;
	} // while
	file.close() ; 
} // WriteOutput()

