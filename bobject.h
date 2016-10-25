# ifndef BOBJECT_H
# define BOBJECT_H                            

struct primObjSint{
	unsigned type;
	unsigned refCount;
	int value;
};

struct primObjF32{
	unsigned type;
	unsigned refCount;
	float value;
};

union any{
	primObjSint sint; primObjF32 f32;
};
const int sizeOfAny = sizeof(any);

# endif // BOBJECT_H