;! \file   PricerOpt.nasm
;! \brief  32-bit uint to ascii conversion in assembler.
;! 
;! x86 processors only. Use NASM to assemble.
; 
; Copyright (c) 2009 by Michael Ellison. All Rights Reserved.

%ifdef _X8632
bits 32

%ifdef _WIN32
section .code
%endif

;----------------------------------------------------------------------------
; Convert a 32-bit unsigned int to ASCIIZ.
; max number is 10 bytes: 4294967295, so buffer should be 11+
;
; void PricerUI32ToA(PXUInt32 val, char* buffer);
;----------------------------------------------------------------------------
%ifdef PRICER_NO_LEADING_UNDERSCORE
            global  PricerUI32ToA
                    PricerUI32ToA:
%else
            global _PricerUI32ToA
                   _PricerUI32ToA:
%endif

   push     ebp
   mov      ebp,esp
   sub      esp,40
   push     edi
   push     ebx

   mov      eax,[ebp+8]       ; eax = val
   mov      edi,[ebp+12]      ; edi = buffer
   mov      ebx,'0'           ; EBX = '0' is added for ascii conversion

CheckUnder10_32:
   cmp      eax,9
   jg       NotSingle_32      ; if it's < 10, just toss it in directly.
   
Under10_32:                   ; special case - less than 10, convert
   add      eax,ebx           ; directly and get outta here.
   stosw                      ; save number and a trailing 0.
   pop      ebx
   pop      edi
   leave                      ; Exit
   ret
   
NotSingle_32:                 ; Point EDI at end of string...
   add      edi,2             ; We need to know length...
                              ; It's at least two bytes long if >= 10.
   mov      edx,99
FindLength_32:                ; Compare against successive 9's, incrementing
   cmp      eax,edx           ; EDI each time (99, 999, 9999, etc)
   jle      PrepConvert_32
   inc      edi               ; Value is still bigger, add a digit
   shl      edx,1             ; Multiply by 10 using shift/add
   mov      ecx,edx           ; x*2*4 + x*2 = x*10
   shl      edx,2
   add      edx,ecx
   add      edx,9             ; Add the next 9 and loop
   jmp      FindLength_32
   
PrepConvert_32: 
   mov      edx,eax           ; put value in edx, 0 in eax
   mov      eax,0
   
   std                        ; set the direction flag so we write backwards.
   stosb                      ; Toss in the ending null byte

   mov      ecx,10            ; ECX = 10 for division
Convert_32:
   mov      eax,edx           ; move current value into eax
   mov      edx,0             ; Divide EDX:EAX/10 
                              ; places remainder (val % 10) in EDX
   div      ecx               ; Quotient in EAX

   xchg     edx,eax           ; Swap Quotient with 0-9 val into al
   add      eax,ebx           ; Add '0' to it to convert to ASCII.
   stosb                      ; Save it to the buffer
   or       edx,edx           ; check for 0
   jnz      Convert_32        ; Loop until fully converted.

   cld                        ; Reset direction so caller doesn't
                              ; run backwards.

   pop      ebx               ; Clean up and exit.
   pop      edi
   leave
   ret
;----------------------------------------------------------------------------
;_X8632
%endif

