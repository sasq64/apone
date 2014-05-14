/*
    CMP_fft.h version 0.80 by Pieter Suurmond, may 29, 2003.
*/
extern "C" {
short fft(short* input, short* output, short input_size);
}
/*
    Last arguments determines the size of the FFT.
    Currently 3 Lebedinsky-tables are used: for 1024 point FFT, 
    for 2048 point, and for 4096 point.

    Size of the output is always one more than half of input size.
    1024 inputs give 513 outputs.
    Returns 0 on success.
    Returns 1 if an inappropriate input size was requested.
*/
