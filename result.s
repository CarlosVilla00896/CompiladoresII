.data
string0: .asciiz"a = "
string1: .asciiz"b = "
string2: .asciiz"a+b = "
string3: .asciiz"a-b = "
string4: .asciiz"a*b = "
string5: .asciiz"a/b = "
string6: .asciiz"a>b = "
string7: .asciiz"a<b = "
string8: .asciiz"a>=b = "
string9: .asciiz"a<=b = "
string10: .asciiz"true&&false = "

.globl main
.text
main: 
addiu $sp, $sp, -12

sw $ra, 0($sp)

li $t0, 5

li $t1, 100

mult $t0, $t1
mflo $t0

sw $t0, 4($sp)

li $t0, 2

li $t1, 100

mult $t0, $t1
mflo $t0

sw $t0, 8($sp)


lw $t0, 4($sp)

la $a0, string0
li $v0, 4
syscall
addi $a0, $zero, 0xA
li $v0, 11
move $a0, $t0
li $v0, 1
syscall
addi $a0, $zero, 0xA
li $v0, 11


lw $t0, 8($sp)

la $a0, string1
li $v0, 4
syscall
addi $a0, $zero, 0xA
li $v0, 11
move $a0, $t0
li $v0, 1
syscall
addi $a0, $zero, 0xA
li $v0, 11


lw $t0, 4($sp)

lw $t1, 8($sp)

add $t0, $t0, $t1

la $a0, string2
li $v0, 4
syscall
addi $a0, $zero, 0xA
li $v0, 11
move $a0, $t0
li $v0, 1
syscall
addi $a0, $zero, 0xA
li $v0, 11


lw $t0, 4($sp)

lw $t1, 8($sp)

sub $t0, $t0, $t1

la $a0, string3
li $v0, 4
syscall
addi $a0, $zero, 0xA
li $v0, 11
move $a0, $t0
li $v0, 1
syscall
addi $a0, $zero, 0xA
li $v0, 11


lw $t0, 4($sp)

lw $t1, 8($sp)

mult $t0, $t1
mflo $t0

la $a0, string4
li $v0, 4
syscall
addi $a0, $zero, 0xA
li $v0, 11
move $a0, $t0
li $v0, 1
syscall
addi $a0, $zero, 0xA
li $v0, 11


lw $t0, 4($sp)

lw $t1, 8($sp)

div $t0, $t1
mflo $t0

la $a0, string5
li $v0, 4
syscall
addi $a0, $zero, 0xA
li $v0, 11
move $a0, $t0
li $v0, 1
syscall
addi $a0, $zero, 0xA
li $v0, 11


lw $t0, 4($sp)

lw $t1, 8($sp)

slt $t0, $t1, $t0

la $a0, string6
li $v0, 4
syscall
addi $a0, $zero, 0xA
li $v0, 11
move $a0, $t0
li $v0, 1
syscall
addi $a0, $zero, 0xA
li $v0, 11


lw $t0, 4($sp)

lw $t1, 8($sp)

slt $t0, $t0, $t1

la $a0, string7
li $v0, 4
syscall
addi $a0, $zero, 0xA
li $v0, 11
move $a0, $t0
li $v0, 1
syscall
addi $a0, $zero, 0xA
li $v0, 11


lw $t0, 4($sp)

lw $t1, 8($sp)

sge $t0, $t0, $t1

la $a0, string8
li $v0, 4
syscall
addi $a0, $zero, 0xA
li $v0, 11
move $a0, $t0
li $v0, 1
syscall
addi $a0, $zero, 0xA
li $v0, 11


lw $t0, 4($sp)

lw $t1, 8($sp)

sle $t0, $t0, $t1

la $a0, string9
li $v0, 4
syscall
addi $a0, $zero, 0xA
li $v0, 11
move $a0, $t0
li $v0, 1
syscall
addi $a0, $zero, 0xA
li $v0, 11


addi $t0, $zero, 1

move $t1, $zero

addi $t2, $zero, 0
beq $t0, $t2, finalLabel12
beq $t1, $t2, finalLabel12
label11:
addi $t2, $zero, 1
finalLabel12:

la $a0, string10
li $v0, 4
syscall
addi $a0, $zero, 0xA
li $v0, 11
move $a0, $t2
li $v0, 1
syscall
addi $a0, $zero, 0xA
li $v0, 11


lw $ra, 0($sp)
addiu $sp, $sp, 12
jr $ra

