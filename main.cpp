// 26/10/16
// Initial VM

#include <iostream>
#include <cstdint>
#include <chrono>
#include <vector>
#include <memory>
#include <map>
#include <thread>
//#include <fstream>

using std::cout;
using std::endl;
using std::string;
//using std::ofstream;

// STRUCTURES
union Any;
struct Func{
    unsigned type;
    std::unique_ptr<int> argTypes;
    std::unique_ptr<Any> args;
    std::unique_ptr<int> linkedFuncs;
};

struct Type{
    unsigned type;
    std::unique_ptr<int> parentTypes;
    std::unique_ptr<Any> mockCompObject;
};

struct FuncObj{
    unsigned type;
    unsigned func;
};

struct TypeObj{
    unsigned type;
    unsigned dataType;
};

// ANY
union Any{
    int Int; unsigned UInt; int8_t I8; uint8_t U8;
    int16_t I16; uint16_t U16; int32_t I32; uint32_t U32;
    int64_t I64; uint64_t U64;
    float F32; double F64;
    FuncObj funcObj; TypeObj typeObj;
};


// MAPS
std::map<unsigned, Func*> funcMap;
std::map<unsigned, Type*> typeMap;

void hello(){
	cout << "OLLEH, DLROW" << endl;
}


void vm();
int main(){
	cout << "\t\tHELLO THERE!" << endl;



	std::chrono::time_point<std::chrono::system_clock> start, end;
	start = std::chrono::system_clock::now();
	//-----------------------BEGIN----------------------------------
    cout << "size of : " << sizeof(FuncObj) << endl;
    std::thread t1(hello);
    t1.join();
	vm();
    //------------------------END-----------------------------------
	std::chrono::duration<double> elapsed = end - start;
	cout << "Time taken: " << elapsed.count() << "s" << endl;

	return 0;
}

void vm(){

    // If you don't understand how the jump tables work.
    // Please do not touch this code. I will have to hunt you down if you do.
    // That's a promise.


    // JUMP TABLES
    void *primitiveTable[] = {
        // MOV
        &&INT_MOV, &&UINT_MOV, &&I8_MOV, &&U8_MOV,
        &&I16_MOV, &&U16_MOV, &&I32_MOV, &&U32_MOV,
        &&I64_MOV, &&U64_MOV, &&F32_MOV, &&F64_MOV,
        // ADD
        &&INT_ADD, &&UINT_ADD, &&I8_ADD, &&U8_ADD,
        &&I16_ADD, &&U16_ADD, &&I32_ADD, &&U32_ADD,
        &&I64_ADD, &&U64_ADD, &&F32_ADD, &&F64_ADD,
        // SUB
        &&INT_SUB, &&UINT_SUB, &&I8_SUB, &&U8_SUB,
        &&I16_SUB, &&U16_SUB, &&I32_SUB, &&U32_SUB,
        &&I64_SUB, &&U64_SUB, &&F32_SUB, &&F64_SUB,
        // MUL
        &&INT_MUL, &&UINT_MUL, &&I8_MUL, &&U8_MUL,
        &&I16_MUL, &&U16_MUL, &&I32_MUL, &&U32_MUL,
        &&I64_MUL, &&U64_MUL, &&F32_MUL, &&F64_MUL,
        // DIV
        &&INT_DIV, &&UINT_DIV, &&I8_DIV, &&U8_DIV,
        &&I16_DIV, &&U16_DIV, &&I32_DIV, &&U32_DIV,
        &&I64_DIV, &&U64_DIV, &&F32_DIV, &&F64_DIV,
        // MOD
        &&INT_MOD, &&UINT_MOD, &&I8_MOD, &&U8_MOD,
        &&I16_MOD, &&U16_MOD, &&I32_MOD, &&U32_MOD,
        &&I64_MOD, &&U64_MOD, &&F32_MOD, &&F64_MOD,
        // EXP
        &&INT_EXP, &&UINT_EXP, &&I8_EXP, &&U8_EXP, 
        &&I16_EXP, &&U16_EXP, &&I32_EXP, &&U32_EXP, 
        &&I64_EXP, &&U64_EXP, &&F32_EXP, &&F64_EXP, 
        // RT
        &&INT_RT, &&UINT_RT, &&I8_RT, &&U8_RT,
        &&I16_RT, &&U16_RT, &&I32_RT, &&U32_RT,
        &&I64_RT, &&U64_RT, &&F32_RT, &&F64_RT,
        // UNM
        &&INT_UNM, &&UINT_UNM, &&I8_UNM, &&U8_UNM,
        &&I16_UNM, &&U16_UNM, &&I32_UNM, &&U32_UNM,
        &&I64_UNM, &&U64_UNM, &&F32_UNM, &&F64_UNM,
        // EQ
        &&INT_EQ, &&UINT_EQ, &&I8_EQ, &&U8_EQ,
        &&I16_EQ, &&U16_EQ, &&I32_EQ, &&U32_EQ,
        &&I64_EQ, &&U64_EQ, &&F32_EQ, &&F64_EQ,
        // NEQ
        &&INT_NEQ, &&UINT_NEQ, &&I8_NEQ, &&U8_NEQ,
        &&I16_NEQ, &&U16_NEQ, &&I32_NEQ, &&U32_NEQ,
        &&I64_NEQ, &&U64_NEQ, &&F32_NEQ, &&F64_NEQ,
        // LT
        &&INT_LT, &&UINT_LT, &&I8_LT, &&U8_LT,
        &&I16_LT, &&U16_LT, &&I32_LT, &&U32_LT,
        &&I64_LT, &&U64_LT, &&F32_LT, &&F64_LT,
        // LE
        &&INT_LE, &&UINT_LE, &&I8_LE, &&U8_LE,
        &&I16_LE, &&U16_LE, &&I32_LE, &&U32_LE,
        &&I64_LE, &&U64_LE, &&F32_LE, &&F64_LE,
        // GT
        &&INT_GT, &&UINT_GT, &&I8_GT, &&U8_GT,
        &&I16_GT, &&U16_GT, &&I32_GT, &&U32_GT,
        &&I64_GT, &&U64_GT, &&F32_GT, &&F64_GT,
        // GE
        &&INT_GE, &&UINT_GE, &&I8_GE, &&U8_GE,
        &&I16_GE, &&U16_GE, &&I32_GE, &&U32_GE,
        &&I64_GE, &&U64_GE, &&F32_GE, &&F64_GE,
        // BNOT
        &&INT_BNOT, &&UINT_BNOT, &&I8_BNOT, &&U8_BNOT,
        &&I16_BNOT, &&U16_BNOT, &&I32_BNOT, &&U32_BNOT,
        &&I64_BNOT, &&U64_BNOT, &&F32_BNOT, &&F64_BNOT,
        // BOR
        &&INT_BOR, &&UINT_BOR, &&I8_BOR, &&U8_BOR,
        &&I16_BOR, &&U16_BOR, &&I32_BOR, &&U32_BOR,
        &&I64_BOR, &&U64_BOR, &&F32_BOR, &&F64_BOR,
        // BAND
        &&INT_BAND, &&UINT_BAND, &&I8_BAND, &&U8_BAND,
        &&I16_BAND, &&U16_BAND, &&I32_BAND, &&U32_BAND,
        &&I64_BAND, &&U64_BAND, &&F32_BAND, &&F64_BAND,
        // BXOR
        &&INT_BXOR, &&UINT_BXOR, &&I8_BXOR, &&U8_BXOR,
        &&I16_BXOR, &&U16_BXOR, &&I32_BXOR, &&U32_BXOR,
        &&I64_BXOR, &&U64_BXOR, &&F32_BXOR, &&F64_BXOR,
        // SHL
        &&INT_SHL, &&UINT_SHL, &&I8_SHL, &&U8_SHL,
        &&I16_SHL, &&U16_SHL, &&I32_SHL, &&U32_SHL,
        &&I64_SHL, &&U64_SHL,
        // SHR
        &&INT_SHR, &&UINT_SHR, &&I8_SHR, &&U8_SHR,
        &&I16_SHR, &&U16_SHR, &&I32_SHR, &&U32_SHR,
        &&I64_SHR, &&U64_SHR,
        // CASTS
    };

    void *instructionTable[] = {
        &&MOV, &&P_MOV,
        &&ADD, &&SUB, &&MUL, &&DIV, &&MOD, &&EXP, &&RT, &&UNM, &&P_ADD, &&P_SUB, &&P_MUL, &&P_DIV, &&P_MOD, &&P_EXP, &&P_RT, &&P_UNM,
        &&EQ, &&NEQ, &&LT, &&LE, &&GT, &&GE, &&P_EQ, &&P_NEQ, &&P_LT, &&P_LE, &&P_GT, &&P_GE,
        &&CAST, &&P_CAST,
        &&BNOT, &&BOR, &&BAND, &&BXOR, &&P_BNOT, &&P_BOR, &&P_BAND, &&P_BXOR,
        &&SHL, &&SHR, &&P_SHL, &&P_SHR,
        &&JMP, &&INDEX, &&CALL, &&RET
    };

    // INSTRUCTION JUMP ACTIONS
    {
        // 46 instructions
        MOV:{}
        P_MOV:{}
        ADD:{}
        SUB:{}
        MUL:{}
        DIV:{}
        MOD:{}
        EXP:{}
        RT:{}
        UNM:{}
        P_ADD:{}
        P_SUB:{}
        P_MUL:{}
        P_DIV:{}
        P_MOD:{}
        P_EXP:{}
        P_RT:{}
        P_UNM:{}
        EQ:{}
        NEQ:{}
        LT:{}
        LE:{}
        GT:{}
        GE:{}
        P_EQ:{}
        P_NEQ:{}
        P_LT:{}
        P_LE:{}
        P_GT:{}
        P_GE:{}
        CAST:{}
        P_CAST:{}
        BNOT:{}
        BOR:{}
        BAND:{}
        BXOR:{}
        P_BNOT:{}
        P_BOR:{}
        P_BAND:{}
        P_BXOR:{}
        SHL:{}
        SHR:{}
        P_SHL:{}
        P_SHR:{}
        JMP:{}
        INDEX:{}
        CALL:{}
        RET:{}
    }


    // PRIMITIVE JUMP ACTIONS
    {
        // MOV
        INT_MOV:{}
        UINT_MOV:{}
        I8_MOV:{}
        U8_MOV:{}
        I16_MOV:{}
        U16_MOV:{}
        I32_MOV:{}
        U32_MOV:{}
        I64_MOV:{}
        U64_MOV:{}
        F32_MOV:{}
        F64_MOV:{}
        // ADD
        INT_ADD:{}
        UINT_ADD:{}
        I8_ADD:{}
        U8_ADD:{}
        I16_ADD:{}
        U16_ADD:{}
        I32_ADD:{}
        U32_ADD:{}
        I64_ADD:{}
        U64_ADD:{}
        F32_ADD:{}
        F64_ADD:{}
        // SUB
        INT_SUB:{}
        UINT_SUB:{}
        I8_SUB:{}
        U8_SUB:{}
        I16_SUB:{}
        U16_SUB:{}
        I32_SUB:{}
        U32_SUB:{}
        I64_SUB:{}
        U64_SUB:{}
        F32_SUB:{}
        F64_SUB:{}
        // MUL
        INT_MUL:{}
        UINT_MUL:{}
        I8_MUL:{}
        U8_MUL:{}
        I16_MUL:{}
        U16_MUL:{}
        I32_MUL:{}
        U32_MUL:{}
        I64_MUL:{}
        U64_MUL:{}
        F32_MUL:{}
        F64_MUL:{}
        // DIV
        INT_DIV:{}
        UINT_DIV:{}
        I8_DIV:{}
        U8_DIV:{}
        I16_DIV:{}
        U16_DIV:{}
        I32_DIV:{}
        U32_DIV:{}
        I64_DIV:{}
        U64_DIV:{}
        F32_DIV:{}
        F64_DIV:{}
        // MOD
        INT_MOD:{}
        UINT_MOD:{}
        I8_MOD:{}
        U8_MOD:{}
        I16_MOD:{}
        U16_MOD:{}
        I32_MOD:{}
        U32_MOD:{}
        I64_MOD:{}
        U64_MOD:{}
        F32_MOD:{}
        F64_MOD:{}
        // EXP
        INT_EXP:{}
        UINT_EXP:{}
        I8_EXP:{}
        U8_EXP:{}
        I16_EXP:{}
        U16_EXP:{}
        I32_EXP:{}
        U32_EXP:{}
        I64_EXP:{}
        U64_EXP:{}
        F32_EXP:{}
        F64_EXP:{}
        // RT
        INT_RT:{}
        UINT_RT:{}
        I8_RT:{}
        U8_RT:{}
        I16_RT:{}
        U16_RT:{}
        I32_RT:{}
        U32_RT:{}
        I64_RT:{}
        U64_RT:{}
        F32_RT:{}
        F64_RT:{}
        // RT
        INT_UNM:{}
        UINT_UNM:{}
        I8_UNM:{}
        U8_UNM:{}
        I16_UNM:{}
        U16_UNM:{}
        I32_UNM:{}
        U32_UNM:{}
        I64_UNM:{}
        U64_UNM:{}
        F32_UNM:{}
        F64_UNM:{}
        // EQ
        INT_EQ:{}
        UINT_EQ:{}
        I8_EQ:{}
        U8_EQ:{}
        I16_EQ:{}
        U16_EQ:{}
        I32_EQ:{}
        U32_EQ:{}
        I64_EQ:{}
        U64_EQ:{}
        F32_EQ:{}
        F64_EQ:{}
        // NEQ
        INT_NEQ:{}
        UINT_NEQ:{}
        I8_NEQ:{}
        U8_NEQ:{}
        I16_NEQ:{}
        U16_NEQ:{}
        I32_NEQ:{}
        U32_NEQ:{}
        I64_NEQ:{}
        U64_NEQ:{}
        F32_NEQ:{}
        F64_NEQ:{}
        // LT
        INT_LT:{}
        UINT_LT:{}
        I8_LT:{}
        U8_LT:{}
        I16_LT:{}
        U16_LT:{}
        I32_LT:{}
        U32_LT:{}
        I64_LT:{}
        U64_LT:{}
        F32_LT:{}
        F64_LT:{}
        // LE
        INT_LE:{}
        UINT_LE:{}
        I8_LE:{}
        U8_LE:{}
        I16_LE:{}
        U16_LE:{}
        I32_LE:{}
        U32_LE:{}
        I64_LE:{}
        U64_LE:{}
        F32_LE:{}
        F64_LE:{}
        // GT
        INT_GT:{}
        UINT_GT:{}
        I8_GT:{}
        U8_GT:{}
        I16_GT:{}
        U16_GT:{}
        I32_GT:{}
        U32_GT:{}
        I64_GT:{}
        U64_GT:{}
        F32_GT:{}
        F64_GT:{}
        // GE
        INT_GE:{}
        UINT_GE:{}
        I8_GE:{}
        U8_GE:{}
        I16_GE:{}
        U16_GE:{}
        I32_GE:{}
        U32_GE:{}
        I64_GE:{}
        U64_GE:{}
        F32_GE:{}
        F64_GE:{}
        // BNOT
        INT_BNOT:{}
        UINT_BNOT:{}
        I8_BNOT:{}
        U8_BNOT:{}
        I16_BNOT:{}
        U16_BNOT:{}
        I32_BNOT:{}
        U32_BNOT:{}
        I64_BNOT:{}
        U64_BNOT:{}
        F32_BNOT:{}
        F64_BNOT:{}
        // BOR
        INT_BOR:{}
        UINT_BOR:{}
        I8_BOR:{}
        U8_BOR:{}
        I16_BOR:{}
        U16_BOR:{}
        I32_BOR:{}
        U32_BOR:{}
        I64_BOR:{}
        U64_BOR:{}
        F32_BOR:{}
        F64_BOR:{}
        // BAND
        INT_BAND:{}
        UINT_BAND:{}
        I8_BAND:{}
        U8_BAND:{}
        I16_BAND:{}
        U16_BAND:{}
        I32_BAND:{}
        U32_BAND:{}
        I64_BAND:{}
        U64_BAND:{}
        F32_BAND:{}
        F64_BAND:{}
        // BXOR
        INT_BXOR:{}
        UINT_BXOR:{}
        I8_BXOR:{}
        U8_BXOR:{}
        I16_BXOR:{}
        U16_BXOR:{}
        I32_BXOR:{}
        U32_BXOR:{}
        I64_BXOR:{}
        U64_BXOR:{}
        F32_BXOR:{}
        F64_BXOR:{}
        // SHL
        INT_SHL:{}
        UINT_SHL:{}
        I8_SHL:{}
        U8_SHL:{}
        I16_SHL:{}
        U16_SHL:{}
        I32_SHL:{}
        U32_SHL:{}
        I64_SHL:{}
        U64_SHL:{}
        // SHR
        INT_SHR:{}
        UINT_SHR:{}
        I8_SHR:{}
        U8_SHR:{}
        I16_SHR:{}
        U16_SHR:{}
        I32_SHR:{}
        U32_SHR:{}
        I64_SHR:{}
        U64_SHR:{}
    }
}











