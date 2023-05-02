[BITS 16]
[ORG 0x7c00]

start:
	xor ax,ax
	mov ds,ax
	mov es,ax
	mov ss,ax
	mov sp,0x7c00

PrintMessage:
	mov ah,0x13
	mov al,1
	mov bx,0xa
	xor dx,dx
	mov bp,Message
	mov cx,MessageLen
	int 0x10

End:
	hlt
	jmp End

Message:	db "Hello world"
MessageLen: 	equ $-Message
