// uint32 test_and_set(uint32 *ptr, uint32 new_value) {  
//    uint32 old_value = *ptr;  
//    *ptr = new_value;  
//    return old_value;  
// }



//   1. Provide an assembly implementation of the atomic test_and_set function. The function should have the 
//   following declaration: 
//   uint32 test_and_set(uint32 *ptr, uint32 new_value); 
//   and it should conceptually implement the following code atomically: 
//   uint32 test_and_set(uint32 *ptr, uint32 new_value) {  
//   uint32 old_value = *ptr;  
//   *ptr = new_value;  
//   return old_value;  
//   } 
//   Your implementation should be based on the XCHG x86 instruction (see Intel Architecture Software 
//   Developer’s Manual, Volume 2) and be written entirely in assembly. Note that the GNU assembler (as) used 
//   in the development-system machine uses the AT&T System V/386 assembler syntax, which is slightly 
//   different from the Intel x86 syntax described in the manual. For example, the ctxsw.S file is written using 
//   the AT&T syntax.  
//   You can find information on the differences between the Intel and AT&T syntax here: 
//   http://www.cs.cmu.edu/afs/cs/academic/class/15213-f01/docs/gas-notes.txt 
//   Name the file containing the test_and_set implementation testandset.S and place it in the system 
//   folder.  Comment each line of this assembly file. 
//   Note: To verify the correctness of your implementation, you can write your own test case files, in which you 
//   invoke several test_and_set operations in sequence and verify that the results are as expected

        .text
        .globl test_and_set

test_and_set:
        pushl %ebp        // Push EBP on stack
        movl  %esp, %ebp  // Reloads EBP

        movl  8(%ebp), %edx  // &ptr adress is in edx
        movl  12(%ebp), %eax // new_value is in eax

        xchgl %eax, (%edx)   // both values are exchanged
        
        popl  %ebp
        ret