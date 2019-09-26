#ifndef _RESAMPLER_H_
#define _RESAMPLER_H_

#include <vector>
#include <complex>

class Resampler {
public:
    Resampler(unsigned P, unsigned Q, unsigned taps);

protected:
    std::vector<std::vector<double>> partitions;
    std::vector<std::pair<int, int>> paths;
    unsigned P, Q;
    void init(unsigned taps, double cutoff);
    void resize(size_t n);
};

template <typename T>
class ComplexResampler : public Resampler {
public:
    ComplexResampler(unsigned P, unsigned Q, unsigned taps = 384);
    void resample(const std::vector<std::complex<T>> &input, std::vector<std::complex<T>> &output);
private:
    std::vector<std::complex<T>> history;
};

template <typename T>
class RealResampler : public Resampler {
public:
    RealResampler(unsigned P, unsigned Q, unsigned taps = 128);
    void resample(const std::vector<T> &input, std::vector<T> &output);
private:
    std::vector<T> history;
};

#endif /* _RESAMPLER_H_ */
