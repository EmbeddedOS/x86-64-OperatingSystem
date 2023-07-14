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
	cmp bx, 0xAA55
	jne NotSupport

LoadLoader:
	mov si, ReadPacket
	mov word[si], 0x10
	mov word[si+2], 0x5
	mov word[si+4], 0x7E00
	mov word[si+6], 0x0
	mov dword[si+8], 0x1
	mov dword[si+12], 0x0

	mov dl,[DriveID]
	mov ah,0x42
	int 0x13
	jc ReadError

	mov dl,[DriveID]
	jmp 0x7E00

NotSupport:
ReadError:
	mov ah,0x13
	mov al,1
	mov bx,0xA
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

# Clear memory from current address to address at byte 446th (0x1BE).
times (0x1BE-($-$$)) db 0

# End of boot sector, we need 16 * 4 = 64 bytes for 4 partition entries. Some
# BIOS will try to find the valid partition entries. We want the BIOS treat our
# image as a hard disk and boot from them, so we need to define these entries.
# The first  partition entry:
db 0x80					# Boot indicator, 0x80 means boot-able partion.
db 0, 2, 0				# Starting of CHS value (Cylinder, Head, Sector).
db 0xF0					# Type of sector.
db 0xFF, 0xFF, 0xFF		# Ending of CHS value (Cylinder, Head, Sector).
dd 1					# Starting sector.
dd (20*16*63 - 1)		# Size of our disk: 10MB.

# Other entries are set to 0.
times (16*3) db 0

# Final two bytes are signature of the boot sector - magic numbers.
db 0x55
db 0xaa