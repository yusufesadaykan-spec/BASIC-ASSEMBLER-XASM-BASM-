## Basic-Assembler
a very basic light weight assembler that could create birnaries for arm64 x86_64 risc-v (currently risc-v and x86_64)
## Basic-Assemblers names
you can call basicAssembler as basm xasm normally program calls it xasm(i prefer to say too)
## Sytanx
in the basicAssembler xasm you are writing normally there is no sections here is an example:
```asm
#ifndef helloLib
#define helloLib
#include<otherLib.asm>
; hello there
exit:
mov rax, 60
mov rdi, 50
syscall

#endif
```
# Features
Memory-management:

  in the basicAssembler there is no db or else you only use resb
  ```asm
    
  section .text ; there is no section
  resb hello 50 ; bad example
    
  hello:
  resb 50 ; good example
  ret

  mov al, 69 ; E
  mov [hello+49], al 
    
  ```
# How to use
to use basicAssembler install and run it
```shell
basm -arch x86_64 input.asm -o output.bin
# only bin support you should link it your self
```
# What is next
1. we are going to add arm64 support
2. more macros


