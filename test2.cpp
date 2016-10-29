// 25/10/16
// Dynamic Union Arrays
# include <iostream>
# include <cstdint>
using std::cout;
using std::endl;


union any{
	int sint; float f32;
};


union bro{
	int sint; float f32; double f64;
};


int main(){
	cout << "\t\tHELLO THERE!" << endl;
	
	any  array [5];
	array[0].sint = 23;
	array[1].f32 = 5.5;
	array[2].f32 = 4.5;
	array[3].sint = 24;
	
	// Array of unions. You can change member in each union.
	cout << "array[0].sint: " << array[0].sint << "\narray[1].f32: "  << array[1].f32 << "\narray[2].f32: "  << array[2].f32 << "\narray[3].sint: "  << array[3].sint << endl;
	
	array[0].f32 = 5.5;
	array[1].sint = 23;
	array[2].sint = 24;
	array[3].f32 = 4.5;
	
	cout << "array[0].f32: " << array[0].f32 << "\narray[1].sint: "  << array[1].sint << "\narray[2].sint: "  << array[2].sint << "\narray[3].f32: "  << array[3].f32 << endl;

	
	return 0;
}