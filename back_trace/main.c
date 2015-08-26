//gcc main.c -ldl -rdynamic
//arm-linux-gnueabi-gcc main.c -ldl -rdynamic -o bt_arm -DTARGET

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define __USE_GNU 
#include <dlfcn.h>

void fun_call();

const int MAX_STACK_FRAME = 20;
int stack_fp_array[20];

void get_call_stack_fp()
{
	printf("addr of get_call_stack_fp : 0x%x\n", get_call_stack_fp);
	int i = 0;
	int *pfp_register = 0;
	int *pfp_next_register = 0;
	int cnt = 0;
	int ret = 0;
#ifdef TARGET
	__asm__(
		"mov %0, fp\n" 
		: "=r"(pfp_register)
	);
	pfp_next_register = pfp_register;
	for ( i = 0; i < MAX_STACK_FRAME && pfp_next_register != 0; i++)
	{
		pfp_next_register = (int*)*(pfp_register-1);	//get ebp
		stack_fp_array[i] = *pfp_register; 	//get eip		
		pfp_register = pfp_next_register;
	}	
#else
	__asm__(
		"mov %%ebp, %0\n" 
		: "=r"(pfp_register)
	);

	pfp_next_register = pfp_register;
	for ( i = 0; i < MAX_STACK_FRAME && pfp_next_register != 0; i++)
	{
		pfp_next_register = (int*)*pfp_register;	//get ebp
		stack_fp_array[i] = *(pfp_register + 1); 	//get eip		
		pfp_register = pfp_next_register;
	}
#endif
	
}

void print_stack_frame()
{
	printf("\nEIP in stack frame:\n");	
	int i = 0;
	for( ; i < MAX_STACK_FRAME && stack_fp_array[i] != 0; i++)
	{
		printf("stack_fp_array[%d] = 0x%08x.\n", i, stack_fp_array[i]);
	}
	
	printf("\nCall stack:\n");	
#ifdef __USE_GNU
	Dl_info symbol;	
	for( i = 0; i < MAX_STACK_FRAME && stack_fp_array[i] != 0; i++)
	{
		if ( 0 == dladdr((void*) stack_fp_array[i], &symbol))
		{
			printf("dladdr Failed!\n");	
		}
		else
		{
			printf("#%-2d 0x%08x, in %s at %s:0x%08x\n",i, symbol.dli_fbase, symbol.dli_sname, symbol.dli_fname, symbol.dli_saddr);
		}
	}
#else
	printf("Not define Dl_info ...\n");
#endif
	
}

int print_current_stack_frame()
{
	int addr = (int) print_current_stack_frame;
	printf("print_current_stack_frame is 0x%x\n", addr);
	
#ifdef __USE_GNU
	Dl_info symbol;
	if ( 0 == dladdr((void*) addr, &symbol))
	{
		printf("Called dladdr Failed!\n");	
	}
	else
	{
		printf("This Frame : dli_fname:[%s], dli_fbase:0x[%x], dli_sname[%s], dli_saddr:[%x]\n\n",symbol.dli_fname, symbol.dli_fbase, symbol.dli_sname, symbol.dli_saddr);
	}
#else
	printf("Not define Dl_info ...\n");
#endif
	return 0;	
}


void fun_call()
{
	printf("addr of fun_call : 0x%x\n", fun_call);
	get_call_stack_fp();
}

static void statci_fun()
{
	printf("addr of statci_fun : 0x%x\n", statci_fun);
	fun_call();
}

void myfunc(int ncalls)
{
	printf("addr of myfunc : 0x%x, ncalls = %d\n", myfunc, ncalls);
    if (ncalls > 1)
        myfunc(ncalls - 1);
    else
        statci_fun();
}

int main(int argc, char *argv[])
{
	//print_current_stack_frame();
	
	int num = 3;
    num = (argc != 2 ? num : atoi(argv[1]));
	myfunc(num);	
	print_stack_frame();	
    exit(EXIT_SUCCESS);
}


