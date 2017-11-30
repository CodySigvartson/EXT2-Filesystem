/*
Operator	Description																													Example
&			Binary AND Operator copies a bit to the result if it exists in both operands.												(A & B) = 12, i.e., 0000 1100
|			Binary OR Operator copies a bit if it exists in either operand.																(A | B) = 61, i.e., 0011 1101
^			Binary XOR Operator copies the bit if it is set in one operand but not both.												(A ^ B) = 49, i.e., 0011 0001
~			Binary Ones Complement Operator is unary and has the effect of 'flipping' bits.												(~A ) = -61, i.e,. 1100 0011 in 2's complement form.
<<			Binary Left Shift Operator. The left operands value is moved left by the number of bits specified by the right operand.		A << 2 = 240 i.e., 1111 0000
>>			Binary Right Shift Operator. The left operands value is moved right by the number of bits specified by the right operand.	A >> 2 = 15 i.e., 0000 1111
*/


/////////////////////////////////////////////////////////////////////////
// test_bit() test bit value
/////////////////////////////////////////////////////////////////////////
int test_bit(char *buf, int bit){
    return buf[bit/8] & (1 << (bit % 8)); // bits operator (and) see above. <------------------------------need more comments here.
}

/////////////////////////////////////////////////////////////////////////
// set_bit() set bit value
/////////////////////////////////////////////////////////////////////////
int set_bit(char *buf, int bit){
    buf[bit/8] |= (1 << (bit % 8)); // bits operator (or) see above. <------------------------------need more comments here.
}

/////////////////////////////////////////////////////////////////////////
// clear_bit() clear any bit to 0
/////////////////////////////////////////////////////////////////////////
int clear_bit(char *buf, int bit){
    buf[bit/8] &= ~(1 << (bit % 8)); // bits operator (not) see above. <------------------------------need more comments here.
}