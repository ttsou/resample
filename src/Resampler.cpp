/*
 * Polyphase Resampler
 *
 * Copyright (C) 2019 Tom Tsou <tom@tsou.cc>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <algorithm>
#include <complex>
#include <vector>
#include <stdexcept>

#include "Resampler.h"

/*
 * Undefine to remove saturating accumulation on integral types
 */
#define SATURATE

/*
 * Initial precomputed number of path lengths. Runtime recomputed if needed for
 * larger vector sizes
 */
#define DEFAULT_PATH_LEN		128

using namespace std;

Resampler::Resampler(unsigned P, unsigned Q, unsigned taps)
    : partitions(P, vector<double>(taps)), P(P), Q(Q)
{
    init(taps, P > Q ? P : Q);
    resize(DEFAULT_PATH_LEN);
}

/* 
 * Prototype filter design using Blackman-harris window. Taps are normalized
 * with DC filter gain divided by 'P'.
 *
 * https://en.wikipedia.org/wiki/Window_function#Blackman-Harris_window
 */
void Resampler::init(unsigned taps, double cutoff)
{
    vector<double> proto(partitions.size() * taps);
    double a[] = { 0.35875, 0.48829, 0.14128, 0.01168 };
    double beta, i = 0.0, sum = 0.0;

    auto sinc = [](double x) {
        if (x == 0.0) return 1.0;
        return sin(M_PI * x) / (M_PI * x);
    };

    for (auto &p:proto) {
        p = sinc((i - proto.size()/2.0) / cutoff);
        p *= a[0] -
             a[1] * cos(2 * M_PI * i / (proto.size())) +
             a[2] * cos(4 * M_PI * i / (proto.size())) -
             a[3] * cos(6 * M_PI * i / (proto.size()));
        sum += p;
        i++;
    }
    beta = partitions.size() / sum;
    for (unsigned j = 0; j < taps; j++)
        for (unsigned p = 0; p < P; p++)
            partitions[p][j] = proto[j * P + p] * beta;
    for (auto &p:partitions) reverse(p.begin(), p.end());
}

template <typename T>
ComplexResampler<T>::ComplexResampler(unsigned P, unsigned Q, unsigned taps)
    : Resampler(P, Q, taps), history(taps-1)
{

}

template <typename T>
RealResampler<T>::RealResampler(unsigned P, unsigned Q, unsigned taps)
    : Resampler(P, Q, taps), history(taps-1)
{

}

#define COPY_INPUT(T) \
    if (input.size() % Q || output.size() % P || \
        input.size() / Q != output.size() / P || input.size() < history.size()) \
        throw invalid_argument("Invalid vector size(s)"); \
    if (output.size() > paths.size()) resize(output.size()); \
    vector<T> x(input.size() + history.size()); \
    copy(history.begin(), history.end(), x.begin()); \
    copy(input.begin(), input.end(), x.begin()+history.size()); \
    copy(input.end()-history.size(), input.end(), history.begin());

template <typename T>
void ComplexResampler<T>::resample(const vector<complex<T>> &input, vector<complex<T>> &output)
{
    COPY_INPUT(complex<T>)

    auto pi = begin(paths);
    for (auto oi = output.begin(); oi != output.end(); oi++) {
        auto h = partitions[pi->second];
        auto xi = x.begin() + pi->first;
        complex<double> accum(0.0);
        auto xii = xi;
        for (auto hi = h.begin(); hi != h.end(); hi++, xii++)
            accum += complex<double>(*hi * xii->real(), *hi * xii->imag());
#ifdef SATURATE
        if (is_integral<T>::value) {
            double a = accum.real();
            double b = accum.imag();
            a = max((double) numeric_limits<T>::min(), a);
            a = min((double) numeric_limits<T>::max(), a);
            b = max((double) numeric_limits<T>::min(), b);
            b = min((double) numeric_limits<T>::max(), b);
            accum = complex<double>(a, b);
        }
#endif
        *oi = accum;
        pi++;
    }
}

template <typename T>
void RealResampler<T>::resample(const vector<T> &input, vector<T> &output)
{
    COPY_INPUT(T)

    auto pi = begin(paths);
    for (auto oi = output.begin(); oi != output.end(); oi++) {
        auto h = partitions[pi->second];
        auto xi = x.begin() + pi->first;
        double accum = 0.0;
        auto xii = xi;
        for (auto hi = h.begin(); hi != h.end(); hi++)
            accum += *hi * (double) *xii++;
#ifdef SATURATE
        if (is_integral<T>::value) {
            accum = max((double) numeric_limits<T>::min(), accum);
            accum = min((double) numeric_limits<T>::max(), accum);
        }
#endif
        *oi = accum;
        pi++;
    }
}

void Resampler::resize(size_t n)
{
    paths.resize(n);
    unsigned i = 0;
    for (auto &p:paths) {
        p = pair<unsigned, unsigned>((Q * i) / P, (Q * i) % P);
        i++;
    }
}

template class ComplexResampler<double>;
template class ComplexResampler<float>;
template class ComplexResampler<long>;
template class ComplexResampler<short>;
template class ComplexResampler<int>;
template class ComplexResampler<char>;

template class RealResampler<double>;
template class RealResampler<float>;
template class RealResampler<long>;
template class RealResampler<short>;
template class RealResampler<int>;
template class RealResampler<char>;
