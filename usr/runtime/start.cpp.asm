section .text
global Start
extern main
extern exit
extern __constructor_array_start
extern __constructor_array_end
extern __destructor_array_start
extern __destructor_array_end

Start:
; 1. Call all global constructors of static, global objects.
CallGlobalConstructors:
   mov rbx, __constructor_array_start
   jmp CheckConstructorList
CallConstructor:
   call [rbx]
   add rbx, 0x08
CheckConstructorList:
   cmp rbx, __constructor_array_end
   jb CallConstructor

; 2. Call user main function.
    call main

; 3. Call all global destructors of static, global objects.
CallGlobalDestructors: 
   mov rbx, __destructor_array_start
   jmp CheckDestructorList
CallDestructor:
   call [rbx]
   add rbx, 0x08
CheckDestructorList:
   cmp rbx, __destructor_array_end
   jb CallDestructor

; 4. Call exit.
    call exit
    jmp $