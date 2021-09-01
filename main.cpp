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

struct Line{ // 每一行 
	bool error ; // 若文法有誤 則為true 
	string errormsg ; // 錯誤訊息 
	string str ; // 整行statement
	int line ; // 在第幾行 (輸出需要)  
	int entry7 ; // 指向table7的第幾個entry  (CALL之類的可能會用到 因為可以CALL 同個Fn很多次) 
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
	int pointerTable ; // 指向第幾個TABLE 
	int pointerIndex ; // 指向此table的第幾個(table皆由1算起) 目前沒用 
};
struct Label{
	string str ;
	int line ; // 在table6的第幾行 
};
struct Token{ // 每一個Token的資訊 	
	string str ;
	int tableType ;
	int tableValue ;
	int level ; // 專門給delimeter來比大小用的 
	bool isArrayOne ; // 是否為一維陣列 
	bool isArrayTwo ; // 是否為二維陣列 
	int pointAT ; // 指向arrayTable的該陣列的entry 
};
struct ArrayTable{
	Token array ; // 陣列
	Token index1 ; // 其index 
	Token index2 ;  // 第二個index (此為二維陣列) 
	bool two_dimention ; // 是否為二維陣列 
};
struct ERROR{
	string errormsg ;
	int line = -1 ;
};
static string fileName ; // 檔案名稱 
static vector<Token> token ; // 切好的token之 vector 
static vector<Line> all; // 去判斷Syntax的vector
static vector<string> variable ; // 用來判斷有無"尚未定義"的變數 
static vector<string> label ; // LABEL&GTO 判斷有無undefined symbol
static vector<int> table7 ; // imformation table 
static vector<Table6> table6 ; // 中間碼 
static vector<Label> gto ; // 給GTO看Label在table6中的第幾行 
static vector<Token> table0 ; // 放暫時變數T1....
static vector<Token> condi ; // 放condition &statement
static stack<Token> op1 ; 
static stack<Token> op2 ; 
static Table5 table5[1000] ; // identifier table
static vector <ArrayTable> arrayTable ; // 暫存ARRAY資訊 
static string table3[100] ; // integer table
//static vector<Token> labelTable ; // 已給address之Label 
static vector<Label> labelTable ; // 已給address之Label 
static queue<ERROR> err ;
// =================================================================================================================
class Syntax{ // start 確認文法是否正確  =============================================================================================================
public:	
	void GetLine() ; // 每次取一行去判斷文法 
	void SyntaxAnalysis(int index, int startLine, int endLine) ; // // 傳此行所在(all中)的index 及此行第一個/最後一個token所在token中(vector)的位址 
    bool IsDataType ( string str ) ; // 判斷str是否為'type'如ARRAY, BOOLEAN...等 
	void Convert2BigString( string in , string &out ) ; // 轉成大寫 
	int CheckDataType( string dataType) ; // 回傳其dataType編號 
	void GenerateIntermediate() ; // 產生中間碼 
	bool CheckCondition( ) ; // 判斷condi是否正確 
	bool CheckStatement( int index ) ; // 判斷statemet是否正確 		
	void GenerateCode( Token operator1, Token operand1, Token operand2 ) ; // 產生指定operand/operator之中間碼 
	void GenerateCondition() ; // 產生condi的中間碼 	
	void GenerateCALL( int statementStart, int a ) ; // 產生CALL之中間碼 
	void GenerateGTO( int statementStart ) ; // 產生GTO之中間碼 
	void GenerateAssignment( int statementStart ) ; // 產生assignment之中間碼 i為跑token之index 
	void CheckLevel( Token &t ) ; // 判斷delimeter之大小 會存入level中 數字越大代表等級越高(越大) 
	void GenerateStatement( int statementStart, int statementEnd, int a, bool &isGTO ) ; // 產生statement之中間碼 
	void GenerateArray(Token operator1, Token operand1, Token operand2 ) ; // 產生有關array之assignment之中間碼 
	void WriteOutput ( ) ;
};

void Syntax::GetLine() { // 取一行去判斷文法 
	int i = 0 ; // 跑token用 
	int index = 0 ; // 跑all用 
	int startLine = 0 ; // 記錄此行的第一個token所在(token內的)位址 
	int endLine = 0 ; // 記錄此行的最後一個token(也就是換行)所在(token內的)位址 
	bool hasSemicolon = false ; // 是否遇到分號 
	Line temp ;
	
	while ( i < token.size() ) {
		temp.str = temp.str + token.at(i).str ;
		if ( token.at(i).str == "\n" ) { // 遇到完整一行的最後一token 
			all.push_back(temp) ;
			if ( hasSemicolon == false ) { // 整行statement沒有分號 
				all.at(index).error = true ;
				all.at(index).errormsg = "最後字元非結束指令 ';'" ;
			} // if 
			else { // 是完整statement 要進入其他文法判斷 
				endLine = i ;
				SyntaxAnalysis(index, startLine, endLine) ; // 傳此行所在(all中)的index 及此行第一個/最後一個token所在token中(vector)的位址 
			} // else 
			hasSemicolon = false ; // init
			index++ ; // 換下一行囉 
			temp.str = "" ; // init
			startLine = i + 1 ; // 下一行第一個Token所在token(vector)中的位址 
		} // if 
		else if ( token.at(i).str == ";" ) { // 遇到分號 (沒有分號直接為error) 
			hasSemicolon = true ;
		} // else if 
		
		i++ ;
	} // while 
} // end GetLine()取一行去判斷文法 
void Syntax::SyntaxAnalysis(int index, int startLine, int endLine ) { 
// index為此statement所在all(vector)中的位址 
// startLine 為 這整行第一個token在token(vector)中的位址
// endLine為最後一個 也就是"換行"所在位址 
	bool comma = false ; // 判斷是否遇到逗號 
    int i = startLine ;
    string dataType = "" ; // 存dataType  
    int countDimention = 1 ; // 該array是幾維的 
    int countArray = 1 ; // 計算此行有多少個array 
    all.at(index).error = false ; // init
    all.at(index).assignment = false ; // init 不為assignment 
    while ( i < endLine ) { // endLine 為換行所在位址 
    	while ( token.at(i).str == " " || token.at(i).str == "\t" ) {
    		i++ ;
		} // while 
    	if ( token.at(i).str == ";" ) { // 檢查分號後面是否有東西 
    		i++ ;
    		while ( token.at(i).str == " " ) { // 空白跳掉 
				i++ ;
			} // while
		
    		if ( token.at(i).str != "\n" ) {
				all.at(index).error = true ;
				all.at(index).errormsg = "';'後面不應接東西" ;
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
				while ( token.at(i).str == " " ) { // 空白跳掉 
					i++ ;
				} // while
		
				if ( token.at(i).tableType != 5 ) { // 後面非id 
					all.at(index).error = true ;
					all.at(index).errormsg = "PROGRAM後面非identifier" ;
					break ;
				} // if 
				i++ ;
				while ( token.at(i).str == " " ) { // 空白跳掉 
					i++ ;
				} // while
				if ( token.at(i).str != ";" ) {
					all.at(index).error = true ;
					all.at(index).errormsg = "PROGRAM後面錯誤(接太多東西)" ;
					break ; 
				} // if 
				i++ ;
				while ( token.at(i).str == " " ) { // 空白跳掉 
					i++ ;
				} // while
				if ( token.at(i).str != "\n" ) {
					all.at(index).error = true ;
					all.at(index).errormsg = "';'後面不應接東西" ;
					break ;
				} // if 
				break ;
			} // if (PROGRAM)-------------------------------------------------------------------------(PROGRAM)
			else if (token.at(i).tableValue == 23 ) { // SUBROUTINE
			// SUBROUTINE<identifier>(<parameter group>{,<parameter group>});
				i++ ;
				while ( token.at(i).str == " " ) { // 空白跳掉 
					i++ ;
				} // while
				if ( token.at(i).tableType != 5 ) { // 後面非id 
					all.at(index).error = true ;
					all.at(index).errormsg = "SUBROUTINE後面非identifier";
					break ;
				} // if 
				i++ ;
				while ( token.at(i).str == " " ) { // 空白跳掉 
					i++ ;
				} // while
				if (token.at(i).str != "(" ) {
					all.at(index).error = true ;
					all.at(index).errormsg = "SUBROUTINE<identifier>後面應接'('" ;
					break ;
				} // if 
				i++ ;
				while ( token.at(i).str == " " ) { // 空白跳掉 
					i++ ;
				} // while
				if ( !IsDataType(token.at(i).str) ) { // <parameter group>:=<type>:<parameter>{,<parameter>}
					all.at(index).error = true ;
					all.at(index).errormsg = "SUBROUTINE<identifier>( 後面應接'type'" ;
					break ;
				} // if 
				i++ ;
				while ( token.at(i).str == " " ) { // 空白跳掉 
					i++ ;
				} // while
				if ( token.at(i).str != ":" ) {
					all.at(index).error = true ;
					all.at(index).errormsg = "SUBROUTINE<identifier>(<type> 後應接':'" ;
					break ;
				} // if 
				i++ ;
				while ( token.at(i).str == " " ) { // 空白跳掉 
					i++ ;
				} // while
				if ( token.at(i).tableType != 5 ) { // 非id 多判斷一次 避免冒號:後面直接是分號; 
		        	all.at(index).error = true ;
					all.at(index).errormsg = "SUBROTINE的<parameter group>中的<parameter>錯誤" ;
					break ;
				} // if 
				while ( token.at(i).str != ";" ) { // 判斷id,id,id,id... ;
					while ( token.at(i).str == " " ) { // 空白跳掉 
						i++ ;
					} // while
			        if ( token.at(i).tableType != 5 ) { // 非id 
			        	all.at(index).error = true ;
						all.at(index).errormsg = "SUBROTINE的<parameter group>中的<parameter>錯誤" ;
						break ;
					} // if 
					else { // 為id 
					    comma = false ; // init
						i++ ;
						while ( token.at(i).str == " " ) { // 空白跳掉 
							i++ ;
						} // while
						if ( token.at(i).str == ")" ) {
							i++ ;
							while ( token.at(i).str == " " ) { // 空白跳掉 
								i++ ;
							} // while
							if ( token.at(i).str != ";") {
								all.at(index).error = true ;
								all.at(index).errormsg = "SUBROTINE的')'後面應接';'" ;
								break ;
							} // if 
							else {
								i++ ;
								while ( token.at(i).str == " " ) { // 空白跳掉 
									i++ ;
								} // while
								if ( token.at(i).str != "\n" ) {
									all.at(index).error = true ;
									all.at(index).errormsg = "';'後面不應接東西" ;
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
							all.at(index).errormsg = "SUBROTINE的<parameter group>中，<parameter>後應接','或')'" ;
							break ;
						} // else 
						if ( comma == true && token.at(i).str == ";" ) { // 逗號後面直接遇到分號 
							all.at(index).error = true ;
							all.at(index).errormsg = "SUBROTINE的逗號後面不應有分號'" ;
							break ;
						} //  if
					} // else 
				} // while 
				
				// 判斷完文法 將所有variable放入variable的vector中(之前的所有variable要清空)
				variable.clear() ;
				i = startLine ;
				while ( token.at(i).str != ":" ) { 
					i++ ;
				} // while  
				i++ ; // id 或 " " 
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
				while ( token.at(i).str == " " ) { // 空白跳掉 
					i++ ;
				} // while
				if ( !IsDataType(token.at(i).str) ) { //<type>
					all.at(index).error = true ;
					all.at(index).errormsg = "VARIABLE 後面應接'type'" ;
					break ;
				} // if 
				i++ ;
				while ( token.at(i).str == " " ) { // 空白跳掉 
					i++ ;
				} // while
				if ( token.at(i).str != ":" ) { // :
					all.at(index).error = true ;
					all.at(index).errormsg = "VARIABLE<type> 後面應接':'" ;
					break ;
				} // if 
				i++ ;
				while ( token.at(i).str == " " ) { // 空白跳掉 
					i++ ;
				} // while
				if ( token.at(i).tableType != 5 ) { // 非id 多判斷一次 避免冒號:後面直接是分號; 
			        	all.at(index).error = true ;
						all.at(index).errormsg = "VARIABLE<type>:後面應接<identifier>" ;
						break ;
					} // if 
				while ( token.at(i).str != ";" ) { // 判斷id,id,id,id... ;
					while ( token.at(i).str == " " ) { // 空白跳掉 
						i++ ;
					} // while
			        if ( token.at(i).tableType != 5 ) { // 非id 
			        	all.at(index).error = true ;
						all.at(index).errormsg = "VARIABLE<type>:後面應接<identifier>" ;
						break ;
					} // if 
					else { // 為id 
						comma = false ; // init
						i++ ;
						while ( token.at(i).str == " " ) { // 空白跳掉 
							i++ ;
						} // while
						if ( token.at(i).str == "," ) {
							comma = true ;
							i++ ;
						} // if 
						else if (token.at(i).str == ";" ) { // 遇到分號檢查後面有無東西 
							i++ ;
							while ( token.at(i).str == " " ) { // 空白跳掉 
								i++ ;
							} // while
							if ( token.at(i).str != "\n" ) {
								all.at(index).error = true ;
								all.at(index).errormsg = "';'後面不應接東西"; 
								break ;
							} // if 
							break ;
						} // else if 			
						else {
							all.at(index).error = true ;
							all.at(index).errormsg = "VARIABLE<type>:<identifier>後應接','或';'" ;
							break ;
						} // else 
						if ( comma == true && token.at(i).str == ";" ) { // 逗號後面直接遇到分號 
							all.at(index).error = true ;
							all.at(index).errormsg = "VARIABLE<type>:<identifier>,後應接'<identifier>'" ;
							break ;
						} //  if						 
					} // else 
				} // while 
				// 判斷完VARIABLE後(確認文法正確) 將變數放入variable這個vactor中
				i = startLine ;
				while ( i < endLine ) {
					while ( token.at(i).str != ":" ) {
						i++ ;
					} // while 
					i++ ;  // id 或 " "
					while ( token.at(i).str != ";" ) { // 有變數尚未放入vector中 
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
				while ( token.at(i).str == " " ) { // 空白跳掉 
					i++ ;
				} // while
				if ( token.at(i).tableType != 5 ) { // 非id 多判斷一次 避免LABEL後面直接是分號; 
		        	all.at(index).error = true ;
					all.at(index).errormsg = "LABEL後面應接<identifier>" ;
					break ;
				} // if 
				while ( token.at(i).str != ";" ) { // 判斷id,id,id,id... ;
					while ( token.at(i).str == " " ) { // 空白跳掉 
						i++ ;
					} // while
			        if ( token.at(i).tableType != 5 ) { // 非id 
			        	all.at(index).error = true ;
						all.at(index).errormsg = "LABEL後面應接<identifier>" ;
						break ;
					} // if 
					else { // 為id 
						comma = false ; // init
						i++ ;
						while ( token.at(i).str == " " ) { // 空白跳掉 
							i++ ;
						} // while
						if ( token.at(i).str == "," ) {
							comma = true ;
							i++ ;
						} // if 
						else if (token.at(i).str == ";" ) { // 遇到分號檢查後面有無東西 
							i++ ;
							while ( token.at(i).str == " " ) { // 空白跳掉 
								i++ ;
							} // while
							if ( token.at(i).str != "\n" ) {
								all.at(index).error = true ;
								all.at(index).errormsg = "';'後面不應接東西" ;
								break ;
							} // if 
							break ;
						} // else if 			
						else {
							all.at(index).error = true ;
							all.at(index).errormsg = "LABEL<identifier>後應接','或';'" ;
							break ;
						} // else 
						if ( comma == true && token.at(i).str == ";" ) { // 逗號後面直接遇到分號 
							all.at(index).error = true ;
							all.at(index).errormsg = "LABEL<identifier>,後應接'<identifier>'" ;
							break ;
						} //  if 
					} // else 
				} // while 
				// 判斷完文法 將所有label放入label的vector中
				i = startLine ;
				while ( token.at(i).tableValue != 15 ) { // LABEL
					i++ ;
				} // while  
				i++ ; // label 或 " " 
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
				while ( token.at(i).str == " " ) { // 空白跳掉 
					i++ ;
				} // while
				if ( !IsDataType(token.at(i).str) ) { // <parameter group>:=<type>:<parameter>{,<parameter>}
					all.at(index).error = true ;
					all.at(index).errormsg = "DIMENSION後面應接'type'" ;
					break ;
				} // if 
				dataType = token.at(i).str ; // 暫存 放table7時會用到 
				i++ ;
				while ( token.at(i).str == " " ) { // 空白跳掉 
					i++ ;
				} // while
				if ( token.at(i).str != ":" ) {
					all.at(index).error = true ;
					all.at(index).errormsg = "DIMENSION<type> 後面應接':'" ;
					break ;
				} // if 
				i++ ;
				while ( token.at(i).str == " " ) { // 空白跳掉 
					i++ ;
				} // while
				if ( token.at(i).tableType != 5 ) { // 非id
		        	all.at(index).error = true ;
					all.at(index).errormsg = "DIMENSION<type>: 後面應接<identifier>";
					break ;
				} // if 	
				string arrayName = token.at(i).str ;	
				while ( token.at(i).str != "\n" ) { // 判斷A(), B() ;
					while ( token.at(i).str == " " ) { // 跳掉空白 
						i++ ;
					} // while
					if ( token.at(i).tableType != 5 ) { // 非id
			        	all.at(index).error = true ;
						all.at(index).errormsg = "DIMENSION<type>: 後面應接<identifier>";
						break ;
					} // if 
					arrayName = token.at(i).str ;
					i++ ;
					while ( token.at(i).str == " " ) { // 跳掉空白 
						i++ ;
					} // while 
					if ( token.at(i).str != "(" ) {
						all.at(index).error = true ;
						all.at(index).errormsg = "DIMENSION<type>:<identifier> 後面應接'('" ;
						break ;
					} // if 
					else { // 為 ( 判斷括號裡面東西有無正確 
					    comma = false ; // init
						i++ ;
						while ( token.at(i).str == " " ) { // 空白跳掉 
							i++ ;
						} // while
						if ( token.at(i).tableType != 3 ) { // 非integer 多判斷一次 避免括號(後面直接是分號; 
				        	all.at(index).error = true ;
							all.at(index).errormsg = "DIMENSION<type>:<identifier>( 後面應接<unsigned integer>" ;
							break ;
						} // if  
						while ( token.at(i).str != ";" ) {
							while ( token.at(i).str == " " ) { // 空白跳掉 
								i++ ;
							} // while
							if ( token.at(i).tableType != 3 ) { // 非integer
					        	all.at(index).error = true ;
								all.at(index).errormsg = "DIMENSION<type>:<identifier>( 後面應接<unsigned integer>" ;
								break ;
							} // if
							else { // 是integer 要判斷後面是有括號還是逗號 
								i++ ;
								while ( token.at(i).str == " " ) { // 空白跳掉 
									i++ ;
								} // while
								if ( token.at(i).str != ")" && token.at(i).str != "," ) {
									all.at(index).error = true ;
									all.at(index).errormsg = "DIMENSION的integer後面應接')'或','" ;
									break ;
								} // if 
								else {
									if ( token.at(i).str == ")" ) {
										break ; // 要出去外面判斷還有沒有下個array 
									} // if 
									else if ( token.at(i).str == "," ) {
										comma = true ;
										i++ ;
									} // else if 	 					
									else {
										all.at(index).error = true ;
										all.at(index).errormsg = "DIMENSION<array declaration>，<unsigned integer>後應接','或')'" ;
										break ;
									} // else 
									if ( comma == true && token.at(i).tableType != 3 ) { // 逗號後面不是數字 
										all.at(index).error = true ;
										all.at(index).errormsg = "DIMENSION的逗號後面應接integer'" ;
										break ;
									} //  if
								} // else 
							} // else integer 要判斷後面是有括號還是逗號 							
						} // while 判斷括號裡面
						i++ ;
						while ( token.at(i).str == " " ) { // 跳掉空白 
							i++ ;
						} // while 
						if ( token.at(i).str != "," && token.at(i).str != ";" ) {
							all.at(index).error = true ;
							all.at(index).errormsg = "DIMENSION的)後面應接逗號或分號'" ;
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
								all.at(index).errormsg = "分號後面不應接東西" ;
							} // if 
							
						} // else if 
					} // else  													
				} // while 判斷有幾個array(A(),B(), C() ....) 
				// 判斷完畢 將array放入table7(vector)
						
				
				dataType = "" ; // init
				i = startLine ;
				int tempi = i ; // 暫存i的值 
				while ( token.at(i).str != ":" ) {
					i++ ; 
				} // while
				i++ ;
				while ( countArray > 0 ) {					
					
					while ( token.at(i).str == " " ) { // 跳掉空白 
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
					tempi = i ; // 先暫存i的值，目前為(後第一個數字 
					while( token.at(i).str != ")" ) { // 先計算有多少dimention 
						if ( token.at(i).str == "," ) countDimention++ ;
						i++ ;
					} // while 
					table7.push_back(countDimention) ;
					i = tempi ; // 回頭 
					while( token.at(i).str != ")" ) { // 再放size(int) 
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
				while ( token.at(i).str == " " ) { // 空白跳掉 
					i++ ;
				} // while
		
				if ( token.at(i).tableType != 5 ) { // 後面非id 
					all.at(index).error = true ;
					all.at(index).errormsg = "GTO後面非identifier" ;
					break ;
				} // if 
				string token_label = token.at(i).str ; // 先暫存label，用來判斷是否為undefined symbol  
				i++ ;
				while ( token.at(i).str == " " ) { // 空白跳掉 
					i++ ;
				} // while
				if ( token.at(i).str != ";" ) {
					all.at(index).error = true ;
					all.at(index).errormsg = "GTO後面錯誤(接太多東西)" ;
					break ; 
				} // if 
				i++ ;
				while ( token.at(i).str == " " ) { // 空白跳掉 
					i++ ;
				} // while
				if ( token.at(i).str != "\n" ) {
					all.at(index).error = true ;
					all.at(index).errormsg = "';'後面不應接東西" ;
					break ;
				} // if 
				
				// 判斷完文法 接下來判斷是否為undefined symbol
				int num = 0 ;
				bool exist = false ;
				while ( num < label.size() ) {
					if ( token_label == label.at(num) ) {
						exist = true ;
					} // if 
					num++ ;
				} // while 
				if ( exist == false ) { // 不存在此label 
					all.at(index).error = true ;
					all.at(index).errormsg = "Undefined Label" ;
				} // if 
				break ;
			} // else if (GTO)-------------------------------------------------------------------------(GTO)
			else if (token.at(i).tableValue == 3 ) { // CALL
			// CALL<id> (id {,id}) ;
				i++ ;
				while ( token.at(i).str == " " ) { // 空白跳掉 
					i++ ;
				} // while		
				if ( token.at(i).tableType != 5 ) { // 後面非id 
					all.at(index).error = true ;
					all.at(index).errormsg = "CALL後面非identifier" ;
					break ;
				} // if 
				i++ ;
				while ( token.at(i).str == " " ) { // 空白跳掉 
					i++ ;
				} // while
				if (token.at(i).str != "(" ) {
					all.at(index).error = true ;
					all.at(index).errormsg = "CALL<identifier>後面應接'('" ;
					break ;
				} // if 
				i++ ;
				while ( token.at(i).str == " " ) { // 空白跳掉 
					i++ ;
				} // while
				if ( token.at(i).tableType != 5 ) { // 後面非id 多判斷一次較保險 
					all.at(index).error = true ;
					all.at(index).errormsg = "CALL<identifier>( 後面非identifier" ;
					break ;
				} // if 
				while ( token.at(i).str != ";" ) { // 判斷id,id,id,id... ;
					while ( token.at(i).str == " " ) { // 空白跳掉 
						i++ ;
					} // while
			        if ( token.at(i).tableType != 5 ) { // 非id 
			        	all.at(index).error = true ;
						all.at(index).errormsg = "CALL括號中的東西非<identifier>" ;
						break ;
					} // if 
					else { // 為id 
					    comma = false ; // init
						i++ ;
						while ( token.at(i).str == " " ) { // 空白跳掉 
							i++ ;
						} // while
						if ( token.at(i).str == ")" ) {
							i++ ;
							while ( token.at(i).str == " " ) { // 空白跳掉 
								i++ ;
							} // while
							if ( token.at(i).str != ";") {
								all.at(index).error = true ;
								all.at(index).errormsg = "CALL的')'後面應接';'" ;
								break ;
							} // if 
							else {
								i++ ;
								while ( token.at(i).str == " " ) { // 空白跳掉 
									i++ ;
								} // while
								if ( token.at(i).str != "\n" ) {
									all.at(index).error = true ;
									all.at(index).errormsg = "';'後面不應接東西" ;
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
							all.at(index).errormsg = "CALL括號中，<identifier>後應接','或')'" ;
							break ;
						} // else 
						if ( comma == true && token.at(i).str == ";" ) { // 逗號後面直接遇到分號 
							all.at(index).error = true ;
							all.at(index).errormsg = "CALL的逗號後面不應有分號'" ;
							break ;
						} //  if
					} // else 
				} // while 
				// 判斷完文法 接下來判斷是否為undefined symbol
				i = startLine ;
				while ( token.at(i).str != "(" ) {
					i++ ;
				} // while
				i++ ;
				while ( i < endLine ) {	 
					if ( token.at(i).str == ";" ) {
						break ;
					} // if 
					while ( token.at(i).str == " " ) { // 跳掉空白 
						i++ ;
					} // while 
					int num = 0 ;
					bool exist = false ;
					if ( token.at(i).str  == " " || token.at(i).str == "," || token.at(i).str == ")" ) {
						i++ ;
					} // if 
					else { // 為id 
						while ( num < variable.size() ) { // 找有沒有在vector中 
							if ( token.at(i).str == variable.at(num) ) {
								exist = true ;
							} // if 
							num++ ;
						} // while 
						if ( exist == false ) { // 不存在此label 
							all.at(index).error = true ;
							all.at(index).errormsg = "Undefined Variable" ;
							break ;
						} // if 
						i++ ;				
					} // else 										
				} // while 
				// 判斷完文法就要將變數放到table7
				int countVariable = 1 ;
				i = startLine ;
				while ( token.at(i).str != "(" ) {
					i++ ;
				} // while
				i++ ;
				while ( token.at(i).str == " " ) { // 跳掉空白 
					i++ ;
				} // while 
				int tempii = i ; // 此為括號後第一個變數 
				while ( token.at(i).str != ";" ) {
					while ( token.at(i).str != ")" ) { // 計算有多少變數 
						if ( token.at(i).str == "," ) {
							countVariable++ ;
						} // if 						
						i++ ;
					} // while
					all.at(index).entry7 = table7.size() ; // 此行指令翻中間碼時 (table6).n8 = entry7 
					table7.push_back(countVariable) ;//共幾個variables										
					i = tempii ; // 回頭再放將變數放入table7中
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
			    while ( token.at(i).str == " " ) { // 跳過空白 
			    	i++ ;
				} // while 
				Token condition ;
				condi.clear() ; // 先清空再放 
				
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
					all.at(index).errormsg = "IF的'THEN'錯誤" ;
					break ;
				} // if 
				if ( condiCorrect == false ) {
					all.at(index).error = true ;
					all.at(index).errormsg = "IF的'condition'錯誤" ;
					break ;
				} // if 
				i++ ;
				condi.clear() ; // 先清空再放
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
					all.at(index).errormsg = "IF的'ELSE'錯誤" ;
					break ;
				} // if 
				if ( condiCorrect == false ) {
					all.at(index).error = true ;
					all.at(index).errormsg = "IF的'statement'錯誤" ;
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
					all.at(index).errormsg = "IF的'statement'錯誤" ;
					break ;
				} // if 
				break ;
			} // else if (IF)-------------------------------------------------------------------------(IF)
			else { // 其他不管 我們只要處理以上幾個就好 
				break;
			} // else  
		} // else if (reserve word)
		else { // 可能為assignment  
			int countLeft = 0 ; // 計算左右括號數量 
			int countRight = 0 ;
			all.at(index).assignment = true ;
			if ( token.at(i).tableType != 5 ) {
				all.at(index).error = true ;
				all.at(index).errormsg = "assignment第一個token應為identifier" ;
				break ;
			} // if 
			i++ ;
			while( token.at(i).str == " " ) { // 跳過空格 
				i++ ; 
			} // while
			if ( token.at(i).str == "(" ) { // 判斷括號裡面 
				while ( token.at(i).str != ")" ) {
					i++ ;
					while( token.at(i).str == " " ) { // 跳過空格 
						i++ ; 
					} // while
					if ( token.at(i).tableType != 3 && token.at(i).tableType != 5 ) {
						all.at(index).error = true ;
						all.at(index).errormsg = "assignment括號中應為identifier或unsigned-integer" ;
						break ;
					} // if 
					i++ ;
					while( token.at(i).str == " " ) { // 跳過空格 
						i++ ; 
					} // while
					if ( token.at(i).str == ")" ) {
						i++ ;
						break ;
					} // if 
					if ( token.at(i).str != "," ) {
						all.at(index).error = true ;
						all.at(index).errormsg = "assignment括號中不合文法" ;
						break ;
					} // if 
					else {
						i++ ;
						while( token.at(i).str == " " ) { // 跳過空格 
							i++ ; 
						} // while
						if ( token.at(i).tableType != 3 && token.at(i).tableType != 5 ) {
							all.at(index).error = true ;
							all.at(index).errormsg = "assignment括號中應為identifier或unsigned-integer" ;
							break ;
						} // if 
						i++ ;
						while( token.at(i).str == " " ) { // 跳過空格 
							i++ ; 
						} // while
						if ( token.at(i).str != ")" ) {
							all.at(index).error = true ;
							all.at(index).errormsg = "assignment少了右括號" ;
							break ;
						} // if 
					} // else  
				} // while  				
			} // if
			if ( all.at(index).error == false ) { //等號前符合文法
				
				while( token.at(i).str == " " ) { // 跳過空格 
					i++ ; 
				} // while
				if ( token.at(i).str != "=" ) {
					all.at(index).error = true ;
					all.at(index).errormsg = "assignment少了等號" ;
					break ;
				} // if 
				i++ ;
				while( token.at(i).str == " " ) { // 跳過空格 
					i++ ; 
				} // while
				// 判斷等號後面!!!
				bool ope1 = false ; // operand
				bool ope2 = true ; // operator
				bool left = false ; // 左括號 
				bool right = false ; // 右括號 
				
				// 先判斷第一個 
				if ( token.at(i).str == ")" ) {
					all.at(index).error = true ;
					all.at(index).errormsg = "assignment等號右邊不能以)開頭" ;
					break ;
				} // else if
				else if ( token.at(i).str != "(" && token.at(i).tableType != 5 &&token.at(i).tableType != 3 &&token.at(i).tableType != 4 ) {
					all.at(index).error = true ;
					all.at(index).errormsg = "assignment等號右邊不能以delimeter開頭(除左括號外)" ;
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
							all.at(index).errormsg = "assignment中括號(位置)不對" ;
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
							all.at(index).errormsg = "assignment中operand/operator數量不對" ;
							break ;
						} // if 
						ope2 = false ;
					} // else if 
					else if ( token.at(i).tableType == 1 ) {
						ope2 = true ;
						if ( ope1 == false ) {
							all.at(index).error = true ;
							all.at(index).errormsg = "assignment中operand/operator數量不對" ;
							break ;
						} // if 
						ope1 = false ;
					} // else if 
					i++ ;
				} // while 
				if ( all.at(index).error == false && ope2 == true ) {
					all.at(index).error = true ;
					all.at(index).errormsg = "assignment中不應以Delimeter結尾(右括號除外)" ;
					break ;
				} // if 
			} // if //等號前符合文法
			if ( all.at(index).error == false && countLeft != countRight ) {
				all.at(index).error = true ;
				all.at(index).errormsg = "assignment中括號數量不正確" ;
				break ;
			} // if 
			break ;
			 // 判斷完文法要再判斷是否為undefined symbol(有variable的vector負責存所有的變數) 
		} // else 
	} // while 
} // end SyntaxAnalysis() 判斷此行文法 
bool Syntax::IsDataType( string str ) { // 判斷是否為'type' 如ARRAY, BOOLEAN..... 
	string out = "" ;
	Convert2BigString(str, out) ;
	if ( out == "ARRAY" || out == "BOOLEAN" || out == "CHARACTER" || out == "INTEGER" || out == "LABEL" || out == "REAL" ) {
		return true ;
	} // if 
	return false ;
} // end IsDataType() 判斷是否為'type' 如ARRAY, BOOLEAN..... 
void Syntax::Convert2BigString( string in , string &out ) {
	int i = 0 ;
	out = in ;
	while ( i < in.length() ) {
		if ( in[i] >= 'a'&& in[i] <= 'z' ) {
			out[i] = in[i] - 32 ;
		} // if 
		i++ ;
	} // while
} // 轉成大寫 
int Syntax::CheckDataType( string str ) { // 回傳其dataType號碼 
	if ( str == "ARRAY" ) return 1 ;
	else if ( str == "BOOLEAN" ) return 2 ;
	else if ( str == "CHARACTER" ) return 3 ;
	else if ( str == "INTEGER" ) return 4 ;
	else if ( str == "LABEL" ) return 5 ;
	else if ( str == "REAL" ) return 6 ;
	else return -1 ; 	
} // CheckDataType()

void Syntax::GenerateIntermediate() { // 產生中間碼 
	int i = 0 ; // 跑 token (vector) 
	int a = 0 ; // 跑 all (vector) 
	Table6 temp ; 
	int statementStart = 0 ; // 紀錄statement的起始位址 
	int statementEnd = 0 ; // statement結束位置 
	bool firstLine = true ;
	bool hasLabel = false ;
	string tempLabel = "" ;
	while ( a < all.size() ) {
		firstLine = true ;
		hasLabel = false ;
		tempLabel = "" ;
		while ( token.at(i).str != "\n" ) { // 判斷一整行 
			if ( all.at(a).error ) { // 有錯誤不翻中間碼 
				ERROR e ;
				char ca[100] ;
				itoa(a+1, ca, 10) ;
				string caa = ca ;
				e.errormsg = "line " + caa + ":	" + all.at(a).errormsg ;
				e.line = table6.size() ; 
				err.push(e) ;
				break ;
			} // if
			else { // 文法無誤 要翻中間碼 
				string str = "" ;
				if ( firstLine && token.at(i).tableType == 5 ) { // 遇到Label 
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
						//舉例 CALL SI(A, B, C, D) ;
						statementStart = i ;
						while ( token.at(i).str != ";" ) {
							i++ ;
						} // while  
						GenerateCALL(statementStart, a) ; // 產生CALL之中間碼 
			            break ; // 完成CALL 
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
						GenerateGTO( statementStart ) ; // 產生GTO之中間碼 
						
						break ;
					} // else if GTO----------------------------------------------(GTO)
					else if ( token.at(i).tableValue == 12) { // IF --------------------------------------------(IF)
						string ifStatement= "" ;
						i++ ;
						int thenNum ;
						while ( token.at(i).str == " " ) { // 跳掉空白 
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
						GenerateCondition() ; // 產生condition中間碼 
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
						if ( hasLabel ) { // 若前面有label 
							labelTable.at(labelTable.size()-1).line = table6.size() ;
						} // if 
						table6.push_back(temp) ;
						int ifNum = table6.size() ; // 後來要回來填 
						tempStr = "" ;
						i++ ;
						while ( token.at(i).str == " " ) { // 跳掉空白 
							tempStr = tempStr + token.at(i).str ;
							i++ ;
						} // while 
						
						
						Token t ; // 暫存變數代稱 
						char ch[10] = {'\0'} ;
						// Condition
						statementStart = i ; // statement開始位置 
						while( token.at(i).str != "ELSE" ) {
							tempStr = tempStr + token.at(i).str ;
							i++ ;
						} // while
						tempStr = tempStr + token.at(i).str ;
						statementEnd = i ; // statement結束位置 
						bool isGTO = false ;
						GenerateStatement( statementStart, statementEnd, a, isGTO ) ; // 產生statement之中間碼 
						if ( isGTO ) { // THEN中有GTO 就不用產生第二個GTO了 
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
							thenNum =table6.size(); // 要回來填的
							table6.push_back(temp) ;
						} // else 
												
						// 回去填IF的ELSE的位置						
						table6.at(ifNum-1).n8 = table6.size()+1;
											
						i++ ;
						while ( token.at(i).str == " " ) { // 跳掉空白 
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
				else if ( !hasLabel && all.at(a).assignment == true ) { // 為assignment 
					statementStart = i ;
					while ( token.at(i).str != ";" ) {
						i++ ;
					} // while 
					
					GenerateAssignment( statementStart ) ; // 產生assignment之中間碼  
				} // else if 為assignment 
			} // else // 文法無誤 要翻中間碼
				
			i++ ;
			while ( token.at(i).str == " " ) { // 避免分號後有空白而不是分號 
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
	
} // 產生中間碼 
bool Syntax::CheckCondition() { // 判斷condition是否正確 
	int i = 0 ;
	bool isID = false ;
	bool isTWO = true ;
	while( i < condi.size() ) {
		if( condi.at(i).str == " " ) {
			;
		} // if
		else {
			if ( condi.at(i).tableType == 5 || condi.at(i).tableType == 3 || condi.at(i).tableType == 4 ) { // id or 數字 
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
} // CheckCondition()判斷condition是否正確 
bool Syntax::CheckStatement( int index ) { // 判斷Statement是否正確 
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
			if ( condi.at(i).tableType == 2 ) { // GTO或CALL 
				if (condi.at(i).tableValue == 11 ) { // GTO
					// GTO<label>; 
					isGTO = true ;
					i++ ;
					while ( condi.at(i).str == " " ) { // 空白跳掉 
						i++ ;
					} // while
			
					if ( condi.at(i).tableType != 5 ) { // 後面非id 
						return false ;
					} // if 
					string token_label = condi.at(i).str ; // 先暫存label，用來判斷是否為undefined symbol  
					i++ ;
					while ( condi.at(i).str == " " ) { // 空白跳掉 
						i++ ;
					} // while
					if ( condi.at(i).str != ";" ) {
						return false ;
					} // if 					
					// 判斷完文法 接下來判斷是否為undefined symbol
					int num = 0 ;
					bool exist = false ;
					while ( num < label.size() ) {
						if ( token_label == label.at(num) ) {
							exist = true ;
						} // if 
						num++ ;
					} // while 
					if ( exist == false ) { // 不存在此label 
						return false ;
					} // if 
					break ;
				} // else if (GTO)-------------------------------------------------------------------------(GTO)
				else if (condi.at(i).tableValue == 3 ) { // CALL
				// CALL<id> (id {,id}) ;
				    isCALL = true ;
					i++ ;
					while ( condi.at(i).str == " " ) { // 空白跳掉 
						i++ ;
					} // while		
					if ( condi.at(i).tableType != 5 ) { // 後面非id 
						return false ;
					} // if 
					i++ ;
					while ( condi.at(i).str == " " ) { // 空白跳掉 
						i++ ;
					} // while
					if (condi.at(i).str != "(" ) {
						return false ;
					} // if 
					i++ ;
					while ( condi.at(i).str == " " ) { // 空白跳掉 
						i++ ;
					} // while
					if ( condi.at(i).tableType != 5 ) { // 後面非id 多判斷一次較保險 
						return false ;
					} // if 
					while ( condi.at(i).str != ";" ) { // 判斷id,id,id,id... ;
						while ( condi.at(i).str == " " ) { // 空白跳掉 
							i++ ;
						} // while
				        if ( condi.at(i).tableType != 5 ) { // 非id 
				        	return false ;
						} // if 
						else { // 為id 
						    bool comma = false ; // init
							i++ ;
							while ( condi.at(i).str == " " ) { // 空白跳掉 
								i++ ;
							} // while
							if ( condi.at(i).str == ")" ) {
								i++ ;
								while ( condi.at(i).str == " " ) { // 空白跳掉 
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
							if ( comma == true && condi.at(i).str == ";" ) { // 逗號後面直接遇到分號 
								return false ;
							} //  if
						} // else 
					} // while 
					// 判斷完文法 接下來判斷是否為undefined symbol
					i = startLine ;
					while ( condi.at(i).str != "(" ) {
						i++ ;
					} // while
					i++ ;
					while ( condi.at(i).str != ";" ) {	 
						if ( condi.at(i).str == ";" ) {
							break ;
						} // if 
						while ( condi.at(i).str == " " ) { // 跳掉空白 
							i++ ;
						} // while 
						int num = 0 ;
						bool exist = false ;
						if ( condi.at(i).str  == " " || condi.at(i).str == "," || condi.at(i).str == ")" ) {
							i++ ;
						} // if 
						else { // 為id 
							while ( num < variable.size() ) { // 找有沒有在vector中 
								if ( condi.at(i).str == variable.at(num) ) {
									exist = true ;
								} // if 
								num++ ;
							} // while 
							if ( exist == false ) { // 不存在此label 
								return false ;
							} // if 
							i++ ;				
						} // else 										
					} // while 
					// 判斷完文法就要將變數放到table7
					int countVariable = 1 ;
					i = startLine ;
					while ( condi.at(i).str != "(" ) {
						i++ ;
					} // while
					i++ ;
					while ( condi.at(i).str == " " ) { // 跳掉空白 
						i++ ;
					} // while 
					int tempii = i ; // 此為括號後第一個變數 
					while ( condi.at(i).str != ";" ) {
						while ( condi.at(i).str != ")" ) { // 計算有多少變數 
							if ( condi.at(i).str == "," ) {
								countVariable++ ;
							} // if 						
							i++ ;
						} // while
						all.at(index).entry7 = table7.size() ; // 此行指令翻中間碼時 (table6).n8 = entry7 
						table7.push_back(countVariable) ;//共幾個variables										
						i = tempii ; // 回頭再放將變數放入table7中
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
			} // if GTO或CALL 
			else { // assignment
				if ( condi.at(i).tableType == 5 || condi.at(i).tableType == 3 || condi.at(i).tableType == 4 ) { // id or 數字 
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
} // CheckStatement()判斷Statement是否正確
void Syntax::GenerateCondition() { // 產生condi的中間碼 
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
			t = op1.top() ; // 取Y 
			temp.n5 = 5 ;
			temp.n6 = t.tableValue ;
			op1.pop() ; // 丟Y 
			t = op1.top() ; // 取X 
			temp.n3 = 5 ;
			temp.n4 = t.tableValue ;
			op1.pop() ; // 丟X 
			t = op2.top() ; // 取GT
			temp.n1 = 2 ;
			temp.n2 = t.tableValue ;
			op2.pop() ; // 丟GT 
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
			op1.push(t) ; // 放T1
			op2.push(condi.at(i) ) ; // 放AND 
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
		t = op1.top() ; // 取Y 
		temp.n5 = 5 ;
		temp.n6 = t.tableValue ;
		op1.pop() ; // 丟Y 
		t = op1.top() ; // 取X 
		temp.n3 = t.tableType ;
		temp.n4 = t.tableValue ;
		op1.pop() ; // 丟X 
		t = op2.top() ; // 取GT
		temp.n1 = 2 ;
		temp.n2 = t.tableValue ;
		op2.pop() ; // 丟GT 
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
} //  產生condi的中間碼 
void Syntax::GenerateAssignment( int statementStart ) { // 產生assignment之中間碼 
	stack<Token> op1 ; // operand
	stack<Token> op2 ; // operator
	int i = statementStart ;
	Token temp ;
	Token operand1 ; // operand 1
	Token operand2 ; // operand 2 
	Token operator1 ; // operator
	bool isOp1 = false ;
	bool isArray = false ; // 上一個是左括號 
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
		if ( token.at(i).tableType != 1 ) { // operand 直接放 
		    isOp1 = true ;
			op1.push( token.at(i) ) ;
		} // if 
		else { // delimeter 
			if ( op2.empty() ) {
				CheckLevel( token.at(i) ) ;
				op2.push( token.at(i) ) ;
				isArray = false  ;					
				if ( token.at(i).str == "(" ) { // 左括號到stack中便最小 
					token.at(i).level = 0 ;
					if ( isOp1 ) {
						isArray = true ;
					} // if 
				} // if 
			} // if 
			else { // 比大小決定要不要pop() 
				CheckLevel( token.at(i) ) ;
				temp = op2.top() ; // 取top準備比較大小
				if ( temp.level < token.at(i).level ) { // 要放stack  (除右括號外) 
					if ( token.at(i).str == ")" ) {
						operand2 = op1.top() ;
						op1.pop() ;
						operand1 = op1.top() ;
						op1.pop() ;
						operator1 = op2.top() ;
						op2.pop() ;
						if ( operator1.str == "(" ) { // 有可能是array 
							if ( !isArray) { // 不是array 
								op1.push(operand1) ;
							    op1.push(operand2) ;
							} // if 
							else { // 是array 
								token.at(i).isArrayOne = true ;
								Token AT ;
								AT.str = "at" ;
								AT.tableType = 8 ; // 代表array
								AT.tableValue = arrayTable.size() ; // tableValue 指向arrayTable的entry 
								AT.pointAT = arrayTable.size() ;
								ArrayTable at ;
								at.array = operand1 ;
								at.index1 = operand2 ;
								at.two_dimention = false ;
								arrayTable.push_back(at) ;
								AT.isArrayTwo = false ;
								AT.isArrayOne = true ;
								op1.push(AT) ; // 把替代ARRAY丟入stack 
							} // else 是array 

						} // if 
						else if ( operator1.str == "," ) { // 是二維陣列 
							token.at(i).isArrayTwo = true ;
							op2.pop() ; // pop左括號
							Token operand0 = op1.top() ; // 取ARRAY NAME
							op1.pop() ;
							Token AT ;
							AT.str = "at" ;
							AT.tableType = 8 ; // 代表array
							AT.tableValue = arrayTable.size() ; // tableValue 指向arrayTable的entry 
							AT.pointAT = arrayTable.size() ;
							ArrayTable at ;
							at.array = operand0 ;
							at.index1 = operand1 ;
							at.index2 = operand2 ;
							at.two_dimention = true ;
							arrayTable.push_back(at) ;
							AT.isArrayTwo = true ;
							AT.isArrayOne = false ;
							op1.push(AT) ; // 把替代ARRAY丟入stack 
						} // else if  是二維陣列 
						else {							
							op2.pop() ; // pop 左括號
							GenerateCode( operator1, operand1, operand2 ) ;
							// 再將暫時變數T1丟進stack 
							op1.push(table0.at(table0.size()-1) ); 
						} // else 
							
					} // if 
					else {	
						isArray = false  ;					
						if ( token.at(i).str == "(" ) { // 左括號到stack中便最小 
							token.at(i).level = 0 ;
							if ( isOp1 ) {
								isArray = true ;
							} // if 
						} // if 
						op2.push(token.at(i)) ;
					} // else 
				} // if // 要放stack  (除右括號外)
				else if ( temp.level == token.at(i).level || temp.level > token.at(i).level ){ // 要pop() 產生中間碼  (若為次方 則先放入stack)
					if ( token.at(i).str == "^" ) {
						op2.push(token.at(i)) ;
					} // if 
					else { // 真的要POP()
						if ( !op2.empty() ) {
							// 從op1中取兩個數 算完放入暫時變數再push回去
							if ( token.at(i).str == ";" ) {
								break;
							} // if 
							if ( temp.level == token.at(i).level || temp.level > token.at(i).level ) {	// 要pop()
														
								if ( token.at(i).str == ")" ) {
									operand2 = op1.top() ;
									op1.pop() ;
									operand1 = op1.top() ;
									op1.pop() ;
									operator1 = op2.top() ;
									op2.pop() ;
									if ( operator1.str == "("  ) { // 有可能是array 
									    if ( !isArray   ) { // 不是array 
								            op1.push(operand1) ;
							                op1.push(operand2) ;
						            	} // if 
							            else if( isOp1 == true ) { // 是array 
							            	token.at(i).isArrayOne = true ;
							            	Token AT ;
							            	AT.str = "at" ;
							            	AT.tableType = 8 ; // 代表array
							            	AT.tableValue = arrayTable.size() ; // tableValue 指向arrayTable的entry 
							            	AT.pointAT = arrayTable.size() ;
							            	AT.isArrayOne = true ;
							            	ArrayTable at ;
							            	at.array = operand1 ;
							            	at.index1 = operand2 ;
							            	at.two_dimention = false ;
							            	arrayTable.push_back(at) ;
							            	AT.isArrayTwo = false ;
								            AT.isArrayOne = true ;
							            	op1.push(AT) ; // 把替代ARRAY丟入stack 
						            	} // if 是array 

						            } // if 
						            else if ( operator1.str == "," ) { // 是二維陣列 
							            token.at(i).isArrayTwo = true ;
						            	op2.pop() ; // pop左括號
							            Token operand0 = op1.top() ; // 取ARRAY NAME
							            op1.pop() ;
							            Token AT ;
							            AT.str = "at" ;
						            	AT.tableType = 8 ; // 代表array
						            	AT.isArrayTwo = true ;
						            	AT.tableValue = arrayTable.size() ; // tableValue 指向arrayTable的entry 
							            AT.pointAT = arrayTable.size() ;
						            	ArrayTable at ;
						            	at.array = operand0 ;
						            	at.index1 = operand1 ;
						            	at.index2 = operand2 ;
						            	at.two_dimention = true ;
						            	arrayTable.push_back(at) ;
						            	AT.isArrayTwo = true ;
								        AT.isArrayOne = false ;
						            	op1.push(AT) ; // 把替代ARRAY丟入stack 
					            	} // else if  是二維陣列 
									else {	
									    if ( operand1.tableType != 8 && operand2.tableType != 8 ) {
									    	GenerateCode( operator1, operand1, operand2 ) ;
										    // 再將暫時變數T1丟進stack 
										    op1.push(table0.at(table0.size()-1) ); 
										    i-- ;
										} // if 
										else { // 有array 
											GenerateArray( operator1, operand1, operand2 ) ; // 產生有關array之assignment之中間碼 
										
										}	// else 有array														
										
									} // else 
																		
								} // if 右括號 
								else {
									operand2 = op1.top() ;
									op1.pop() ;
									operand1 = op1.top() ;
									op1.pop() ;
									operator1 = op2.top() ;
									op2.pop() ;
									GenerateCode( operator1, operand1, operand2 ) ;
									// 再將暫時變數T1丟進stack 
									op1.push(table0.at(table0.size()-1)) ; 
									i-- ;
								} // else 														
							} // if 要pop() 
							else { // push!
								op2.push(token.at(i)) ; 
							} // else 																													
						} // if
					} // else 真的要POP() 
				} // else if // 要pop() 產生中間碼  (若為次方 則先放入stack)				
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
			else { // 有array 
				GenerateArray( operator1, operand1, operand2 ) ; // 產生有關array之assignment之中間碼 
			
			}	// else 有array				
		} // if 
		else {
			if ( operand1.tableType != 8 && operand2.tableType != 8 ) {
				GenerateCode( operator1, operand1, operand2 ) ;
				// 再將暫時變數T1丟進stack 
				op1.push(table0.at(table0.size()-1)) ; 
			} // if 
			else {
				GenerateArray( operator1, operand1, operand2 ) ; // 產生有關array之assignment之中間碼 
				op1.push(table0.at(table0.size()-1)) ; 
			} // else 
		} // else 
	} // while
		
} // 產生assignment之中間碼 
void Syntax::GenerateArray( Token operator1, Token operand1, Token operand2 ) { // 產生有關array之assignment之中間碼 
	Table6 temp ;
	Token t ; // 暫時變數 
	char ch[100] ;
	string s ;
	Token n34;
	Token n56 ;
	int i = 0 ;
	Token n78 ; 
	if ( operator1.str == "=" ) { // 等於的情況
		if ( operand2.tableType == 8 ) { // 需生成T1暫時變數  等於說處理完之後等號右邊不會是array了 
			if ( operand2.isArrayTwo ) {// 二維陣列
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
				
			} // if  二維陣列 
			else { // 一維陣列 
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
			} // else 一維陣列 
			operand2 = t ;
			 
		} // if  需生成T1暫時變數  等於說處理完之後等號右邊不會是array了 
		
		if ( operand1.tableType == 8 ) {
			if ( operand1.isArrayTwo ) { // 二維陣列 
				
				
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
				// 降成一維了				
			} // if 
			n78 = t ;
			if ( operand1.isArrayOne ) { // 改n78 
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
	} // if  等於的情況
	else { // +-*/的情況 
		if ( operand2.tableType == 8 ) { // 需生成T1暫時變數  等於說處理完之後等號右邊不會是array了 
			if ( operand2.isArrayTwo ) {// 二維陣列
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
				
			} // if  二維陣列 
			else { // 一維陣列 
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
			} // else 一維陣列 
			operand2 = t ;
			 
		} // if  需生成T1暫時變數  等於說處理完之後等號右邊不會是array了 
		if ( operand1.tableType == 8 ) { // 需生成T1暫時變數
			if ( operand1.isArrayTwo ) {// 二維陣列
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
				
			} // if  二維陣列 
			else { // 一維陣列 
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
			} // else 一維陣列 
			operand1 = t ;			 
		} // if  需生成T1暫時變數  
		// 正常加減成除 
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

} // 產生有array之assignment之中間碼 

void Syntax::CheckLevel( Token &t ) { // 確認delimeter之大小  
	if ( t.str == "(" ) t.level = 6 ;
	else if ( t.str == "^" ) t.level = 5 ;
	else if ( t.str == "*" ) t.level = 4 ;
	else if ( t.str == "/" ) t.level = 4 ;
	else if ( t.str == "+" ) t.level = 3 ;
	else if ( t.str == "-" ) t.level = 3 ;
	else if ( t.str == ")" ) t.level = 2 ;
	else if ( t.str == "=" ) t.level = 1 ;
} // 確認delimeter之大小 
void Syntax::GenerateCode( Token operator1, Token operand1, Token operand2 ) {  // 產生指定operand/operator之中間碼 
	Table6 temp ;
	Token var ; // 暫時變數 
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
} // 產生指定operand/operator之中間碼 
void Syntax::GenerateCALL( int statementStart, int a ) { // 產生CALL之中間碼 
	Table6 temp ;
	string str = "" ;
	int i = statementStart ; // statement起始位置 
	
	str = str + token.at(i).str ;
	temp.n1 = 2 ; // CALL
	temp.n2 = 3 ;
	i++ ;
	while ( token.at(i).str == " " ) { // 跳掉空白 
		str = str + token.at(i).str ;
		i++ ;
	} // while 
	str = str + token.at(i).str ;
	temp.n3 = token.at(i).tableType ; // SI
	temp.n4 = token.at(i).tableValue ;
	i++ ;
	temp.n5 = -1 ; // 空 
	temp.n6 = -1 ; // 空 
	temp.n7 = 7 ; // 指向table7 
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
} // 產生 CALL之中間碼 
void Syntax::GenerateGTO( int statementStart ) { // 產生GTO之中間碼

	Table6 temp ;
	Label l ;
	string str = "" ;
	int i = statementStart ; // statement起始位置 
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
				
} // 產生GTO之中間碼 
void Syntax::GenerateStatement( int statementStart, int statementEnd, int a, bool &isGTO  ) { // 產生statement之中間碼  呼叫generateGTO/ CALL / assignment()
	Token end ; // 分號 
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
	token.at(i) = end ; // 替換成分號再去做事
	if ( isGTO ) {
		GenerateGTO( statementStart ) ;
	} // if 
	else if ( isCALL ) {
		GenerateCALL( statementStart, a ) ;
	} // else if 
	else {
		GenerateAssignment( statementStart ) ;
	} // else 
	 
} // 產生Statement之中間碼 
// end class Syntax 判斷文法======================================================================================================================
class Lex{ // start 切Token ==========================================================================================================
	string table1[12] ; // delimiter table
	string table2[25] ; // reserve word table
	
	string table4[100] ; // real number table
	
	vector<string> inputFile ; // 讀入的檔案 
	 
public:
	bool ReadFile() ; // 讀input檔
	void ReadTable(string number) ; 
	void GetLine() ; // 每次取一行去切Token <<呼叫 LexicalAnalysis()>>
	void LexicalAnalysis(string line, int &subroutine ) ; // 切這行的Token 
	bool IsDelimeter( char ch ) ; // 判斷是否為delimeter 
	void CheckWhichTable ( Token &temp, int &subroutine ) ; // 判斷在哪個table中若有需要則放入相對應的table中
	void Convert2BigString( string in , string &out ) ; // 轉成大寫 
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
} // 轉成大寫 
bool Lex::ReadFile() { // 讀input檔 
	FILE *file = NULL ; 
	cout << "請輸入檔案名稱: \n" ; 
	cin >> fileName ;
	
	string oriFileName = fileName + ".txt" ;

    file = fopen( oriFileName.c_str(), "r" ) ; // open the file
	if ( file == NULL ) {
		cout << "檔案不存在!\n" ;
		fclose( file ) ; 
		return false ;
	}
	else {
		bool done = false ; // 是否已放入vector 
		char ch = '\0' ;
		string temp = "" ; // 暫存讀入的東西	
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
} // ReadFile 讀檔 
void Lex::ReadTable( string number ) { // 讀table 
	FILE *file = NULL ; 
	
	string oriFileName = "table" + number + ".table" ;

    file = fopen( oriFileName.c_str(), "r" ) ; // open the file
	if ( file == NULL ) {
		cout << "檔案不存在!\n" ;
		fclose( file ) ; 
	} // if
	else {	
		bool done = false ;
		int i = 0 ; // 跑table的index	
		char ch = '\0' ;
		string temp = "" ; // 暫存讀入的東西	
		while ( fscanf(file, "%c", &ch ) != EOF ) { // 讀char直到讀完了 
		    done = false ;
			if ( ch != '\n' ){
				temp = temp + ch ;
			} // if 
			else {
				done = true ;
				if ( number == "1" ) { // table1的東西 
					table1[i] = temp ;
				} // if 
				else if ( number == "2" ) { // table2的東西 
					table2[i] = temp ;
				} // else if 
				temp = "" ; 
				i++ ;
			} // else 			
		} // while  
		if ( !done ) {
		    if ( number == "1" ) { // table1的東西 
				table1[i] = temp ;
			} // if 
			else if ( number == "2" ) { // table2的東西 
				table2[i] = temp ;
			} // else if 
		} // if			
		fclose( file ) ; 	
	} // else 		
} // ReadTable()讀table1跟table2 
void Lex::GetLine() { // 每次取一行去切token  <<呼叫 LexicalAnalysis()>>
	int i = 0 ; // 跑inputFile的index
	Token temp ;
	temp.str = "\n" ;
	temp.tableType = -1 ;
	temp.tableValue = -1 ;
	int subroutine = -1 ;
	while( i < inputFile.size() ) {
		LexicalAnalysis( inputFile[i], subroutine ) ;
		token.push_back(temp) ; // 放換行進去 
		i++ ;
	} // while 
} // GetLine() 每次取一行去切token  <<呼叫 LexicalAnalysis()>>
void Lex::LexicalAnalysis( string line, int &subroutine ) { // 切這行的Token 
	int i = 0 ; // 用來當line的index
	Token temp ;// 用來暫存token
	Token a ;
	a.str = "" ;
	a.tableType = -1 ;
	a.tableValue = -1 ;

	temp = a ;// 讓temp 回歸空的 
	while ( i < line.size() ) {
		if ( line[i] == ' ' || line[i] == '\t' || line[i] == '\n' ) { // 遇到White Space 
			 if ( temp.str != "" ) { // temp有東西  
			     CheckWhichTable( temp, subroutine ) ;  // 判斷在哪個table中若有需要則放入相對應的table中 
			     token.push_back(temp) ; // 放入Token的vector中 
			     temp = a ;
			 } // if 
			 if ( line[i] != '\n' ) {
		         temp.str = " " ;
				 temp.tableType = -1 ;
				 temp.tableValue = -1 ;
				 token.push_back(temp) ; // 將"空白"放入Token的vector中 
				 temp = a ;
			 } // if 
				 
		} // if 
		else if ( IsDelimeter( line[i] ) ) { // 若是delineter 
			if ( temp.str != "" ) { // temp有東西  
			     CheckWhichTable( temp, subroutine ) ;  // 判斷在哪個table中若有需要則放入相對應的table中
			     token.push_back(temp) ; // 放入Token的vector中 
			     temp = a ; // 讓temp 回歸空的 
			} // if
			
			temp.str = temp.str + line[i] ; // 換delimeter放入table中 
			CheckWhichTable( temp, subroutine ) ;
			token.push_back(temp) ; // 放入Token的vector中 
			temp = a ;
		} // else if 
		else { // 其他狀況則放入temp中 
			temp.str = temp.str + line[i] ;			
		} // else 
		i++ ;
	} // while 
	if ( temp.str != "" ) { // temp有東西  
	     CheckWhichTable( temp, subroutine ) ;  // 判斷在哪個table中若有需要則放入相對應的table中 
	     token.push_back(temp) ; // 放入Token的vector中 
	     temp = a ;
	 } // if  
} // LexicalAnalysis() 
bool Lex::IsDelimeter( char ch ) { // 判斷是否為Delimeter 
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
void Lex::CheckWhichTable( Token &temp, int &subroutine ) { // 判斷在哪個table中若有需要則放入相對應的table中
	int i = 0 ;
	bool done = false ; // 是否判斷完成 
	string bigStr = "" ;
	Convert2BigString( temp.str, bigStr ) ;
	while ( i < 12 ) { // 判斷table1 
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
		while ( i < 25 ) { // 判斷table2 
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
		while ( i < temp.str.size() ) { // 此迴圈負責判斷是否為integer或real number (有小數點) 
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
		if ( isNum && isInteger ) { // 是Table3 integer
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
		else if ( isNum ) { // 是Table4 real
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
		else if ( !isNum && !isInteger ) {// 是Table5 identifier
			temp.tableType = 5 ;
			
			while ( table5[index].str != "" ) {
				if ( table5[index].str == temp.str && table5[index].subroutine == subroutine ) { // 遇到相同的就跳過不擺進去 subroutine不相同 
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
// end class Lex 切Token======================================================================================================================
int main(int argc, char** argv) {
	Lex l ; 
	cout << "*********************************\n" ;
	cout << "*********************************\n" ;
	cout << "*****       歡迎光臨      *******\n" ;
	cout << "*****   FRANCIS COMPILER  *******\n" ;
	cout << "*********************************\n" ;
	cout << "*********************************\n" ;
	cout << "*********************************\n\n" ;
	
	while ( !l.ReadFile() ) ;// 讀檔 
	l.ReadTable("1") ; // 讀table1 
	l.ReadTable("2") ; // 讀table2 
	l.GetLine() ;
	
	//判斷Token的Vector有沒有正確 
	/*int i = 0 ;
	while ( i < token.size() ) {
		if ( token[i].str == "\n" ) {
			cout << "\n" ;
		}
		cout << token[i].str << "( " << token.at(i).tableType << ", " << token[i].tableValue << ")	\n" ;
		i++ ;
	} // while */
	table7.push_back(-1) ; // 因為從1開始算，所以一開始0的位置先放-1 
	Token z ;
	table0.push_back(z) ; // 因為從1開始算
	Syntax s ;
	s.GetLine() ;
	// 判斷SYNTAX有無正確 
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
	// 判斷table7有無正確
	/*int k = 0 ;
	while ( k < table7.size()) {
		cout << table7.at(k) << "\n" ;
		k++ ;
	} // while*/
	s.GenerateIntermediate() ; // 產生中間碼 
	// 判斷中間碼有沒正確
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
		
		if ( i == er.line ) { // 有error 
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

