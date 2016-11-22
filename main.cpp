// 26/10/16
// Initial VM
#include <string>
#include <cctype>
#include <algorithm>
#include <iostream>
#include <cstdint>
#include <chrono>
#include <vector>
#include <memory>
#include <map>
#include <fstream>
#if defined(_WIN32)
#include <windows.h>
#endif // defined

using std::cout;
using std::endl;
using std::cin;
using std::string;
using std::find_if_not;
using std::isspace;
using std::ifstream;

/*!
 * @file main.cpp
 * @brief The default vm
 * This is the default vm implemented entirely in 1 file.
 * It may not be the best practice, but this was implemented
 * as a proof of concept. Refined implementation will considered
 * later.
 * @author Steve (Nypro) Akin
 * @bug None that I know of. :-)
 */

/*!
 * @todo might remove all refCount fields if I see that I can rely
 * on shared_ptr internal refCount.
 */


// STRUCTURES
// PRIMITIVES
/*!
 * This union contains all possible 'bare' primitive values
 * supported by Astro.
 */
union PrimitiveValue{
    bool bl;
    int64_t i64; uint64_t u64;
    double f64;
};
/*!
 * This struct represents a primitive value on the stack
 */
struct StackPrimitive{
    unsigned type;
    PrimitiveValue value;
};


union IndiePtr;
struct Buffer;
struct ComplexPtr;
union StackObj;
struct Func;
struct Type;


/*!
 * This union contains pointers to objects and bare primitives.
 * Array objects are not included because they will be the ones
 * to own this union.
 */
union IndiePtr{
	std::unique_ptr<ComplexPtr> complex;
	std::unique_ptr<bool> bl;
	std::unique_ptr<int64_t> i64;
	std::unique_ptr<uint64_t> u64;
	std::unique_ptr<double> f64;
};

struct ComplexPtr{
	unsigned type;
	std::shared_ptr<StackObj> obj;
};


/*!
 * This struct represents an object of a user-defined type.
 */
struct ComplexObj{
    unsigned header;
    std::shared_ptr<StackObj> body;
    // type, size, refCount, fields...
};


union BufferOpt{
	IndiePtr indiePtr; // A array of indie objects
	std::unique_ptr<Buffer> buffer; // An array of Buffers
};
/*!
 * This struct represents a buffer.
 * Buffer can only be referenced once (iso), i.e. they
 * are meant to be owned by only one variable.
 * Buffers are plainly lists. They refer to contiguous blocks of
 * of memory and are sometimes preferred over list objects.
 * They are meant for low-level implementation in the language
 * and therefore should be used sparingly.
 */
struct Buffer{
	unsigned type;
	unsigned dimen; // dimen (e.g. 3D)
	std::unique_ptr<unsigned> dimenSizes; // e.g. 2x4x3
	BufferOpt content;
};


/*!
 * This struct represents an object that can fit on the stack.
 */
union StackObj{
	StackPrimitive prim;
	ComplexPtr complex;
	Buffer buffer;
	StackObj(){};
	~StackObj(){};
};


// FUNCTION AND TYPE
/*!
 * This struct represents the function content.
 * While function objects can be passed around through
 * variables, Funcs are the contents the function objects point
 * to.
 */
struct Func{ // NEEDS REWORK
	unsigned overloadTop;
	unsigned format;
	unsigned instructions;
};


/*!
 * This struct represents the type content.
 * While type objects can be passed around through
 * variables, Types are the contents the type objects point
 * to.
 */
struct Type{ // NEEDS REWORK
	unsigned ancestorSize;
	std::unique_ptr<uint16_t> ancestors; // ancestors is sorted.
	unsigned constructorsSize;
	std::unique_ptr<unsigned> constructors;
	unsigned normalDestructor;
	unsigned exceptionDestructor;
};


// INSTRUCTION
/*!
 * This struct represents an opcode with no address.
 * Example: RET__
 */
struct NoAddyInstr{
	int8_t opcode;
};
/*!
 * This struct represents an opcode with only one address.
 * Examples: INCR A; MKBUF A; RET A
 */
struct OneAddyInstr{
	int8_t opcode;
	int8_t addy1;
};
/*!
 * This struct represents an opcode with two addresses.
 * Examples: STEP A B; CATCH A B
 */
struct TwoAddyInstr{
	int8_t opcode;
	int8_t addy1;
	int8_t addy2;
};
/*!
 * This struct represents an opcode with three addresses.
 * Examples: ADD A B C; EQ A B C; SHL A B C
 */
struct ThreeAddyInstr{
	int8_t opcode;
	int8_t addy1;
	int8_t addy2;
	int8_t addy3;
};
/*!
 * This union contains all types of opcodes.
 */
union Instr{
	NoAddyInstr addy0Instr;
	OneAddyInstr addy1Instr;
	TwoAddyInstr addy2Instr;
	ThreeAddyInstr addy3Instr;
};




void sizes();
void vm(unsigned stackSize);
void useCommandlineArgs(int argc, char *argv[]);
void useConsoleArgs();
// MAIN
int main(int argc, char *argv[]){
	cout << "\t\tHELLO THERE!" << endl;

	std::chrono::time_point<std::chrono::system_clock> start, end;
	start = std::chrono::system_clock::now();
	//-----------------------BEGIN----------------------------------
	/*SIZES*/
	sizes();
	/*SIZES*/
	if(argc > 1) useCommandlineArgs(argc, argv);
	else useConsoleArgs();
	vm(1000);
	//------------------------END-----------------------------------
	end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed = end - start;
	cout << "> execution time: " << elapsed.count() << "s" << endl;

	return 0;
}



// VM
void vm(unsigned stackSize){

	/*!
     * If you don't understand how the jump tables work.
     * Please do not edit them.
     * If you do, I will hunt you down.
     * Consider that a promise.
     */

	
	// LISTS
	StackObj stack [stackSize];
	std::unique_ptr<unsigned> stackHeadList;
	
	std::unique_ptr<Func> funcList;
	std::unique_ptr<StackObj> layoutList;
	
	std::unique_ptr<Type> typeList;
	
	std::unique_ptr<StackObj> globalList;
	
	std::unique_ptr<Instr> instructionList;
	
	std::unique_ptr<uint64_t> attributeList;


    // JUMP TABLES
    void *primitiveOpTable[] = {
        // MOV_V
        &&BL_MOV_V, &&I64_MOV_V, &&U64_MOV_V, &&F64_MOV_V,
        // MOV_R
        &&BL_MOV_R, &&I64_MOV_R, &&U64_MOV_R, &&F64_MOV_R,
        // ADD
        &&I64_ADD, &&U64_ADD, &&F64_ADD,
        // SUB
        &&I64_SUB, &&U64_SUB, &&F64_SUB,
        // MUL
        &&I64_MUL, &&U64_MUL, &&F64_MUL,
        // DIV
        &&I64_DIV, &&U64_DIV, &&F64_DIV,
        // MOD
        &&I64_MOD, &&U64_MOD, &&F64_MOD,
        // EXP
        &&I64_EXP, &&U64_EXP, &&F64_EXP,
        // INCR
        &&BL_INCR, &&I64_INCR, &&U64_INCR, &&F64_INCR,
        // DECR
        &&BL_DECR, &&I64_DECR, &&U64_DECR, &&F64_DECR,
        // STEP
        &&BL_STEP, &&I64_STEP, &&U64_STEP, &&F64_STEP,
        // RT
        &&I64_ROOT, &&U64_ROOT, &&F64_ROOT,
        // UNM
        &&I64_UNM, &&U64_UNM, &&F64_UNM,
        // EQ
        &&BL_EQ, &&BL_EQ__, &&I64_EQ, &&I64_EQ__, &&U64_EQ, &&U64_EQ__, &&F64_EQ, &&F64_EQ__,
        // NEQ
        &&BL_NEQ, &&BL_NEQ__, &&I64_NEQ, &&I64_NEQ__, &&U64_NEQ, &&U64_NEQ__, &&F64_NEQ, &&F64_NEQ__,
        // LT
        &&BL_LT, &&BL_LT__, &&I64_LT, &&I64_LT__, &&U64_LT, &&U64_LT__, &&F64_LT, &&F64_LT__,
        // LE
        &&BL_LE, &&BL_LE__, &&I64_LE, &&I64_LE__, &&U64_LE, &&U64_LE__, &&F64_LE, &&F64_LE__,
        // GT
        &&BL_GT, &&BL_GT__, &&I64_GT, &&I64_GT__, &&U64_GT, &&U64_GT__, &&F64_GT, &&F64_GT__,
        // GE
        &&BL_GE, &&BL_GE__, &&I64_GE, &&I64_GE__, &&U64_GE, &&U64_GE__, &&F64_GE, &&F64_GE__,
        // AND
        &&BL_AND,
        // OR
        &&BL_OR,
        // BNOT
        &&I64_BNOT, &&U64_BNOT,
        // BOR
        &&I64_BOR, &&U64_BOR,
        // BAND
        &&I64_BAND, &&U64_BAND,
        // BXOR
        &&I64_BXOR, &&U64_BXOR,
        // SHL
        &&I64_SHL, &&U64_SHL,
        // SHR
        &&I64_SHR, &&U64_SHR,
        // LEN
        &&LIST_LEN, &&BUFFER_LEN, &&STR_LEN, &&CHARS_LEN, &&TUPLE_LEN, &&COMPLEX_LEN,
        // CAST
        &&UINT_TO_INT, &&UINT_TO_FLOAT,
        &&INT_TO_UINT, &&INT_TO_FLOAT,
        &&FLOAT_TO_INT, &&FLOAT_TO_UINT,
    };



    void *instructionTable[] = {
        &&MOV_V, &&MOV_R, &&MOV_WR,
        &&ADD, &&SUB, &&MUL, &&DIV, &&MOD, &&EXP, &&ROOT, &&UNM,
        &&INCR, &&DECR, &&STEP,
        &&CAST,
        &&EQ, &&LT, &&LE, &&EQ__, &&LT__, &&LE__,
        &&AND, &&OR,
        &&BNOT, &&BOR, &&BAND, &&BXOR,
        &&SHL, &&SHR,
        &&MKBUF, &&MKCMP,
        &&STAGEGLOBAL_,
        // last index is checked to determine if pulling/pushing barePrimitive
        &&PULLBUF_, &&PUSHBUF_,
        &&PULLINDEX, &&PUSHINDEX, // indexer
        &&PULLNEXT, // generator
        &&PULLCMP_V_, &&PULLCMP_R_, &&PULLCMP_WR_,
        // type is checked to determine if pulling/pushing stackPrimitive
        // no null checks when you are pulling a value or ref, 0only when you are pushing it.
        &&PUSHCMP_V_, &&PUSHCMP_R_, &&PUSHCMP_WR_,
        &&PUSHCMP_VNULL_, &&PUSHCMP_RNULL_, &&PUSHCMP_WRNULL_,
        &&INDEX, &&INDEX_,
        &&TYPECHECK, &&TYPECHECKSUB,
        &&SWITCH,
        &&CALL, &&RUN, &&EMIT, &&EMITTOP,
        &&STEPBACK,
        &&LENCMP, &&LENBUF,
        &&DIMEN, &&DIMENSIZE,
        &&CATCH, &&THROW,
        &&JMP, &&JMPBACK,
        &&RET, &&RET__,
        &&CLEAR,
        &&PRINT, &&PRINTLN, &&SCAN,
        &&OPENF, &&READF, &&WRITEF, &&CLOSEF,
        &&EXIT,
    };

    // INSTRUCTION JUMP ACTIONS // 85 instructions
    {
        // MOV
        MOV_V:{}
        MOV_R:{}
        MOV_WR:{}
        // ADD, SUB, MUL, DIV, MOD, EXP, ROOT, UNM
        ADD:{}
        SUB:{}
        MUL:{}
        DIV:{}
        MOD:{}
        EXP:{}
        ROOT:{}
        UNM:{}
        // INCR, DECR, STEP
        INCR:{}
        DECR:{}
        STEP:{}
        // CAST
        CAST:{}
        // EQ, NEQ, LT, LE, GT, GE
        EQ:{}
        LT:{}
        LE:{}
        EQ__:{}
        LT__:{}
        LE__:{}
        // AND OR
        AND:{}
        OR:{}
        // BNOT, BAND, BOR, BXOR,
        BNOT:{}
        BOR:{}
        BAND:{}
        BXOR:{}
        // SHL, SHR
        SHL:{}
        SHR:{}
        // MK
        MKBUF:{}
        MKCMP:{}
        // STAGEGLOBAL
        STAGEGLOBAL_:{}
        // PULL, PUSH
        PULLBUF_:{}
        PUSHBUF_:{}
        PULLINDEX:{}
        PUSHINDEX:{}
        PULLNEXT:{}
        PULLCMP_V_:{}
        PULLCMP_R_:{}
        PULLCMP_WR_:{}
        PUSHCMP_V_:{}
        PUSHCMP_R_:{}
        PUSHCMP_WR_:{}
        PUSHCMP_VNULL_:{}
        PUSHCMP_RNULL_:{}
        PUSHCMP_WRNULL_:{}
        // INDEX
        INDEX:{}
        INDEX_:{}
        // TYPECHECK
        TYPECHECK:{}
        TYPECHECKSUB:{}
        // SWITCH
        SWITCH:{}
        //  CALL, EMIT
        CALL:{}
        RUN:{}
        EMIT:{}
        EMITTOP:{}
        // STEPBACK
        STEPBACK:{}
        // LEN
        LENCMP:{}
        LENBUF:{}
        // DIMEN
        DIMEN:{}
        DIMENSIZE:{}
        // CATCH, THROW
        CATCH:{}
        THROW:{}
        // JMP, JMPBACK
        JMP:{}
        JMPBACK:{}
        // RET
        RET:{}
        RET__:{}
        // CLEAR
        CLEAR:{}
        // PRINT, SCAN
        PRINT:{}
        PRINTLN:{}
        SCAN:{}
        // OPENF, READF, WRITEF, CLOSEF
        OPENF:{}
        READF:{}
        WRITEF:{}
        CLOSEF:{}
        // EXIT
        EXIT:{}
    }


    // PRIMITIVE OP JUMP ACTIONS
    {
        // MOV_V
        BL_MOV_V:{}
        I64_MOV_V:{}
        U64_MOV_V:{}
        F64_MOV_V:{}
        // MOV_R
        BL_MOV_R:{}
        I64_MOV_R:{}
        U64_MOV_R:{}
        F64_MOV_R:{}
        // ADD
        I64_ADD:{}
        U64_ADD:{}
        F64_ADD:{}
        // SUB
        I64_SUB:{}
        U64_SUB:{}
        F64_SUB:{}
        // MUL
        I64_MUL:{}
        U64_MUL:{}
        F64_MUL:{}
        // DIV
        I64_DIV:{}
        U64_DIV:{}
        F64_DIV:{}
        // MOD
        I64_MOD:{}
        U64_MOD:{}
        F64_MOD:{}
        // EXP
        I64_EXP:{}
        U64_EXP:{}
        F64_EXP:{}
        // INCR
        BL_INCR:{}
        I64_INCR:{}
        U64_INCR:{}
        F64_INCR:{}
        // DECR
        BL_DECR:{}
        I64_DECR:{}
        U64_DECR:{}
        F64_DECR:{}
        // STEP
        BL_STEP:{}
        I64_STEP:{}
        U64_STEP:{}
        F64_STEP:{}
        // RT
        I64_ROOT:{}
        U64_ROOT:{}
        F64_ROOT:{}
        // UNM
        I64_UNM:{}
        U64_UNM:{}
        F64_UNM:{}
        // EQ
        BL_EQ:{}
        BL_EQ__:{}
        I64_EQ:{}
        I64_EQ__:{}
        U64_EQ:{}
        U64_EQ__:{}
        F64_EQ:{}
        F64_EQ__:{}
        // NEQ
        BL_NEQ:{}
        BL_NEQ__:{}
        I64_NEQ:{}
        I64_NEQ__:{}
        U64_NEQ:{}
        U64_NEQ__:{}
        F64_NEQ:{}
        F64_NEQ__:{}
        // LT
        BL_LT:{}
        BL_LT__:{}
        I64_LT:{}
        I64_LT__:{}
        U64_LT:{}
        U64_LT__:{}
        F64_LT:{}
        F64_LT__:{}
        // LE
        BL_LE:{}
        BL_LE__:{}
        I64_LE:{}
        I64_LE__:{}
        U64_LE:{}
        U64_LE__:{}
        F64_LE:{}
        F64_LE__:{}
        // GT
        BL_GT:{}
        BL_GT__:{}
        I64_GT:{}
        I64_GT__:{}
        U64_GT:{}
        U64_GT__:{}
        F64_GT:{}
        F64_GT__:{}
        // GE
        BL_GE:{}
        BL_GE__:{}
        I64_GE:{}
        I64_GE__:{}
        U64_GE:{}
        U64_GE__:{}
        F64_GE:{}
        F64_GE__:{}
        // AND
        BL_AND:{}
        // OR
        BL_OR:{}
        // BNOT
        I64_BNOT:{}
        U64_BNOT:{}
        // BOR
        I64_BOR:{}
        U64_BOR:{}
        // BAND
        I64_BAND:{}
        U64_BAND:{}
        // BXOR
        I64_BXOR:{}
        U64_BXOR:{}
        // SHL
        I64_SHL:{}
        U64_SHL:{}
        // SHR
        I64_SHR:{}
        U64_SHR:{}
		// LEN
        LIST_LEN:{}
        BUFFER_LEN:{}
        STR_LEN:{}
        CHARS_LEN:{}
        TUPLE_LEN:{}
        COMPLEX_LEN:{}
        // CAST
        UINT_TO_INT:{}
        UINT_TO_FLOAT:{}
        INT_TO_UINT:{}
        INT_TO_FLOAT:{}
        FLOAT_TO_INT:{}
        FLOAT_TO_UINT:{}
    }

}

// SETUP
void readCompiled(const string &astc){
}


// COMMANDLINE, CONSOLE
void useCommandlineArgs(int argc, char *argv[]){
	cout << "\t\tFrom Commandline" << argc << endl;
}


/* TERMINAL (e.g. BASH) REPL COLORS */
#if defined(__linux__) || defined(__CYGWIN__)
#define RST  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define FRED(x) KRED x RST
#define FGRN(x) KGRN x RST
#define FYEL(x) KYEL x RST
#define FBLU(x) KBLU x RST
#define FMAG(x) KMAG x RST
#define FCYN(x) KCYN x RST
#define FWHT(x) KWHT x RST

#define BOLD(x) "\x1B[1m" x RST
#define UNDL(x) "\x1B[4m" x RST
/* CMD REPL COLORS */
#elif defined(_WIN32)
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#define WRED 12 // correct
#define WGRN 10
#define WYEL 14
#define WBLU 9
#define WMAG 13
#define WCYN 11
#define WWHT 15
#endif // defined

string trim(const string &str);
void useConsoleArgs(){
    #if defined(__linux__) || defined(__CYGWIN__)
	cout
		<<"\t\tFrom Console[UNIX]"
		<<"\n"
		<<"\n  ___ __  ______  _      __ ____  ______ "
		<<"\n /  "<<BOLD(FMAG("_"))<<"'  |/  "<<BOLD(FMAG("____"))<<"|/ |____|  / "<<BOLD(FMAG("___"))<<"|/  "<<BOLD(FMAG("_"))<<"   |"; cout/////
		<<"\n|  / |  |"<<BOLD(FMAG("___"))<<" \\__'   "<<BOLD(FMAG("___"))<<"/|   /   |  | |  |"; cout/////
		<<"\n|  \\_|  |___\\_  |  |____|  |    |  |_|  |"
		<<"\n \\"<<BOLD(FMAG("___"))<<"'"<<BOLD(FMAG("__"))<<"|"<<BOLD(FMAG("_______"))<<"/\\"<<BOLD(FMAG("______"))<<"|"<<BOLD(FMAG("__"))<<"|    |"<<BOLD(FMAG("_______"))<<"/"
		<<"\n                             "<<BOLD(FYEL("0.1 "))<<BOLD(FYEL("by Nypro"))
		<<endl;
    #elif defined(_WIN32)
	cout
		<<"\t  From Console [WINDOWS]"
		<<"\n"
		<<"\n  ___ __  ______  _      __ ____  ______ "
		<<"\n /  "; SetConsoleTextAttribute(hConsole, WMAG);
		cout << "_"; SetConsoleTextAttribute(hConsole, WWHT);
		cout << "'  |/  "; SetConsoleTextAttribute(hConsole, WMAG);
		cout << "____"; SetConsoleTextAttribute(hConsole, WWHT);
		cout << "|/ |____|  / "; SetConsoleTextAttribute(hConsole, WMAG);
		cout << "___"; SetConsoleTextAttribute(hConsole, WWHT);
		cout << "|/  "; SetConsoleTextAttribute(hConsole, WMAG);
		cout << "_"; SetConsoleTextAttribute(hConsole, WWHT);
		cout << "   |"; ////
		cout <<"\n|  / |  |"; SetConsoleTextAttribute(hConsole, WMAG);
		cout << "___"; SetConsoleTextAttribute(hConsole, WWHT);
		cout << " \\__'   "; SetConsoleTextAttribute(hConsole, WMAG);
		cout << "___"; SetConsoleTextAttribute(hConsole, WWHT);
		cout << "/|   /   |  | |  |"; ////
		cout <<"\n|  \\_|  |___\\_  |  |____|  |    |  |_|  |"
		<<"\n \\"; SetConsoleTextAttribute(hConsole, WMAG);
		cout << "___"; SetConsoleTextAttribute(hConsole, WWHT);
		cout << "'"; SetConsoleTextAttribute(hConsole, WMAG);
		cout << "__"; SetConsoleTextAttribute(hConsole, WWHT);
		cout << "|"; SetConsoleTextAttribute(hConsole, WMAG);
		cout << "_______"; SetConsoleTextAttribute(hConsole, WWHT);
		cout << "/\\"; SetConsoleTextAttribute(hConsole, WMAG);
		cout << "______"; SetConsoleTextAttribute(hConsole, WWHT);
		cout << "|"; SetConsoleTextAttribute(hConsole, WMAG);
		cout << "__"; SetConsoleTextAttribute(hConsole, WWHT);
		cout << "|    |"; SetConsoleTextAttribute(hConsole, WMAG);
		cout << "_______"; SetConsoleTextAttribute(hConsole, WWHT);
		cout << "/"
		<<"\n                             "; SetConsoleTextAttribute(hConsole, WYEL);
		cout << "0.1 by Nypro"
		<<endl;
		SetConsoleTextAttribute(hConsole, WWHT);
    #endif
	//  MOCK
	#if defined(__linux__) || defined(__CYGWIN__)
	cout
		//<< "MOCK"
		<< BOLD(FYEL("\n::: ")) <<"func add(a, b): a + b"
		<< "\n> add function {0 overloads}"
		<< BOLD(FYEL("\n\n::: ")) <<"add(2, 3)"
		<< "\n> 5"
		//<< "\nMOCK"
		<< endl;
	#elif defined(_WIN32)
	SetConsoleTextAttribute(hConsole, WYEL);
	cout
		//<< "MOCK"
		<< "\n::: "; SetConsoleTextAttribute(hConsole, WWHT);
	cout
		<< "func add(a, b): a + b "
		<<  "\n> add function {0 overloads}"; SetConsoleTextAttribute(hConsole, WYEL);
	cout
		<< "\n\n::: "; SetConsoleTextAttribute(hConsole, WWHT);
	cout
		<<"add(2, 3)"
		<< "\n> 5"
		//<< "\nMOCK"
		<< endl;
	#endif
	bool more = true;
	string input = "";
	while(more){
		#if defined(__linux__) || defined(__CYGWIN__)
		cout << BOLD(FYEL("\n::: "));
		#elif defined(_WIN32)
		SetConsoleTextAttribute(hConsole, WYEL);
		cout << "\n::: ";
		SetConsoleTextAttribute(hConsole, WWHT);
		#endif
		std::getline(cin, input);
		/*
		*/
		if(trim(input) == "exit"|trim(input) == "^D") more = false;
		else{
			cout << "> " << input;
			cout << endl;
			more = true;
		}
	}
}

void sizes(){
	cout
		<< "sizeof(uint8_t):\t" << sizeof(uint8_t) << " bytes"
		<< "\nsizeof(uint16_t):\t" << sizeof(uint16_t) << " bytes"
		<< "\nsizeof(uint32_t):\t" << sizeof(uint32_t) << " bytes"
		<< "\nsizeof(uint64_t):\t" << sizeof(uint64_t) << " bytes"
		<< "\nsizeof(int):\t\t" << sizeof(int) << " bytes"
		<< "\nsizeof(Instr):\t\t" << sizeof(Instr) << " bytes"
		<< "\nsizeof(StackObj):\t" << sizeof(StackObj) << " bytes"
		<< "\nsizeof(StackPrimitive):\t" << sizeof(StackPrimitive) << " bytes"
		<< "\nsizeof(IndiePtr):\t" << sizeof(IndiePtr) << " bytes"
		<< "\nsizeof(ComplexPtr):\t" << sizeof(ComplexPtr) << " bytes"
		<< "\nsizeof(Buffer):\t\t" << sizeof(Buffer) << " bytes"
		<< "\nsizeof(Type):\t\t" << sizeof(Type) << " bytes"
		<< "\nsizeof(Func):\t\t" << sizeof(Func) << " bytes"
		<< endl;
}

// UTILITIES //
inline string trim(const string &str){
	auto wsfront = find_if_not(str.begin(), str.end(), [](int c){ return isspace(c); });
	auto wsback = find_if_not(str.rbegin(), str.rend(), [](int c){ return isspace(c); }).base();
	return (wsback <= wsfront ? string() : string(wsfront,wsback));
}
