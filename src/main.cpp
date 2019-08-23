#include <unistd.h>
#include <getopt.h>
#include <iostream>
#include <fstream>
#include <map>
#include <complex>
#include <vector>
#include "Resampler.h"

#define BLOCKSIZE   4096

using namespace std;

struct resample_args {
    string infile;
    string outfile;
    string type = "fc32";
    unsigned p, q;
};

static std::map<string, pair<string, size_t>> sample_type_map {
    { "fc64", { "complex double", sizeof(complex<double>) } },
    { "fc32", {  "complex float", sizeof(complex<float>) } },
    { "sc64", {   "complex long", sizeof(complex<long>) } },
    { "sc32", {    "complex int", sizeof(complex<int>) } },
    { "sc16", {  "complex short", sizeof(complex<short>) } },
    {  "sc8", {   "complex char", sizeof(complex<char>) } },
    {  "f64", {         "double", sizeof(double) } },
    {  "f32", {          "float", sizeof(float) } },
    {  "s64", {           "long", sizeof(long) } },
    {  "s32", {            "int", sizeof(int) } },
    {  "s16", {          "short", sizeof(short) } },
    {   "s8", {           "char", sizeof(char) } },
};

static void print_help()
{
    fprintf(stdout, "Options:\n"
            "  -h, --help         This text\n"
            "  -i, --ifile        Input file\n"
            "  -o, --ofile        Output file\n"
            "  -p, --numerator    Rational rate numerator 'P'\n"
            "  -q, --denominator  Rational rate denominator 'Q'\n"
            "  -t, --sampletype   Sample type (default=fc32)\n"
            );
    fprintf(stdout, "\nSample Types:\n");
    for (auto p:sample_type_map)
        fprintf(stdout, "  %4s - %s\n", p.first.c_str(), p.second.first.c_str());
}

static void print_done(size_t num, size_t bytes, string file, string type)
{
    cout << "Wrote " << num << " '" << sample_type_map[type].first << "' samples ("
         << bytes << " bytes) to file " << file << endl;
}

static void print_version()
{
    fprintf(stdout, "resample version-0.1\n");
}

static bool handle_options(int argc, char **argv, resample_args &args)
{
    int option;
    static struct option long_options[] = {
        { "help", 0, 0, 'h' },
        { "version", 0, 0, 'v' },
        { "ifile", 1, 0, 'i' },
        { "ofile", 1, 0, 'o' },
        { "numerator", 1, 0, 'p' },
        { "denominator", 1, 0, 'q' },
        { "sampletype", 2, 0, 't' },
    };
    while ((option = getopt_long(argc, argv, "hvi:o:p:q:t:", long_options, NULL)) != -1) {
        switch (option) {
        case 'h':
                print_help();
                return false;
        case 'v':
                print_version();
                return false;
        case 'i':
                args.infile = string(optarg);
                break;
        case 'o':
                args.outfile = string(optarg);
                break;
        case 'p':
                args.p = atoi(optarg);
                break;
        case 'q':
                args.q = atoi(optarg);
                break;
        case 't':
                args.type = string(optarg);
                break;
        };
    }

    if (args.infile.empty() || args.outfile.empty() || !args.p || !args.q) {
        print_help();
        return false;
    }
    if (!sample_type_map.count(args.type)) {
        cout << "Unknown sample type " << args.type << endl;
        print_help();
        return false;
    }
    return true;
}

#define RUN_COMPLEX_RESAMPLER(T) \
    try { \
        run_resampler(ComplexResampler<T>(args.p, args.q), \
                      vector<complex<T>>(n_blks*args.q), vector<complex<T>>(n_blks*args.p)); \
    } catch (exception &e) { \
        cout << e.what() << endl; \
    }

#define RUN_REAL_RESAMPLER(T) \
    try { \
        run_resampler(RealResampler<T>(args.p, args.q), \
                      vector<T>(n_blks*args.q), vector<T>(n_blks*args.p)); \
    } catch (exception &e) { \
        cout << e.what() << endl; \
    }

int main(int argc, char **argv)
{
    resample_args args;
    if (!handle_options(argc, argv, args)) return -1;

    ifstream istr(args.infile, std::ifstream::binary);
    if (istr.fail()) {
        cout << "Failed to open input file " << args.infile << endl;
        return -1;
    }

    ofstream ostr;
    ostr.open(args.outfile, ios::out | ios::binary);
    if (ostr.fail()) {
        cout << "Failed to open output file " << args.outfile << endl;
        istr.close();
        return -1;
    }

    int type_sz = sample_type_map[args.type].second;
    int blk_sz = type_sz * args.q;
    int n_blks = blk_sz > BLOCKSIZE ? 1 : BLOCKSIZE / blk_sz;
    size_t n_wr = 0;

    auto run_resampler = [&](auto resampler, auto input, auto output) {
        while (!istr.eof()) {
            istr.read((char *) input.data(), input.size()*type_sz);
            auto n_rd = istr.gcount();
            if (n_rd != n_blks * blk_sz) {
                if (n_rd < blk_sz) break;
                n_blks = n_rd / blk_sz;
                input.resize(n_blks * args.q);
                output.resize(n_blks * args.p);
            }
            resampler.resample(input, output);
            ostr.write((char *) output.data(), output.size() * type_sz);
            n_wr += output.size();
        }
    };

    if      (args.type == "fc64") RUN_COMPLEX_RESAMPLER(double)
    else if (args.type == "fc32") RUN_COMPLEX_RESAMPLER(float)
    else if (args.type == "sc64") RUN_COMPLEX_RESAMPLER(long)
    else if (args.type == "sc32") RUN_COMPLEX_RESAMPLER(int)
    else if (args.type == "sc16") RUN_COMPLEX_RESAMPLER(short)
    else if (args.type ==  "sc8") RUN_COMPLEX_RESAMPLER(char)
    else if (args.type ==  "f64") RUN_REAL_RESAMPLER(double)
    else if (args.type ==  "f32") RUN_REAL_RESAMPLER(float)
    else if (args.type ==  "s64") RUN_REAL_RESAMPLER(long)
    else if (args.type ==  "s32") RUN_REAL_RESAMPLER(int)
    else if (args.type ==  "s16") RUN_REAL_RESAMPLER(short)
    else if (args.type ==   "s8") RUN_REAL_RESAMPLER(char)

    print_done(n_wr, n_wr*type_sz, args.outfile, args.type);

    istr.close();
    ostr.close();
}
