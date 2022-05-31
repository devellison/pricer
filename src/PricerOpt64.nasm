;! \file   PricerOpt64.nasm
;! \brief  64-bit uint to ascii conversion in assembler.
;! 
;! x86-64 processors only. Use NASM to assemble.
;! Calling convention is for Mac OSX / Linux. 
;! Windows and others will require a stub.
; 
; Copyright (c) 2009 by Michael Ellison. All Rights Reserved.

%ifdef _X8664
bits 64

global _PricerUI64ToA
;----------------------------------------------------------------------------
; Convert a 64-bit unsigned int to ASCIIZ.
; max number is 20 bytes, so buffer should be 21+.
;
; void PricerUI64ToA(PXUInt64 val, char* buffer);
;----------------------------------------------------------------------------
_PricerUI64ToA:
   push     rbp
   mov      rbp,rsp
   sub      rsp,64            ; give ourselves a scratch buffer on the stack.

   mov      rax,rdi           ; rdi is first arg (val), move to rax
                              ; rsi is second arg (buffer)
CheckUnder10_64:
   cmp      rax,10
   jge      NotSingle_64      ; if it's < 10, just toss it in directly.

Under10_64:                   ; special case - less than 10, convert
                              ; directly and get outta here.
   mov      rdi,rsi           ; point rdi to target buffer
   add      al,'0'            ; convert al to ascii
   stosw                      ; save number and a trailing 0.
   leave
   ret
   
NotSingle_64:
   mov      rdi,rsp           ; point rdi at our scratch buffer

PrepConvert_64:               ; Setup for loop with value in rdx
   mov      rcx,10            ; rcx = divisor (10)
   mov      rdx,rax

; Here we're writing the string backwards to our stack space.
; We'll flip it around into the buffer once we know how long
; it is.

Convert_64:
   mov      rax,rdx           ; move current value into rax
   xor      rdx,rdx           ; Divide rdx:rax/10 
                              ; places remainder (val % 10) in rdx
   div      rcx               ; Quotient in rax

   xchg     rdx,rax           ; Swap Quotient with 0-9 val into al
   add      al,'0'            ; Add '0' to it to convert to ASCII.
   stosb                      ; Save it to the buffer
   or       rdx,rdx           ; check for 0
   jnz      Convert_64        ; Loop until zero.

FlipString_64:
; The stack now has the string on it, reversed.  We need to
; copy it and reverse it into the caller's buffer.

   mov      rcx,rdi
   sub      rcx,rsp           ; calc length
   dec      rdi               ; point rdi back to last written byte

   xchg     rsi,rdi           ; way faster if we made it align,
CopyToBuffer_64:              ; but it's already way faster
   std                        ; than sprintf() dreamed of being.
   lodsb
   cld
   stosb
   loop      CopyToBuffer_64
   xor       rax,rax
   stosb
   leave
   ret

;----------------------------------------------------------------------------
;_X8664
%endif

