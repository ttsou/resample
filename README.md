Resample - Rational Rate Sample Rate Converter
==============================================

Convert the sample rate of a raw binary signal file. A variety of complex and real sample types are supported. The rate conversion method uses a polyphase filterbank with Blackman-Harris window function.

https://en.wikipedia.org/wiki/Sample-rate_conversion \
https://en.wikipedia.org/wiki/Polyphase_quadrature_filter \
https://en.wikipedia.org/wiki/Window_function#Blackman–Harris_window

Dependencies
============

A C++14 supported compiler is required.

Build
=====
```
$ autoreconf -i
$ ./configure
$ make
$ make install
```

Run
===
```
$ ./resample -h
Options:
  -h, --help         This text
  -i, --ifile        Input file
  -o, --ofile        Output file
  -p, --numerator    Rational rate numerator 'P'
  -q, --denominator  Rational rate denominator 'Q'
  -t, --sampletype   Sample type (default=fc32)

Sample Types:
   f32 - float
   f64 - double
  fc32 - complex float
  fc64 - complex double
   s16 - short
   s32 - int
   s64 - long
    s8 - char
  sc16 - complex short
  sc32 - complex int
  sc64 - complex long
   sc8 - complex char
```
