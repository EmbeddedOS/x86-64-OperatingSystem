[BITS 16]
[ORG 0x7e00]

start:
	mov [DriveID],dl
      	
	mov eax,0x80000000
      	cpuid
       	cmp eax,0x80000001
  	jb NotSupport
	
	mov eax,0x80000001
	cpuid
	
	test edx,(1<<29)
	jz NotSupport
        
	test edx,(1<<26)
        jz NotSupport

LoadKernel:
    	mov si,ReadPacket
 	mov word[si],0x10
      	mov word[si+2],0x64
	mov word[si+4],0x0
      	mov word[si+6],0x1000
       	mov dword[si+8],0x6
       	mov dword[si+12],0x0

       	mov dl,[DriveID]
       	mov ah,0x42
       	int 0x13
       	jc ReadError

        mov ah,0x13
        mov al,1
        mov bx,0xa
        xor dx,dx
        mov bp,Message
        mov cx,MessageLen
        int 0x10

ReadError:
NotSupport:
End:
        hlt
        jmp End

DriveID: 	db 0
Message:        db "Kernel is running"
MessageLen:     equ $-Message
ReadPacket:     times 16 db 0
