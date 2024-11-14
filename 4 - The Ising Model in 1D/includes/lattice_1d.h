#ifndef LATTICE_1D_H
#define LATTICE_1D_H

#include <vector>
#include <lattice.h>

class Lattice1D final : public Lattice {
public:
    Lattice1D(const size_t lattice_size, const double j, const double h) : Lattice(j, h), spins(lattice_size, 1) {}

	double flip_fetch_magnetization_diff(size_t i) override;
	[[nodiscard]] constexpr size_t num_sites() const noexcept override;

	[[nodiscard]] double energy() const override;
	[[nodiscard]] double energy_diff(size_t i) const override;

	[[nodiscard]] double magnetization() const override;
	[[nodiscard]] double magnetization_diff(size_t i) const override;

	static double magnetization_diff(short old_spin);

private:
    std::vector<short> spins;
};
#endif //LATTICE_1D_H
