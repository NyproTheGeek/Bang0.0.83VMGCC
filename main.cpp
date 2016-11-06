// 26/10/16
// Initial VM

#include <iostream>
#include <cstdint>
#include <chrono>
#include <vector>
#include <memory>
#include <map>
//#include <fstream>

using std::cout;
using std::endl;
using std::string;
//using std::ofstream;

// STRUCTURES
// PRIMITIVES
union PrimitiveValue{
    bool bl;
    int64_t i64; uint64_t u64;
    double f64;
};

struct StackPrimitive{
    unsigned type;
    PrimitiveValue value;
};

struct FuncObj{
    unsigned type;
    unsigned func;
};

struct TypeObj{
    unsigned type;
    unsigned dataType;
};

union Any;
struct AnyPtr;
union HashOpt;
struct StrObj;

struct AnyPtr{
    unsigned type;
    std::unique_ptr<Any> any;
};

// COMPLEX OBJ
struct ComplexObj{
    unsigned type;
    unsigned size;
    unsigned refCount;
    std::unique_ptr<AnyPtr> anyPtr;
};

struct HashObj{
	unsigned type;
	unsigned size;
	std::unique_ptr<StrObj> strKey;
	std::unique_ptr<HashOpt> content;
};

union HashOpt{
	 PrimitiveValue primVal;
	 ComplexObj complexObj;
	 HashObj hashObj;
};

struct MapObj{
	unsigned type;
	unsigned refCount;
	unsigned dimens[2]; // dimen, dimenSizesLength
	std::unique_ptr<unsigned> dimenSizes;
	std::unique_ptr<HashObj> hashObj;
};

union BufferOpt{
	std::unique_ptr<Any> any;
	std::unique_ptr<BufferOpt> bufferOpt;
};

struct BufferObj{
	unsigned type;
	unsigned dimens[2]; // dimen, dimenSizesLength
	std::unique_ptr<unsigned> dimenSizes;
	BufferOpt content;
};

struct StrObj{
	unsigned type; // + ascii_flag
	unsigned size;
	unsigned codePointSize;
	std::unique_ptr<uint8_t> content;
};

struct CharsObj{
	unsigned type; // + ascii_flag
	unsigned size;
	std::unique_ptr<uint32_t> content;
};

// ANY
union Any{
    BufferObj bufferObj; MapObj mapObj;
    StrObj strObj; CharsObj charsObj;
    ComplexObj complexObj; FuncObj funcObj;
    TypeObj typeObj; StackPrimitive stackPrim;
    // smart pointers claimed the members are nontrivial,
    // so these are needed. Pfft
    Any(){};
    ~Any(){};
};


// STACK
union Stack{ // IRRELEVANT
   AnyPtr anyPtr;
};

// FUNCTION AND TYPE
struct Func{
    unsigned type; // Func
    unsigned format;
    unsigned instructions;
//    std::unique_ptr<unsigned[]> callbackFuncs; // function index
//    std::unique_ptr<unsigned[]> locals; // types
//    std::pair<unsigned, unsigned> outersNeeded; // function index, outersOwned index
//    std::unique_ptr<Any> outersOwned;
//    std::unique_ptr<Any> constants;
//    std::unique_ptr<Any> globals;
//    unsigned backFuncsSize;
//    unsigned localsSize;
//    unsigned paramsSize;
//    unsigned outersOwnedSize;
//    unsigned constantsSize;
//    unsigned globalsSize;
//    unsigned instructionTop;
//    unsigned instructionBottom;
};

struct Type{
    unsigned type; // DataType
    std::unique_ptr<unsigned> parentTypes; // all of them
    std::unique_ptr<Any> mockObject;
    std::unique_ptr<unsigned> constructors;
    unsigned destructor;
    std::unique_ptr<unsigned> parentTypesSize;
    std::unique_ptr<unsigned> constructorsSize;
};

// LISTS
std::unique_ptr<Func> funcList;
std::unique_ptr<Type> typeList;

void vm(unsigned stackSize);
int main(){
	cout << "\t\tHELLO THERE!" << endl;

	std::chrono::time_point<std::chrono::system_clock> start, end;
	start = std::chrono::system_clock::now();
	//-----------------------BEGIN----------------------------------
	vm(1000);
    //------------------------END-----------------------------------
	std::chrono::duration<double> elapsed = end - start;
	cout << "Time taken: " << elapsed.count() << "s" << endl;

	return 0;
}

void vm(unsigned stackSize){

    // If you don't understand how the jump tables work.
    // Please do not touch them.
    // If you do, I will hunt you down.
    // Consider that a promise.

    // Stacks
    //int

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
        &&BL_EQ, &&I64_EQ, &&U64_EQ, &&F64_EQ,
        // NEQ
        &&BL_NEQ, &&I64_NEQ, &&U64_NEQ, &&F64_NEQ,
        // LT
        &&BL_LT, &&I64_LT, &&U64_LT, &&F64_LT,
        // LE
        &&BL_LE, &&I64_LE, &&U64_LE, &&F64_LE,
        // GT
        &&BL_GT, &&I64_GT, &&U64_GT, &&F64_GT,
        // GE
        &&BL_GE, &&I64_GE, &&U64_GE, &&F64_GE,
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
        // CASTS
        &&BL_I64_CAST, &&BL_U64_CAST, &&BL_F64_CAST,
        &&I64_BL_CAST, &&I64_U64_CAST, &&I64_F64_CAST,
        &&U64_BL_CAST, &&U64_I64_CAST, &&U64_F64_CAST,
        &&F64_BL_CAST, &&F64_I64_CAST, &&F64_U64_CAST,
    };

    void *instructionTable[] = {
        &&MOV_V, &&MOV_R, &&P_MOV_V, &&P_MOV_R, &&N_MOV_V, &&N_MOV_R,
        &&ADD, &&SUB, &&MUL, &&DIV, &&MOD, &&EXP, &&ROOT, &&UNM,
        &&INCR, &&DECR, &&STEP,
        &&CAST,
        &&EQ_, &&NEQ_, &&LT_, &&LE_, &&GT_, &&GE_,
        &&TRUE_, &&FALSE_,
        &&AND, &&NOT,
        &&BNOT, &&BOR, &&BAND, &&BXOR,
        &&SHL, &&SHR,
        &&JMP_, &&JMPBACK_,
        &&CALL, &&CALL_, &&N_CALL,
        &&STEPBACK, &&STEPBACK_,
        &&DISPATCH, &&DISPATCH_, &&N_DISPATCH, &&N_DISPATCH_,
        &&TMATCH1, &&TMATCH2, &&TMATCH3, &&TMATCH_,
        &&TRY_, &&CATCH_,
        &&THROW, &&RETHROW,
        &&TCHECK_,
        &&FROMMAP_, &&FROMBUF_, &&FROMSTR_, &&FROMCHARS_, &&FROMCOMP_,
        &&IGETKEY, &&IGETKEY_, &&SGETKEY, &&SGETKEY_, &&N_GETKEY, &&NGETKEY_,
        &&UPDATE,
        &&PRINT, &&SCAN,
        &&OPENFILE, &&READFILE, &&WRITEFILE, &&CLOSEFILE,
        &&RET_V, &&RET_R,
        &&CLEAR,
        &&EXIT,
    };

    // INSTRUCTION JUMP ACTIONS // _ instructions
    {
        MOV_V:{}
        MOV_R:{}
        P_MOV_V:{}
        P_MOV_R:{}
        N_MOV_V:{}
        N_MOV_R:{}
        ADD:{}
        SUB:{}
        MUL:{}
        DIV:{}
        MOD:{}
        EXP:{}
        ROOT:{}
        UNM:{}
        INCR:{}
        DECR:{}
        STEP:{}
        CAST:{}
        EQ_:{}
        NEQ_:{}
        LT_:{}
        LE_:{}
        GT_:{}
        GE_:{}
        TRUE_:{}
        FALSE_:{}
        AND:{}
        NOT:{}
        BNOT:{}
        BOR:{}
        BAND:{}
        BXOR:{}
        SHL:{}
        SHR:{}
        JMP_:{}
        JMPBACK_:{}
        CALL:{}
        CALL_:{}
        N_CALL:{}
        STEPBACK:{}
        STEPBACK_:{}
        DISPATCH:{}
        DISPATCH_:{}
        N_DISPATCH:{}
        N_DISPATCH_:{}
        TMATCH1:{}
        TMATCH2:{}
        TMATCH3:{}
        TMATCH_:{}
        TRY_:{}
        CATCH_:{}
        THROW:{}
        RETHROW:{}
        TCHECK_:{}
		FROMMAP_:{}
		FROMBUF_:{}
		FROMSTR_:{}
		FROMCHARS_:{}
		FROMCOMP_:{}
		IGETKEY:{}
		IGETKEY_:{}
		SGETKEY:{}
		SGETKEY_:{}
		N_GETKEY:{}
		NGETKEY_:{}
		UPDATE:{}
		PRINT:{}
		SCAN:{}
		OPENFILE:{}
		READFILE:{}
		WRITEFILE:{}
		CLOSEFILE:{}
		RET_V:{}
		RET_R:{}
		CLEAR:{}
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
        I64_EQ:{}
        U64_EQ:{}
        F64_EQ:{}
        // NEQ
        BL_NEQ:{}
        I64_NEQ:{}
        U64_NEQ:{}
        F64_NEQ:{}
        // LT
        BL_LT:{}
        I64_LT:{}
        U64_LT:{}
        F64_LT:{}
        // LE
        BL_LE:{}
        I64_LE:{}
        U64_LE:{}
        F64_LE:{}
        // GT
        BL_GT:{}
        I64_GT:{}
        U64_GT:{}
        F64_GT:{}
        // GE
        BL_GE:{}
        I64_GE:{}
        U64_GE:{}
        F64_GE:{}
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
        // CASTS
        BL_I64_CAST:{}
        BL_U64_CAST:{}
        BL_F64_CAST:{}
        I64_BL_CAST:{}
        I64_U64_CAST:{}
        I64_F64_CAST:{}
        U64_BL_CAST:{}
        U64_I64_CAST:{}
        U64_F64_CAST:{}
        F64_BL_CAST:{}
        F64_I64_CAST:{}
        F64_U64_CAST:{}
    }

}




