// 26/10/16
// Computed gotos
// Dynamic union arrays

#include <iostream>
#include <cstdint>
#include <chrono>
using std::cout;
using std::endl;


union any{
	int sint; float f32;
};

int main(){
	cout << "\t\tHELLO THERE!" << endl;
	const int size = 10;
	any array[size];
	array[0].sint = 23;
	array[1].f32 = 5.5f;
	array[2].f32 = 4.5f;
	array[3].sint = 14;
	array[4].sint = 16;
	array[5].f32 = 0.6f;
	array[6].f32 = 3.3f;
	array[7].sint = 20000;
	array[8].sint = 173;
	array[9].f32 = 3.147;


	any array2[size];

	void *jmpTable[10] = {
		&&sint_copy,
		&&f32_copy,
		&&f32_copy,
		&&sint_copy,
		&&sint_copy,
		&&f32_copy,
		&&f32_copy,
		&&sint_copy,
		&&sint_copy,
		&&f32_copy
	};

	std::chrono::time_point<std::chrono::system_clock> start, end;
	start = std::chrono::system_clock::now();

	for (int i = 0; i < size; i++){
		goto body;

		f32_copy:
			array2[i].f32 = array[i].f32;
			goto bottom;

		sint_copy:
			array2[i].sint = array[i].sint;
			goto bottom;
			
		body:
			goto *jmpTable[i];

		bottom:
			{}
	}

	end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed = end - start;

	cout << "array2[0].sint: " << array2[0].sint << "\narray2[1].f32: "  << array2[1].f32 << "\narray2[2].f32: "  << array2[2].f32 << "\narray2[3].sint: "  << array2[3].sint << "\narray2[6].f32: " << array2[6].f32 << "\narray2[7].sint: "  << array2[7].sint << "\narray2[8].sint: "  << array2[8].sint << "\narray2[9].f32: "  << array2[9].f32 << endl;

	array2[6].sint = 2000;
	array2[7].f32 = 3.3f;
	array2[8].f32 = 3.147;
	array2[9].sint = 173;

	cout << "\t\tCHANGING ELEMENT TYPES\narray2[6].sint: " << array2[6].sint << "\narray2[7].f32: "  << array2[7].f32 << "\narray2[8].f32: "  << array2[8].f32 << "\narray2[9].sint: "  << array2[9].sint << endl;

	array2[6].f32 = 3.3f;
	array2[7].sint = 20000;
	array2[8].sint = 173;
	array2[9].f32 = 3.147;

	cout << "\t\tCHANGING ELEMENT TYPES\narray2[6].f32: " << array2[6].f32 << "\narray2[7].sint: "  << array2[7].sint << "\narray2[8].sint: "  << array2[8].sint << "\narray2[9].f32: "  << array2[9].f32 << endl;

	cout << "Time taken: " << elapsed.count() << "s" << endl;

	return 0;
}
