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

union Any;
struct AnyPtr;
union IndiePtr;
struct HashObj;
struct BufferObj;
struct StrObj;
struct CharsObj;
struct FuncObj;
struct TypeObj;
struct ComplexObj;
struct Func;
struct Type;

/*!
 * This struct contains a pointer to any object that can be
 * classified as Any. It also holds the type of the object
 * pointed to.
 */
struct AnyPtr{
    unsigned type;
    std::unique_ptr<Any> any;
};

/*!
 * This union contains pointers to objects and bare primitives.
 * Array objects are not included because they will be the ones
 * to own this union.
 */
union IndiePtr{
	std::unique_ptr<StrObj> strObj; std::unique_ptr<CharsObj> charsObj;
	std::unique_ptr<FuncObj> funcObj; std::unique_ptr<TypeObj> typeObj;
	std::unique_ptr<ComplexObj> complexObj;
	std::unique_ptr<bool> bl;
	std::unique_ptr<int64_t> i64;
	std::unique_ptr<uint64_t> u64;
	std::unique_ptr<double> f64;
};

union StackObj{
	StackPrimitive prim;
	AnyPtr any;
	StackObj(){};
    ~StackObj(){};
};


// COMPLEX OBJ
/*!
 * This struct represents an object of a user-defined type.
 */
struct ComplexObj{
    unsigned type;
    unsigned size;
    unsigned refCount;
    std::unique_ptr<StackObj> obj; // An array StackObjs
};

union HashOpt{
	IndiePtr indiePtr; // A array of an indie object
	std::unique_ptr<HashObj> hashObj; // An array of HashObjs
};

/*!
 * This struct represents an hash object.
 * Hash objects cannot be used directly, instead they are
 * contained within list objects or other nested hash objects.
 */
struct HashObj{
	unsigned type;
	unsigned size;
	std::unique_ptr<StrObj> strKey;
	HashOpt content; // An array of HashOpts
};


/*!
 * This struct represents a list object.
 * Bang's list objects have dual function. They can be used to
 * as a dynamic array or hashmaps.
 */
struct ListObj{
	unsigned type;
	unsigned refCount;
	unsigned dimen; // dimen (e.g. 3D)
	std::unique_ptr<unsigned> dimenSizes; // e.g. 2x4x3
	std::unique_ptr<HashObj> hashObj; // An array of HashObjs
};

union BufferOpt{
	IndiePtr indiePtr; // A array of an indie object
	std::unique_ptr<BufferObj> bufferObj; // An array of BufferObjs
};

/*!
 * This struct represents a buffer object.
 * Buffer objects can only be referenced once (iso), i.e. they
 * are meant to be owned by only one variable.
 * Buffers are plainly lists. They refer to contiguous blocks of
 * of memory and are sometimes preferred over list objects.
 * They are meant for low-level implementation in the language
 * and therefore should be used sparingly.
 */
struct BufferObj{
	unsigned type;
	unsigned dimen; // dimen (e.g. 3D)
	std::unique_ptr<unsigned> dimenSizes; // e.g. 2x4x3
	BufferOpt content;
};

/*!
 * This struct represents a tuple object.
 * A tuple object is 1-dimensional.
 */
struct TupleObj{
	unsigned type;
	unsigned size;
	unsigned refCount;
	IndiePtr indiePtr; // A array of an indie object
};

/*!
 * This struct represents a string object.
 * String supports UTF-8 encoding.
 * String is immutable, so content cannot be changed once
 * assigned.
 * If type field ascii bit flag is set, then the string content
 * can be interpreted byte by byte. This allows 0(N) access
 * codePointSize can be calculated and cached in order to reduce
 * repetitive calculation.
 */
struct StrObj{
	unsigned type; // + ascii_flag
	unsigned size;
	unsigned refCount;
	unsigned codePointSize;
	std::unique_ptr<uint8_t> content;
};

/*!
 * This struct represents a chars object.
 * Chars supports UTF-32 encoding.
 * Chars is mutable, so content can be changed. UTF-32 was chosen
 * for this reason, because it will allow 0(N) access to
 * characters.
 */
struct CharsObj{
	unsigned type;
	unsigned size;
	unsigned refCount;
	std::unique_ptr<uint32_t> content;
};

/*!
 * This struct represents a function object.
 * Bang's functions are first class functions. They can be
 * passed around as variables.
 */
struct FuncObj{
    unsigned type;
    std::shared_ptr<Func> func;
    std::unique_ptr<FuncObj> cofunctions;
    unsigned cofunctionsSize;
};

/*!
 * This struct represents a datatype object.
 * Bang's datatypes are first class structures. They can be
 * passed around as variables.
 */
struct TypeObj{
    unsigned type;
    std::shared_ptr<Func> dataType;
};

// FUNCTION AND TYPE
/*!
 * This struct represents the function content.
 * While function objects can be passed around through
 * variables, Funcs are the contents the function objects point
 * to.
 */
struct Func{
    unsigned type; // = Func
    unsigned format;
    unsigned instructions;
};


/*!
 * This struct represents the type content.
 * While type objects can be passed around through
 * variables, Types are the contents the type objects point
 * to.
 */
struct Type{
    unsigned type; // = DataType
    ComplexObj mockObject;
    std::unique_ptr<unsigned> ancestorTypes; // all of them
    std::unique_ptr<unsigned> constructors;
    unsigned normalDestructor;
    unsigned exceptionDestructor;
    unsigned ancestorTypesSize;
    unsigned constructorsSize;
};

// ANY
/*!
 * This union contains all structures defined under Any.
 */
union Any{
    BufferObj bufferObj; ListObj listObj; TupleObj tupleObj;
    StrObj strObj; CharsObj charsObj;
    ComplexObj complexObj;
    FuncObj funcObj; TypeObj typeObj;
    Any(){};
    ~Any(){};
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
 * Examples: INCR A; MKLIST A; RET A
 */
struct OneAddyInstr{
	int8_t opcode;
	int8_t addy1;
};

/*!
 * This struct represents an opcode with two addresses.
 * Examples: STEP A B; CATCH A B; MOV_R A B
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
 * This union contains all types of opcodes and attribute.
 */
union Instr{
	NoAddyInstr addy0Instr;
	OneAddyInstr addy1Instr;
	TwoAddyInstr addy2Instr;
	ThreeAddyInstr addy3Instr;
	uint32_t attribute;
};

// LISTS
std::unique_ptr<Func> funcList;
std::unique_ptr<Type> typeList;
std::unique_ptr<StackObj> formatList;
std::unique_ptr<StackObj> globalList;

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

    // STACK


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
        // CONCAT
        &&STR_CONCAT, &&CHARS_CONCAT,
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
        &&UINT_TO_INT, &&UINT_TO_FLOAT, &&UINT_TO_STR, &&UINT_TO_CHARS,
        &&INT_TO_UINT, &&INT_TO_FLOAT, &&INT_TO_STR, &&INT_TO_CHARS,
        &&FLOAT_TO_INT, &&FLOAT_TO_UINT, &&FLOAT_TO_STR, &&FLOAT_TO_CHARS,
        &&STR_TO_INT, &&STR_TO_UINT, &&STR_TO_FLOAT, &&STR_TO_CHARS,
        &&CHARS_TO_INT, &&CHARS_TO_UINT, &&CHARS_TO_FLOAT, &&CHARS_TO_STR,
    };



    void *instructionTable[] = {
        &&MOV_V, &&MOV_R, &&MOV_WR, &&P_MOV_V, &&P_MOV_R, &&P_MOV_WR, &&U_MOV_V, &&U_MOV_R, &&U_MOV_WR,
        &&ADD, &&SUB, &&MUL, &&DIV, &&MOD, &&EXP, &&ROOT, &&UNM,
        &&CONCAT,
        &&INCR, &&DECR, &&STEP,
        &&CAST,
        &&EQ, &&NEQ, &&LT, &&LE, &&GT, &&GE, &&EQ__, &&NEQ__, &&LT__, &&LE__, &&GT__, &&GE__,
        &&AND, &&OR,
        &&BNOT, &&BOR, &&BAND, &&BXOR,
        &&SHL, &&SHR,
        &&MKLIST, &&MKBUFFER, &&MKSTR, &&MKCHARS, &&MKTUPLE, &&MKTYPE,
        &&SETLIST, &&SETBUFFER, &&SETSTR, &&SETCHARS, &&SETTUPLE, &&SETCOMPLEX,
        &&IGETKEY, &&IGETKEY_, &&SGETKEY, &&SGETKEY_, &&U_GETKEY, &&U_GETKEY_,
        &&IGETINDEX, &&IGETINDEX_, &&U_GETINDEX, &&U_GETINDEX_,
        &&TCHECKST, &&TCHECKCV,
        &&NIL, &&U_NIL, &&D_NIL,
        &&CALL, &&RUN,
        &&STEPBACK,
        &&LEN,
        &&CATCH, &&THROW,
        &&JMP, &&JMPBACK,
        &&RET, &&RET__,
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
    	P_MOV_V:{}
    	P_MOV_R:{}
    	P_MOV_WR:{}
    	U_MOV_V:{}
    	U_MOV_R:{}
    	U_MOV_WR:{}
    	// ADD, SUB, MUL, DIV, MOD, EXP, ROOT, UNM
        ADD:{}
        SUB:{}
        MUL:{}
        DIV:{}
        MOD:{}
        EXP:{}
        ROOT:{}
        UNM:{}
        // CONCAT
        CONCAT:{}
        // INCR, DECR, STEP
        INCR:{}
        DECR:{}
        STEP:{}
        // CAST
        CAST:{}
        // EQ, NEQ, LT, LE, GT, GE
        EQ:{}
        NEQ:{}
        LT:{}
        LE:{}
        GT:{}
        GE:{}
        EQ__:{}
        NEQ__:{}
        LT__:{}
        LE__:{}
        GT__:{}
        GE__:{}
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
        MKLIST:{}
        MKBUFFER:{}
        MKSTR:{}
        MKCHARS:{}
        MKTUPLE:{}
        MKTYPE:{}
        // SET
        SETLIST:{}
        SETBUFFER:{}
        SETSTR:{}
        SETCHARS:{}
        SETTUPLE:{}
        SETCOMPLEX:{}
        // KEYS, INDICES
        IGETKEY:{}
        IGETKEY_:{}
        SGETKEY:{}
        SGETKEY_:{}
        U_GETKEY:{}
        U_GETKEY_:{}
        IGETINDEX:{}
        IGETINDEX_:{}
        U_GETINDEX:{}
        U_GETINDEX_:{}
        // TCHECK
        TCHECKST:{}
        TCHECKCV:{}
        // NIL
        NIL:{}
        U_NIL:{}
        D_NIL:{}
        // CALL, RUN
        CALL:{}
        RUN:{}
        // STEPBACK
        STEPBACK:{}
        LEN:{}
        // CATCH, THROW
        CATCH:{}
        THROW:{}
        // JMP, JMPBACK
        JMP:{}
        JMPBACK:{}
        // RET
        RET:{}
        RET__:{}
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
        // CONCAT
        STR_CONCAT:{}
        CHARS_CONCAT:{}
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
        UINT_TO_STR:{}
        UINT_TO_CHARS:{}
        INT_TO_UINT:{}
        INT_TO_FLOAT:{}
        INT_TO_STR:{}
        INT_TO_CHARS:{}
        FLOAT_TO_INT:{}
        FLOAT_TO_UINT:{}
        FLOAT_TO_STR:{}
        FLOAT_TO_CHARS:{}
        STR_TO_INT:{}
        STR_TO_UINT:{}
        STR_TO_FLOAT:{}
        STR_TO_CHARS:{}
        CHARS_TO_INT:{}
        CHARS_TO_UINT:{}
        CHARS_TO_FLOAT:{}
        CHARS_TO_STR:{}
    }

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
		if(trim(input) == "exit") more = false;
		else{
			cout << "> " << input;
			cout << endl;
			more = true;
		}
	}
}

void sizes(){
	cout
		<< "sizeof(Instr):\t\t" << sizeof(Instr) << " bytes"
		<< "\nsizeof(StackObj):\t" << sizeof(StackObj) << " bytes"
		<< "\nsizeof(Any):\t\t" << sizeof(Any) << " bytes"
		<< "\nsizeof(Type):\t\t" << sizeof(Type) << " bytes"
		<< "\nsizeof(Func):\t\t" << sizeof(Func) << " bytes"
		<< "\nsizeof(ComplexObj):\t" << sizeof(ComplexObj) << " bytes"
		<< endl;
}

// UTILITIES //
inline string trim(const string &str){
	auto wsfront = find_if_not(str.begin(), str.end(), [](int c){ return isspace(c); });
	auto wsback = find_if_not(str.rbegin(), str.rend(), [](int c){ return isspace(c); }).base();
	return (wsback <= wsfront ? string() : string(wsfront,wsback));
}


// 14/11/16
//"; SetConsoleTextAttribute(hConsole, k); cout << "
// Getting CMD Colors
/*for(int k = 1; k < 255; k++){
    // pick the colorattribute k you want
    SetConsoleTextAttribute(hConsole, k);
    cout << k << " I want to be nice today!" << endl;
}*/

/*	cout
		<<"\tFrom Console [LINUX]"
		<<"\n"
		<<"\n  ___ __  ______  _      __ ____  ______ "
		<<"\n /  "<<BOLD(FMAG("_"))<<"'  |/  "<<BOLD(FMAG("____"))<<"|/ |____|  / "<<BOLD(FMAG("___"))<<"|/  "<<BOLD(FMAG("_"))<<"   |"; cout/////
		<<"\n|  / |  |"<<BOLD(FMAG("___"))<<" \\__'   "<<BOLD(FMAG("___"))<<"/|   /   |  | |  |"; cout/////
		<<"\n|  \\_|  |___\\_  |  |____|  |    |  |_|  |"
		<<"\n \\"<<BOLD(FMAG("___"))<<"'"<<BOLD(FMAG("__"))<<"|"<<BOLD(FMAG("_______"))<<"/\\"<<BOLD(FMAG("______"))<<"|"<<BOLD(FMAG("__"))<<"|    |"<<BOLD(FMAG("_______"))<<"/"
		<<"\n                             "<<BOLD(FYEL("0.1 "))<<BOLD(FYEL("by Nypro"))
//		<<"\n"
//		<<"\n\t[GCC 4.9.1] (Android)"
//		<<"\n\t[GCC 4.9.2] (Windows)"
		<<endl;*/

/*cout
		<<"\tFrom Console [LINUX]"
		<<"\n"
		<<"\n  ___ __  ______  _      __ ____  ______ "
		<<"\n /  _'  |/  ____|/ |____|  / ___|/  _   |"
		<<"\n|  / |  |___ \\__'   ___/|   /   |  | |  |"
		<<"\n|  \\_|  |___\\_  |  |____|  |    |  |_|  |"
		<<"\n \\___'__|_______/\\______|__|    |_______/"
		<<"\n                             0.1 by Nypro"
		<<"\n"
//		<<"\n\t[GCC 4.9.1] (Android)"
//		<<"\n\t[GCC 4.9.2] (Windows)"
		<<endl;*/


// REPL DESIGN
// 13/11/16
/*
	cout
		<<"\tFrom Console [LINUX]"
		<<"\n"
		<<"\n  ___ __  ______  _      __ ____  ______ "
		<<"\n /  _'  |/  ____|/ |____|  / ___|/  _   |"
		<<"\n|  / |  |___ \__'   ___/|   /   |  | |  |"
		<<"\n|  \_|  |___\_  |  |____|  |    |  |_|  |"
		<<"\n \___'__|_______/\______|__|    |_______/"
		<<"\n                             0.1 by Nypro"
		<<"\n"
		<<endl;
*/
/*cout
		<< "\t\tFrom Console [LINUX]"
		<< "\n        __         _________  ____________  ________     _______  "
		<< "\n       /  \       /   ______||____    ____||   __   \   /  ___  \ "
		<< "\n      /    \      |  /            |  |     |  |  \   | |  /   \  |"
		<< "\n     /  /\  \     |  \_____       |  |     |  |   |  | |  |   |  |"
		<< "\n    /  /__\  \    \______   \     |  |     |  |   |  | |  |   |  |"
		<< "\n   /  ______  \          \  |     |  |     |  |__/  /  |  |   |  |"
		<< "\n  /  /      \  \         |  |     |  |     |   ___  \  |  |   |  |"
		<< "\n /  /        \  \  ______/  |     |  |     |  |   \  \ |  \___/  |"
		<< "\n/__/          \__\|_________/     |__|     |__|    \__\ \_______/ "
		<< "\n                                                   0.0.83 by Nypro"
		<< "\n"
		<< endl;
*/
