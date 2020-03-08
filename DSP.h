#pragma once
#include <cmath>
#include <algorithm>
#include <complex>
#include <vector>

constexpr double PI = 3.14159265359;

std::vector<std::complex<double>> FFT(std::vector<std::complex<double>> &samples)
{
	int N = samples.size();

	if (N == 1) { return samples; }

	int M = N / 2;

	std::vector<std::complex<double>> even(M, 0);
	std::vector<std::complex<double>> odd(M, 0);

	for (int i = 0; i < M; i++)
	{
		even[i] = samples[2 * i];
		odd[i] = samples[2 * i + 1];
	}

	std::vector<std::complex<double>> Feven(M, 0);
	Feven = FFT(even);

	std::vector<std::complex<double>> Fodd(M, 0);
	Fodd = FFT(odd);

	std::vector<std::complex<double>> freqBins(N, 0);

	for (int k = 0; k < N / 2; k++)
	{
		std::complex<double> cExp = std::polar(1.0, -2 * PI * k / N) * Fodd[k];
		freqBins[k] = Feven[k] + cExp;

		freqBins[k + N / 2] = Feven[k] - cExp;
	}

	return freqBins;
}