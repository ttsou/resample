#include <unistd.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <complex>
#include <vector>
#include <climits>
#include <algorithm>

#include "Resampler.h"

using namespace std;

static const double rate = 1e6;
static const double ampl = 0.99;
static const size_t test_sz = 8192;
static const double pass_limit = 0.005;
static const size_t ntaps = 128;

struct test_case {
    int num;
    double freq;
    string type;
    int p, q;
    double rmse;
    bool pass;
};

static vector<double> freqs { 2e3, 5e3, 7e3 };
static vector<string> types { "fc64", "fc32", "sc64", "sc32", "sc16", "sc8", "f64", "f32", "s64", "s32", "s16", "s8" };
static vector<int> pq { 1, 2, 3, 4, 5, 6, 7 };

static void print_test_result(test_case &test)
{
    cout << "Test Case " << test.num << endl;
    cout << "==============" << endl;
    cout << "  Tone Frequency:    " << test.freq << endl;
    cout << "  Sample type:       " << test.type << endl;
    cout << "  Ratio:             " << test.p << "/" << test.q << endl;
    cout << "  Error (RMSE):      " << test.rmse << endl;
    cout << "  Result:            " << (test.pass ? "Pass" : "Fail") << endl;
    cout << endl;
}

#define COMPLEX_TEST(T, SCALE) \
{ \
    vector<complex<T>> input(test_sz/test.q * test.q); \
    vector<complex<T>> output(input.size() * test.p / test.q); \
    vector<complex<T>> target(output.size()); \
    for (unsigned i = 0; i < input.size(); i++) \
        input[i] = complex<double>(sin(2.0*M_PI*i*test.freq/rate) * (double) SCALE * ampl, \
                                   cos(2.0*M_PI*i*test.freq/rate) * (double) SCALE * ampl); \
    double nrate = rate * test.p / test.q; \
    for (unsigned i = 0; i < target.size(); i++) \
        target[i] = complex<double>(sin(2.0*M_PI*i*test.freq/nrate) * (double) SCALE * ampl, \
                                    cos(2.0*M_PI*i*test.freq/nrate) * (double) SCALE * ampl); \
    ComplexResampler<T> resampler(test.p, test.q, ntaps); \
    resampler.resample(input, output); \
    test.rmse = complex_rmse(target, output, ntaps*test.p/test.q/2)/SCALE; \
    test.pass = test.rmse < pass_limit; \
    print_test_result(test); \
}

#define REAL_TEST(T, SCALE) \
{ \
    vector<T> input(test_sz/test.q * test.q); \
    vector<T> output(input.size() * test.p / test.q); \
    vector<T> target(output.size()); \
    for (unsigned i = 0; i < input.size(); i++) \
        input[i] = sin(2.0*M_PI*i*test.freq/rate) * (double) SCALE * ampl; \
    double nrate = rate * test.p / test.q; \
    for (unsigned i = 0; i < target.size(); i++) \
        target[i] = sin(2.0*M_PI*i*test.freq/nrate) * (double) SCALE * ampl; \
    RealResampler<T> resampler(test.p, test.q, ntaps); \
    resampler.resample(input, output); \
    test.rmse = real_rmse(target, output, ntaps*test.p/test.q/2) / SCALE; \
    test.pass = test.rmse < pass_limit; \
    print_test_result(test); \
}

static void run_test(test_case &test) 
{
    auto complex_rmse = [](auto a, auto b, int offset) {
        double error = 0.0;
        for (auto ai = a.begin(), bi = b.begin()+offset; bi != b.end(); ai++, bi++) {
            double c = ai->real() - bi->real();
            double d = ai->imag() - bi->imag();
            error += c*c + d*d;
        }
        return sqrt(error) / distance(b.begin()+offset, b.end());
    };

    auto real_rmse = [](auto a, auto b, int offset) {
        double error = 0.0;
        for (auto ai = a.begin(), bi = b.begin()+offset; bi != b.end(); ai++, bi++) {
            double c = *ai - *bi;
            error += c*c;
        }
        return sqrt(error) / distance(b.begin()+offset, b.end());
    };

    if      (test.type == "fc64") COMPLEX_TEST(double, 1.0)
    else if (test.type == "fc32") COMPLEX_TEST(float, 1.0)
    else if (test.type == "sc64") COMPLEX_TEST(long, numeric_limits<long>::max())
    else if (test.type == "sc32") COMPLEX_TEST(int, numeric_limits<int>::max())
    else if (test.type == "sc16") COMPLEX_TEST(short, numeric_limits<short>::max())
    else if (test.type ==  "sc8") COMPLEX_TEST(char, numeric_limits<char>::max())
    else if (test.type ==  "f64") REAL_TEST(double, 1.0)
    else if (test.type ==  "f32") REAL_TEST(float, 1.0)
    else if (test.type ==  "s64") REAL_TEST(long, numeric_limits<long>::max())
    else if (test.type ==  "s32") REAL_TEST(int, numeric_limits<int>::max())
    else if (test.type ==  "s16") REAL_TEST(short, numeric_limits<short>::max())
    else if (test.type ==   "s8") REAL_TEST(char, numeric_limits<char>::max())
}

static void print_final_results(int count, int pass)
{
    cout << "Completed " << count << " tests: " << pass << " passed and " << count-pass << " failed" << endl;
}

int main(int argc, char **argv)
{
    vector<test_case> tests;
    int num = 0;

    for (auto freq:freqs)
        for (auto type:types)
            for (auto p:pq)
                for (auto q:pq)
                    tests.push_back({
                        .num = num++,
                        .freq = freq,
                        .type = type,
                        .p = p,
                        .q = q,
                        .rmse = numeric_limits<double>::max(),
                        .pass = false,
                    });
    int pass = 0;
    for (auto &test:tests) {
        run_test(test);
        pass += test.pass;
    }
    print_final_results(num, pass);
}
