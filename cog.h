#pragma once
//use #define COG_IMPLEMENTATION
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <wchar.h>
/*
	Initial Defines
*/
#ifndef global_alloc
#define global_alloc(amnt) malloc(amnt)
#endif 
#ifndef ARENA_CHUNK_SIZE
#define ARENA_CHUNK_SIZE 4096*2
#endif
#ifndef str_type
#define str_type wchar_t
#endif
#define nil 0
typedef unsigned char Byte;
/*
Arena stuff
*/
typedef struct{
	void * allocation;
	void * next;
	void * prev;
}FreeableAllocation;
typedef struct{
	void * buffer;
	void * end;
	void * ptr;
	void * last_allocation;
	void * next;
	FreeableAllocation *freeable_list;
}Arena;
Arena *init_arena();
Arena * sized_init_arena(size_t size);
void free_arena(Arena * arena);
void * arena_alloc(Arena * arena, size_t amnt);
void * arena_realloc(Arena * arena, void * ptr, size_t initial_size, size_t requested_size);
void * arena_alloc_freeable(Arena * arena, size_t amnt);
void arena_free(Arena * arena, void * ptr);
/*
Memory stuff
*/

void mem_shift(void * start, size_t size, size_t count, size_t distance);
void slice_cpy(void * target, void * source, size_t element_size, size_t count);


/*
Slice stuff
*/

#define enable_slice_type(T)\
	typedef struct {\
		T* arr;\
		size_t len;\
		size_t alloc_len;\
		Arena * arena;\
	}T##Slice;

#define make(T, parent_arena)\
	(T##Slice){.arr = (T*)arena_alloc(parent_arena,sizeof(T)*8), .alloc_len = 8, .len = 0, .arena = parent_arena}
#define make_destroyable(T, parent_arena)\
	(T##Slice){.arr = (T*)arena_alloc_freeable(parent_arena,sizeof(T)*8), .alloc_len = 8, .len = 0, .arena = parent_arena}
#define array(T) T.arr
#define append(v,q)\
	if(v.len+1>v.alloc_len){\
		v.arr = arena_realloc(v.arena, v.arr,v.alloc_len*sizeof(v.arr[0]), v.alloc_len*2*sizeof(v.arr[0]));\
		v.alloc_len *= 2;\
		array(v)[v.len] = q;\
		v.len++;\
	}\
	else{\
		array(v)[v.len] = q;\
		v.len++;\
	}
#define destroy(v)\
	if(v.arena == nil){\
		free(array(v));\
		v.len = 0;\
		v.alloc_len = 0;\
	} else{\
		arena_free(v.arena, v.arr);\
		v.arr = nil;\
		v.len = 0;\
	}\

#define len(v)\
	v.len

#define resize(v, q)\
	if(v.alloc_len<q){\
		int l = v.alloc_len*2;\
		if(l %8 != 0){\
			l += 8-l%8;\
		}\
		while(l<q){\
			l*=2;\
		}\
		v.arr = arena_realloc(v.arena, v.arr,v.alloc_len*sizeof(v.arr[0]), l*sizeof(v.arr[0]));\
		v.alloc_len = q;\
	}\
	if(q<v.len){\
		v.len = q;\
	}\

#define append_slice(v,q)\
	if(sizeof(v.arr[0]) == sizeof(q.arr[0])){\
		resize(v,len(v)+len(q));\
		slice_cpy(&v.arr[len(v)], &q.arr[0], sizeof(v.arr[0]), len(q));\
		len(v) += len(q);\
	} else{\
		printf("not equal types :(");\
		exit(1);\
	}


#define remove(v, q)\
	if(q>=0 && q<len(v)){\
		memmove(&v.arr[q], &v.arr[q+1],&v.arr[len(v)-1]-&v.arr[q+1]);\
		v.len--;\
	}\


#define insert(v, value, index)\
	if(index>=0 && index<len(v)){\
		if(v.alloc_len>len(v)+1){\
			v.arr = arena_realloc(v.arena,v.arr, v.alloc_len*2*sizeof(v.arr[0]));\
			v.alloc_len *= 2;\
		}\
		mem_shift(&v.arr[index], sizeof(v.arr[0]), len(v)-index, 1);\
		v.arr[index] = value;\
		v.len++;\
	}

void * mem_clone(Arena * arena, void * start, size_t element_size, size_t count);

#define clone(_arena, slice)\
	(typeof(slice)){.arr = mem_clone(_arena,slice.arr, sizeof(slice.arr[0]), len(slice)), .len =slice.len , .alloc_len = slice.alloc_len, .arena = _arena};



/*
basic slices for convenience
*/

enable_slice_type(int);
enable_slice_type(long);
typedef  uint32_t unsignedInt;
typedef  uint64_t unsignedLong;
enable_slice_type(unsignedInt);
enable_slice_type(unsignedLong);
enable_slice_type(float);
enable_slice_type(double);


/*
String stuff
*/

enable_slice_type(str_type);
typedef str_typeSlice String;
String new_string(Arena * arena, const char* str);
String new_string_wide(Arena * arena, const wchar_t* str);
void _strconcat(String * a, const char* b, size_t b_size);
String string_format(Arena *arena, const char * fmt, ...);
bool StringEquals(String a, String b);
String RandomString(Arena * arena, int minlen, int maxlen);
#define str_concat(a, b)\
	_strconcat(&a,(const char *)b, sizeof(b[0]));

#define str_append(a,b)\
	resize(a, len(a)+1);\
	a.arr[len(a)-1] = b

/*
HashFunctions
*/
size_t HashBytes(Byte * byte, size_t size);
size_t HashInt(int in);
size_t HashFloat(float fl);
size_t HashLong(long lg);
size_t HashDouble(double db);
size_t HashString(String str);
/*
Hashtable
*/
//maps Ts to Us
// I love you so much - Anna/toast <3
#define enable_hash_type(T,U)\
typedef struct{\
	T key;\
	U value;\
}T##U##KeyValuePair;\
enable_slice_type(T##U##KeyValuePair);\
typedef struct{\
	T##U##KeyValuePairSlice * Table;\
	size_t TableSize;\
	size_t (*hash_func)(T);\
	bool (*eq_func)(T,T);\
	Arena * arena;\
}T##U##HashTable;\
static T##U##HashTable T##U##HashTable_create(Arena * arena, size_t size,size_t (*hash_func)(T),bool (*eq_func)(T,T)){\
	T##U##HashTable out = (T##U##HashTable){.Table = arena_alloc_freeable(arena, sizeof(T##U##KeyValuePairSlice)*size), .TableSize = size, .hash_func = hash_func, .eq_func = eq_func, .arena = arena};\
	for(int i =0; i<size; i++){\
		out.Table[i] = make_destroyable(T##U##KeyValuePair,arena);\
		T##U##KeyValuePairSlice tmp = out.Table[i];\
		resize(tmp,128);\
		out.Table[i] = tmp;\
	}\
	return out;\
}\
static void T##U##HashTable_resize(T##U##HashTable * table, size_t new_size){\
	T##U##KeyValuePairSlice * new_table = arena_alloc(table->arena, new_size);\
	for(int i =0; i<new_size; i++){\
		new_table[i] = make(T##U##KeyValuePair, table->arena);\
	}\
	for(int i =0; i<table->TableSize; i++){\
		for(int j = 0; j<len(table->Table[i]); j++){\
			size_t hashval = table->hash_func(table->Table[i].arr[j].key);\
			size_t hash = hashval%new_size;\
			T##U##KeyValuePair pair = {.key = table->Table[i].arr[j].key, .value = table->Table[i].arr[j].value};\
			append(new_table[hash], pair);\
		}\
	}\
	T##U##KeyValuePairSlice * old = table->Table;\
	size_t old_len = table->TableSize;\
	table->Table = new_table;\
	table->TableSize = new_size;\
	for(int i =0; i<old_len; i++){\
		destroy(old[i]);\
	}\
	arena_free(table->arena,old);\
}\
static U* T##U##HashTable_find(T##U##HashTable* table, T key){\
	size_t hashval = table->hash_func(key);\
	size_t hash = hashval%table->TableSize;\
	for(int i =0 ; i<table->Table[hash].len; i++){\
		T##U##KeyValuePair p = table->Table[hash].arr[i];\
		if(table->eq_func(p.key, key)){\
			return &table->Table[hash].arr[i].value;\
		}\
	}\
	return nil;\
}\
static T##U##KeyValuePair* T##U##HashTable_find_kv(T##U##HashTable* table, T key){\
	size_t hashval = table->hash_func(key);\
	size_t hash = hashval%table->TableSize;\
	for(int i =0 ; i<table->Table[hash].len; i++){\
		T##U##KeyValuePair p = table->Table[hash].arr[i];\
		if(table->eq_func(p.key, key)){\
			return &table->Table[hash].arr[i];\
		}\
	}\
	return nil;\
}\
static void T##U##HashTable_insert(T##U##HashTable* table, T key, U value){\
	size_t hashval = table->hash_func(key);\
	size_t hash = hashval%table->TableSize;\
	T##U##KeyValuePair pair = (T##U##KeyValuePair){.key = key,.value = value};\
	T##U##KeyValuePairSlice tmp = table->Table[hash];\
	int tl = tmp.len;\
	append(tmp, pair);\
	table->Table[hash] = tmp;\
}\
static void T##U##HashTable_destroy(T##U##HashTable * table){\
	for(int i =0; i<table->TableSize; i++){\
		destroy(table->Table[i]);\
	}\
	arena_free(table->arena, table->Table);\
}
/*
Implementation
*/

#ifdef COG_IMPLEMENTATION
/*
Arena stuff
*/
bool is_arena_allocated(Arena * arena, void * ptr){
	Arena * tmp = arena;
	while(tmp!= nil){
		if(ptr>=tmp->buffer && ptr<=tmp->end){
			return 1;
		}
		tmp = tmp->next;
	}
	return 0;
}
FreeableAllocation * findAllocation(Arena * arena, void * ptr){
	FreeableAllocation * current = arena->freeable_list;
	while(current != nil){
		if(current->allocation == ptr){
			return current;
		}
		current = current->next;
	}
	return nil;
}
Arena *init_arena(){
	Arena * out = (Arena *)global_alloc(sizeof(Arena));
	out->buffer = global_alloc(ARENA_CHUNK_SIZE);
	out->end= out->buffer+ARENA_CHUNK_SIZE;
	out->ptr = out->buffer;
	out->last_allocation = nil;
	out->next = nil;
	out->freeable_list = nil;
	return out;
}
Arena * sized_init_arena(size_t sz){
	size_t size = sz;
	if(size<ARENA_CHUNK_SIZE){
		size = ARENA_CHUNK_SIZE;
	}
	if (size%8 != 0){
		size += 8-(size%8);
	}
	Arena * out = (Arena *)global_alloc(sizeof(Arena));
	out->buffer = global_alloc(size);
	out->end = out->buffer+size;
	out->ptr = out->buffer;
	out->last_allocation = nil;
	out->next = nil;
	return out;
}
void free_arena(Arena * in_arena){
	Arena * arena = in_arena;
	while(arena){
		free(arena->buffer);
		arena->ptr = nil;
		arena->next = nil;
		arena->end = nil;
		arena->last_allocation = nil;
		arena->buffer = nil;
			FreeableAllocation * list = arena->freeable_list;
			void * previous = nil;
			while(list){
				assert(previous == list->prev);
				previous = list;
				FreeableAllocation * next = list->next;
				free(list->allocation);
				FreeableAllocation * tmp = list;
				list = next;
				free(tmp);
			}
		Arena * old = arena;
		arena = old->next;
		free(old);
	}
}
void * arena_alloc(Arena * arena, size_t amnt){
	if(amnt<1){
		return nil;
	}
	if(arena == nil){
		return global_alloc(amnt);
	}
	if(arena->ptr+amnt>=arena->end){
		if(arena->next == nil){
			if(amnt>(arena->end-arena->buffer)*2){
				arena->next = sized_init_arena(amnt);
			} else{
				arena->next = sized_init_arena((arena->end-arena->buffer)*2);
			}
		}
		return arena_alloc((Arena *)arena->next,amnt);
	}
	size_t size = amnt;
	if(size%8 != 0){
		size += (8-size%8);
	}
	arena->last_allocation = arena->ptr;
	void * out = arena->ptr;
	arena->ptr = (void *)((size_t)arena->ptr+size);
	return out;
}
void * arena_realloc(Arena * arena, void * ptr, size_t initial_size, size_t requested_size){
	if(!is_arena_allocated(arena, ptr)){
		FreeableAllocation * alc = findAllocation(arena, ptr);
		if(alc){
			void * ptrl = ptr;
			ptrl = realloc(ptrl, requested_size);
			alc->allocation = ptrl;
			return ptrl;
		}
		return nil;
	}
	if (arena == nil){
		return realloc(ptr, requested_size);
	}
	if(ptr == arena->last_allocation){
		if((size_t)(arena->end)-(size_t)(arena->last_allocation)>=requested_size){
			arena->ptr = arena->last_allocation+requested_size;
			if((size_t)(arena->ptr) %8 != 0){
				arena->ptr = (void *)((size_t)arena->ptr+8-((size_t)(arena->ptr)%8));
			}
			return arena->last_allocation;
		}
	}
	char * nptr =(char *)arena_alloc(arena, requested_size);
	for(int i =0; i<initial_size; i++){
		nptr[i] = ((char *)ptr)[i];
	}
	return nptr;
}
void * arena_alloc_freeable(Arena * arena, size_t amnt){
	void * ptr = calloc(1, amnt);
	FreeableAllocation * alc = calloc(1, sizeof(FreeableAllocation));
	alc->allocation = ptr;
	FreeableAllocation * tmp = arena->freeable_list;
	alc->next = tmp;
	alc->prev = nil;
	arena->freeable_list = alc;
	if(tmp){
		tmp->prev = alc;
	}
	return ptr;
}
void arena_free(Arena * arena, void * ptr){
	FreeableAllocation* allc = findAllocation(arena, ptr);
	if(!allc){
		return;
	}
	if(allc->prev){
		((FreeableAllocation*)allc->prev)->next = allc->next;
	}
	if(allc->next){
		((FreeableAllocation*)allc->next)->prev = allc->prev;
	}
	if(allc == arena->freeable_list){
		arena->freeable_list = nil;
	}
	free(allc->allocation);
	free(allc);
}

/*
Memory Stuff
*/

void mem_shift(void * start, size_t size, size_t count, size_t distance){
	char * data = (char *)start;
	for(int j = 0; j<size*distance; j++){
		for (int i = count*size; i>0; i--){
			data[i] = data[i-1];
		}
	}
}
void slice_cpy(void * target, void * source, size_t element_size, size_t count){
	for(int i = 0; i<count*element_size; i++){
		((char *)target)[i] = ((char *)source)[i];
	}
}
void * mem_clone(Arena * arena, void * start, size_t element_size, size_t count){
	char * out = arena_alloc(arena, element_size*count);
	for(int i =0; i<element_size*count; i++){
		out[i] = ((char *)(start))[i];
	}
	return (void*)out;
}


/*
String Stuff
*/

String new_string(Arena * arena, const char* str){
	String out = make(str_type, arena);
	int l = strlen(str);
	for(int i = 0; i<l; i++){
		append(out, (str_type)str[i]);
	}
	append(out, '\0');
	return out;
}
String new_string_wide(Arena * arena, const wchar_t* str){
	String out = make(str_type, arena);
	int l = wcslen(str);
	for(int i = 0; i<l; i++){
		append(out, (str_type)str[i]);
	}
	append(out, '\0');
	return out;
}
void _strconcat(String * a, const char* b, size_t b_size){
	if(b_size <4){
		resize((*a), len((*a))+strlen(b));
		for(int i=0; i<strlen(b); i++){
			a->arr[a->len-1] = (str_type)(b[i]);
			a->len++;
		}
	}
	else {
		resize((*a), len((*a))+wcslen((const wchar_t *)b));
		const wchar_t * v = (const wchar_t *)b;
		for(int i=0; i<wcslen(v); i++){
			a->arr[a->len-1] = (str_type)(v[i]);
			a->len++;
		}
	}
}
String string_format(Arena *arena, const char * fmt, ...){
	String s =new_string(arena, "");
	va_list args;
	va_start(args, fmt);
	int l = strlen(fmt);
	for(int i = 0; i<l; i++){
		if(fmt[i] != '%'){
			if(s.len<1){
				s.arr[len(s)] = fmt[i];
				s.len++;
			} else{
				str_append(s, fmt[i]);
			}
			append(s, '\0');
		}
		else{
			if(fmt[i+1] == 'c'){
				char buff[2];
				buff[0] = (char)(va_arg(args,int));
				buff[1] = '\0';
				str_concat(s, buff);
				i++;
			}
			if(fmt[i+1] == 'l' && fmt[i+2] == 'u'){
				char buff[128];
				snprintf(buff,127, "%lu", va_arg(args,unsigned long));
				str_concat(s,buff);
				i+= 2;
			}
			if(fmt[i+1] == 'u'){		
				char buff[128];
				snprintf(buff,127, "%u", va_arg(args,unsigned int));
				str_concat(s,buff);
				i+= 1;
			}
			if(fmt[i+1] == 'l' && fmt[i+2] == 'd'){
				char buff[128];
				snprintf(buff,127, "%ld", va_arg(args,long));
				str_concat(s,buff);
				i+= 2;
			}
			if(fmt[i+1] == 'l' && fmt[i+2] == 's'){
				str_concat(s,va_arg(args, wchar_t *));
				i+= 2;
			}
			else if(fmt[i+1] == 's'){
				char * s2 =  va_arg(args,char *);
				str_concat(s, s2);
				i++;
			}
			else if(fmt[i+1] == 'f'){
				char buff[128];
				snprintf(buff,127,"%f", va_arg(args,double));
				str_concat(s,buff);
				i++;
			}
			else if(fmt[i+1] == 'd'){
				char buff[128];
				snprintf(buff,127,"%d", va_arg(args,int));
				str_concat(s,buff);
				i++;
			}
			else if(fmt[i+1] == '%'){
				append(s, '%');
				i++;
			}
		}
	}
	va_end(args);
	return s;
}
bool StringEquals(String a, String b){
	if(len(a) != len(b)){
		return 0;
	}
	for(int i= 0; i<len(a); i++){
		if(a.arr[i] != b.arr[i]){
			return 0;
		}
	}
	return 1;
}
String RandomString(Arena * arena,int minlen, int maxlen){
	int length = rand()%(maxlen-minlen)+minlen;
	String out = new_string(arena, "");
	resize(out, length+1);
	out.len = length+1;
	for(int i =0; i<length; i++){
		char c = rand()%(90-65)+65;
		if(rand()%2){
			c += 32;
		}
		out.arr[i] = c;
	}
	out.arr[length+1] = 0;
	return out;
}
/*
Hashing
*/

size_t HashBytes(Byte * bytes, size_t size){
	size_t out = 0;
	const size_t pmlt = 31;
	size_t mlt = 31;
	for(int i =0; i<size;i++){
		out += bytes[i]*mlt;
		mlt*=pmlt;
	}
	return out;
}
size_t HashInt(int in){
	int tmp = in;
	return HashBytes((void *)&tmp, sizeof(tmp));
}
size_t HashFloat(float fl){
	float tmp = fl;
	return HashBytes((void *)&tmp, sizeof(tmp));
}
size_t HashLong(long lg){
	long tmp = lg;
	return HashBytes((void *)&tmp, sizeof(tmp));
}
size_t HashDouble(double db){
	double tmp = db;
	return HashBytes((void *)&tmp, sizeof(tmp));
}
size_t HashString(String str){
	size_t out = 0;
	const size_t pmlt = 31;
	size_t mlt = 1;
	for(int i =0; i<str.len;i++){
		out += str.arr[i]*mlt;
		mlt*=pmlt;
	}
	return out;
}
#endif

