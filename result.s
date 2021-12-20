.data
string0: .asciiz"Array before: "
string1: .asciiz"\nArray after: "

.globl main
.text
BubbleSort: 
addiu $sp, $sp, -12

sw $ra, 0($sp)

sw $a0, 4($sp)
sw $a1, 8($sp)
li $t0, 3

li $t1, 0

li $a0, 4
mult $a0, $t1
mflo $t1
lw $t2, 4($sp)
add $t1, $t1, $t2
li $t3, 3

sw $t3, 0($t1)

li $t0, 8

li $t1, 1

li $a0, 4
mult $a0, $t1
mflo $t1
lw $t2, 4($sp)
add $t1, $t1, $t2
li $t4, 8

sw $t4, 0($t1)

li $t0, 11

li $t1, 2

li $a0, 4
mult $a0, $t1
mflo $t1
lw $t2, 4($sp)
add $t1, $t1, $t2
li $t5, 11

sw $t5, 0($t1)

li $t0, 14

li $t1, 3

li $a0, 4
mult $a0, $t1
mflo $t1
lw $t2, 4($sp)
add $t1, $t1, $t2
li $t6, 14

sw $t6, 0($t1)

li $t0, 17

li $t1, 4

li $a0, 4
mult $a0, $t1
mflo $t1
lw $t2, 4($sp)
add $t1, $t1, $t2
li $t7, 17

sw $t7, 0($t1)

li $t0, 18

li $t1, 5

li $a0, 4
mult $a0, $t1
mflo $t1
lw $t2, 4($sp)
add $t1, $t1, $t2
li $t8, 18

sw $t8, 0($t1)

li $t0, 43

li $t1, 6

li $a0, 4
mult $a0, $t1
mflo $t1
lw $t2, 4($sp)
add $t1, $t1, $t2
li $t9, 43

sw $t9, 0($t1)

lw $t0, 4($sp)

move $v0, $t0


lw $ra, 0($sp)
addiu $sp, $sp, 12
jr $ra
main: 
addiu $sp, $sp, -64

sw $ra, 0($sp)

li $t0, 7

sw $t0, 4($sp)

li $t0, 11

sw $t0, 8($sp)
li $t0, 14

sw $t0, 12($sp)
li $t0, 3

sw $t0, 16($sp)
li $t0, 8

sw $t0, 20($sp)
li $t0, 18

sw $t0, 24($sp)
li $t0, 17

sw $t0, 28($sp)
li $t0, 43

sw $t0, 32($sp)


la $t0, 8($sp)

la $a0, string0
li $v0, 4
syscall
lw $t9, 0($t0)
move $a0, $t9
li $v0, 1
syscall
lw $t9, 4($t0)
move $a0, $t9
li $v0, 1
syscall
lw $t9, 8($t0)
move $a0, $t9
li $v0, 1
syscall
lw $t9, 12($t0)
move $a0, $t9
li $v0, 1
syscall
lw $t9, 16($t0)
move $a0, $t9
li $v0, 1
syscall
lw $t9, 20($t0)
move $a0, $t9
li $v0, 1
syscall
lw $t9, 24($t0)
move $a0, $t9
li $v0, 1
syscall

la $t0, 8($sp)

lw $t1, 4($sp)

move $a0, $t0
move $a1, $t1
jal BubbleSort
move $t0, $v0

la $t0, 8($sp)

la $a0, string1
li $v0, 4
syscall
lw $t9, 0($t0)
move $a0, $t9
li $v0, 1
syscall
lw $t9, 4($t0)
move $a0, $t9
li $v0, 1
syscall
lw $t9, 8($t0)
move $a0, $t9
li $v0, 1
syscall
lw $t9, 12($t0)
move $a0, $t9
li $v0, 1
syscall
lw $t9, 16($t0)
move $a0, $t9
li $v0, 1
syscall
lw $t9, 20($t0)
move $a0, $t9
li $v0, 1
syscall
lw $t9, 24($t0)
move $a0, $t9
li $v0, 1
syscall


lw $ra, 0($sp)
addiu $sp, $sp, 64
jr $ra

