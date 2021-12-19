.data
string0: .asciiz"El valor de array[0] es: "

.globl main
.text
main: 
addiu $sp, $sp, -24

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

la $a0, string0
li $v0, 4
syscall
move $a0, $t0
li $v0, 1
syscall 


lw $ra, 0($sp)
addiu $sp, $sp, 24
jr $ra

