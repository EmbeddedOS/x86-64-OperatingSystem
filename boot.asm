[BITS 16]
[ORG 0x7C00]	# The boot code start at address 0x7C00 and ascending.

start:
	# 1. Clear segment registers.
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov ss, ax

	# 2. Stack pointer start at address 0x7c00 and grows downwards.
	mov sp, 0x7C00

TestDiskExtension:
	mov [DriveID], dl
	mov ah, 0x41
	mov bx, 0x55AA
	int 0x13
	jc NotSupport
	cmp bx,0xaa55
	jne NotSupport

LoadLoader:
	mov si,ReadPacket
	mov word[si],0x10
	mov word[si+2],0x5
	mov word[si+4],0x7e00
	mov word[si+6],0x0
	mov dword[si+8],0x1
	mov dword[si+12],0x0

	mov dl,[DriveID]
	mov ah,0x42
	int 0x13
	jc ReadError

	mov dl,[DriveID]
	jmp 0x7e00

NotSupport:
ReadError:
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

DriveID:  	db 0
Message:	db "An error is occured when booting system."
MessageLen: 	equ $-Message
ReadPacket: 	times 16 db 0

times (0x1be-($-$$)) db 0
	db 80h
	db 0,2,0
	db 0f0h
	db 0ffh,0ffh,0ffh
	dd 1
	dd (20*16*63-1)
	
	times (16*3) db 0

	db 0x55
	db 0xaa 
