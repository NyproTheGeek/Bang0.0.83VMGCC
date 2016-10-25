// 25/10/16

#include <cstdio>
#include <iostream>
#include <chrono>
#include <ctime>
#include <cstdint>
#include <bitset>
//using std::chrono;


#define HALT      0
#define SET_X     1
#define DEC_X     2
#define JUMP_X_NZ 3

#define STORE_ADDRESS(index,label) data[index] = &&label
#define JUMP_TO_IP() goto **(ip++)

#define GET_OPCODES 0
#define RUN         1

void execute( void** data, int operation )
{
    int x = 0;
    void** ip;

    if (operation == GET_OPCODES) goto get_opcodes;

    ip = data;
    JUMP_TO_IP();

op_halt:
    return;

op_set_x:
    x = *((int*)(ip++));
    JUMP_TO_IP();

op_dec_x:
    --x;
    JUMP_TO_IP();

op_jump_x_nz:
    {
        void* target = *(ip++);
        if (x) ip = (void**) target;
        JUMP_TO_IP();
    }

get_opcodes:
    STORE_ADDRESS(HALT,op_halt);
    STORE_ADDRESS(SET_X,op_set_x);
    STORE_ADDRESS(DEC_X,op_dec_x);
    STORE_ADDRESS(JUMP_X_NZ,op_jump_x_nz);
}

bool isNegativeZero(double);
bool isNegativeDouble(double);
void checkEndianess();
long fibonacci(unsigned n);
int main()
{
    void* opcodes[10];
    void* program[20];

    execute( opcodes, GET_OPCODES );

    program[0] = opcodes[SET_X];
    program[1] = (void*) 1000000; /*1 million : 0.6s [Android], 0.016s [PC] */
    program[2] = opcodes[DEC_X];
    program[3] = opcodes[JUMP_X_NZ];
    program[4] = &program[2];
    program[5] = opcodes[HALT];

    std::chrono::time_point<std::chrono::system_clock> start_time, end_time;


    start_time = std::chrono::system_clock::now();
    execute( program, RUN );
//    std::cout << "f(42) = " << fibonacci(42) << '\n';
    end_time = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_seconds = end_time - start_time;

    printf( "Direct threaded done in %1f seconds.\n", elapsed_seconds.count());


   std::cout << "Zero is negative: " << (isNegativeZero(-0.0d) == true ? "true" : "false") << std::endl;
    std::cout << "Number is negative: " << (isNegativeDouble(52.0d) == true ? "true" : "false")  << std::endl;

    printf("Size of double: %d\n", sizeof(double));

    double dbl = -1.0d;
     std::cout << "Double 1.0: " <<std::bitset<sizeof(uint64_t)*8>( *reinterpret_cast<uint64_t*>(&dbl)) << std::endl;


   checkEndianess();

    return 0;
}

bool isNegativeZero(double n){
	return n == 0 && *reinterpret_cast<uint64_t *>(&n) != 0;
}

bool isNegativeDouble(double n){
	uint64_t x =  *reinterpret_cast<uint64_t*>(&n);
	// std::cout << "double : " <<std::bitset<sizeof(uint64_t)*8>( *reinterpret_cast<uint64_t*>(&x)) << std::endl;
	return (x >> 63) == 1;
	// x = x >> 63;
	// std::cout << "double : " <<std::bitset<sizeof(uint64_t)*8>( *reinterpret_cast<uint64_t*>(&x)) << std::endl;
	// return x == 1;
}

void checkEndianess(){
	int x = 7;
	char* p2 = reinterpret_cast<char*>(&x);
    if(p2[0] == '\x7')
        std::cout << "This system is little-endian\n";
    else
        std::cout << "This system is big-endian\n";
}


long fibonacci(unsigned n)
{
    if (n < 2) return n;
    return fibonacci(n-1) + fibonacci(n-2);
}
