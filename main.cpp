# include <iostream>
# include <string>
# include <vector>
# include <fstream>
# include <queue>
# include <iomanip>
# include <stack>

using namespace std ;

# define HASHMOD 100

# define TABLE_TMP 0
# define TABLE_DELIMITER 1
# define TABLE_RWORD 2
# define TABLE_INT 3
# define TABLE_RNUM 4
# define TABLE_IDENTIFIER 5
# define TABLE_QUADRUPLE 6
# define TABLE_INFO 7

# define TYPE_ARRAY 1
# define TYPE_BOOL 2
# define TYPE_CHAR 3
# define TYPE_INT 4
# define TYPE_LABEL 5
# define TYPE_REAL 6

struct Token {
    
    string str ; // the origin string of the token
    int table ; // the table which this token belong with
    int no ; // the no. of this token in this table
    
    Token() {
        str = "" ;
        table = -1 ;
        no = -1 ;
    } // constructor()
    
} ; // Token

struct Unit {  // the standard form for the information in the quadruple table EX: (5,8) in table 5 and index 8
    
    int tableNo ;
    int index ;
    
    Unit() {
        tableNo = -1 ;
        index = -1 ;
    } // Unit
    
} ; // Unit

struct Statement {
    
    string originStr ;
    Token instr ;
    string label ;
    vector<Token> tokens ;
    bool gramCorrect ; // record the grammer correctivity
    
    Statement() {
        originStr = "" ;
        label = "" ;
        gramCorrect = true ;
    } // constructor()
    
} ; // Statement

struct ErrorInfo {
    
    int indexOfStmt ; // to know which statement is wrong
    string message ; // store the error message, then show the whole error meesage on the string after the process are done
    
    ErrorInfo() {
        indexOfStmt = -1 ;
        message = "" ;
    } // constructor()
    
} ; // ErrorInfo

struct Table5Info {
    
    string identifier ; // the name of the identifier
    int subroutine ; // where this identifier belongs to
    int type ; // distinguish this identifier whether it's Array(1) Boolean(2) Character(3) Integer(4) Label(5) Real number(6)
    Unit pointer ; // where this identifier locate in Table6
    
    Table5Info(){
        
        identifier = "" ;
        subroutine = -1 ;
        type = -1 ;
        
    } // Table
    
} ; // struct Table5Info

struct quaForm { // the data structure for the quadruple form
    
    Unit units[ 4 ] ;
    string stmt ;
    
    quaForm() {
        for ( int i = 0 ; i < 4 ; i ++ ) {
            Unit tmp ;
            units[ i ] = tmp ;
        } // for()
        stmt = "" ;
    } // constructor()
    
} ; // quaForm

struct fix {
    
    int quaIndex ;
    int unitNo ;
    string labelName ;
    
    fix() {
        
        quaIndex = -1 ;
        unitNo = -1 ;
        labelName = "" ;
        
    } // constructor()
    
} ; // fix()

class G {
    
private:
    
    fstream file ;
    string fileName ;
        
    vector<string> tmpBuffer ; // use to temparary string some input data
    
    int recentTmpSequenceNum ;
    
public:
    
    stack<string> subroutine ; // when we trensfer to a new subroutine then we push it into the stack
    
    vector<string> delSet ; // used to store the delimiter terms for future comparison
    vector<string> rsWord ; // used to store the reserved word terms for futer comparison
    vector<Statement> input ; // input file
    
    vector<Token> table0 ;
    string table3[ HASHMOD ] ; // integer table
    string table4[ HASHMOD ] ; // real number table
    Table5Info table5[ HASHMOD ] ; // the table that put the identifier
    vector<quaForm> table6 ;
    vector<int> table7 ; // information table
    vector<ErrorInfo> errorTable ; // store the error messages, then print out
    vector<fix> fixSet ; // put the forward reference
    
    G() {
        
        setUpTableInfo() ; // set up the table info in G constructor
        errorTable.clear() ;
        // initialize some tables
        for ( int i = 0 ; i < HASHMOD ; i ++ ) { // initialize the table 5
            Table5Info tmp ;
            table5[ i ] = tmp ;
        } // for()
        recentTmpSequenceNum = 1 ;
        
        table7.push_back( -1 ) ;
        
    } // constructor()
    
    void createOutputFile() {
        
        string outputName = fileName + "_output.txt" ;
        
        file.open( outputName, ios :: out ) ;
        
        if ( file ) {
            
            for ( int i = 0 ; i < table6.size() ; i ++ ) {
                
                file << i + 1 << "\t(" ;
                
                for ( int j = 0 ; j < 4 ; j ++ ) {
                    
                    if ( table6[ i ].units[ j ].tableNo != -1 ) {
                        
                        string tmp = "(" + intToStr( table6[ i ].units[ j ].tableNo ) + "," + intToStr(  table6[ i ].units[ j ].index  ) + ")" ;
                         
                        file << setw( 7 ) << tmp ;
                        
                    } // if()
                    else {
                        
                        file << setw( 7 ) << " " ;
                        
                    } // else()
                    
                    if ( j < 3 ) {
                        
                        file << "," ;
                        
                    } // if()
                    
                } // for()
                
                file << ")" << "\t" << table6[ i ].stmt << endl ;
                
            } // for()
            
        } // if()
        
        file << endl ;
        
        for ( int i = 0 ; i < errorTable.size() ; i ++ ) {
            
            file << "### Error " << "(Line." << errorTable[ i ].indexOfStmt + 1 << ")" << ": " << errorTable[ i ].message << "! ###" << endl ;
            
        } // for()
        
        if ( errorTable.size() != 0 ) {
        
            file << "Syntax Error" ;
            
        } // if()
        
        errorTable.clear() ;
        
    } // createOutputFile()
    
    int getRecentTmpNum() {
        
        return recentTmpSequenceNum - 1 ;
        
    } // getRecentTmpNum()
    
    int getNewTmpNum() {
        
        int ans = recentTmpSequenceNum ;
        
        recentTmpSequenceNum ++ ;
        
        return ans ;
        
    } // getNewTmpNum()
    
    string cleanReturnChar( string str ) {
        
        string tmpLine = "" ;
        
        for ( int i = 0 ; i < str.length() && str[ i ] != '\r' ; i ++ ) {
            
            tmpLine += str[ i ] ;
            
        } // for()
        
        return tmpLine ;
        
    } // cleanReturnChar()
    
    bool isBlank( char ch ) {
        
        if ( ch == ' ' || ch == '\t' ) {
            
            return true ;
            
        } // if()
        
        return false ;
        
    } // isBlank()
    
    bool isDelimiter( char ch ) {
        
        string str = "" ;
        str += ch ;
        
        vector<string> :: iterator it = find( delSet.begin(), delSet.end(), str ) ;
        
        if ( it != delSet.end() ) {
            
            return true ;
            
        } // if()
        
        return false ;
        
    } // isDelimiter()
    
    bool isRSWord( string str ) {
        
        str = lowerToUp( str ) ; // transfer the strings intp upper case to make sure we won't miss
        
        vector<string> :: iterator it = find( rsWord.begin(), rsWord.end(), str ) ;
        
        if ( it != rsWord.end() ) {
            
            return true ;
            
        } // if()
        
        return false ;
        
    } // isRSWord()
    
    int getRSWordIndex( string str ) {
        
        int index = -1 ;
        
        for ( int i = 0 ; i < rsWord.size() && index == -1 ; i ++ ) {
            
            if ( lowerToUp( str ) == rsWord[ i ] ) {
                
                index = i ;
                
            } // if()
            
        } // for()
        
        return index ;
        
    } // getRSWordIndex()
    
    bool labelExist( string labelName, int &index ) {
        
        for ( int i = 0 ; i < HASHMOD ; i ++ ) {
            
            if ( labelName == table5[ i ].identifier && table5[ i ].type == TYPE_LABEL ) {
                
                index = i ;
                
                return true ;
                
            } // if()
            
        } // for()
        
        index = -1 ;
        
        return false ;
        
    } // labelExist()
    
    bool variableExist( string string, int routine, int &index ) {
        
        for ( int i = 0 ; i < HASHMOD ; i ++ ) {
            
            if ( string == table5[ i ].identifier && table5[ i ].subroutine == routine && table5[ i ].type != TYPE_LABEL && table5[ i ].type != -1 ) {
                
                index = i ;
                
                return true ;
                
            } // if()
            
        } // for()
        
        index = -1 ;
        
        return false ;
        
    } // variable()
    
    bool subroutineExist( string str, int &index ) {
        
        for ( int i = 0 ; i < HASHMOD ; i ++ ) {
            
            if ( str == table5[ i ].identifier && table5[ i ].subroutine == -1 ) {
                
                index = i ;
                
                return true ;
                
            } // if()
            
        } // for()
        
        return false ;
        
    } // subroutineExist()
    
    int findSubroutine( string label ) {
        
        for ( int i = 0 ; i < HASHMOD ; i ++ ) {
            
            if ( label == table5[ i ].identifier ) {
                
                return i ;
                
            } // if()
            
        } // for()
        
        return  -1 ;
        
    } // findSubroutine()
    
    int transferToAscii( string str ) { // get an ASCII code of a string
        
        int ans = 0 ;
        
        for ( int i = 0 ; i < str.length() ; i ++ ) {
            
            ans += str[ i ] ;
            
        } // for()
        
        return ans ;
        
    } // transferToAscii()
    
    string upperToLower( string str ) {
        
        string tmpStr = "" ;
        
        for ( int i = 0 ; i < str.length() ; i ++ ) {
            
            if ( str[ i ] >= 'A' && str[ i ] <= 'Z' ) {
                
                tmpStr += str[ i ] + 32 ;
                
            } // if()
            else {
                
                tmpStr += str[ i ] ;
                
            } // else()
            
        } // for()
        
        return tmpStr ;
        
    } // upperToLower()

    string lowerToUp( string str ) {
        
        string tmpStr = "" ;
        
        for ( int i = 0 ; i < str.length() ; i ++ ) {
            
            if ( str[ i ] >= 'a' && str[ i ] <= 'z' ) {
                
                tmpStr += str[ i ] - 32 ;
                
            } // if()
            else {
                
                tmpStr += str[ i ] ;
                
            } // else()
            
        } // for()
        
        return tmpStr ;
        
    } // upperToLower()
    
    bool isIntStr( string str ) {
        
        for ( int i = 0 ; i < str.length() ; i ++ ) {
            
            if ( !( str[ i ] <= '9' && str[ i ] >= '0' ) ) {
                
                return false ;
                
            } // if()
            
        } // for()
        
        return true ;
        
    } // isInt()

    bool isFloatStr( string str ) {
        
        // before the '.' must be a legal int
        // after the '.' must also be a legal int
        
        string tmpStr1 = "" ;
        string tmpStr2 = "" ;
        
        for ( int i = 0 ; i < str.length() ; i ++ ) {
            
            if ( str[ i ] == '.' ) {
                
                tmpStr1 = str.substr( 0, i - 1 ) ;
                tmpStr2 = str.substr( i + 1 ) ;
                
                if ( isIntStr( tmpStr1 ) && isIntStr( tmpStr2 ) ) {
                    
                    return true ;
                    
                } // if()
                else {
                    
                    return false ;
                    
                } // else()
                
            } // if()
            
        } // for()
        
        return false ;
        
    } // isFloatStr()

    bool isNumStr( string str ) {
        
        // case1 : integer
        // case2 : float
        
        if ( isIntStr( str ) || isFloatStr( str ) ) {
            
            return true ;
            
        } // if()
        
        return false ;
        
    } // isNumStr()

    double strToNum( string str ) {
        
        if ( isIntStr( str ) ) {
            
            return atoi( str.c_str() ) ;
            
        } // if()
        else if ( isFloatStr( str ) ) {
            
            return atof( str.c_str() ) ;
            
        } // else if()
        else {
                
            cout << "### Error: [ " << str << " ] is not a legal number format! ###" << endl ;
                
        } // else()
        
        return -1 ;
        
    } // strToNum()
    
    string intToStr( int num ) {
            
        string tmp = "" ;
        
        if ( num == 0 ) {
            
            return "0" ;
            
        } // if()
            
        while ( num > 0 ) {
                
            char ch = '0' + ( num % 10 ) ;
            tmp = ch + tmp ;
                
            num /= 10 ;
                
        } // while()
            
        return tmp ;
            
    } // intToStr()
    
    // read any input file with string and store in vector<string> buffer
    bool readFile( string str, vector<string> &buffer ) {
        
        string name = str + ".txt" ;
        
        fileName = str ;
        
        file.open( name, ios :: in ) ;
        
        if ( file ) { // successfully open
            
            string line = "" ;
            
            while ( getline( file, line ) ) {
                
                line = cleanReturnChar( line ) ; // get rid of the '\r'
                
                buffer.push_back( line ) ;
                
            } // while()
            
        } // if()
        else {
            
            cout << "### Error: Fail to open file <" << name << "> !###" << endl << endl ;
            
            return false ;
            
        } // else()
        
        file.close() ; // close the file
        
        return true ;
        
    } // readInputFile()
    
    // now we have Table1( delimeter ) and Table2( reserved word ) to store
    void setUpTableInfo() {
        
        bool readSuccess = false ;
        
        readSuccess = readFile( "Table1", delSet ) ; // first read the delimiter file
        
        while ( !readSuccess ) {
            
            cout << "### Error: Fail to set up the delimiter! Please check whether the file exist!###" << endl << endl ;
            
            readSuccess = readFile( "Table1", delSet ) ; // first read the delimiter file
            
        } // while()
        
        readSuccess = readFile( "Table2", rsWord ) ; // second read the reserved word file
        
        while ( !readSuccess ) {
            
            cout << "### Error: Fail to set up the reserved word! Please check whether the file exist!###" << endl << endl ;
            
            readSuccess = readFile( "Table2", delSet ) ; // first read the delimiter file
            
        } // while()
        
    } // setUpTableInfo()
    
    void test_SetUpTableInfo() {
        
        setUpTableInfo() ;
        
        cout << "[Delimiter]" << endl ;
        
        for ( int i = 0 ; i < delSet.size() ; i ++ ) {
            
            cout << i + 1 << ": " << delSet[ i ] << endl ;
            
        } // for()
        
        cout << "[Reserved word]" << endl ;
        
        for ( int i = 0 ; i < rsWord.size() ; i ++ ) {
            
            cout << i + 1 << ": " << rsWord[ i ] << endl ;
            
        } // for()
        
    } // testSetUpTable
    
    void addToIdentifierTable( string name, int subroutine, int type, Unit pointer ) {
        
        int ascii = transferToAscii( name ) ;
        
        int index = ascii % HASHMOD ;
        
        // when the location has another identifier ( different name ) then keep looking for an empty space
        // when the location has the same name of the identifier but the routine is different then keep looking for an empty space
        while ( table5[ index ].identifier != ""
               || ( name == table5[ index ].identifier && subroutine != table5[ index ].subroutine ) ) { // the column is not empty
            
            index ++ ;
            
            if ( index == HASHMOD ) {
                
                index = 0 ; // start with the begin
                
            } // if()
            
        } // if()
        
        Table5Info tmp ;
        tmp.identifier = name ;
        tmp.subroutine = subroutine ;
        tmp.type = type ;
        tmp.pointer = pointer ;
        
        table5[ index ] = tmp ;
        
    } // addToIdentifierTable()
    
    void addToQuadrupleTable( vector<Unit> units, string stmtStr ) {
        
        quaForm quaStmt ;
        
        for ( int i = 0 ; i < 4 ; i ++ ) {
            
            quaStmt.units[ i ] = units[ i ] ;
            
            quaStmt.stmt = stmtStr ;
            
        } // for()
        
        table6.push_back( quaStmt ) ;
        
    } // addToQuadrupleTable()
    
    void addNumToTable( string numStr, int &tableNo, int &tableIndex ) {
        
        int ascii = transferToAscii( numStr ) ;
        
        int index = ascii % HASHMOD ;
        
        if ( isNumStr( numStr ) ) {
            
            if ( isFloatStr( numStr ) ) {
                
                while ( table4[ index ] != ""
                       && ( numStr != table4[ index ] ) ) { // the column is not empty
                    
                    index ++ ;
                    
                    if ( index == HASHMOD ) {
                        
                        index = 0 ; // start with the begin
                        
                    } // if()
                    
                } // if()
                
                if ( numStr != table4[ index ] ) {

                    table4[ index ] = numStr ; // add tot table4 real number table
                    
                    tableNo = TABLE_RNUM ;
                    
                    tableIndex = index ;
                    
                } // if()
                
            } // if()
            else if ( isIntStr( numStr) ) {
                
                while ( table3[ index ] != ""
                       && ( numStr != table3[ index ] ) ) { // the column is not empty
                    
                    index ++ ;
                    
                    if ( index == HASHMOD ) {
                        
                        index = 0 ; // start with the begin
                        
                    } // if()
                    
                } // if()
                
                if ( numStr != table3[ index ] ) {

                    table3[ index ] = numStr ; // add tot table4 real number table
                    
                    tableNo = TABLE_INT ;
                    
                    tableIndex = index ;
                    
                } // if()
                
            } // else if()
            
        } // if()
        
    } // addNumToTable()
    
    Unit findNumInTable( string numStr ) {
        
        Unit ans ;
        bool isFind = false ;
        
        for ( int i = 0 ; i < HASHMOD && !isFind ; i ++ ) {
            
            if ( numStr == table3[ i ] ) {
                
                ans.tableNo = TABLE_INT ;
                ans.index = i ;
                
                isFind = true ;
                
            } // if()
            else if ( numStr == table4[ i ] ) {
                
                ans.tableNo = TABLE_RNUM ;
                ans.index = i ;
                
                isFind = true ;
                
            } // else if()
            
        } // for()
        
        return ans ;
        
    } // findNumInTable()
    
    void addToTable0( string str ) {
        
        Token tmp ;
        
        tmp.str = str ;
        tmp.table = TABLE_TMP ;
        tmp.no = ( int ) table0.size() ;
        
        table0.push_back( tmp ) ;
        
    } // addToTable0()
    
    void addTable7( int type, vector<int> content, Unit &unit ) { // add the array information to the table7
        
        // sequentially add to the information table
        
        // return the table location
        
        unit.tableNo = TABLE_INFO ;
        unit.index = ( int ) table7.size() ;
        
        table7.push_back( type ) ; // first put in the type
        table7.push_back( ( int ) content.size() ) ; // put in the dimension num
        
        for ( int i = 0 ; i < content.size() ; i ++ ) {
            
            table7.push_back( content[ i ] ) ;
            
        } // for()
        
    } // addTable7()
    
    void addArgeToTable7( int numOfargu, vector<int> content, Unit &unit ) {
        
        unit.tableNo = TABLE_INFO ;
        unit.index = ( int ) table7.size() ;
        
        table7.push_back( numOfargu ) ;
        
        for ( int i = 0 ; i < content.size() ; i ++ ) {
            
            table7.push_back( content[ i ] ) ;
            
        } // for()
        
    } // addArgeToTable7()
    
    void addError( int index, string errorMessage ) {
        
        ErrorInfo tmp ;
        tmp.indexOfStmt = index ;
        tmp.message = errorMessage ;
        
        errorTable.push_back( tmp ) ; // add the new error message into the error table
        
    } // addError()
    
    void pringError() {
        
        for ( int i = 0 ; i < errorTable.size() ; i ++ ) {
            
            cout << "### Error " << "(Line." << errorTable[ i ].indexOfStmt + 1 << ")" << ": " << errorTable[ i ].message << "! ###" << endl ;
            
        } // for()
        
    } // printError()
    
    void printIdentifierTable() {
        
        cout << "[Identifier Table]:" << endl ;
        
        for ( int i = 0 ; i < HASHMOD ; i ++ ) {
            
            if ( table5[ i ].identifier != "" ) {
                
                cout << "Index: " << setw( 4 ) << i << " ( " << table5[ i ].identifier << ", " << table5[ i ].subroutine << " ) " <<
                table5[ i ].type << "   [pointer: (" << table5[ i ].pointer.tableNo << "," << table5[ i ].pointer.index << ")" << endl ;
                
            } // if()
            
        } // for()
        
    } // printIdentifierTable()
    
    void printTable6() {
        
        cout << endl << "[中間碼]: " << endl ;
        
        for ( int i = 0 ; i < table6.size() ; i ++ ) {
            
            cout << i + 1 << "\t(" ;
            
            for ( int j = 0 ; j < 4 ; j ++ ) {
                
                if ( table6[ i ].units[ j ].tableNo != -1 ) {
                    
                    string tmp = "(" + intToStr( table6[ i ].units[ j ].tableNo ) + "," + intToStr(  table6[ i ].units[ j ].index  ) + ")" ;
                     
                    cout << setw( 7 ) << tmp ;
                    
                } // if()
                else {
                    
                    cout << setw( 7 ) << " " ;
                    
                } // else()
                
                if ( j < 3 ) {
                    
                    cout << "," ;
                    
                } // if()
                
            } // for()
            
            cout << ")" << "\t" << table6[ i ].stmt << endl ;
            
        } // for()
        
    } // printTable6()
    
    void printInfoTable() {
        
        cout << "[Information table]: " << endl ;
        
        for ( int i = 0 ; i < table7.size() ; i ++ ) {
            
            cout << "[Index: " << i << "]   " << table7[ i ] << endl ;
            
        } // for()
        
    } // printInfoTable()
    
} ; // class G

G g ;

class LexicalAnalysis {
    
private:
    
    vector<string> tmpBuffer ;
    
public:
    
    LexicalAnalysis() {
        
        // step1 : read the origin input file
        string str = "" ;
        cout << "Enter the source input file name: " ;
        cin >> str ;
        
        while ( !g.readFile( str, tmpBuffer ) ) { // if fail to read the source file, try multiple times till success
            
            cout << "### Error: Fail to read the source file < " << str << ".txt > ! Try Again! ###" << endl << endl ;
            cout << "Enter the source input file name: " ;
            cin >> str ;
            
        } // while()
        
        // assert : now we have the original source file data store in tmpBuffer with string type
        
        // we have to transfer the string in tmpBuffer to Statement type and store in input( in "g" )
        
        for ( int i = 0 ; i < tmpBuffer.size() ; i ++ ) {
            
            Statement stmt ;
            stmt.originStr = tmpBuffer[ i ] ;
            
            g.input.push_back( stmt ) ;
            
        } // for()
        
        tmpBuffer.clear() ; // clear the tmp buffer
        
    } // constructor()
    
    void getToken() {

        for ( int i = 0 ; i < g.input.size() ; i ++ ) { // get a statement from the input data
            
            queue<char> tmpStr ;
            
            for ( int j = 0 ; j < g.input[ i ].originStr.size() ; j ++ ) { // deal with the char one by one
                
                char ch = g.input[ i ].originStr[ j ] ;
                
                if ( g.isBlank( ch ) ) { // this char is a blank
                    
                    // identify the string in the queue
                    string tokenStr = "" ;
                    
                    while ( tmpStr.size() > 0 ) {
                        
                        tokenStr += tmpStr.front() ; // get the first character and concat
                        tmpStr.pop() ; // clean the first char that we already concat
                        
                    } // while()
                    
                    // assert : now we have get a new token
                    if ( tokenStr != "" ) {
                        
                        Token tmpToken ;
                        tmpToken.str = tokenStr ;
                        
                        g.input[ i ].tokens.push_back( tmpToken ) ;
                        
                    } // if()
                    
                } // if()
                else if ( g.isDelimiter( ch ) ) { // this char is a delimiter, need to clear queue and itself
                    
                    // identify the string in the queue
                    string tokenStr = "" ;
                    
                    while ( tmpStr.size() > 0 ) {
                        
                        tokenStr += tmpStr.front() ; // get the first character and concat
                        tmpStr.pop() ; // clean the first char that we already concat
                        
                    } // while()
                    
                    // assert : now we have get a new token
                    Token tmpToken ;
                    
                    if ( tokenStr != "" ) {
                        
                        tmpToken.str = tokenStr ;
                        
                        g.input[ i ].tokens.push_back( tmpToken ) ;
                        
                    } // if()
                    
                    // assert : now make the token of this delimiter
                    
                    tmpToken.str = "" ;
                    tmpToken.str += ch ;
                    
                    g.input[ i ].tokens.push_back( tmpToken ) ;
                    
                } // else if()
                else {
                    
                    tmpStr.push( ch ) ; // maybe a part of token, so push in the queue
                    
                } // else
                
            } // for()
            
            // assert : queue tmpStr must be empty, if is not then clean it
            string tokenStr = "" ;
            while ( tmpStr.size() > 0 ) {
                
                tokenStr += tmpStr.front() ; // get the first character and concat
                tmpStr.pop() ; // clean the first char that we already concat
                
            } // while()
            
            // assert : now we have get a new token
            Token tmpToken ;
            
            if ( tokenStr != "" ) {
                
                tmpToken.str = tokenStr ;
                
                g.input[ i ].tokens.push_back( tmpToken ) ;
                
            } // if()
            
        } // for()
        
    } // getToken()
    
    void test_LexicalAbalysis() {
        
        for ( int i = 0 ; i < g.input.size() ; i ++ ) {
            
            cout << "[line" << setw(3) << i + 1 << "]: " << g.input[ i ].originStr << endl ;
            
        } // for()
        
    } // test_LexicalAbalysis()
    
    void test_getToken() {
        
        for ( int i = 0 ; i < g.input.size() ; i ++ ) {
            
            cout << "[line" << setw(3) << i + 1 << "]: " ;
            
            for ( int j = 0 ; j < g.input[ i ].tokens.size() ; j ++ ) {
                
                cout << g.input[ i ].tokens[ j ].str ;
                
                if ( j < g.input[ i ].tokens.size() - 1 ) {
                    
                    cout << " | " ;
                    
                } // if()
                
            } // for()
            
            cout << endl ;
            
        } // for()
        
    } // test_getToken()
    
} ; // class LexicalAnalysis

// this part only check for the syntax, but not about the content, so if the variable doesn't exist, it won't have error message here
class SyntaxAnalysis {
    
private:
    
    // use the queue structure to check the grammer of the statement
    // when this token is still available then keep working on the next token
    // when encounter any error then return false immediately
    
    bool isDataType( string str, int &type ) {
        
        str = g.lowerToUp( str ) ; // to make sure thar each character is in upper case
        
        if ( str == "ARRAY" ) {
            
            type = TYPE_ARRAY ;
            
            return true ;
            
        } // if()
        else if ( str == "BOOLEAN" ) {
            
            type = TYPE_BOOL ;
            
            return true ;
            
        } // else if()
        else if ( str == "CHARACTER" ) {
            
            type = TYPE_CHAR ;
            
            return true ;
            
        } // else if()
        else if ( str == "INTEGER" ) {
            
            type = TYPE_INT ;
            
            return true ;
            
        } // else if()
        else if ( str == "LABEL" ) {
            
            type = TYPE_LABEL ;
            
            return true ;
            
        } // else if()
        else if ( str == "REAL" ) {
            
            type = TYPE_REAL ;
            
            return true ;
            
        } // else if()
        
        return false ;
        
    } // isDataType()
    
    // <type>:=INTEGER|REAL|BOOLEAN
    bool _type_( int index, queue<Token> &tokens, int &type ) {
        
        Token recentToken ;
        
        if ( tokens.size() > 0 ) {
            
            recentToken = tokens.front() ;
            
        } // if()
        
        string typeName = g.lowerToUp( recentToken.str ) ;
        
        if ( typeName == "INTEGER" || typeName == "REAL" || typeName == "BOOLEAN" ) {
            
            isDataType( typeName, type ) ;
            
            tokens.pop() ;
            
            return true ;
            
        } // if()
        
        return false ;
        
    } // _type_()
    
    bool _unsignedInteger_( int index, queue<Token> &tokens ) {
        
        Token recentToken ;
        
        if ( tokens.size() > 0 ) {
            
            recentToken = tokens.front() ;
            
        } // if()
        
        for ( int i = 0 ; i < recentToken.str.length() ; i ++ ) {
            
            if ( recentToken.str[ i ] < '0' || recentToken.str[ i ] > '9' ) {
                
                return false ;
                
            } // if()
            
        } // for()
        
        tokens.pop() ;
        
        return true ;
        
    } // _unsignedInteger_()
    
    // <subscripted variable>:=<identifier>(<unsigned integer>{,<unsigned integer>})
    bool _subscriptedVariable_( int index, queue<Token> &tokens, string expected ) {
        
        Token recentToken ;
        
        if ( tokens.size() > 0 ) {
            
            recentToken = tokens.front() ;
            
        } // if()
        
        if ( expected == "" && _identifier_( index, tokens ) ) {
            
            return _subscriptedVariable_( index, tokens, "(" ) ;
            
        } // if()
        else if ( expected == "(" && recentToken.str == "(" ) {
            
            tokens.pop() ;
            
            return _subscriptedVariable_( index, tokens, "unsignInt" ) ;
            
        } // else if()
        else if ( expected == "unsignInt" && _unsignedInteger_( index, tokens ) ) {
            
            return  _subscriptedVariable_( index, tokens, ",or)" ) ;
            
        } // else if()
        else if ( expected == ",or)" ) {
            
            if ( recentToken.str == "," ) {
                
                tokens.pop() ;
                
                return _subscriptedVariable_( index, tokens, "unsignInt" ) ;
                
            } // if()
            else if ( recentToken.str == ")" ) {
                
                tokens.pop() ;
                
                return true ;
                
            } // else if()
            
        } // else if()
        
        return false ;
        
    } // _subscriptedVariable_()
    
    bool _arrayDeclaration_( int index, queue<Token> &tokens, int &type, string expected ) {
        
        Token recentToken ;
        
        if ( tokens.size() > 0 ) {
            
            recentToken = tokens.front() ;
            
        } // if()
        
        if ( expected == "" ) {
            
            if ( _type_( index, tokens, type ) ) {
                
                type = 1 ;
                
                return _arrayDeclaration_( index, tokens, type, ":" ) ;
                
            } // if()
            else {
                
                g.addError( index, "missing <type> in array declaration" ) ;
                
                return false ;
                
            } // else()
            
        } // if()
        else if ( expected == ":" ) {
            
            if ( recentToken.str == ":" ) {

                tokens.pop() ;
                
                return _arrayDeclaration_( index, tokens, type, "subscriptedVariable" ) ;
                
            } // if()
            else {
                
                g.addError( index, "missing [:] in array declaration" ) ;
                
                return false ;
                
            } // else()
            
        } // else if()
        else if ( expected == "subscriptedVariable" ) {
            
            Token tmpVar = recentToken ;
            
            if ( _subscriptedVariable_( index, tokens, "" ) ) {
                
                Unit pointer ;
                
                g.addToIdentifierTable( tmpVar.str, g.findSubroutine( g.subroutine.top() ), type, pointer ) ;
                
                if ( tokens.size() > 0 && tokens.front().str == "," ) { // there is some more arguments
                    
                    tokens.pop() ;
                    
                    return _arrayDeclaration_( index, tokens, type, "subscriptedVariable" ) ;
                    
                } // if()
                else {
                    
                    return true ;
                    
                } // else()
                
            } // if()
            
        } // else if()
        
        return false ;
        
    } // _arrayDeclaration_()
    
    bool _ArrayDeclarationPart_( int index, queue<Token> &tokens, int &type, string expected ) {
        
        Token recentToken ;
        
        if ( tokens.size() > 0 ) {
        
            recentToken = tokens.front() ; // we get the first token to check
            
        } // if()
        
        if ( expected == "" ) {
            
            if ( g.lowerToUp( recentToken.str ) == "DIMENSION" ) {
                
                tokens.pop() ; // pop out "DIMENSION"
                
                if ( _arrayDeclaration_( index, tokens, type, "" ) ) {
                    
                    return _ArrayDeclarationPart_( index, tokens, type, ";" ) ;
                    
                } // if()
                else {
                    
                    g.addError( index, "invalid array declaration" ) ;
                    
                    return false ;
                    
                } // else()
                
            } // if()
            else {
                
                g.addError( index, "missing [DIMENSION]" ) ;
                
            } // else()
            
        } // if()
        else if ( expected == ";" ) {
            
            if ( recentToken.str == ";" ) {
                
                return true ;
                
            } // if()
            else {
                
                g.addError( index, "missing [;] behind the statement" ) ;
                
                return false ;
                
            } // else()
            
        } // else if()
        
        return false ;
        
    } // _ArrayDeclarationPart_()
    
    bool parBalance( queue<Token> tokens ) { // (), (, ), ((, ))
        
        stack<string> box ;
        
        for ( Token token = tokens.front() ; !tokens.empty() ; tokens.pop(), token = tokens.front() ) {
            
            if ( token.str == "(" || token.str == ")" ) {
                
                if ( token.str == "(" ) {
                    
                    box.push( "(" ) ;
                    
                } // if()
                else if ( token.str == ")" ) {
                    
                    if ( box.empty() || box.top() == ")" ) {
                        
                        cout << "### Error: Missing '(' in the expression! ###" << endl ;
                        
                        return false ;
                        
                    } // if()
                    else if ( box.top() == "(" ) {
                        
                        box.pop() ;
                        
                    } // else if()
                    
                } // else if()
                
            } // if()
            
        } // for()
        
        if ( !box.empty() ) {
            
            cout << "### Error: Missing ')' in the expression! ###" << endl ;
            
            return false ;
            
        } // if()
        
        return true ;
        
    } // parBalance()
    
    bool isPar( string str ) {
        
        if ( str == "(" || str == ")" ) {
            
            return true ;
            
        } // if()
        
        return false ;
        
    } // isPar()
    
    bool isOp( string str ) {
        
        if ( str == "+" || str == "-" || str == "*" || str == "/" || str == "↑" || str == "#" ) {
            
            return true ;
            
        } // if()
        
        return false ;
        
    } // isOp()
    
    bool isID( string str ) {
        
        if ( !g.isRSWord( str ) && !g.isDelimiter( str[ 0 ] ) && !g.isNumStr( str ) ) {
            
            return true ;
            
        } // if()
        
        return false ;
        
    } // isID()
    
    // parameter: index -> where this processed statement is belong in the whole input buffer
    //            tokens -> the tokens from this statement
    bool _Program_( int index, queue<Token> tokens, bool hasIdentifier ) {
        // the rule of the program: first we should see the first string is PROGRAM
        // and if the next string is an available string then it will be an identifier
        
        Token recentToken = tokens.front() ; // we get the first token to check
        
        if ( g.lowerToUp( recentToken.str ) == "PROGRAM" ) { // this is the first string we expect to see
            
            // assert: the first token is correct
            
            if ( tokens.size() == 1 ) { // if there is only this token and no identifier
                
                g.addError( index, "Missing an identifier behind reserved word [PROGRAM]" ) ;
                
                return false ;
                
            } // if()
            
            tokens.pop() ;
            
            return _Program_( index, tokens, hasIdentifier ) ;
            
        } // if()
        else if ( recentToken.str == ";" && hasIdentifier ) { // this is the last string we expect to see, if there are still tokens then this statement is wrong
            
            if ( tokens.size() > 1 ) { // there is more than the token of ";"
                
                g.addError( index, "Unexpected token behind the [;]" ) ; // add error message
                
                return false ; // return that this is a wrong statement
                
            } // if()
            
            return true ;
            
        } // else if()
        else if ( tokens.size() > 0 ) { // expected that only the identifier token will go through this part
            
            // assume that there missing the ";"
            if ( tokens.size() == 1 && hasIdentifier ) {
                
                g.addError( index, "Missing [;] behind the statement" ) ;
                
                return false ;
                
            } // if()
            else if ( tokens.size() == 1 && recentToken.str == ";" ) { // there is missing identifier
                
                g.addError( index, "Missing identifier in the statement [PROGRAM]" ) ;
                
                return false ;
                
            } // else()
            
            // assert: identifier is nonormal
            
            Unit pointer ;
            pointer.tableNo = TABLE_QUADRUPLE ;
            pointer.index = 0 ;
            
            g.addToIdentifierTable( recentToken.str, -1, -1, pointer ) ; // where the first label is defined
            g.subroutine.push( recentToken.str ) ; // add to record the recent subroutine
            
            tokens.pop() ; // throw away the first one we just check
            
            return _Program_( index, tokens, true ) ; // keep checking the next token
            
        } // else()
        
        return false ;
        
    } // _Program_()
    
    // can see as a PROGRAM and VARIABLE
    bool _Subroutine_( int index, queue<Token> tokens, int &type, string expected ) {
        
        Token recentToken ;
        
        if ( tokens.size() > 0 ) {

            recentToken = tokens.front() ; // we get the first token to check
                
        } // if()
        else {
            
            g.addError( index, "Missing [;]" ) ;
            
            return false ;
            
        } // else()
        
        if ( expected == "SUBROUTINE" && g.lowerToUp( recentToken.str ) == "SUBROUTINE" ) {
            
            tokens.pop() ;
            
            return _Subroutine_( index, tokens, type, "idName" ) ;
            
        } // if()
        
        if ( expected == "idName" && !g.isRSWord( recentToken.str ) && !g.isDelimiter( recentToken.str[ 0 ] ) ) {
            
            // assert : an available id name
            
            Unit none ;
            g.addToIdentifierTable( recentToken.str, -1, type, none ) ;
            
            g.subroutine.push( recentToken.str ) ; // new subroutine
            
            tokens.pop() ;
            
            return _Subroutine_( index, tokens, type, "(" ) ;
            
        } // if()
        else if ( expected == "idName" && ( g.isRSWord( recentToken.str ) || g.isDelimiter( recentToken.str[ 0 ] ) ) ) {
            
            g.addError( index, "Unavailable id name in subroutine definition" ) ;
            
            return false ;
            
        } // else if()
        
        // assert : after the name of the subroutine, we need ( )
        
        if ( expected == "(" && recentToken.str == "(" ) {
            
            tokens.pop() ;
            
            return _Subroutine_( index, tokens, type, "dataType" ) ;
            
        } // if()
        else if ( expected == "(" && recentToken.str != "(" ) {
            
            g.addError( index, "Missing [(] in the definition of subroutine" ) ;
            
            return false ;
            
        } // else if()
        
        if ( expected == "dataType" && isDataType( recentToken.str, type ) ) { // expect datatype and if true set the parameter type
            
            tokens.pop() ;
            
            return _Subroutine_( index, tokens, type, ":" ) ;
            
        } // if()
        else if ( expected == "dataType" && !isDataType( recentToken.str, type ) ) { // missing data type
            
            g.addError( index, "Missing <data type> in the definition of subroutine" ) ;
            
            return false ;
            
        } // else()
        
        if ( expected == ":" && recentToken.str == ":" ) {
            
            tokens.pop() ;
            
            return _Subroutine_( index, tokens, type, "id" ) ;
            
        } // if()
        else if ( expected == ":" && recentToken.str != ":" ) {
            
            g.addError( index, "Missing [:] in the definition of subroutine" ) ;
            
            return false ;
            
        } // else if()
        
        if ( expected == "id" && !g.isRSWord( recentToken.str ) && !g.isDelimiter( recentToken.str[ 0 ] ) ) {
            
            // assert: need to add an new identifier
            
            Unit none ; // this situation no need the pointer
            g.addToIdentifierTable( recentToken.str, g.findSubroutine( g.subroutine.top() ), type, none ) ;
            
            tokens.pop() ;
            
            return _Subroutine_( index, tokens, type, ",or)" ) ;
            
        } // if()
        else if ( expected == "id" && ( g.isRSWord( recentToken.str ) || g.isDelimiter( recentToken.str[ 0 ] ) ) ) {
            
            g.addError( index, "Unavailable id name in the definition of subroutine" ) ;
            
            return false ;
            
        } // else if()
        
        if ( expected == ",or)" && ( recentToken.str == "," || recentToken.str == ")" ) ) {

            tokens.pop() ;
            
            if ( recentToken.str == "," ) { // keep finding next id
                
                return _Subroutine_( index, tokens, type, "id" ) ;
                
            } // if()
            else { // recentToken.str == ")"
                
                return _Subroutine_( index, tokens, type, ";" ) ;
                
            } // else()
            
        } // if()
        else if ( expected == ",or)" && recentToken.str != "," && recentToken.str != ")" ) {
            
            g.addError( index, "Missing [,] or [)] in the definition of subroutine" ) ;
            
            return false ;
            
        } // else if()
        
        if ( expected == ";" && recentToken.str == ";" ) {
            
            if ( tokens.size() > 1 ) {
                
                g.addError( index, "Unexpected token occur after [;]" ) ;
                
                return false ;
                
            } // if()
            
            return true ;
            
        } // if()
        
        return false ;
        
    } // _Subroutine_()
    
    bool _variableDeclaration_( int index, queue<Token> &tokens, int &type, string expected ) {
        
        Token recentToken ;
        
        if ( tokens.size() > 0 ) {
            
            recentToken = tokens.front() ;
            
        } // iF()
        
        if ( expected == "" ) {
        
            if ( _type_( index, tokens, type ) ) {
                
                return _variableDeclaration_( index, tokens, type, ":" ) ;
                
            } // if()
            else {
                
                g.addError( index, "missing <type> in variable declaration" ) ;
                
                return false ;
                
            } // else()
            
        } // if()
        else if ( expected == ":" ) {
            
            if ( recentToken.str == ":" ) {
                
                tokens.pop() ;
                
                return _variableDeclaration_( index, tokens, type, "identifier" ) ;
                
            } // if()
            else {
                
                g.addError( index, "missing [:] in variable declaration" ) ;
                
                return false ;
                
            } // else()
            
        } // else if()
        else if ( expected == "identifier" ) {
            
            Token tmpIdent = recentToken ;
            
            if ( _identifier_( index, tokens ) ) {
                
                Unit pointer ;
                
                g.addToIdentifierTable( tmpIdent.str, g.findSubroutine( g.subroutine.top() ), type, pointer ) ;
                
                if ( tokens.size() > 0 && tokens.front().str == "," ) { // there is more than one identifier
                    
                    tokens.pop() ;
                    
                    return _variableDeclaration_( index, tokens, type, "identifier" ) ;
                    
                } // if()
                else {
                    
                    return true ;
                    
                } // else()
                
            } // if()
            else {
                
                g.addError( index, "missing identifier in variable declaration" ) ;
                
                return false ;
                
            } // else()
            
        } // else if()
        
        
        return false ;
        
    } // _variableDeclaration_()
    
    bool _variableDeclarationPart_( int index, queue<Token> tokens, string expected ) {
        
        Token recentToken ;
        
        if ( tokens.size() > 0 ) {
            
            recentToken = tokens.front() ;
            
        } // if()
        
        if ( expected == "" ) {
            
            if ( g.lowerToUp( recentToken.str ) == "VARIABLE" ) {
                
                tokens.pop() ;
                
                int type = -1 ;
                
                if ( _variableDeclaration_( index, tokens, type, "" ) ) {
                    
                    return _variableDeclarationPart_( index, tokens, ";" ) ;
                    
                } // if()
                else {
                    
                    return false ;
                    
                } // else()
                
            } // if()
            else {
                
                g.addError( index, "missing [VARIABLE]" ) ;
                
                return false ;
                
            } // else()
            
        } // if()
        else if ( expected == ";" ) {
            
            if ( recentToken.str == ";" ) {
                
                return true ;
                
            } // if()
            else {
                
                g.addError( index, "miaaing [;] behind the statement" ) ;
                
                return false ;
                
            } // else()
            
        } // else if()
        
        return false ;
        
    } // _variableDeclarationPart_()
    
    bool _label_( int index, queue<Token> &tokens ) {
        
        Token recentToken ;
        
        if ( tokens.size() > 0 ) {
            
            recentToken = tokens.front() ;
            
        } // if()
        
        if ( _identifier_( index, tokens ) ) {
            
            return true ;
            
        } // if()
        
        return false ;
        
    } // _label_()
    
    bool _labelDeclarationPart_( int index, queue<Token> tokens, string expected ) {
        
        Token recentToken ;
        
        if ( tokens.size() > 0 ) {
            
            recentToken = tokens.front() ;
            
        } // if()
        
        if ( expected == "" ) { // "LABEL
            
            if ( g.lowerToUp( recentToken.str ) == "LABEL" ) {
                
                tokens.pop() ;
                
                return _labelDeclarationPart_( index, tokens, "label" ) ;
                
            } // if()
            else {
                
                g.addError( index, "missing [LABEL]" ) ;
                
                return false ;
                
            } // else()
            
        } // if()
        else if ( expected == "label" ) {
            
            Token tmpLabel = tokens.front() ;
            
            if ( _label_( index, tokens ) ) {
                
                Unit pointer ;
                
                g.addToIdentifierTable( tmpLabel.str, g.findSubroutine( g.subroutine.top() ), TYPE_LABEL, pointer ) ;
                
                return _labelDeclarationPart_( index, tokens, ",or;" ) ;
                
            } // if()
            else {
                
                g.addError( index, "invalid label" ) ;
                
                return false ;
                
            } // else()
            
        } // else if ()
        else if ( expected == ",or;" ) {
            
            if ( recentToken.str == "," ) {
                
                tokens.pop() ;
                
                return _labelDeclarationPart_( index, tokens, "label" ) ;
                
            } // if()
            else if ( recentToken.str == ";" ) {
                
                return true ;
                
            } // else if()
            
        } // else if()
        
        return false ;
        
    } // _labelDeclarationPart_()
    
    bool _relationOp_( Token token ) {
        
        string tokenName = g.lowerToUp( token.str ) ;
        
        if ( tokenName == "EQ" || tokenName == "NE" || tokenName == "GT" || tokenName == "GE" || tokenName == "LT" || tokenName == "LE" ) {
            
            return true ;
            
        } // if()
        
        return false ;
        
    } // _relationOp_()
    
    bool _relations_( Token token ) {
        
        if ( _relationOp_( token ) || g.lowerToUp( token.str ) == "OR" || g.lowerToUp( token.str ) == "AND" ) {
            
            return true ;
            
        } // if()
        
        return false ;
        
    } // isRelations
    
    // <variable>:=<identifier>|<idnetifier>(<unsigned integer>|<identifier>{,<unsigned integer>| <identifier> })
    bool _variable_( int index, queue<Token> &tokens, string expected ) {
        
        Token recentToken ;
        
        if ( tokens.size() > 0 ) {
            
            recentToken = tokens.front() ;
            
        } // if()
        
        int indexTmp = -1 ;
        
        if ( expected == "" && g.variableExist( recentToken.str, g.findSubroutine( g.subroutine.top() ), indexTmp ) ) { // 改
            
            tokens.pop() ;
            
            if ( tokens.size() > 0 && tokens.front().str == "(" ) { // keep tracking
                
                return _variable_( index, tokens, "(" ) ;
                
            } // if()
            else {
                
                return true ;
                
            } // else()
            
        } // if()
        
        if ( expected == "(" && recentToken.str == "(" ) {
            
            tokens.pop() ;
            
            return _variable_( index, tokens, "index" ) ;
            
        } // if()
        else if ( expected == "(" && recentToken.str != "(" ) {
            
            g.addError( index, "missing [(] in the variable" ) ;
            
            return false ;
            
        } // else()
        
        if ( expected == "index" ) {
            
            // this token must be a variable or a integer
            
            int index = -1 ;
            
            if ( g.variableExist( recentToken.str, g.findSubroutine( g.subroutine.top() ), index ) || g.isIntStr( recentToken.str ) ) {
                
                tokens.pop() ;
                
                if ( tokens.size() > 0 && tokens.front().str == "," ) { // there is more arguments
                    
                    tokens.pop() ; // pop out ","
                    
                    return _variable_( index, tokens, "index" ) ;
                    
                } // if()
                
                return _variable_( index, tokens, ")" ) ;
                
            } // if()
            else {
                
                g.addError( index, "invalid index in the variable" ) ;
                
                return false ;
                
            } // else()
            
        } // if()
        
        if ( expected == ")" && recentToken.str == ")" ) {
            
            tokens.pop() ;
            
            return true ;
            
        } // if()
        else if ( expected == ")" && recentToken.str != ")" ) {
            
            g.addError( index, "missing [)] in the variable" ) ;
            
            return false ;
            
        } // else()
        
        return false ;
        
    } // _variable_()
    
    bool _unsignConstant_( int index, queue<Token> &tokens ) {
        
        Token recentToken ;
        
        if ( tokens.size() > 0 ) {
            
            recentToken = tokens.front() ;
            
        } // if()
        
        if ( recentToken.str == "+" || recentToken.str == "-" ) {
            
            tokens.pop() ;
            
            return false ;
            
        } // if()
        else {
            
            if ( g.isNumStr( recentToken.str ) ) {
                
                tokens.pop() ;
                
                return true ;
                
            } // if()
            else {
                
                g.addError( index, "invelid unsign constant" ) ;
                
                return false ;
                
            } // else()
            
        } // else()
        
    } // _unsignConstant_()
    
    bool _constant_( int index, queue<Token> &tokens ) {
        
        Token recentToken ;
        
        if ( tokens.size() > 0 ) {
            
            recentToken = tokens.front() ;
            
        } // if()
        
        if ( recentToken.str == "+" || recentToken.str == "-" ) {
            
            tokens.pop() ;
            
            return _constant_( index, tokens ) ;
            
        } // if()
        else {
            
            if ( g.isNumStr( recentToken.str ) ) {
                
                tokens.pop() ;
                
                return true ;
                
            } // if()
            else {
                
                g.addError( index, "invelid constant" ) ;
                
                return false ;
                
            } // else()
            
        } // else()
        
    } // _constant_()
    
    bool _conditionVariable_( int index, queue<Token> &tokens ) {
        // stop recursion when meet <relation>
        
       if ( _variable_( index, tokens, "" ) || _constant_( index, tokens ) ) {
            
            return true ;
            
        } // if()
        else {
            
            return false ;
            
        } // else()
        
    } // _conditionVariable_()
    
    bool _condition_( int index, queue<Token> &tokens, string expected ) {
        
        Token recentToken ;
        
        if ( tokens.size() > 0 ) {
            
            recentToken = tokens.front() ;
            
        } // if()
        
        if ( g.lowerToUp( recentToken.str ) == "THEN" ) { // condition part is over
            
            // no need to pop
            
            return true ; // for now, the statement is still correct, move on to the then part
            
        } // if()
        
        if ( expected == "conditionVariable1" ) { // <condition variable>:=<variable>|<constant>
            
            if ( _conditionVariable_( index, tokens ) ) {
                
                return _condition_( index, tokens, "relations" ) ;
                
            } // if()
            
        } // if()
        
        if ( expected == "relations" ) {
            
            if ( _relations_( recentToken ) ) {
                
                tokens.pop() ;
                
                return _condition_( index, tokens, "conditionVariable2" ) ;
                
            } // if()
            else {
                
                g.addError( index, "invalid relation in condition" ) ;
                
                return false ;
                
            } // else()
            
        } // if()
        
        if ( expected == "conditionVariable2" ) {
            
            if ( _conditionVariable_( index, tokens ) ) {
                
                return true ;
                
            } // if()
            
        } // if()
        
        return false ;
        
    } // _condition_()
    
    // <adding operator>:=  +|-|OR
    bool _addingOp_( int index, queue<Token> &tokens ) {
        
        Token recentToken ;
        
        if ( tokens.size() > 0 ) {
            
            recentToken = tokens.front() ;
            
        } // if()
        
        if ( recentToken.str == "+" || recentToken.str == "-" || g.lowerToUp( recentToken.str ) == "OR" ) {
            
            tokens.pop() ;
            
            return true ;
            
        } // if()
        
        return false ;
        
    } // _addingOp_()
    
    // <multiplying operator>:=*|/|AND|↑
    bool _multiplyingOp_( int index, queue<Token> &tokens ) {
        
        Token recentToken ;
        
        if ( tokens.size() > 0 ) {
            
            recentToken = tokens.front() ;
            
        } // if()
        
        if ( recentToken.str == "*" || recentToken.str == "↑" || g.lowerToUp( recentToken.str ) == "AND" || recentToken.str == "#" ) {
            
            tokens.pop() ;
            
            return true ;
            
        } // if()
        
        return false ;
        
    } // _multiplyingOp_()
    
    // <factor>:=<variable>|<unsigned constant>|(<expression>)
    bool _factor_( int index, queue<Token> &tokens, string expexted ) {
        
        Token recentToken ;
        
        if ( tokens.size() > 0 ) {
            
            recentToken = tokens.front() ;
            
        } // if()
        
        if ( recentToken.str == "(" ) { // (<expression>
            
            tokens.pop() ;
            
            if ( _expression_( index, tokens ) ) {
                
                if ( tokens.front().str == ")" ) {
                    
                    tokens.pop() ;
                    
                    return true ;
                    
                } // if()
                else {
                    
                    g.addError( index, "missing [)] in the factor" ) ;
                    
                    return false ;
                    
                } // else()
                
            } // if()
            
        } // if()
        else {
            
            queue<Token> tmpQueue = tokens ;
            
            if ( _variable_( index, tokens, "" ) ) {
                
                return true ;
                
            } // if()
           
            tokens = tmpQueue ;
            
            if ( _unsignConstant_( index, tokens ) ) {
                
                return true ;
                
            } // if()
            
        } // else()
        
        g.addError( index, "invalid factor" ) ;
        
        return false ;
        
    } // _factor_()
    
    // <term>:=<factor>|<term><multiplying operator><factor>
    bool _term_( int index, queue<Token> &tokens ) {
        
        Token recentToken ;
        
        if ( tokens.size() > 0 ) {
            
            recentToken = tokens.front() ;
            
        } // if()
        
        queue<Token> tmpQueue = tokens ;
        
        if ( _factor_( index, tokens, "" ) ) { // <factor>
            
            if ( !tokens.empty() && tokens.front().str != ";" ) {
                
                if ( _multiplyingOp_( index, tokens ) ) { // changehere
                    
                    if ( _term_( index, tokens ) ) {
                        
                        return true ;
                        
                    } // if()
                    
                } // if()
                
            } // if()
            
            return true ;
            
        } // if()
        /*
        tokens = tmpQueue ;
        
        if ( _term_( index, tokens ) ) { // <term><multiplying operator><factor>
            
            if ( _multiplyingOp_( index, tokens ) ) {
                
                if ( _factor_( index, tokens, "factor" ) ) {
                    
                    return true ;
                    
                } // if()
                
            } // if()
            
        } // if()
        */
        g.addError( index, "invalid term" ) ;
        
        return false ;
        
    } // _term_()
    
    bool _sign_( int index, queue<Token> &tokens ) {
        
        Token recentToken ;
        
        if ( tokens.size() > 0 ) {
            
            recentToken = tokens.front() ;
            
        } // if()
        
        if ( recentToken.str == "+" || recentToken.str == "-" ) {
            
            tokens.pop() ;
            
            return true ;
            
        } // if()
        
        g.addError( index, "missing sign" ) ;
        
        return false ;
        
    } // _sign_()
    
    // <simple expression>:=<term>|<sign><term>|<simple expression><adding operator><term>
    bool _simpleExpression_( int index, queue<Token> &tokens ) {
        
        Token recentToken ;
        
        if ( tokens.size() > 0 ) {
            
            recentToken = tokens.front() ;
            
        } // if()
        
        if ( _term_( index, tokens ) ) { // <term>
            
            if ( !tokens.empty() && tokens.front().str != ";" && tokens.front().str != ")" ) {
                
               if ( _addingOp_( index, tokens ) ) {
                    
                    if ( _term_( index, tokens ) ) {
                        
                        return true ;
                        
                    } // if()
                    else {
                        
                        return false ;
                        
                    } // else()
                    
                } // if()
               else {
                   
                   return false ;
                   
               } // else()
                
            } // if()
            
            return true ;
            
        } // if()
        
        g.addError( index, "invalid simple expression" ) ;
        
        return false ;
        
    } // _simpleExpression_()
    
    // <expression>:=<simple expression>|<simple expression><relational operator><simple expression>
    bool _expression_( int index, queue<Token> &tokens ) {
        
        Token recentToken ;
        
        if ( tokens.size() > 0 ) {
            
            recentToken = tokens.front() ;
            
        } // if()
        
        if ( _simpleExpression_( index, tokens ) ) {
            
            if ( !tokens.empty() && _relationOp_( tokens.front() ) ) {
                
                tokens.pop() ; // pop out the relational operation
                
                if ( tokens.size() > 0 && _simpleExpression_( index, tokens ) ) { // <simple expression><relational operator><simple expression>
                    
                    return true ;
                    
                } // if()
                
            } // if()
            else {
                
                return true ; // <simple expression>
                
            } // else()
            
        } // if()
        
        g.addError( index, "invalid expression" ) ;
        
        return false ;
        
    } // _expression_()
    
    bool _AssignStatement_( int index, queue<Token> &tokens, string expected ) {
        
        Token recentToken ;
        
        if ( tokens.size() > 0 ) {
            
            recentToken = tokens.front() ;
            
        } // if()
        
        if ( expected == "" ) {
            
            if ( _variable_( index, tokens, "" ) ) {

                return _AssignStatement_( index, tokens, "=" ) ;
                
            } // if()
            else {
                
                return false ;
                
            } // else()
            
        } // if()
        else if ( expected == "=" ) {
            
            if ( recentToken.str == "=" ) {
                
                tokens.pop() ;
                
                return _AssignStatement_( index, tokens, "expression" ) ;
                
            } // if()
            else {
                
                return false ;
                
            } // else()
            
        } // else if()
        else if ( expected == "expression" ) {
            
            if ( _expression_( index, tokens ) ) {
                
                return true ;
                
            } // if()
            
        } // else if()
        
        return false ;
        
    } // _AssignStatement_()
    
    // <argument>:= <identifier>|<constant>
    bool _argument_( int index, queue<Token> &tokens ) {
        
        Token recentToken ;
        
        if ( tokens.size() > 0 ) {
            
            recentToken = tokens.front() ;
            
        } // if()
        
        queue<Token> tmpQueue = tokens ;
        
        if ( _identifier_( index, tokens ) ) {
            
            return true ;
            
        } // if()
        
        tokens = tmpQueue ; // recover
        
        if ( _constant_( index, tokens ) ) {
            
            return true ;
            
        } // if()
        
        g.addError( index, "invalid argument" ) ;
        
        return false ;
        
    } // _argument_()
    
    // <identifier>:=<letter>{<letter>|<digit>}
    bool _identifier_( int index, queue<Token> &tokens ) {
        
        Token recentToken ;
        
        if ( tokens.size() > 0 ) {
            
            recentToken = tokens.front() ;
            
        } // if()
        
        if ( recentToken.str[ 0 ] >= '0' && recentToken.str[ 0 ] <= '9' ) {
            
            g.addError( index, "invalid identifier" ) ;
            
            return false ;
            
        } // if()
        
        tokens.pop() ;
        
        return true ;
        
    } // _identifier_()
    
    // <subroutine identifier>:=<idnetifier>
    bool _subroutineIdentifier_( int index, queue<Token> &tokens ) {
        
        Token recentToken ;
        
        if ( tokens.size() > 0 ) {
            
            recentToken = tokens.front() ;
            
        } // if()
        
        if ( _identifier_( index, tokens ) ) {
            
            return true ;
            
        } // if()
        
        g.addError( index, "invalid subroutine identifier" ) ;
        
        return false ;
        
    } // _subroutineIdentifier_()
    
    // <call statement>:=CALL<subroutine identifier>(<argument>{,<argument>})
    bool _CallStatement_( int index, queue<Token> &tokens, string expected ) {
        
        Token recentToken ;
        
        if ( tokens.size() > 0 ) {
            
            recentToken = tokens.front() ;
            
        } // if()
        
        if ( expected == "" ) {
            
            tokens.pop() ;
            
            return _CallStatement_( index, tokens, "subroutineIdentifer" ) ;
            
        } // if()
        else if ( expected == "subroutineIdentifer" && _subroutineIdentifier_( index, tokens ) ) {
            
            return _CallStatement_( index, tokens, "(" ) ;
            
        } // if()
        else if ( expected == "(" && recentToken.str == "(" ) {
            
            tokens.pop() ;
            
            return _CallStatement_( index, tokens, "argument" ) ;
            
        } // if()
        else if ( expected == "argument" && _argument_( index, tokens ) ) {
            
            return _CallStatement_( index, tokens, ",or)" ) ;
            
        } // if()
        else if ( expected == ",or)" ) {
            
            if ( recentToken.str == "," ) {
                
                tokens.pop() ;
                
                return _CallStatement_( index, tokens, "argument" ) ;
                
            } // if()
            else if ( recentToken.str == ")" ) {
                
                tokens.pop() ;
                
                return true ;
                
            } // else()
            
        } // else if()
        
        return false ;
        
    } // _callStatement()
    
    bool _IOStatement_( int index, queue<Token> tokens ) {
        
        Token recentToken ;
        
        if ( tokens.size() > 0 ) {
            
            recentToken = tokens.front() ;
            
        } // if()
        
        if ( g.lowerToUp( recentToken.str ) == "INPUT" || g.lowerToUp( recentToken.str ) == "OUTPUT" ) {
            
            if ( _variable_( index, tokens, "" ) ) {
                    
                return true ;
                
            } // if()
            
        } // if()
        
        return false ;
        
    } // _IOStatement_()
    
    bool _GotoStatement_( int index, queue<Token> &tokens, string expected ) {
        
        Token recentToken ;
        
        if ( tokens.size() > 0 ) {
            
            recentToken = tokens.front() ;
            
        } // if()
        
        int labelIndex = -1 ;
        
        if ( expected == "" ) {
            
            tokens.pop() ;
            
            return _GotoStatement_( index, tokens, "label" ) ;
            
        } // if()
        else if ( expected == "label" && g.labelExist( recentToken.str, labelIndex ) ) {
            
            tokens.pop() ;
            
            return true ;
            
        } // else if()
        
        g.addError( index, "invalid <GTO> statement" ) ;
        
        return false ;
        
    } // gotoStatement()
    
    // <empty statement>:=
    bool _emptyStatement_( int index, queue<Token> tokens ) {
        
        Token recentToken ;
        
        if ( tokens.size() > 0 ) {
            
            recentToken = tokens.front() ;
            
        } // if()
        
        if ( recentToken.str == ";" || tokens.size() == 0 ) {
            
            return true ;
            
        } // if()
        
        return false ;
        
    } // _emptyStatement_()
    
    // <statement I>:=<empty statement>|<assign statement>|<call statement>|<IO statement>|<go to statement>
    bool _statementI_( int index, queue<Token> &tokens ) {
        
        Token recentToken ;
        
        if ( tokens.size() > 0 ) {
            
            recentToken = tokens.front() ;
            
        } // if()
        
        queue<Token> tmpQ1, tmpQ2, tmpQ3, tmpQ4 ;
        tmpQ1 = tokens ;
        tmpQ2 = tokens ;
        tmpQ3 = tokens ;
        tmpQ4 = tokens ;
        
        if ( _emptyStatement_( index, tokens ) ) {
            
            return true ;
            
        } // if()
        else if ( _AssignStatement_( index, tmpQ1, "" ) ) {
            
            tokens = tmpQ1 ;
            
            return true ;
            
        } // else if()
        else if ( _CallStatement_( index, tmpQ2, "call" ) ) {
            
            tokens = tmpQ2 ;
            
            return true ;
            
        } // else if()
        else if ( _IOStatement_( index, tmpQ3 ) ) {
            
            tokens = tmpQ3 ;
            
            return true ;
            
        } // else if()
        else if ( _GotoStatement_( index, tmpQ4, "" ) ) {
            
            tokens = tmpQ4 ;
            
            return true ;
            
        } // else if()
        
        return false ;
        
    } // _statementI_()
    
    bool _IfStatement_( int index, queue<Token> &tokens, string expected ) {
        
        // two situations :
        // first: If <condition> Then <statement I>
        // second: If <condition> Then <statement I> Else <statement I>
        
        // strategy: seperate the whole statement and check the different part
        Token recentToken ;
        
        if ( tokens.size() > 0 ) {
            
            recentToken = tokens.front() ;
            
        } // if()
        
        if ( expected == "" && g.lowerToUp( recentToken.str ) == "IF" ) {
            
            tokens.pop() ;
            
            return _IfStatement_( index, tokens, "condition" ) ;
            
        } // if()
        else if ( expected == "condition" ) {
            
            if ( _condition_( index, tokens, "conditionVariable1" ) ) {
                
                return _IfStatement_( index, tokens, "THEN" ) ;
                
            } // if()
            
        } // if()
        else if ( expected == "THEN" && g.lowerToUp( recentToken.str ) == "THEN" ) {
            
            tokens.pop() ;
            
            return _IfStatement_( index, tokens, "statementI" ) ;
            
        } // if()
        else if ( expected == "statementI" && _statementI_( index, tokens ) ) {
            
            if ( ( tokens.size() > 0 && tokens.front().str == ";" ) || tokens.size() == 0 ) {
                
                return true ;
                
            } // if()
            else { // ELSE part
                
                recentToken = tokens.front() ;
                
                if ( g.lowerToUp( recentToken.str ) == "ELSE" ) {
                    
                    tokens.pop() ;
                    
                    return _IfStatement_( index, tokens, "statementI" ) ;
                    
                } // if()
                
            } // else()
            
        } // else if()
        
         g.addError( index, "missing [THEN] in <IF> statement" ) ;
        
        return false ;
        
    } // _IfStatement_()
 
    queue<Token> makeQueue( vector<Token> tokens ) {
        
        queue<Token> queue ;
        
        for ( int i = 0 ; i < tokens.size() ; i ++ ) {
            
            queue.push( tokens[ i ] ) ;
            
        } // for()
        
        return queue ;
        
    } // makeQueue()
    
    void checkGrammer( int index, Statement &stmt ) {
        
        string fToken ;
        queue<Token> qToken = makeQueue( stmt.tokens ) ;
        int type = -1 ;

        // case1: assignment
        // case2: label
        // find the first token to distinguish what the token is
        int labelIndex = -1 ;
        
        if ( g.labelExist( stmt.tokens[ 0 ].str, labelIndex ) && stmt.tokens[ 1 ].str != "=" ) { // the first token is a label ( must has been added to the table )
            
            fToken = stmt.tokens[ 1 ].str ;
            stmt.instr = stmt.tokens[ 1 ] ;
            stmt.label = stmt.tokens[ 0 ].str ;
            
            if ( fToken != "(" ) {
            
                qToken.pop() ; // remove the label from the queue, in case of promblem exist during grammer checking
                
            } // if()
            
        } // if()
        else {
            
            fToken = stmt.tokens[ 0 ].str ;
            stmt.instr = stmt.tokens[ 0 ] ;
            
        } // else()
        
        if ( fToken == "PROGRAM" ) {
            
            if ( _Program_( index, qToken, false ) ) {
                
                stmt.gramCorrect = true ;
                
            } // if()
            else {
                
                stmt.gramCorrect = false ;
                
                g.subroutine.push( "error" ) ;
                
            } // else()
            
        } // if()
        else if ( fToken == "VARIABLE" ) {
            
            if ( _variableDeclarationPart_( index, qToken, "" ) ) {
                
                stmt.gramCorrect = true ;
                
            } // if()
            else {
                
                stmt.gramCorrect = false ;
                
            } // else()
            
        } // else if()
        else if ( fToken == "DIMENSION" ) {
            
            if ( _ArrayDeclarationPart_( index, qToken, type, "" ) ) {
                
                stmt.gramCorrect = true ;
                
            } // if()
            else {
                
                stmt.gramCorrect = false ;
                
            } // else()
            
        } // else if()
        else if ( fToken == "LABEL" ) {
            
            if ( _labelDeclarationPart_( index, qToken, "" ) ) {
                
                stmt.gramCorrect = true ;
                
            } // if()
            else {
                
                stmt.gramCorrect = false ;
                
            } // else()
            
        } // else if()
        else if ( fToken == "GTO" ) {
            
            if ( _GotoStatement_( index, qToken, "" ) ) {
                
                if ( qToken.front().str == ";" ) {
                    
                    stmt.gramCorrect = true ;
                    
                } // if()
                else {
                    
                    g.addError( index, "missing [;] behind the statement" ) ;
                    
                } // else()
                
            } // if()
            else {
                
                stmt.gramCorrect = false ;
                
            } // else()
            
        } // else if()
        else if ( fToken == "SUBROUTINE" ) {
            
            if ( _Subroutine_( index, qToken, type, "SUBROUTINE" ) ) {
                
                stmt.gramCorrect = true ;
                
            } // if()
            else {
                
                stmt.gramCorrect = false ;
                
            } // else()
            
        } // else if()
        else if ( fToken == "CALL" ) {
            
            if ( _CallStatement_( index, qToken, "" ) ) {
                
                if ( qToken.front().str == ";" ) {
                    
                    stmt.gramCorrect = true ;
                    
                } // if()
                else {
                    
                    g.addError( index, "missing [;] behind the statement" ) ;
                    
                } // else()
                
            } // if()
            else {
                
                stmt.gramCorrect = false ;
                
            } // else()
            
        } // else if()
        else if ( fToken == "IF" ) {
            
            if ( _IfStatement_( index, qToken, "" ) ) {
                
                if ( qToken.front().str == ";" ) {
                    
                    stmt.gramCorrect = true ;
                    
                } // if()
                else {
                    
                    g.addError( index, "missing [;] behind the statement" ) ;
                    
                } // else()
                
            } // if()
            else {
                
                stmt.gramCorrect = false ;
                
            } // else()
            
        } // else if()
        else if ( fToken == "ENP" || fToken == "ENS" || stmt.tokens[ 1 ].str == "END" || stmt.tokens[ 1 ].str == "ENS" ) {
             
            g.subroutine.pop() ;
            
        } // else if()
        else { // ASSIGNMENT
            
            if ( _AssignStatement_( index, qToken, "" ) ) {
                
                if ( qToken.empty()) {
                    
                    g.addError( index, "missing [;] behind the statement" ) ;
                    
                    stmt.gramCorrect = false ;
                    
                } // if()
                else {
                    
                    stmt.gramCorrect = true ;
                    
                } // else()
                
            } // if()
            else {
                
                stmt.gramCorrect = false ;
                
            } // else()
            
        } // else()
        
    } // checkGrammer()
    
public:
    
    void syntax_analyze() { // used to check eack statement whether the statement is legal
        
        for ( int i = 0 ; i < g.input.size() ; i ++ ) {
            
            checkGrammer( i, g.input[ i ] ) ;
            
        } // for()
        
    } // syntaxAnalysis()
    
    void testSA() {
        
        for ( int i = 0 ; i < g.input.size() ; i ++ ) {
            
            cout << g.input[ i ].originStr << " [" ;
            
            if ( g.input[ i ].gramCorrect ) {
                
                cout << "true] Instr:" << g.input[ i ].instr.str << "   [label: " << g.input[ i ].label << "]" << endl ;
                
            } // if()
            else {
                
                cout << "false] Instr:" << g.input[ i ].instr.str << endl ;
                
            } // else()
            
        } // for()
        
    } // testSA()
    
    void testIsProgram() {
           
        queue<Token> queue ;
        Token tmp1 ;
        tmp1.str = "PROGRAM" ;
        Token tmp2 ;
        tmp2.str = "main" ;
        Token tmp3 ;
        tmp3.str = ";" ;
        Token tmp4 ;
        tmp4.str = "str" ;
           
        queue.push( tmp1 ) ;
        queue.push( tmp2 ) ;
        queue.push( tmp3 ) ;
        //queue.push( tmp4 ) ;
        
        if ( _Program_( 1, queue, false ) ) {
        
             cout << "The result of syntax analysis (program) : true" << endl ;
               
        } // if()
        else {
            
            cout << "The result of syntax analysis (program) : false" << endl ;
            
        } // else()
         
    } // testIsProgram()
    
    void testIsVariable() {
           
        queue<Token> queue ;
        Token tmp1 ;
        tmp1.str = "VARIABLE" ;
        Token tmp2 ;
        tmp2.str = "INTEGER" ;
        Token tmp3 ;
        tmp3.str = ":" ;
        Token tmp4 ;
        tmp4.str = "X" ;
        Token tmp5 ;
        tmp5.str = "," ;
        Token tmp6 ;
        tmp6.str = "Y" ;
        Token tmp7 ;
        tmp7.str = ";" ;
           
        queue.push( tmp1 ) ;
        queue.push( tmp2 ) ;
        queue.push( tmp3 ) ;
        queue.push( tmp4 ) ;
        queue.push( tmp5 ) ;
        queue.push( tmp6 ) ;
        queue.push( tmp7 ) ;
        
        if ( _variableDeclarationPart_( 1, queue, "" ) ) {
        
             cout << "The result of syntax analysis (variable) : true" << endl ;
               
        } // if()
        else {
            
            cout << "The result of syntax analysis (variable) : false" << endl ;
            
        } // else()
         
    } // testIsProgram()
    
    void testIsDimension() {
           
        queue<Token> queue ;
        Token tmp1 ;
        tmp1.str = "DIMENSION" ;
        Token tmp2 ;
        tmp2.str = "INTEGER" ;
        Token tmp3 ;
        tmp3.str = ":" ;
        Token tmp4 ;
        tmp4.str = "A" ;
        Token tmp5 ;
        tmp5.str = "(" ;
        Token tmp6 ;
        tmp6.str = "12" ;
        Token tmp7 ;
        tmp7.str = ")" ;
        Token tmp8 ;
        tmp8.str = ";" ;
           
        queue.push( tmp1 ) ;
        queue.push( tmp2 ) ;
        queue.push( tmp3 ) ;
        queue.push( tmp4 ) ;
        queue.push( tmp5 ) ;
        queue.push( tmp6 ) ;
        queue.push( tmp7 ) ;
        queue.push( tmp8 ) ;
        
        int type = -1 ;
        
        if ( _ArrayDeclarationPart_( 1, queue, type, "" ) ) {
        
             cout << "The result of syntax analysis (DIMENSION) : true" << endl ;
               
        } // if()
        else {
            
            cout << "The result of syntax analysis (DIMENSION) : false" << endl ;
            
        } // else()
         
    } // testIsProgram()
    
    void testIsLabel() {
           
        queue<Token> queue ;
        Token tmp1 ;
        tmp1.str = "LABEL" ;
        Token tmp2 ;
        tmp2.str = "L91" ;
        Token tmp3 ;
        tmp3.str = "," ;
        Token tmp4 ;
        tmp4.str = "L92" ;
        Token tmp5 ;
        tmp5.str = ";" ;
           
        queue.push( tmp1 ) ;
        queue.push( tmp2 ) ;
        queue.push( tmp3 ) ;
        queue.push( tmp4 ) ;
        queue.push( tmp5 ) ;
        
        if ( _labelDeclarationPart_( 1, queue, "" ) ) {
        
             cout << "The result of syntax analysis (LABEL) : true" << endl ;
               
        } // if()
        else {
            
            cout << "The result of syntax analysis (LABEL) : false" << endl ;
            
        } // else()
         
    } // testIsProgram()
    
    void testIsGto() {
           
        queue<Token> queue ;
        Token tmp1 ;
        tmp1.str = "GTO" ;
        Token tmp2 ;
        tmp2.str = "L91" ;
        Token tmp3 ;
        tmp3.str = "," ;
        Token tmp4 ;
        tmp4.str = "L92" ;
        Token tmp5 ;
        tmp5.str = ";" ;
           
        queue.push( tmp1 ) ;
        queue.push( tmp2 ) ;
        //queue.push( tmp3 ) ;
        //queue.push( tmp4 ) ;
        queue.push( tmp5 ) ;
        
        if ( _GotoStatement_( 1, queue, "" ) ) {
        
             cout << "The result of syntax analysis (GTO) : true" << endl ;
               
        } // if()
        else {
            
            cout << "The result of syntax analysis (GTO) : false" << endl ;
            
        } // else()
         
    } // testIsProgram()
    
    void testIsSubroutine() {
           
        queue<Token> queue ;
        Token tmp1 ;
        tmp1.str = "SUBROUTINE" ;
        Token tmp2 ;
        tmp2.str = "A3" ;
        Token tmp3 ;
        tmp3.str = "(" ;
        Token tmp4 ;
        tmp4.str = "INTEGER" ;
        Token tmp5 ;
        tmp5.str = ":" ;
        Token tmp6 ;
        tmp6.str = "X" ;
        Token tmp7 ;
        tmp7.str = "," ;
        Token tmp8 ;
        tmp8.str = "Y" ;
        Token tmp9 ;
        tmp9.str = "," ;
        Token tmp10 ;
        tmp10.str = "K" ;
        Token tmp11 ;
        tmp11.str = ")" ;
        Token tmp12 ;
        tmp12.str = ";" ;
           
        queue.push( tmp1 ) ;
        queue.push( tmp2 ) ;
        queue.push( tmp3 ) ;
        queue.push( tmp4 ) ;
        queue.push( tmp5 ) ;
        queue.push( tmp6 ) ;
        queue.push( tmp7 ) ;
        queue.push( tmp8 ) ;
        queue.push( tmp9 ) ;
        queue.push( tmp10 ) ;
        queue.push( tmp11 ) ;
        queue.push( tmp12 ) ;
        
        int type = -1 ;
        if ( _Subroutine_( 1, queue, type, "SUBROUTINE" ) ) {
        
             cout << "The result of syntax analysis (Subroutine) : true" << endl ;
               
        } // if()
        else {
            
            cout << "The result of syntax analysis (Subroutine) : false" << endl ;
            
        } // else()
         
    } // testIsSubroutine()
    
    void testIsCall() {
           
        queue<Token> queue ;
        Token tmp1 ;
        tmp1.str = "CALL" ;
        Token tmp2 ;
        tmp2.str = "A3" ;
        Token tmp3 ;
        tmp3.str = "(" ;
        Token tmp4 ;
        tmp4.str = "I" ;
        Token tmp5 ;
        tmp5.str = "," ;
        Token tmp6 ;
        tmp6.str = "J" ;
        Token tmp7 ;
        tmp7.str = "," ;
        Token tmp8 ;
        tmp8.str = "K" ;
        Token tmp9 ;
        tmp9.str = ")" ;
        Token tmp10 ;
        tmp10.str = ";" ;
           
        queue.push( tmp1 ) ;
        queue.push( tmp2 ) ;
        queue.push( tmp3 ) ;
        queue.push( tmp4 ) ;
        queue.push( tmp5 ) ;
        queue.push( tmp6 ) ;
        queue.push( tmp7 ) ;
        queue.push( tmp8 ) ;
        queue.push( tmp9 ) ;
        queue.push( tmp10 ) ;
        
        if ( _CallStatement_( 1, queue, "CALL" ) ) {
        
             cout << "The result of syntax analysis (CALL) : true" << endl ;
               
        } // if()
        else {
            
            cout << "The result of syntax analysis (CALL) : false" << endl ;
            
        } // else()
         
    } // testIsCall()
    
    void testIsAssignment() {
        
        queue<Token> queue ;
        Token tmp1 ;
        tmp1.str = "K" ;
        Token tmp2 ;
        tmp2.str = "=" ;
        Token tmp3 ;
        tmp3.str = "(" ;
        Token tmp4 ;
        tmp4.str = "X" ;
        Token tt ;
        tt.str = "-" ;
        Token tmp5 ;
        tmp5.str = "888" ;
        Token tmp6 ;
        tmp6.str = "#" ;
        Token tmp7 ;
        tmp7.str = "Z" ;
        Token tmp8 ;
        tmp8.str = "#" ;
        Token tmp9 ;
        tmp9.str = "46" ;
        Token tmp10 ;
        tmp10.str = "*" ;
        Token tmp11 ;
        tmp11.str = "9" ;
        Token tmp12 ;
        tmp12.str = ")" ;
        Token tmp13 ;
        tmp13.str = "+" ;
        Token tmp14 ;
        tmp14.str = "(" ;
        Token tmp15 ;
        tmp15.str = "X" ;
        Token tmp16 ;
        tmp16.str = "-" ;
        Token tmp17 ;
        tmp17.str = "888" ;
        Token tmp18 ;
        tmp18.str = "#" ;
        Token tmp19 ;
        tmp19.str = "Z" ;
        Token t20 ;
        t20.str = "#" ;
        Token t21 ;
        t21.str = "46" ;
        Token t22 ;
        t22.str = "*" ;
        Token t23 ;
        t23.str = "9" ;
        Token t ;
        t.str = ")" ;
        Token t24 ;
        t24.str = ";" ;
           
        queue.push( tmp1 ) ;
        queue.push( tmp2 ) ;
        queue.push( tmp3 ) ;
        queue.push( tmp4 ) ;
        queue.push( tt ) ;
        queue.push( tmp5 ) ;
        queue.push( tmp6 ) ;
        queue.push( tmp7 ) ;
        queue.push( tmp8 ) ;
        queue.push( tmp9 ) ;
        queue.push( tmp10 ) ;
        queue.push( tmp11 ) ;
        queue.push( tmp12 ) ;
        queue.push( tmp13 ) ;
        queue.push( tmp14 ) ;
        queue.push( tmp15 ) ;
        queue.push( tmp16 ) ;
        queue.push( tmp17 ) ;
        queue.push( tmp18 ) ;
        queue.push( tmp19 ) ;
        queue.push( t20 ) ;
        queue.push( t21 ) ;
        queue.push( t22 ) ;
        queue.push( t23 ) ;
        queue.push( t ) ;
        queue.push( t24 ) ;
        
        if ( _AssignStatement_( 1, queue, "ASSIGNMENT" ) ) {
        
             cout << "The result of syntax analysis (ASSIGNMENT) : true" << endl ;
               
        } // if()
        else {
            
            cout << "The result of syntax analysis (ASSIGNMENT) : false" << endl ;
            
        } // else()
         
    } // testIsAssignment()
                                
} ; // class SyntaxAnalysis

class DataProcessor {
    
private:
    
    void __program__( Statement stmt ) {
        
        if ( stmt.gramCorrect ) {
            
            int routineIndex = -1 ;
            
            for ( int i = 0 ; i < stmt.tokens.size() ; i ++ ) {
                
                if ( g.subroutineExist( stmt.tokens[ i ].str, routineIndex ) ) {
                    
                    g.subroutine.push( stmt.tokens[ i ].str ) ;
                    
                } // if()
                
            } // for()
            
        } // if()
        
    } // __program__()
    
    void __variable__( Statement stmt ) { // variable declaration
        
        vector<Unit> units ;
        
        Unit unit ;
        Unit empty ;
        
        if ( stmt.gramCorrect ) {
            
            // now check whether this statement has label
            
            int labelIndex = -1 ;
            string labelName = stmt.tokens[ 0 ].str ; // because the grammer is correct, label should only exist in the first place
            
            if ( g.labelExist( labelName, labelIndex ) ) { // find the label exist! now we should assign the address for it
                
                assignLabelAddress( labelIndex, ( int ) g.table6.size() + 2 ) ;
                
            } // if()
            
            for ( int i = 0 ; i < stmt.tokens.size() ; i ++ ) {
                
                int labelIndex = -1 ;
                
                if ( g.variableExist( stmt.tokens[ i ].str, g.findSubroutine( g.subroutine.top() ), labelIndex ) ) { // find one variable
                    
                    unit.tableNo = TABLE_IDENTIFIER ;
                    unit.index = labelIndex ;
                    
                    units.push_back( unit ) ; // the real information
                    units.push_back( empty ) ;
                    units.push_back( empty ) ;
                    units.push_back( empty ) ;
                    
                    g.addToQuadrupleTable( units, stmt.tokens[ i ].str ) ;
                    
                    Unit pointer ;
                    pointer.tableNo = TABLE_QUADRUPLE ;
                    pointer.index = ( int ) g.table6.size() - 1 ;
                    
                    units.clear() ;
                    
                } // if()
                
            } // for()
            
        } // if()
        
    } // __variable__()
    
    int classifyType( string str ) {
        
        str = g.lowerToUp( str ) ;
        
        if ( str == "BOOLEAN" ) {
            
            return TYPE_BOOL ;
            
        } // if()
        else if ( str == "CHARACTER" ) {
            
            return TYPE_CHAR ;
            
        } // else if()
        else if ( str == "INTEGER" ) {
            
            return TYPE_INT ;
            
        } // else if ()
        else if ( str == "REAL" ) {
            
            return TYPE_REAL ;
            
        } // else if ()
        
        return -1 ;
        
    } // classifyType()
    
    void __dimension__( Statement stmt ) {
        
        vector<Unit> units ;
        
        Unit unit ;
        Unit empty ;
        
        int type = -1 ;
        
        if ( stmt.gramCorrect ) {
            
            // now check whether this statement has label
            
            int labelIndex = -1 ;
            string labelName = stmt.tokens[ 0 ].str ; // because the grammer is correct, label should only exist in the first place
            
            if ( g.labelExist( labelName, labelIndex ) ) { // find the label exist! now we should assign the address for it
                
                assignLabelAddress( labelIndex, ( int ) g.table6.size() ) ;
                
            } // if()
            
            for ( int i = 0 ; i < stmt.tokens.size() ; i ++ ) {
                
                if ( type == -1 ) {
                    
                    string tmpStr = stmt.tokens[ i ].str ;
                    
                    if ( g.lowerToUp( tmpStr ) == "BOOLEAN" || g.lowerToUp( tmpStr ) == "CHARACTER" || g.lowerToUp( tmpStr ) == "INTEGER" || g.lowerToUp( tmpStr ) == "REAL" ) {
                        
                        type = classifyType( tmpStr ) ;
                        
                    } // if()
                    
                } // if()
                
                int labelIndex = -1 ;
                
                if ( g.variableExist( stmt.tokens[ i ].str, g.findSubroutine( g.subroutine.top() ), labelIndex ) ) { // find one array variable
                    
                    unit.tableNo = TABLE_IDENTIFIER ;
                    unit.index = labelIndex ;
                    
                    units.push_back( unit ) ; // the real information
                    units.push_back( empty ) ;
                    units.push_back( empty ) ;
                    units.push_back( empty ) ;
                    
                    g.addToQuadrupleTable( units, stmt.tokens[ i ].str ) ;
                    
                    // assert : have successfully insert the information of the name of this array
                    
                    // we have to put the array information into information table ( table 7 )
                    vector<int> dimensionSize ;
                    for ( int tmp = i ; stmt.tokens[ tmp ].str != ")" ; tmp ++ ) {
                        
                        if ( g.isNumStr( stmt.tokens[ tmp ].str ) ) {
                            
                            dimensionSize.push_back( g.strToNum( stmt.tokens[ tmp ].str ) ) ;
                            
                        } // if()
                        
                        i = tmp ;
                        
                    } // for()
                    
                    g.addTable7( type, dimensionSize, g.table5[ labelIndex ].pointer ) ;
                    
                    units.clear() ;
                    
                } // if()
                
            } // for()
            
        } // if()
        
    } // __dimension__()
    
    void __labelDeclare__( Statement stmt ) {
        
        vector<Unit> units ;
        
        Unit unit ;
        Unit empty ;
        
        empty.index = -1 ;
        empty.tableNo = -1 ;
        
        if ( stmt.gramCorrect ) {
            
            // now check whether this statement has label
            
            int labelIndex = -1 ;
            string labelName = stmt.tokens[ 0 ].str ; // because the grammer is correct, label should only exist in the first place
            
            if ( g.labelExist( labelName, labelIndex ) ) { // find the label exist! now we should assign the address for it
                
                assignLabelAddress( labelIndex, ( int ) g.table6.size() ) ;
                
            } // if()
            
            for ( int i = 0 ; i < stmt.tokens.size() ; i ++ ) {
                
                int labelIndex = -1 ;
                
                if ( g.labelExist( stmt.tokens[ i ].str, labelIndex ) ) { // find one variable
                    
                    unit.tableNo = TABLE_IDENTIFIER ;
                    unit.index = labelIndex ;
                    
                    units.push_back( unit ) ; // the real information
                    units.push_back( empty ) ;
                    units.push_back( empty ) ;
                    units.push_back( empty ) ;
                    
                    g.addToQuadrupleTable( units, stmt.tokens[ i ].str ) ;
                    
                    Unit pointer ;
                    pointer.tableNo = TABLE_QUADRUPLE ;
                    pointer.index = ( int ) g.table6.size() - 1 ;
                    
                    units.clear() ;
                    
                } // if()
                
            } // for()
            
        } // if()
        
    } // __labelDeclare__()
    
    void __gto__( Statement stmt ) {
        
        vector<Unit> units ;
        string stmtStr = "" ;
        
        if ( stmt.gramCorrect ) {
            
            int instrTableIndex = -1 ;
            
            // now check whether this statement has label
            
            int labelIndex = -1 ;
            string labelName = stmt.tokens[ 0 ].str ; // because the grammer is correct, label should only exist in the first place
            
            if ( g.labelExist( labelName, labelIndex ) ) { // find the label exist! now we should assign the address for it
                
                assignLabelAddress( labelIndex, ( int ) g.table6.size() ) ;
                
            } // if()
            
            for ( int i = 0 ; i < g.rsWord.size() && instrTableIndex == -1 ; i ++ ) {
                
                if ( g.lowerToUp( stmt.instr.str ) == g.rsWord[ i ] ) {
                    
                    instrTableIndex = i + 1 ;
                    
                } // if()
                
            } // for()
            
            Unit instrUnit ;
            instrUnit.tableNo = TABLE_RWORD ;
            instrUnit.index = instrTableIndex ;
            
            Unit labelUnit ;
            Unit empty ;
            
            labelIndex = -1 ;
            
            for ( int i = 0 ; i < stmt.tokens.size() && labelIndex == -1 ; i ++ ) {
                
                if ( g.labelExist( stmt.tokens[ i ].str, labelIndex ) ) {
                    
                    labelUnit = g.table5[ labelIndex ].pointer ;
                    
                    if ( labelUnit.index == -1 ) { // forward reference
                        
                        fix tmp ;
                        tmp.labelName = stmt.tokens[ i ].str ;
                        tmp.quaIndex = ( int ) g.table6.size() ;
                        tmp.unitNo = 3 ;
                        
                        g.fixSet.push_back( tmp ) ;
                        
                    } // if()
                    
                    stmtStr = "GTO " + stmt.tokens[ i ].str ;
                    
                } // if()
                
            } // for()
            
            // then put the four units into contents
            
            units.push_back( instrUnit ) ;
            units.push_back( empty ) ;
            units.push_back( empty ) ;
            units.push_back( labelUnit ) ;
            
            g.addToQuadrupleTable( units, stmtStr ) ;
            
            units.clear() ;
            
        } // if()
        
    } // __gto__()
    
    void __subroutine__( Statement stmt ) {
        
        Unit unit ;
        Unit empty ;
        vector<Unit> units ;
        
        int subroutineIndex = -1 ;
        int recentRoutine = -1 ;
        
        if ( stmt.gramCorrect ) {
            
            for ( int i = 0 ; i < stmt.tokens.size() ; i ++ ) {
                
                if ( recentRoutine == -1 && g.subroutineExist( stmt.tokens[ i ].str, subroutineIndex ) ) {
                    
                    recentRoutine = subroutineIndex ;
                    
                    g.subroutine.push( stmt.tokens[ i ].str ) ; // from now on is the next subroutine
                    
                } // if()
                
                int labelIndex = -1 ;
                
                if ( g.variableExist( stmt.tokens[ i ].str, recentRoutine, labelIndex ) ) { // find one variable
                    
                    unit.tableNo = TABLE_IDENTIFIER ;
                    unit.index = labelIndex ;
                    
                    units.push_back( unit ) ; // the real information
                    units.push_back( empty ) ;
                    units.push_back( empty ) ;
                    units.push_back( empty ) ;
                    
                    g.addToQuadrupleTable( units, stmt.tokens[ i ].str ) ;
                    
                    Unit pointer ;
                    pointer.tableNo = TABLE_QUADRUPLE ;
                    pointer.index = ( int ) g.table6.size() - 1 ;
                    
                    units.clear() ;
                    
                } // if()
                
            } // for()
            
            // now check whether this statement has label
            
            int labelIndex = -1 ;
            string labelName = stmt.tokens[ 0 ].str ; // because the grammer is correct, label should only exist in the first place
            
            if ( g.labelExist( labelName, labelIndex ) ) { // find the label exist! now we should assign the address for it
                
                assignLabelAddress( labelIndex, ( int ) g.table6.size() ) ;
                
            } // if()
            
        } // if()
        
    } // __subroutine__()
    
    void __call__( Statement stmt, int index ) {
        
        Unit arguPointer ;
        Unit instrPointer ;
        Unit subroutinePointer ;
        Unit empty ;
        vector<Unit> units ;
        
        bool subroutineExist = false ;
        
        string stmtStr = "" ;
        
        if ( stmt.gramCorrect ) {
            
            // now check whether this statement has label
            
            int labelIndex = -1 ;
            string labelName = stmt.tokens[ 0 ].str ; // because the grammer is correct, label should only exist in the first place
            
            if ( g.labelExist( labelName, labelIndex ) ) { // find the label exist! now we should assign the address for it
                
                assignLabelAddress( labelIndex, ( int ) g.table6.size() ) ;
                
            } // if()
            
            for ( int i = 0 ; i < g.rsWord.size() && instrPointer.index == -1 ; i ++ ) {
                
                if ( stmt.tokens[ 0 ].str == g.rsWord[ i ] || stmt.tokens[ 1 ].str == g.rsWord[ i ] ) {
                    
                    instrPointer.tableNo = TABLE_RWORD ;
                    
                    instrPointer.index = i + 1 ;
                    
                } // if()
                
            } // for()
            
            int subIndex = -1 ;
            for ( int i = 0 ; i < stmt.tokens.size() && !subroutineExist ; i ++ ) {
                
                if ( g.subroutineExist( stmt.tokens[ i ].str, subIndex ) ) {
                    
                    subroutinePointer.tableNo = TABLE_IDENTIFIER ;
                    subroutinePointer.index = subIndex ;
                    
                    subroutineExist = true ;
                    
                } // if()
                
            } // for()
            
            if ( subroutineExist ) {
                
                vector<int> arguContents ;
                int countArgu = 0 ;
                
                for ( int j = 0 ; j < stmt.tokens.size() && stmt.tokens[ j ].str != ")" ; j ++ ) {
                    
                    int tableNo = -1 ;
                    int tableIndex = -1 ;
                    
                    stmtStr += stmt.tokens[ j ].str ;
                    
                    if ( g.lowerToUp( stmtStr ) == "CALL" ) {
                        
                        stmtStr += " " ;
                        
                    } // if()
                    
                    if ( g.isNumStr( stmt.tokens[ j ].str ) ) {
                        
                        g.addNumToTable( stmt.tokens[ j ].str, tableNo, tableIndex ) ;
                        
                        arguContents.push_back( tableNo ) ;
                        arguContents.push_back( tableIndex ) ;
                        
                        countArgu ++ ;
                        
                    } // if()
                    else if ( g.variableExist( stmt.tokens[ j ].str, g.findSubroutine( g.subroutine.top() ), tableIndex ) ) {
                        
                        tableNo = TABLE_IDENTIFIER ;
                        
                        arguContents.push_back( tableNo ) ;
                        arguContents.push_back( tableIndex ) ;
                        
                        countArgu ++ ;
                        
                    } // else if()
                    
                } // for()
                
                stmtStr += ")" ;
                
                g.addArgeToTable7( countArgu, arguContents, arguPointer ) ;
                
                units.push_back( instrPointer ) ;
                units.push_back( subroutinePointer ) ;
                units.push_back( empty ) ;
                units.push_back( arguPointer ) ;
                
                g.addToQuadrupleTable( units, stmtStr ) ;
                
                units.clear() ;
                
            } // if()
            else {
                
                g.addError( index, "subroutinenot defined" ) ;
                
            } // else()
            
        } // if()
        
    } // __call__()
    
    int getPrecedence( string str ) {
        
        int ans = -1 ;
        
        if ( str == "#" || str == "GT" || str == "GE" || str == "LT" || str == "LE" || str == "EQ" ) {
            
            ans = 9 ;
                
        } // if()
        else if ( str == "↑" || str == "AND" ) {
            
            ans = 8 ;
                
        } // else if()
        else if ( str == "*" || str == "OR" ) {
            
            ans = 7 ;
                
        } // else if()
        else if ( str == "/" || str == "NOT" ) {
            
            ans = 7 ;
                
        } // else if()
        else if ( str == "+" ) {
            
            ans = 6 ;
                
        } // else if()
        else if ( str == "-" ) {
            
            ans = 6 ;
                
        } // else if()
        else if ( str == "(" ) {
            
            ans = 5 ;
                
        } // else if()
        else if ( str == ")" ) {
            
            ans = 5 ;
                
        } // else if()
        else if ( str == "=" ) {
            
            ans = 4 ;
                
        } // else if()
        
        return ans ;
        
    } // getPrecedence()

    bool hasHigherPrecedence( string del, string topOfStack ) {
        
        int op1 = getPrecedence( del ) ;
        int op2 = getPrecedence( topOfStack ) ;
        
        if ( op1 >= op2 && op1 != -1 && op2 != -1 ) {
            
            return true ;
            
        } // if()
        else if ( op1 == -1 || op2 == -1 ) {
            
            cout << "### Error: there is no such delimiter! ###" << endl ;
            
        } // else if()
        
        return false ;
        
    } // hasHigherPrecedence
    
    int getDelIndex( string str ) {
        
        int ans = -1 ;
        
        for ( int i = 0 ; i < g.delSet.size() && ans == -1 ; i ++ ) {
            
            if ( str == g.delSet[ i ] ) {
                
                ans = i ;
                
            } // if()
            
        } // for()
        
        return ans ;
        
    } // getDelIndex()
    
    void arrayMiddleCode( Statement array, vector<Token> index, vector<int> correspond ) {
        
        // we need to transfer the index first
        
        // column major
        
        // if the index size is one then directly generate the middle code
        // the rightmost ( index - 1 ) * the corresponds expect itself
        // then rightmost left shift, ( index - 1 ) * the corresponds
        
        vector<Unit> units ; // add to the quadruple table
        Unit operatorUnit ; // put the operator unit
        Unit operand1Unit ; // put the array name
        Unit operand2Unit ; // put the array index
        Unit resultUnit ; // put the tmp token
        
        int tmpNum = -1 ;
        string tmpName = "" ;
        string stmtStr = "" ;
        
        if ( index.size() == 1 ) { // there is only on index
            
            // first assign the "="
            for ( int i = 0 ; i < g.delSet.size() && operatorUnit.tableNo == -1 ; i ++ ) {
                
                if ( g.delSet[ i ] == "=" ) {
                    
                    operatorUnit.tableNo = TABLE_DELIMITER ;
                    operatorUnit.index = i + 1 ;
                    
                } // if()
                
            } // for()
            
            // second assign the operand 1
            operand1Unit.tableNo = array.tokens[ 0 ].table ; // because the grammer is correct, it's obvious that the first token is the array name
            operand1Unit.index = array.tokens[ 0 ].no ;
            
            // thirt assign the operand 2, must be the index
            int index = -1 ;
            if ( g.variableExist( array.tokens[ 2 ].str, g.findSubroutine( g.subroutine.top() ), index ) ) { // this index is a variable
                
                array.tokens[ 2 ].table = TABLE_IDENTIFIER ;
                array.tokens[ 2 ].no = index ;
                
                operand2Unit.tableNo = array.tokens[ 2 ].table ;
                operand2Unit.index  = array.tokens[ 2 ].no ;
                
            } // if()
            else { // this index is a number
                
                operand2Unit = g.findNumInTable( array.tokens[ 2 ].str ) ;
                
            } // else()
            
            // last assign a tmp token
            tmpNum = g.getNewTmpNum() ;
            tmpName = "T" + g.intToStr( tmpNum ) ;
            resultUnit.tableNo = TABLE_TMP ;
            resultUnit.index = tmpNum ;
            
            units.push_back( operatorUnit ) ;
            units.push_back( operand1Unit ) ;
            units.push_back( operand2Unit ) ;
            units.push_back( resultUnit ) ;
            
            stmtStr = tmpName + " = " ;
            for ( int i = 0 ; i < array.tokens.size() ; i ++ ) {
                
                stmtStr += array.tokens[ i ].str ;
                
            } // for()
            
            g.addToQuadrupleTable( units, stmtStr ) ;
            
            units.clear() ;
            
        } // if()
        else { // there must be two or more index
            
            for ( int j = ( int ) index.size() - 1 ; j >= 0 ; j -- ) { // start we the right most index first becaause this is a column major
      
                operatorUnit.tableNo = -1 ;
                operatorUnit.index = -1 ;
                operand1Unit.tableNo = -1 ;
                operand1Unit.index = -1 ;
                operand2Unit.tableNo = -1 ;
                operand2Unit.index = -1 ;
                resultUnit.tableNo = -1 ;
                resultUnit.index = -1 ;
                stmtStr = "" ;
                 
                if ( j == 0 ) { // the last one, we only need to add the last index
                    
                    // first assign the "+"
                    for ( int k = 0 ; k < g.delSet.size() && operatorUnit.tableNo == -1 ; k ++ ) {
                        
                        if ( g.delSet[ k ] == "+" ) {
                            
                            operatorUnit.tableNo = TABLE_DELIMITER ;
                            operatorUnit.index = k + 1 ;
                            
                        } // if()
                        
                    } // for()
                    
                    // second assign the operand 1
                    int indexNum = -1 ;
                    if ( g.variableExist( index[ j ].str, g.findSubroutine( g.subroutine.top() ), indexNum ) ) { // this index is a variable
                        
                        index[ j ].table = TABLE_IDENTIFIER ;
                        index[ j ].no = indexNum ;
                        
                        operand1Unit.tableNo = index[ j ].table ;
                        operand1Unit.index  = index[ j ].no ;
                        
                    } // if()
                    else { // this index is a number
                        
                        operand1Unit = g.findNumInTable( index[ j ].str ) ;
                        
                    } // else()
                    
                    
                    // third assign the operand 2, the recent exist tmp num
                    operand2Unit.tableNo = TABLE_TMP ;
                    operand2Unit.index = g.getRecentTmpNum() ;
                    
                    // last assign the new tmp num
                    resultUnit.tableNo = TABLE_TMP ;
                    resultUnit.index = g.getNewTmpNum() ;
                    
                    units.push_back( operatorUnit ) ;
                    units.push_back( operand1Unit ) ;
                    units.push_back( operand2Unit ) ;
                    units.push_back( resultUnit ) ;
                    
                    stmtStr = "T" + g.intToStr( resultUnit.index ) + " = " + index[ j ].str + "+" + "T" + g.intToStr( operand2Unit.index ) ;
                    
                    g.addToQuadrupleTable( units, stmtStr ) ;
                    
                    units.clear() ;
                    
                } // if()
                else { // need to minus one, then times the corrsapond
                    
                    // first part minus one
                    
                    // first assign the "-"
                    for ( int k = 0 ; k < g.delSet.size() && operatorUnit.tableNo == -1 ; k ++ ) {
                        
                        if ( g.delSet[ k ] == "-" ) {
                            
                            operatorUnit.tableNo = TABLE_DELIMITER ;
                            operatorUnit.index = k + 1 ;
                            
                        } // if()
                        
                    } // for()
                    
                    // second assign the operand 1
                    int indexNum = -1 ;
                    if ( g.variableExist( index[ j ].str, g.findSubroutine( g.subroutine.top() ), indexNum ) ) { // this index is a variable
                        
                        index[ j ].table = TABLE_IDENTIFIER ;
                        index[ j ].no = indexNum ;
                        
                        operand1Unit.tableNo = index[ j ].table ;
                        operand1Unit.index  = index[ j ].no ;
                        
                    } // if()
                    else { // this index is a number
                        
                        operand1Unit = g.findNumInTable( index[ j ].str ) ;
                        
                    } // else()
                    
                    // third assign 1
                    int tableNo = -1 ;
                    int tableIndex = -1 ;
                    
                    g.addNumToTable( "1", tableNo, tableIndex ) ;
                    
                    operand2Unit.tableNo = tableNo ;
                    operand2Unit.index = tableIndex ;
                    
                    // last assign the new tmp num
                    resultUnit.tableNo = TABLE_TMP ;
                    resultUnit.index = g.getNewTmpNum() ;
                    
                    units.push_back( operatorUnit ) ;
                    units.push_back( operand1Unit ) ;
                    units.push_back( operand2Unit ) ;
                    units.push_back( resultUnit ) ;
                    
                    stmtStr = "T" + g.intToStr( resultUnit.index ) + " = " + index[ j ].str + "-1" ;
                    
                    g.addToQuadrupleTable( units, stmtStr ) ;
                    
                    units.clear() ;
                    
                    // seocond part times correspond
                                    
                    for ( int countCor = j - 1 ; countCor >= 0 ; countCor -- ) {
                        
                        operatorUnit.tableNo = -1 ; // initial
                        operatorUnit.index = -1 ; // initial
                        operand1Unit.tableNo = -1 ; // initial
                        operand1Unit.index = -1 ; // initial
                        operand2Unit.tableNo = -1 ; // initial
                        operand2Unit.index = -1 ; // initial
                        resultUnit.tableNo = -1 ; // initial
                        resultUnit.index = -1 ; // initial
                        stmtStr = "" ; // initial
                        
                        // first assign the "*"
                        for ( int k = 0 ; k < g.delSet.size() && operatorUnit.tableNo == -1 ; k ++ ) {
                            
                            if ( g.delSet[ k ] == "*" ) {
                                
                                operatorUnit.tableNo = TABLE_DELIMITER ;
                                operatorUnit.index = k + 1 ;
                                
                            } // if()
                            
                        } // for()
                        
                        // second assign the operand 1
                        operand1Unit.tableNo = TABLE_TMP ;
                        operand1Unit.index = g.getRecentTmpNum() ;
                        
                        // third assign correspond
                        int tableNo = -1 ;
                        int tableIndex = -1 ;
                        
                        g.addNumToTable( g.intToStr( correspond[ countCor ] ), tableNo, tableIndex ) ;
                        
                        operand2Unit.tableNo = tableNo ;
                        operand2Unit.index = tableIndex ;
                        
                        // last assign the new tmp num
                        resultUnit.tableNo = TABLE_TMP ;
                        resultUnit.index = g.getNewTmpNum() ;
                        
                        units.push_back( operatorUnit ) ;
                        units.push_back( operand1Unit ) ;
                        units.push_back( operand2Unit ) ;
                        units.push_back( resultUnit ) ;
                        
                        stmtStr = "T" + g.intToStr( resultUnit.index ) + " = " + "T" + g.intToStr( operand1Unit.index ) + "*" + g.intToStr( correspond[ countCor ] ) ;
                        
                        g.addToQuadrupleTable( units, stmtStr ) ;
                        
                        units.clear() ;
                        
                    } // for()
                    
                } // else()
                
            } // for()
            
        } // else()
        
    } // arrayMiddleCode()
    
    void __assignment__( Statement stmt ) {
        
        stack<Token> idStack ;
        stack<Token> opStack ;
        
        vector<Unit> units ;
        Unit operatorUnit ;
        Unit operand1Unit ;
        Unit operand2Unit ;
        Unit resultUnit ;
        
        string op1 = "" ;
        string op2 = "" ;
        string op = "" ;
        string stmtStr = "" ;
        string result = "" ;
        
        if ( stmt.gramCorrect ) {
            
            // three situations: (1) variable assign (2) array assign (3) tmpVariable assign
            
            // now check whether this statement has label
            
            int labelIndex = -1 ;
            string labelName = stmt.tokens[ 0 ].str ; // because the grammer is correct, label should only exist in the first place
            
            if ( g.labelExist( labelName, labelIndex ) ) { // find the label exist! now we should assign the address for it
                
                assignLabelAddress( labelIndex, ( int ) g.table6.size() ) ;
                
            } // if()
            
            for ( int i = 0 ; i < stmt.tokens.size() && stmt.tokens[ i ].str != ";" ; i ++ ) {
                
                op1 = "" ;
                op2 = "" ;
                op = "" ;
                result = "" ;
                stmtStr = "" ;
                
                int variableIndex = -1 ;
                
                if ( g.variableExist( stmt.tokens[ i ].str, g.findSubroutine( g.subroutine.top() ), variableIndex ) ) { // now we find a variable
                    
                    stmt.tokens[ i ].table = TABLE_IDENTIFIER ;
                    stmt.tokens[ i ].no = variableIndex ;
                    
                    if ( g.table5[ variableIndex ].type == TYPE_ARRAY ) {
                        
                        // when we assume this is an array
                        // two possibilities: (1) result (2) operand
                        
                        idStack.push( stmt.tokens[ i ] ) ;
                        
                        int varIndex = -1 ;
                        vector<Token> index ;
                        vector<int> correspond ; // put in the actual dimensions
                        Statement tmp ; // use to store the new array
                        
                        int t ;
                        for ( t = i ; stmt.tokens[ t ].str != ")" ; t ++ ) {
                            
                            tmp.tokens.push_back( stmt.tokens[ t ] ) ;
                            
                        } // for()
                        tmp.tokens.push_back( stmt.tokens[ t ] ) ;
                        
                        for ( int index = g.table5[ variableIndex ].pointer.index + 2, count = 0 ; count < g.table7[ g.table5[ variableIndex ].pointer.index + 1 ] ; index ++, count ++ ) {
                            
                            correspond.push_back( g.table7[ index ] ) ;
                            
                        } // for()
                       
                        for ( int j = i + 1 ; stmt.tokens[ j ].str != ")" ; j ++, i ++ ) {
                            
                            if ( g.isNumStr( stmt.tokens[ j ].str ) || g.variableExist( stmt.tokens[ j ].str, g.findSubroutine( g.subroutine.top() ), varIndex ) ) {
                                
                                stmt.tokens[ j ].table = TABLE_IDENTIFIER ;
                                stmt.tokens[ j ].no = varIndex ;
                                
                                index.push_back( stmt.tokens[ j ] ) ;
                                
                            } // if()
                            
                        } // for()
                        i ++ ;
                        
                        arrayMiddleCode( tmp, index, correspond ) ;
                        
                        Token tmpVar ;
                        tmpVar.str = "T" + g.intToStr( g.getRecentTmpNum() ) ;
                        tmpVar.table = TABLE_TMP ;
                        tmpVar.no = g.getRecentTmpNum() ;
                        idStack.pop() ; // pop out the old array name
                        idStack.push( tmpVar ) ;
                        
                    } // if()
                    else if ( g.table5[ variableIndex ].type == TYPE_INT || g.table5[ variableIndex ].type == TYPE_REAL ) {
                        
                        // when encounter a variable, we push it into a stack
                        idStack.push( stmt.tokens[ i ] ) ; // we only need the string
                        
                    } // else if()
                    
                } // if()
                else if ( g.isDelimiter( stmt.tokens[ i ].str[ 0 ] ) ) { // this is a delimiter
                    
                    // need to compare the recent token with the top token in the stack
                    if ( stmt.tokens[ i ].str == "(" ) {
                        
                        opStack.push( stmt.tokens[ i ] ) ;
                        
                    } // if()
                    else if ( stmt.tokens[ i ].str == ")" ) { // need to generate the quadruple code
                        
                        while ( opStack.top().str != "(" ) {
                            
                            string tmpName = "T" + g.intToStr( g.getNewTmpNum() ) ; // get the recent tmp num and make the tmp variable name

                            g.addToTable0( tmpName ) ;
                            
                            resultUnit.tableNo = TABLE_TMP ;
                            resultUnit.index = g.getRecentTmpNum() ;
                            
                            if ( !opStack.empty() ) {

                                operatorUnit.tableNo = opStack.top().table ;
                                operatorUnit.index = opStack.top().no ;
                                op = opStack.top().str ;
                                opStack.pop() ;
                                
                            } // if()
                            
                            if ( !idStack.empty() ) {

                                operand2Unit.tableNo = idStack.top().table ;
                                operand2Unit.index = idStack.top().no ;
                                op2 = idStack.top().str ;
                                idStack.pop() ;
                                
                            } // if()
                            
                            
                            if ( !idStack.empty() ) {
                                
                                operand1Unit.tableNo = idStack.top().table ;
                                operand1Unit.index = idStack.top().no ;
                                op1 = idStack.top().str ;
                                idStack.pop() ;
                                
                            } // if()
                            
                            units.push_back( operatorUnit ) ;
                            units.push_back( operand1Unit ) ;
                            units.push_back( operand2Unit ) ;
                            units.push_back( resultUnit ) ;
                            
                            stmtStr = tmpName + " = " + op1 + op + op2 ;
                            
                            g.addToQuadrupleTable( units, stmtStr ) ;
                            
                            units.clear() ;
                            
                            Token tmp ;
                            tmp.str = tmpName ;
                            tmp.table = resultUnit.tableNo ;
                            tmp.table = resultUnit.index ;
                            idStack.push( tmp ) ;
                            
                        } // while()
                        
                        if ( opStack.top().str == "(" ) {
                            
                            opStack.pop() ;
                            
                        } // if()
                        
                    } // else if()
                    else { // normal delimiter, compare the precedence
                        
                        // when the recent token's precedence is higher, push into opStack
                        if ( opStack.empty() || hasHigherPrecedence( stmt.tokens[ i ].str, opStack.top().str ) ) {
                            
                            stmt.tokens[ i ].table = TABLE_DELIMITER ;
                            
                            for ( int k = 0 ; k < g.delSet.size() && stmt.tokens[ i ].no == -1 ; k ++ ) {
                                
                                if ( stmt.tokens[ i ].str == g.delSet[ k ] ) {
                                    
                                    stmt.tokens[ i ].no = k + 1 ;
                                    
                                } // if()
                                
                            } // for()
                            
                            if ( stmt.tokens[ i ].str == "#" || stmt.tokens[ i ].str == "↑" ) {
                                
                                stmt.tokens[ i ].no = 9 ;
                                
                            } // if()
                            
                            opStack.push( stmt.tokens[ i ] ) ;
                            
                        } // if()
                        else { // need to deal with the tokens in the stack first, and reserve the recent one
                            
                            // need to generate the quadruple form
                            
                            do { // while the recent token is still smaller then the second top of the stack then keep generating
                                
                                string tmpName = "T" + g.intToStr( g.getNewTmpNum() ) ; // get the recent tmp num and make the tmp variable name

                                g.addToTable0( tmpName ) ;
                                
                                resultUnit.tableNo = TABLE_TMP ;
                                resultUnit.index = g.getRecentTmpNum() ;
                                
                                if ( !opStack.empty() ) {

                                    operatorUnit.tableNo = opStack.top().table ;
                                    operatorUnit.index = opStack.top().no ;
                                    op = opStack.top().str ;
                                    opStack.pop() ;
                                    
                                } // if()
                                
                                if ( !idStack.empty() ) {

                                    operand2Unit.tableNo = idStack.top().table ;
                                    operand2Unit.index = idStack.top().no ;
                                    op2 = idStack.top().str ;
                                    idStack.pop() ;
                                    
                                } // if()
                                
                                
                                if ( !idStack.empty() ) {
                                    
                                    operand1Unit.tableNo = idStack.top().table ;
                                    operand1Unit.index = idStack.top().no ;
                                    op1 = idStack.top().str ;
                                    idStack.pop() ;
                                    
                                } // if()
                                
                                units.push_back( operatorUnit ) ;
                                units.push_back( operand1Unit ) ;
                                units.push_back( operand2Unit ) ;
                                units.push_back( resultUnit ) ;
                                
                                stmtStr = tmpName + " = " + op1 + op + op2 ;
                                
                                g.addToQuadrupleTable( units, stmtStr ) ;
                                
                                units.clear() ;
                                
                                Token tmpToken ;
                                tmpToken.str = "T" + g.intToStr( g.getRecentTmpNum() ) ;
                                tmpToken.table = TABLE_TMP ;
                                tmpToken.no = g.getRecentTmpNum() ;
                                idStack.push( tmpToken ) ;
                                
                            } while ( !idStack.empty() && !opStack.empty() && hasHigherPrecedence( opStack.top().str, stmt.tokens[ i ].str ) ) ;
                            
                            stmt.tokens[ i ].table = TABLE_DELIMITER ;
                            stmt.tokens[ i ].no = getDelIndex( stmt.tokens[ i ].str ) + 1 ;
                            opStack.push( stmt.tokens[ i ] ) ;
                            
                        } // else()
                        
                    } // else()
                    
                } // else if()
                else if ( g.isNumStr( stmt.tokens[ i ].str ) ) {
                    
                    int numTable = -1 ;
                    int numIndex = -1 ;
                    
                    g.addNumToTable( stmt.tokens[ i ].str, numTable, numIndex ) ;
                    
                    stmt.tokens[ i ].table = numTable ;
                    stmt.tokens[ i ].no = numIndex ;
                    
                    idStack.push( stmt.tokens[ i ] ) ;
                    
                } // else if()
                
            } // for()
            
            // check the stacks are empty or not

            while ( !opStack.empty() || !idStack.empty() ) {
                
                if ( !opStack.empty() ) {

                    operatorUnit.tableNo = opStack.top().table ;
                    operatorUnit.index = opStack.top().no ;
                    op = opStack.top().str ;
                    opStack.pop() ;
                    
                } // if()
                
                if ( !idStack.empty() ) {

                    operand2Unit.tableNo = idStack.top().table ;
                    operand2Unit.index = idStack.top().no ;
                    op2 = idStack.top().str ;
                    idStack.pop() ;
                    
                } // if()
                
                
                if ( !idStack.empty() ) {
                    
                    operand1Unit.tableNo = idStack.top().table ;
                    operand1Unit.index = idStack.top().no ;
                    op1 = idStack.top().str ;
                    idStack.pop() ;
                    
                } // if()
                
                if ( op == "=" ) {
                    
                    Unit empty ;
                    
                    resultUnit = operand1Unit ;
                    result = op1 ;
                    operand1Unit = operand2Unit ;
                    op1 = op2 ;
                    operand2Unit = empty ;
                    op2 = "" ;
                    
                    stmtStr = result + "=" + op1 ;
                    
                } // if()
                else { // keep generating tmp variables
                    
                    string tmpName = "T" + g.intToStr( g.getNewTmpNum() ) ; // get the recent tmp num and make the tmp variable name
                        
                    g.addToTable0( tmpName ) ;
                        
                    resultUnit.tableNo = TABLE_TMP ;
                    resultUnit.index = g.getRecentTmpNum() ;
                    
                    stmtStr = tmpName + " = " + op1 + op + op2 ;
                    
                    Token tmpToken ;
                    tmpToken.str = "T" + g.intToStr( g.getRecentTmpNum() ) ;
                    tmpToken.table = TABLE_TMP ;
                    tmpToken.no = g.getRecentTmpNum() ;
                    idStack.push( tmpToken ) ;
                        
                } // else()
                
                units.push_back( operatorUnit ) ;
                units.push_back( operand1Unit ) ;
                units.push_back( operand2Unit ) ;
                units.push_back( resultUnit ) ;
                
                g.addToQuadrupleTable( units, stmtStr ) ;
                
                units.clear() ;
                
            } // while()
            
        } // if()
        
    } // __assignment__()
    
    void __if__( Statement stmt, int index ) {
        
        // try to find IF, Then, Else
        int countBlock = 0 ;
        int indexIF = -1 ;
        int indexTHEN = -1 ;
        int indexELSE = -1 ;
        
        int quadrupleIFIndex = -1 ;
        int quadrupleTHENIndex = -1 ;
 
        stack<Token> idStack ;
        stack<Token> opStack ;
        
        vector<Unit> units ;
        Unit operatorUnit ;
        Unit operand1Unit ;
        Unit operand2Unit ;
        Unit resultUnit ;
        
        string op1 = "" ;
        string op2 = "" ;
        string op = "" ;
        string stmtStr = "" ;
        string result = "" ;
    
        Statement stmtThen ;
        Statement stmtElse ;
        
        for ( int i = 0 ; i < stmt.tokens.size() ; i ++ ) {
            
            if ( g.lowerToUp( stmt.tokens[ i ].str ) == "IF" || g.lowerToUp( stmt.tokens[ i ].str ) == "THEN" || g.lowerToUp( stmt.tokens[ i ].str ) == "ELSE" ) {
                
                countBlock ++ ;
                
                if ( g.lowerToUp( stmt.tokens[ i ].str ) == "IF" ) {
                    
                    indexIF = i ;
                    
                } // if()
                else if ( g.lowerToUp( stmt.tokens[ i ].str ) == "THEN" ) {
                    
                    indexTHEN = i ;
                    
                } // else if()
                else if ( g.lowerToUp( stmt.tokens[ i ].str ) == "ELSE" ) {
                    
                    indexELSE = i ;
                    
                } // else if()
                
            } // if()
            
        } // for()
        
        int tmpIndex = -1 ;
        // make a then statement
        stmtThen.gramCorrect = true ;
        
        for ( int i = indexTHEN + 1 ; ( indexELSE != -1 && i < indexELSE ) || ( indexELSE == -1 && i < stmt.tokens.size() ) ; i ++ ) {
            
            if ( i == indexTHEN + 1 && g.labelExist( stmt.tokens[ i ].str, tmpIndex ) ) {
                
                stmtThen.label = stmt.tokens[ i ].str ;
                stmtThen.instr = stmt.tokens[ i + 1 ] ;
                
            } // if()
            else if ( i == indexTHEN + 1 && !g.labelExist( stmt.tokens[ i ].str, tmpIndex ) ){
                
                stmtThen.instr = stmt.tokens[ i ] ;
                
            } // else()
            
            stmtThen.tokens.push_back( stmt.tokens[ i ] ) ;
            
        } // for()
        
        // make a else statement
        if ( indexELSE != -1 ) {
            
            stmtElse.gramCorrect = true ;
            
            for ( int i = indexELSE + 1 ; i < stmt.tokens.size() ; i ++ ) {
                
                if ( i == indexELSE + 1 && g.labelExist( stmt.tokens[ i ].str, tmpIndex ) ) {
                    
                    stmtElse.label = stmt.tokens[ i ].str ;
                    stmtElse.instr = stmt.tokens[ i + 1 ] ;
                    
                } // if()
                else if ( i == indexELSE + 1 && !g.labelExist( stmt.tokens[ i ].str, tmpIndex ) ){
                    
                    stmtElse.instr = stmt.tokens[ i ] ;
                    
                } // else()
                
                stmtElse.tokens.push_back( stmt.tokens[ i ] ) ;
                
            } // for()
            
        } // if()
        
        for ( int i = 0 ; i < stmt.tokens.size() && stmt.tokens[ i ].str != ";" ; i ++ ) {
            
            // IF part
            if ( i > indexIF && i < indexTHEN ) { // deal with the conditions like expression
                
                if ( g.isRSWord( stmt.tokens[ i ].str ) || stmt.tokens[ i ].str == "(" || stmt.tokens[ i ].str == ")" ) { // this is a operator
                    
                    // need to compare the recent token with the top token in the stack
                    if ( stmt.tokens[ i ].str == "(" ) {
                        
                        opStack.push( stmt.tokens[ i ] ) ;
                        
                    } // if()
                    else if ( stmt.tokens[ i ].str == ")" ) { // need to generate the quadruple code
                        
                        while ( opStack.top().str != "(" ) {
                            
                            string tmpName = "T" + g.intToStr( g.getNewTmpNum() ) ; // get the recent tmp num and make the tmp variable name

                            g.addToTable0( tmpName ) ;
                            
                            resultUnit.tableNo = TABLE_TMP ;
                            resultUnit.index = g.getRecentTmpNum() ;
                            
                            if ( !opStack.empty() ) {

                                operatorUnit.tableNo = opStack.top().table ;
                                operatorUnit.index = opStack.top().no ;
                                op = opStack.top().str ;
                                opStack.pop() ;
                                
                            } // if()
                            
                            if ( !idStack.empty() ) {

                                operand2Unit.tableNo = idStack.top().table ;
                                operand2Unit.index = idStack.top().no ;
                                op2 = idStack.top().str ;
                                idStack.pop() ;
                                
                            } // if()
                            
                            
                            if ( !idStack.empty() ) {
                                
                                operand1Unit.tableNo = idStack.top().table ;
                                operand1Unit.index = idStack.top().no ;
                                op1 = idStack.top().str ;
                                idStack.pop() ;
                                
                            } // if()
                            
                            units.push_back( operatorUnit ) ;
                            units.push_back( operand1Unit ) ;
                            units.push_back( operand2Unit ) ;
                            units.push_back( resultUnit ) ;
                            
                            stmtStr = tmpName + " = " + op1 + op + op2 ;
                            
                            g.addToQuadrupleTable( units, stmtStr ) ;
                            
                            units.clear() ;
                            
                            Token tmp ;
                            tmp.str = tmpName ;
                            tmp.table = resultUnit.tableNo ;
                            tmp.table = resultUnit.index ;
                            idStack.push( tmp ) ;
                            
                        } // while()
                        
                        if ( opStack.top().str == "(" ) {
                            
                            opStack.pop() ;
                            
                        } // if()
                        
                    } // else if()
                    else { // normal operator, compare the precedence
                        
                        // when the recent token's precedence is higher, push into opStack
                        if ( opStack.empty() || hasHigherPrecedence( g.lowerToUp( stmt.tokens[ i ].str ), opStack.top().str ) ) {
                            
                            stmt.tokens[ i ].table = TABLE_RWORD ;
                            stmt.tokens[ i ].no = g.getRSWordIndex( stmt.tokens[ i ].str ) + 1 ;
                            
                            opStack.push( stmt.tokens[ i ] ) ;
                            
                        } // if()
                        else { // need to deal with the tokens in the stack first, and reserve the recent one
                            
                            // need to generate the quadruple form
                            
                            do { // while the recent token is still smaller then the second top of the stack then keep generating
                                
                                string tmpName = "T" + g.intToStr( g.getNewTmpNum() ) ; // get the recent tmp num and make the tmp variable name

                                g.addToTable0( tmpName ) ;
                                
                                resultUnit.tableNo = TABLE_TMP ;
                                resultUnit.index = g.getRecentTmpNum() ;
                                
                                if ( !opStack.empty() ) {

                                    operatorUnit.tableNo = opStack.top().table ;
                                    operatorUnit.index = opStack.top().no ;
                                    op = opStack.top().str ;
                                    opStack.pop() ;
                                    
                                } // if()
                                
                                if ( !idStack.empty() ) {

                                    operand2Unit.tableNo = idStack.top().table ;
                                    operand2Unit.index = idStack.top().no ;
                                    op2 = idStack.top().str ;
                                    idStack.pop() ;
                                    
                                } // if()
                                
                                
                                if ( !idStack.empty() ) {
                                    
                                    operand1Unit.tableNo = idStack.top().table ;
                                    operand1Unit.index = idStack.top().no ;
                                    op1 = idStack.top().str ;
                                    idStack.pop() ;
                                    
                                } // if()
                                
                                units.push_back( operatorUnit ) ;
                                units.push_back( operand1Unit ) ;
                                units.push_back( operand2Unit ) ;
                                units.push_back( resultUnit ) ;
                                
                                stmtStr = tmpName + " = " + op1 + op + op2 ;
                                
                                g.addToQuadrupleTable( units, stmtStr ) ;
                                
                                units.clear() ;
                                
                                Token tmpToken ;
                                tmpToken.str = "T" + g.intToStr( g.getRecentTmpNum() ) ;
                                tmpToken.table = TABLE_TMP ;
                                tmpToken.no = g.getRecentTmpNum() ;
                                idStack.push( tmpToken ) ;
                                
                            } while ( !idStack.empty() && !opStack.empty() && hasHigherPrecedence( opStack.top().str, stmt.tokens[ i ].str ) ) ;
                            
                            opStack.push( stmt.tokens[ i ] ) ;
                            
                        } // else()
                        
                    } // else()
                    
                } // if()
                else { // a variable or a number
                    
                    int varIndex = -1 ;
                    
                    if ( g.variableExist( stmt.tokens[ i ].str, g.findSubroutine( g.subroutine.top() ), varIndex ) ) {
                        
                        stmt.tokens[ i ].table = TABLE_IDENTIFIER ;
                        stmt.tokens[ i ].no = varIndex ;
                        
                    } // if()
                    else if ( g.isNumStr( stmt.tokens[ i ].str ) ) {
                        
                        Unit tmp = g.findNumInTable( stmt.tokens[ i ].str ) ;
                        
                        stmt.tokens[ i ].table = tmp.tableNo ;
                        stmt.tokens[ i ].no = tmp.index ;
                        
                    } // else()
                    
                    idStack.push( stmt.tokens[ i ] ) ;
                    
                } // else()
                
            } // if()
            else if ( i == indexTHEN ) { // deal with the op and operand laft in the stack
                
                // check the stacks are empty or not
                
                while ( !opStack.empty() && !idStack.empty() ) {
                    
                    if ( !opStack.empty() ) {

                        operatorUnit.tableNo = opStack.top().table ;
                        operatorUnit.index = opStack.top().no ;
                        op = opStack.top().str ;
                        opStack.pop() ;
                        
                    } // if()
                    
                    if ( !idStack.empty() ) {

                        operand2Unit.tableNo = idStack.top().table ;
                        operand2Unit.index = idStack.top().no ;
                        op2 = idStack.top().str ;
                        idStack.pop() ;
                        
                    } // if()
                    
                    if ( !idStack.empty() ) {
                        
                        operand1Unit.tableNo = idStack.top().table ;
                        operand1Unit.index = idStack.top().no ;
                        op1 = idStack.top().str ;
                        idStack.pop() ;
                        
                    } // if()
                    
                    string tmpName = "T" + g.intToStr( g.getNewTmpNum() ) ; // get the recent tmp num and make the tmp variable name
                        
                    g.addToTable0( tmpName ) ;
                        
                    resultUnit.tableNo = TABLE_TMP ;
                    resultUnit.index = g.getRecentTmpNum() ;
                    
                    stmtStr = tmpName + " = " + op1 + " " + op + " " + op2 ;
                    
                    Token tmpToken ;
                    tmpToken.str = "T" + g.intToStr( g.getRecentTmpNum() ) ;
                    tmpToken.table = TABLE_TMP ;
                    tmpToken.no = g.getRecentTmpNum() ;
                    idStack.push( tmpToken ) ;
                    
                    units.push_back( operatorUnit ) ;
                    units.push_back( operand1Unit ) ;
                    units.push_back( operand2Unit ) ;
                    units.push_back( resultUnit ) ;
                    
                    g.addToQuadrupleTable( units, stmtStr ) ;
                    
                    units.clear() ;
                    
                } // while()
                
                // now check whether this statement has label
                
                int labelIndex = -1 ;
                string labelName = stmt.tokens[ 0 ].str ; // because the grammer is correct, label should only exist in the first place
                
                if ( g.labelExist( labelName, labelIndex ) ) { // find the label exist! now we should assign the address for it
                    
                    assignLabelAddress( labelIndex, ( int ) g.table6.size() ) ;
                    
                } // if()
                
                // generate the quadruple form of the IF part
                
                operatorUnit.tableNo = TABLE_RWORD ;
                operatorUnit.index = g.getRSWordIndex( "IF" ) + 1 ;
                
                operand1Unit.tableNo = TABLE_TMP ;
                operand1Unit.index = g.getRecentTmpNum() ;
                
                Unit tmp ; // quadruple tmp
                tmp.tableNo = TABLE_QUADRUPLE ;
                
                operand2Unit = tmp ;
                resultUnit = tmp ;
                
                units.push_back( operatorUnit ) ;
                units.push_back( operand1Unit ) ;
                units.push_back( operand2Unit ) ;
                units.push_back( resultUnit ) ;
                
                stmtStr = "IF T" + g.intToStr( g.getRecentTmpNum() ) + " GO TO " ;
                
                g.addToQuadrupleTable( units, stmtStr ) ;
                
                units.clear() ;
                
                quadrupleIFIndex = ( int ) g.table6.size() - 1 ;
                
                g.table6[ quadrupleIFIndex ].units[ 2 ].index = ( int ) g.table6.size() + 1 ;
                g.table6[ quadrupleIFIndex ].units[ 3 ].index = ( int ) g.table6.size() + 1 ;
                g.table6[ quadrupleIFIndex ].stmt += g.intToStr( ( int ) g.table6.size() + 1 ) ;
                
                process( stmtThen, index ) ; // processing the Q1 part
                
                if ( countBlock == 2 ) {
                        
                    // no need GOTO statement
                    
                    g.table6[ quadrupleIFIndex ].units[ 2 ].tableNo = -1 ;
                    g.table6[ quadrupleIFIndex ].units[ 2 ].index = -1 ;
                    
                } // if()
                else if ( countBlock == 3 ) {
                    /*
                    Unit gto ;
                    Unit point ;
                    Unit empty ;
                    
                    gto.tableNo = TABLE_RWORD ;
                    gto.index = g.getRSWordIndex( "GTO" ) + 1 ;
                    
                    point.tableNo = TABLE_QUADRUPLE ;
                    
                    units.push_back( gto ) ;
                    units.push_back( empty ) ;
                    units.push_back( empty ) ;
                    units.push_back( point ) ;
                    
                    stmtStr = "" ;
                    for ( int count = 0 ; count < stmtThen.tokens.size() ; count ++ ) {
                        
                        stmtStr += stmtThen.tokens[ count ].str ;
                        
                        if ( count < stmtThen.tokens.size() - 1 ) {
                            
                            stmtStr += " " ;
                            
                        } // if()
                        
                    } // for()
                    
                    g.addToQuadrupleTable( units, stmtStr ) ;
                    
                    units.clear() ;
                    */
                    g.table6[ quadrupleIFIndex ].units[ 3 ].index = ( int ) g.table6.size() + 1 ;
                    
                    quadrupleTHENIndex  = ( int ) g.table6.size() - 1 ;
                    
                } // else if()
                
            } // else if ()
            else if ( i == indexELSE ) {
                
                g.table6[ quadrupleIFIndex ].units[ 3 ].index = ( int ) g.table6.size() + 1 ;
                g.table6[ quadrupleIFIndex ].stmt += ", ELSE GO TO " + g.intToStr( ( int ) g.table6.size() + 1 ) ;
                
                process( stmtElse, index ) ;
                
            } // else if()
            
        } // for()
        
    } // __if__()
    
    void __end__( Statement stmt ) {
        
        string str = g.lowerToUp( stmt.instr.str ) ;
        
        vector<Unit> units ;
        Unit empty ;
        Unit instrPointer ;
        
        string stmtStr = "" ;
        
        // now check whether this statement has label
        
        int labelIndex = -1 ;
        string labelName = stmt.tokens[ 0 ].str ; // because the grammer is correct, label should only exist in the first place
        
        if ( g.labelExist( labelName, labelIndex ) ) { // find the label exist! now we should assign the address for it
            
            assignLabelAddress( labelIndex, ( int ) g.table6.size() ) ;
            
        } // if()
        
        for ( int i = 0 ; i < stmt.tokens.size() ; i ++ ) {
            
            if ( stmt.tokens[ i ].str != ";" ) {
                
                stmtStr += stmt.tokens[ i ].str ;
                
            } // if()
            
            if ( i < stmt.tokens.size() - 1 ) {
                
                stmtStr += " " ;
                
            } // if()
            
        } // for()
        
        if ( str == "ENP" || str == "ENS" ) {
            
            instrPointer.tableNo = TABLE_RWORD ;
            instrPointer.index = g.getRSWordIndex( str ) + 1 ;
            
            units.push_back( instrPointer ) ;
            units.push_back( empty ) ;
            units.push_back( empty ) ;
            units.push_back( empty ) ;
            
            g.addToQuadrupleTable( units, stmtStr ) ;
            
            units.clear() ;
            
            if ( !g.subroutine.empty() ) {

                g.subroutine.pop() ; // leae this subroutine
                
            } // if()
            
        } // if()
        
    } // __end__()
    
    // index is where the label locate in the identifier table
    // unit is the pointer that this label pointed to the qraduple table
    void assignLabelAddress( int labelIndex, int tableIndex ) {
        
        Unit pointer ;
        pointer.tableNo = TABLE_QUADRUPLE ;
        pointer.index = tableIndex ;
        
        g.table5[ labelIndex ].pointer = pointer ;
        
    } // assignLabelAddress()
    
    void processForwardReference() {
        
        int labelIndex = -1 ;
        
        for ( int i = 0 ; i < g.fixSet.size() ; i ++ ) {
            
            if ( g.labelExist( g.fixSet[ i ].labelName, labelIndex ) ) {
                
                g.table6[ g.fixSet[ i ].quaIndex ].units[ g.fixSet[ i ].unitNo ].tableNo = g.table5[ labelIndex ].pointer.tableNo ;
                g.table6[ g.fixSet[ i ].quaIndex ].units[ g.fixSet[ i ].unitNo ].index = g.table5[ labelIndex ].pointer.index + 1 ;
                
            } // if()
            
        } // for()
        
    } // processForwardReference()
    
public:
    
    void processAll() {
        
        for ( int i = 0 ; i < g.input.size() ; i ++ ) {
            
            process( g.input[ i ], i ) ;
            
        } // for()
        
        processForwardReference() ;
        
    } // processAll()
    
    void process( Statement &stmt, int i ) {
        
        // now we go through the statements again, but this time we have to build the information
        string instr = stmt.instr.str ;
        
        if ( instr == "PROGRAM" ) {
            
            __program__( stmt ) ;
            
        } // if()
        else if ( instr == "VARIABLE" ) {
            
            __variable__( stmt) ;
            
        } // else if()
        else if ( instr == "DIMENSION" ) {
            
            __dimension__( stmt ) ;
            
        } // else if()
        else if ( instr == "LABEL" ) {
            
            __labelDeclare__( stmt ) ;
            
        } // else if()
        else if ( instr == "GTO" ) {
            
            __gto__( stmt ) ;
            
        } // else if()
        else if ( instr == "SUBROUTINE" ) {
            
            __subroutine__( stmt ) ;
            
        } // else if()
        else if ( instr == "CALL" ) {
            
            __call__( stmt, i ) ;
            
        } // else if()
        else if ( instr == "IF" ) {
            
            __if__( stmt, i ) ;
            
        } // else if()
        else if ( instr == "ENP" || instr == "ENS" ) {
             
            __end__( stmt ) ;
            
        } // else if()
        else { // ASSIGNMENT
            
            stmt.instr.str = "ASSIGNMENT" ;
            
            __assignment__( stmt ) ;
            
        } // else()
        
    } // process()
    
} ; // class DataProcessor()

int main() {
    
    // g.test_SetUpTableInfo() ;
    
    LexicalAnalysis la ;
    // la.test_LexicalAbalysis() ;
    la.getToken() ;
    //la.test_getToken() ;
    SyntaxAnalysis sa ;
    sa.syntax_analyze() ;
    // sa.testSA() ;
    
    DataProcessor dp ;
    dp.processAll() ;
    
    g.printTable6() ;
    g.pringError() ;
    // g.printIdentifierTable() ;
    // g.printInfoTable() ;
    g.createOutputFile() ;
    
} // main()
