#pragma once
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#ifndef global_alloc
#define global_alloc(amnt) malloc(amnt)
#endif 
#ifndef ARENA_CHUNK_SIZE
#define ARENA_CHUNK_SIZE 4096*2
#endif
#define nil 0
typedef struct{
	void * buffer;
	void * end;
	void * ptr;
	void * last_allocation;
	void * next;
}Arena;
static Arena *init_arena(){
	Arena * out = global_alloc(sizeof(Arena));
	out->buffer = global_alloc(ARENA_CHUNK_SIZE);
	out->end= out->buffer+ARENA_CHUNK_SIZE;
	out->ptr = out->buffer;
	out->last_allocation = nil;
	out->next = nil;
	return out;
}
static Arena * sized_init_arena(size_t size){
	if(size<ARENA_CHUNK_SIZE){
		size = ARENA_CHUNK_SIZE;
	}
	if (size%8 != 0){
		size += 8-(size%8);
	}
	Arena * out = global_alloc(sizeof(Arena));
	out->buffer = global_alloc(size);
	out->end = out->buffer+size;
	out->ptr = out->buffer;
	out->last_allocation = nil;
	out->next = nil;
	return out;
}
static void free_arena(Arena * arena){
	free(arena->buffer);
	if(arena->next){
		free_arena(arena->next);
	}
	arena->ptr = nil;
	arena->next = nil;
	arena->end = nil;
	arena->last_allocation = nil;
	arena->buffer = nil;
	free(arena);
}
static void * arena_alloc(Arena * arena, size_t amnt){
	if(amnt<1){
		return nil;
	}
	if(arena == nil){
		return global_alloc(amnt);
	}
	if(arena->buffer+amnt>arena->end){
		if(arena->next == nil){
			arena->next = sized_init_arena(amnt);
		}
		return arena_alloc(arena->next,amnt);
	}
	size_t size = amnt;
	if(size%8 != 0){
		size += (8-size%8);
	}
	arena->last_allocation = arena->ptr;
	void * out = arena->ptr;
	arena->ptr+= size;
	return out;
}
static void * arena_realloc(Arena * arena, void * ptr, size_t initial_size, size_t requested_size){
	if (arena == nil){
		return realloc(ptr, requested_size);
	}
	if(ptr == arena->last_allocation){
		if(arena->end-arena->last_allocation>=requested_size){
			arena->ptr = arena->last_allocation+requested_size;
			if((size_t)(arena->ptr) %8 != 0){
				arena->ptr += 8-((size_t)(arena->ptr)%8);
			}
			return arena->last_allocation;
		}
	}
	char * nptr = arena_alloc(arena, requested_size);
	for(int i =0; i<initial_size; i++){
		nptr[i] = ((char *)ptr)[i];
	}
	return nptr;
}
static void mem_shift(void * start, size_t size, size_t count, size_t distance){
	char * data = start;
	for(int j = 0; j<size*distance; j++){
		for (int i = count*size; i>0; i--){
			data[i] = data[i-1];
		}
	}
}
static void slice_cpy(void * target, void * source, size_t element_size, size_t count){
	for(int i = 0; i<count*element_size; i++){
		((char *)target)[i] = ((char *)source)[i];
	}
}
#define enable_slice_type(T)\
	typedef struct {\
		T* arr;\
		size_t len;\
		size_t alloc_len;\
		Arena * arena;\
	}T##Slice;

#define make(T, arena)\
	(T##Slice){.arr = arena_alloc(arena,sizeof(T)*8), .alloc_len = 8, .len = 0, .arena = arena}

#define append(v,q)\
	if(v.len+1>v.alloc_len){\
		v.arr = arena_realloc(v.arena, v.arr,v.alloc_len*sizeof(v.arr[0]), v.alloc_len*2*sizeof(v.arr[0]));\
		v.alloc_len *= 2;\
		v.arr[v.len] = q;\
		v.len++;\
	}\
	else{\
		v.arr[v.len] = q;\
		v.len++;\
	}
#define unmake(v)\
	if(v.arena == nil){\
		free(v.arr);\
		v.len = 0;\
		v.alloc_len = 0;\
	} else{\
		v.len = 0;\
	}\

#define len(v)\
	v.len


#define resize(v, q)\
	v.arr = arena_realloc(v.arena, v.arr,v.alloc_len*sizeof(v.arr[0]), q*sizeof(v.arr[0]));\
	v.alloc_len = q;\
	if(v.alloc_len<v.len){\
		v.len = v.alloc_len;\
	}\

#define concat(v,q)\
	if(sizeof(v.arr[0]) == sizeof(v.arr[0])){\
		resize(v,len(v)+len(q));\
		slice_cpy(&v.arr[len(v)], &q.arr[0], sizeof(v.arr[0]), len(q));\
		len(v) += len(q);\
	} else{\
		printf("not equal types");\
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
			v.arr = realloc(v.arr, v.alloc_len*2*sizeof(v.arr[0]));\
			v.alloc_len *= 2;\
		}\
		mem_shift(&v.arr[index], sizeof(v.arr[0]), len(v)-index, 1);\
		v.arr[index] = value;\
		v.len++;\
	}
enable_slice_type(char)
typedef charSlice String;
static String new_string(Arena * arena, const char* str){
	String out = make(char, arena);
	int l = strlen(str);
	for(int i = 0; i<l; i++){
		append(out, str[i]);
	}
	append(out, '\0');
	return out;
}
static void _strconcat(String * a, const char * b){
	for(int i=0; i<strlen(b); i++){
		a->arr[a->len-1] = b[i];
		a->len++;
	}
}
#define strconcat(a, b)\
	resize(a, len(a)+strlen(b));\
	_strconcat(&a, b);
	
String StringFormat(Arena *arena, const char * fmt, ...){
	String s;
	make(char, arena);
	va_list args;
	va_start(args, fmt);
	int l = strlen(fmt);
	for(int i = 0; i<l; i++){
		if(fmt[i] != '%'){
			if(s.len>1){
				s.arr[len(s)-1] = fmt[i];
			} else{
				append(s, fmt[i]);
			}
			append(s, '\0');
		}
		else{
			if(fmt[i+1] == 'c'){
				char buff[2];
				buff[0] = (char)(va_arg(args,int));
				buff[1] = '\0';
				strconcat(s, buff);
				i++;
			}
			if(fmt[i+1] == 'l' && fmt[i+2] == 'u'){
				char buff[128];
				snprintf(buff,127, "%lu", va_arg(args,unsigned long));
				strconcat(s,buff);
				i+= 2;
			}
			if(fmt[i+1] == 'u'){		
				char buff[128];
				snprintf(buff,127, "%u", va_arg(args,unsigned int));
				strconcat(s,buff);
				i+= 1;
			}
			if(fmt[i+1] == 'l' && fmt[i+2] == 'd'){
				char buff[128];
				snprintf(buff,127, "%ld", va_arg(args,long));
				strconcat(s,buff);
				i+= 2;
			}
			else if(fmt[i+1] == 's'){
				char * s2 =  va_arg(args,char *);
				strconcat(s, s2);
				i++;
			}
			else if(fmt[i+1] == 'f'){
				char buff[128];
				snprintf(buff,127,"%f", va_arg(args,double));
				strconcat(s,buff);
				i++;
			}
			else if(fmt[i+1] == 'd'){
				char buff[128];
				snprintf(buff,127,"%d", va_arg(args,int));
				strconcat(s,buff);
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

