# Cog
Go slices and Strings for c
# Slices
To make a slice of type T use make(T, arena) where arena is the arena the slice is allocated on. If the arena is null it uses a global allocator. Slices function as dynamic arrays so they grow to fit the data they are supposed to contain. 
To make a data type sliceable use enable_slice_type(T) to make T sliceable, the slice type name will be TSlice
To append to a slice use append(slice, data).
To get the length of a slice use len(slice). 
To resize a slice use resize(slice, new_length) where new length is the new number of elements of the slice. 
To free the memory of a non arena allocated slice use unmake(slice).
To append one slice to another use append(slice, second slice), note this alters the first slice.
To remove an element from a slice use remove(slice, index). this shrinks the slice.
To insert an element into a slice use insert(slice, element, index), this grows the slice.
To get the underlying array of a slice use slice.arr.
# Arenas
To make an arena use new_arena()
To free an arena use free_arena()
To allocate memory in an arena use arena_alloc(arena, requested size)
To reallocate memory in an arena use arena_realloc(arena,pointer to memory, starting size, requested new size)
if the alloc or realloc functions are called with a null pointer for the arena argument they will use malloc and realloc as standard
# Strings 
strings are fancy slices under the hood.
to make a new string use new_string(arena,c_string ).
to concatenate a c string to a string use str_concat(string, c string).
to create a formated string a la sprintf use format_string(arena, fmt_string, arguments) as you would printf, the only currently allowed arguments are %c, %lu, %u, %ld, %f, %d %s and %ls
to free a string don't,
note by default strings use wchar_ts so they should be printed using %ls. to change this do #define str_type (type) before including cog.h