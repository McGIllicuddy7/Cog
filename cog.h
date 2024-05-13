#pragma once
//use #define COG_IMPLEMENTATION
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>
#include <wchar.h>
#include <sys/time.h>
/*
	Initial Defines
*/
void * debug_alloc(size_t count, size_t size);
void debug_free(void * ptr);
void debug_alloc_and_free_counts();
#ifndef global_alloc
#define global_alloc(count,sz) debug_alloc(count, sz)
#endif 
#ifndef global_free
#define global_free(ptr) debug_free(ptr)
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

#define arr * 
void * new_array(size_t object_size, size_t capacity, Arena * arena);
void delete_array(void * array);
void * array_concat(void * array, void * target, size_t object_size, size_t addr_size);
size_t array_length(void * array);
size_t array_capacity(void * array);
void *array_resize(void * array, size_t new_size, size_t obj_size);
void *array_remove(void * array, size_t idx, size_t obj_size);
void *array_clone(void * array, Arena * arena);
void * new_array_deletable(size_t object_size, size_t capacity, Arena * arena);

#define make(T, sz, _arena) new_array(sizeof(T), sz, _arena)
#define make_destroyable(T, sz, _arena) new_array_deletable(sizeof(T), sz, _arena)
#define clone(array, _arena) array_clone(array, _arena)
#define destroy(target) delete_array(target)
#define resize(target, new_size) target = array_resize(target,new_size, sizeof(*target))
#define append(target, addr) \
    resize(target, len(target)+1);\
    target[len(target)-1] = addr

#define concat(target, addr)     target =array_concat(target, addr, sizeof(*target), sizeof(*addr))
#define remove(target, idx)  target = array_remove(target, idx, sizeof(*target))
#define insert(target, idx,value)\
target = resize(target, len(target)+1);\
memmove(&target[idx+1], &target[idx], (len(target)-idx-1)*sizeof(*target));\
target[idx] = value
#define len(target) array_length(target)
#define cap(target) array_capacity(target)

/*
String stuff
*/

typedef str_type arr String;
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
	a[len(a)-1] = b

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
typedef struct{\
	T##U##KeyValuePair arr * Table;\
	size_t TableSize;\
	size_t (*hash_func)(T);\
	bool (*eq_func)(T,T);\
	Arena * arena;\
}T##U##HashTable;\
static T##U##HashTable * T##U##HashTable_create(Arena * arena, size_t size,size_t (*hash_func)(T),bool (*eq_func)(T,T)){\
	T##U##HashTable out = (T##U##HashTable){.Table = arena_alloc_freeable(arena, sizeof(T##U##KeyValuePair arr)*size), .TableSize = size, .hash_func = hash_func, .eq_func = eq_func, .arena = arena};\
	for(int i =0; i<size; i++){\
		out.Table[i] = make_destroyable(T##U##KeyValuePair,16,arena);\
		resize(out.Table[i],16);\
	}\
	T##U##HashTable * outv = arena_alloc_freeable(arena, sizeof(T##U##HashTable));\
	*outv = out;\
	return outv;\
}\
static void T##U##HashTable_resize(T##U##HashTable * table, size_t new_size){\
	T##U##KeyValuePair arr * new_table = arena_alloc_freeable(table->arena, new_size);\
	for(int i =0; i<new_size; i++){\
		new_table[i] = make_destroyable(T##U##KeyValuePair,8, table->arena);\
	}\
	for(int i =0; i<table->TableSize; i++){\
		for(int j = 0; j<len(table->Table[i]); j++){\
			size_t hashval = table->hash_func(table->Table[i][j].key);\
			size_t hash = hashval%new_size;\
			T##U##KeyValuePair pair = {.key = table->Table[i][j].key, .value = table->Table[i][j].value};\
			append(new_table[hash], pair);\
		}\
	}\
	T##U##KeyValuePair arr * old = table->Table;\
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
	for(int i =0 ; i<len(table->Table[hash]); i++){\
		T##U##KeyValuePair p = table->Table[hash][i];\
		if(table->eq_func(p.key, key)){\
			return &table->Table[hash][i].value;\
		}\
	}\
	return nil;\
}\
static T##U##KeyValuePair* T##U##HashTable_find_kv(T##U##HashTable* table, T key){\
	size_t hashval = table->hash_func(key);\
	size_t hash = hashval%table->TableSize;\
	for(int i =0 ; i<len(table->Table[hash]); i++){\
		T##U##KeyValuePair p = table->Table[hash][i];\
		if(table->eq_func(p.key, key)){\
			return &table->Table[hash][i];\
		}\
	}\
	return nil;\
}\
static void T##U##HashTable_insert(T##U##HashTable* table, T key, U value){\
	size_t hashval = table->hash_func(key);\
	size_t hash = hashval%table->TableSize;\
	T##U##KeyValuePair pair = (T##U##KeyValuePair){.key = key,.value = value};\
	T##U##KeyValuePair arr tmp = table->Table[hash];\
	int tl = len(tmp);\
	append(tmp, pair);\
	table->Table[hash] = tmp;\
}\
static void T##U##HashTable_destroy(T##U##HashTable * table){\
	for(int i =0; i<table->TableSize; i++){\
		destroy(table->Table[i]);\
	}\
	arena_free(table->arena, table->Table);\
	arena_free(table->arena,table);\
}
/*
Utils
*/
long get_time_microseconds();
void begin_profile();
long end_profile();
void end_profile_print(const char * message);
/*
Implementation
*/

#ifdef COG_IMPLEMENTATION 
static int alloc_count = 0;
static int free_count =0;
void * debug_alloc(size_t count, size_t size){
	alloc_count++;
	return calloc(count, size);
}
void debug_free(void * ptr){
	free_count++;
	free(ptr);
}
void debug_alloc_and_free_counts(){
	printf("alloc count: %d, free_count: %d\n", alloc_count, free_count);
}
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
	Arena * out = (Arena *)global_alloc(1,sizeof(Arena));
	out->buffer = global_alloc(1,ARENA_CHUNK_SIZE);
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
	Arena * out = (Arena *)global_alloc(1,sizeof(Arena));
	out->buffer = global_alloc(1,size);
	out->end = out->buffer+size;
	out->ptr = out->buffer;
	out->last_allocation = nil;
	out->next = nil;
	return out;
}
void free_arena(Arena * in_arena){
	Arena * arena = in_arena;
	int i =0;
	while(arena){
		global_free(arena->buffer);
		arena->ptr = nil;
		arena->end = nil;
		arena->last_allocation = nil;
		arena->buffer = nil;
			FreeableAllocation * list = arena->freeable_list;
			void * previous = nil;
			while(list){
				assert(previous == list->prev);
				previous = list;
				FreeableAllocation * next = list->next;
				global_free(list->allocation);
				FreeableAllocation * tmp = list;
				list = next;
				global_free(tmp);
			}
		Arena * old = arena;
		arena = arena->next;
		global_free(old);
	}
}
void * arena_alloc(Arena * arena, size_t amnt){
	if(amnt<1){
		return nil;
	}
	if(arena == nil){
		return global_alloc(1,amnt);
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
	if (arena == nil){
		return realloc(ptr, requested_size);
	}
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
	void * ptr = global_alloc(1, amnt);
	FreeableAllocation * alc = global_alloc(1, sizeof(FreeableAllocation));
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
	if(arena == nil){
		global_free(ptr);
	}
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
	global_free(allc->allocation);
	global_free(allc);
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
void * mem_clone(Arena * arena, void * start, size_t element_size, size_t count){
	char * out = arena_alloc(arena, element_size*count);
	for(int i =0; i<element_size*count; i++){
		out[i] = ((char *)(start))[i];
	}
	return (void*)out;
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
	for(int i =0; i<len(str);i++){
		out += str[i]*mlt;
		mlt*=pmlt;
	}
	return out;
}
/*
Utils
*/
long get_time_microseconds(){
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_usec+tv.tv_sec*1000000;
}
static long profile_time = 0;
void begin_profile(){
	if(profile_time == 0){
		profile_time = get_time_microseconds();
	}
}
long end_profile(){
	if(profile_time != 0){
		long out =  get_time_microseconds()-profile_time;
		profile_time = 0;
		return out;
	}
	return -1;
}
void end_profile_print(const char * message){
	printf("%s took %f seconds\n",message, ((double)end_profile())/1000000);
}

typedef struct{
    size_t length;
    size_t capacity;
    Arena * arena;
}ArrayHeader_t;
size_t to_pow_2(size_t sz){
    int i = 1;
    while(i<sz){
        i*=2;
    }
    return i;
}
ArrayHeader_t* GetHeader(void* array){
    return array-sizeof(ArrayHeader_t);
}
void * new_array(size_t object_size, size_t capacity, Arena * arena){
    size_t space = to_pow_2(capacity);
    void * tmp = arena_alloc(arena,sizeof(ArrayHeader_t)+object_size*space);
    ArrayHeader_t * head = tmp;
    head->length = 0;
    head->capacity = space;
    head->arena = arena;
    void * out = tmp+sizeof(ArrayHeader_t);
    return out;
}
void delete_array(void * array){
    ArrayHeader_t * head = GetHeader(array);
    arena_free(head->arena,head);
}
void * array_concat(void * array, void * target, size_t object_size, size_t addr_size){
    assert(addr_size == object_size);
    assert(array != target);
    ArrayHeader_t * array_head = GetHeader(array);
    int l = array_head->length;
    ArrayHeader_t * target_head = GetHeader(target);
    int v = target_head->length;
    void * out= array_resize(array, array_head->length+v,object_size);
    memcpy(out+l*object_size, target, object_size*v);
    return out;
}
size_t array_length(void * array){
    ArrayHeader_t * head = GetHeader(array);
    return head->length;
}
size_t array_capacity(void * array){
    ArrayHeader_t * head = GetHeader(array);
    return head->capacity;
}
void * array_resize(void * array, size_t new_size, size_t obj_size){
    ArrayHeader_t * head = GetHeader(array);
    if(head->capacity/2< new_size && head->capacity>=new_size){
        head->length = new_size;
        return array;
    }
    ArrayHeader_t old = *head;
    size_t sz = to_pow_2(new_size);
    void * out = arena_realloc(old.arena, head,old.capacity+sizeof(ArrayHeader_t),sz*obj_size+sizeof(ArrayHeader_t));
    head =out;
    head->capacity = sz;
    head->length = new_size;
    return out+sizeof(ArrayHeader_t);
}
void * array_remove(void * array, size_t idx, size_t obj_size){
    if(len(array) == 1){
        arena_free(GetHeader(array)->arena,array);
        return NULL;
    }
    size_t l = len(array);
    void * out = array_resize(array, len(array)-1, obj_size);
    void * start = out+idx*obj_size;
    void * end = out+(idx+1)*obj_size;
    size_t size = (l-idx-1)*obj_size;
    memmove(start, end, size);
    return out;
}
void *array_clone(void * array,Arena * new_arena){
    size_t sz = GetHeader(array)->capacity+sizeof(ArrayHeader_t);
    void * block = arena_alloc(new_arena, sz);
    char * tmp = block;
    char * old = (void *)GetHeader(array);
    for(int i =0; i<sz; i++){
        tmp[i] = old[i];
    }
    return block;
}
void * new_array_deletable(size_t object_size, size_t capacity, Arena * arena){
    size_t space = to_pow_2(capacity);
    void * tmp = arena_alloc_freeable(arena, sizeof(ArrayHeader_t)+object_size*space);
    ArrayHeader_t * head = tmp;
    head->length = 0;
    head->capacity = space;
    head->arena = arena;
    void * out = tmp+sizeof(ArrayHeader_t);
    return out;
}
/*
String Stuff
*/

String new_string(Arena * arena, const char* str){
	int l = strlen(str);
    String out = make(str_type,l,arena);
	for(int i = 0; i<l; i++){
		append(out, (str_type)str[i]);
	}
	append(out, '\0');
	return out;
}
String new_string_wide(Arena * arena, const wchar_t* str){
    int l = wcslen(str);
	String out = make(str_type, l, arena);
	for(int i = 0; i<l; i++){
		append(out, (str_type)str[i]);
	}
	append(out, '\0');
	return out;
}
void _strconcat(String * a, const char* b, size_t b_size){
	if(b_size <4){
        int l = len(*a)-1;
		resize((*a), len((*a))+strlen(b));
		for(int i=0; i<strlen(b); i++){
			(*a)[l+i] = (str_type)(b[i]);
		}
	}
	else {
		resize((*a), len((*a))+wcslen((const wchar_t *)b));
		const wchar_t * v = (const wchar_t *)b;
		for(int i=0; i<wcslen(v); i++){
			(*a)[len(a)-1] = (str_type)(v[i]);
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
            {
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
	if(a == 0 || b == 0){
		return 0;
	}
	if(len(a) != len(b)){
		return 0;
	}
	for(int i= 0; i<len(a); i++){
		if(a[i] != b[i]){
			return 0;
		}
	}
	return 1;
}
String RandomString(Arena * arena,int minlen, int maxlen){
	int length = rand()%(maxlen-minlen)+minlen;
	String out = make(str_type, length+1, arena);
	for(int i= 0; i<length+1; i++){
		out[i] = 0;
	}
    resize(out, length+3);
	for(int i =0; i<length; i++){
		char c = rand()%(90-65)+65;
		if(rand()%2){
			c += 32;
		}
		out[i] = c;
	}
	out[length+1] = 0;
	return out;
}
#endif
