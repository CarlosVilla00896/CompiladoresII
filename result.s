.data
string0: .asciiz"El valor de array[0] es: "

.globl main
.text
main: 
addiu $sp, $sp, -28

sw $ra, 0($sp)


li $t0, 2

li $t1, 0

li $a0, 4
mult $a0, $t1
mflo $t1
la $t2, 4($sp)
add $t1, $t1, $t2
li $t3, 2

sw $t3, 0($t1)

li $t0, 5

li $t1, 1

li $a0, 4
mult $a0, $t1
mflo $t1
la $t2, 4($sp)
add $t1, $t1, $t2
li $t4, 5

sw $t4, 0($t1)

li $t0, 0

li $a0, 4
mult $a0, $t0
mflo $t0
la $t1, 4($sp)
add $t0, $t1, $t0
lw $t0, 0($t0)

li $t1, 1

li $a0, 4
mult $a0, $t1
mflo $t1
la $t2, 4($sp)
add $t1, $t2, $t1
lw $t1, 0($t1)

add $t0, $t0, $t1

li $t1, 2

li $a0, 4
mult $a0, $t1
mflo $t1
la $t2, 4($sp)
add $t1, $t1, $t2
li $t5, 0

li $a0, 4
mult $a0, $t5
mflo $t5
la $t6, 4($sp)
add $t5, $t6, $t5
lw $t5, 0($t5)

li $t6, 1

li $a0, 4
mult $a0, $t6
mflo $t6
la $t7, 4($sp)
add $t6, $t7, $t6
lw $t6, 0($t6)

add $t5, $t5, $t6

sw $t5, 0($t1)

li.s $f0, 4.5

li $t0, 2

li $a0, 4
mult $a0, $t0
mflo $t0
la $t1, 4($sp)
add $t0, $t1, $t0
lw $t0, 0($t0)
mtc1 $t0, $f1
cvt.s.w $f1, $f1

add.s $f0, $f0, $f1

s.s $f0, 24($sp)


l.s $f0, 24($sp)

la $a0, string0
li $v0, 4
syscall
mov.s $f12, $f0
li $v0, 2
syscall


lw $ra, 0($sp)
addiu $sp, $sp, 28
jr $ra

