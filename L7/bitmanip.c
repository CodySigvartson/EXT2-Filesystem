// test bit value
int test_bit(char *buf, int bit){
    return buf[bit/8] & (1 << (bit % 8));
}

// set bit value
int set_bit(char *buf, int bit){
    buf[bit/8] |= (1 << (bit % 8));
}

// clear any bit to 0
int clear_bit(char *buf, int bit){
    buf[bit/8] &= ~(1 << (bit%8));
}