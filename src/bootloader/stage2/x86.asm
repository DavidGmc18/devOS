
bits 16

section _TEXT class=CODE

;
; Unsigned 4 byte divide
;   Inputs:     DX;AX - Dividend
;               CX;BX - Divisor
;   Outputs:    DX;AX - Quotient
;               CX;BX - Remainder
;
global __U4D
__U4D:
    shl edx, 16
    mov dx, ax
    mov eax, edx

    xor edx, edx

    shl ecx, 16
    mov cx, bx

    div ecx ; eax - qout, edx - remainder

    mov ebx, edx
    mov ecx, edx
    shr ecx, 16

    mov edx, eax
    shr edx, 16

    ret


;
; void _cdecl x86_div64_32(uint64_t dividend, uint32_t divisor, uint64_t* quotientOut, uint32_t* remainderOut);
;
global _x86_div64_32
_x86_div64_32:
    ; make new call frame
    push bp             ; save old call frame
    mov bp, sp          ; initialize new call frame

    push bx

    ; divide upper 32 bits
    mov eax, [bp + 8]   ; eax <- upper 32 bits of dividend
    mov ecx, [bp + 12]  ; ecx <- divisor
    xor edx, edx
    div ecx             ; eax = qout, edx = remainder

    ; store upper 32 bits of quotient
    mov bx, [bp + 16]
    mov [bx + 4], eax

    ;divide lower 32 bits
    mov eax, [bp + 4]   ; eax <- lower 32 bits of dividend
                        ; edx <- old remainder 
    div ecx

    ; store results
    mov [bx], eax
    mov bx, [bp + 18]
    mov [bx], edx

    pop bx

    ; restore old call frame
    mov sp, bp
    pop bp
    ret


;
; void _cdecl x86_Video_WriteCharTeletype(char c, uint8_t page);
; int 10h ah=0Eh
; args: character, page
;
global _x86_Video_WriteCharTeletype
_x86_Video_WriteCharTeletype:
    
    ; make new call frame
    push bp             ; save old call frame
    mov bp, sp          ; initialize new call frame

    ; save bx
    push bx

    ; [bp + 0] - old call frame
    ; [bp + 2] - return address (small memory model => 2 bytes)
    ; [bp + 4] - first argument (character)
    ; [bp + 6] - second argument (page)
    ; note: bytes are converted to words (you can't push a single byte on the stack)
    mov ah, 0Eh
    mov al, [bp + 4]
    mov bh, [bp + 6]

    int 10h

    ; restore bx
    pop bx

    ; restore old call frame
    mov sp, bp
    pop bp
    ret


;
; uint8_t _cdecl x86_Disk_Reset(uint8_t drive);
;
global _x86_Disk_Reset
_x86_Disk_Reset:
    push bp
    mov bp, sp

    mov ah, 0
    mov dl, [bp + 4]
    stc
    int 13h

    mov al, ah ; return code

    mov sp, bp
    pop bp
    ret

;
; uint8_t _cdecl x86_Disk_GetDriveParams(uint8_t drive, uint8_t* driveTypeOut, uint16_t* cylindersOut, uint16_t* headsOut, uint16_t* sectorsOut);
;
global _x86_Disk_GetDriveParams
_x86_Disk_GetDriveParams:
    push bp
    mov bp, sp

    push es
    push bx
    push si
    push di

    mov dl, [bp + 4]
    mov ah, 08h
    mov di, 0
    mov es, di
    stc
    int 13h

    mov al, ah ; return code

    mov si, [bp + 6]    ; drive type from bl
    mov [si], bl

    mov bl, ch          ; lower bits in ch
    mov bh, cl          ; upper bits in cl (6-7)
    shr bh, 6
    mov si, [bp + 8]
    mov [si], bx

    xor ch, ch          ; sectors - lower 5 bits in cl
    and cl, 0x3F
    mov si, [bp + 12]
    mov [si], cx

    mov cl, dh          ; heads - dh
    mov si, [bp + 10]
    mov [si], cx

    pop di
    pop si
    pop bx
    pop es

    mov sp, bp
    pop bp
    ret


;
; uint8_t _cdecl x86_Disk_Read(uint8_t drive, uint16_t cylinder, uint16_t head, uint16_t sector, uint8_t count, void far* dataOut);
;                              [bp + 4],      [bp + 6],          [bp + 8],      [bp + 10],       [bp + 12],     [bp + 14]
;
global _x86_Disk_Read
_x86_Disk_Read:
    push bp
    mov bp, sp

    push bx
    push es

    mov dl, [bp + 4] 

    mov ch, [bp + 6]
    mov cl, [bp + 7]
    shl cl, 6

    mov al, [bp + 10]
    and al, 0x3f
    or cl, al

    mov dh, [bp + 8]

    mov al, [bp + 12]

    mov bx, [bp + 16]
    mov es, bx
    mov bx, [bp + 14]

    mov ah, 02h
    stc
    int 13h

    mov al, ah ; return code

    pop es
    pop bx

    mov sp, bp
    pop bp
    ret
