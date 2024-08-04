#ifndef VOXEL_UTIL_BIOME_HPP
#define VOXEL_UTIL_BIOME_HPP

#include <functional>

#include "omega/math/bezier.hpp"
#include "omega/util/random.hpp"
#include "omega/util/types.hpp"
#include "voxel/entity/block.hpp"

enum class BiomeType : i8 {
    SNOWY_MOUNTAINS = 0,
    FOREST,
    PLAINS,
    // BEACH,
    // WATER,
    // CAVE,
    STONY_MOUNTAINS,
    DESERT,
    JUNGLE,
    TUNDRA,
    BADLANDS
};

struct Biome {
    Biome(const omega::math::Range<f32> &temp,
          const omega::math::Range<f32> &humidity,
          const omega::math::Range<f32> &h,
          BiomeType type)
        : temperature(temp), humidity(humidity), height(h), biome(type) {}

    bool is_biome(f32 t, f32 hu, f32 h) const {
        return temperature.contains(t) && humidity.contains(hu) &&
               height.contains(h);
    }

    BiomeType biome;

    omega::math::Range<f32> temperature;
    omega::math::Range<f32> humidity;
    omega::math::Range<f32> height;
};

struct BiomeManager {
    std::vector<Biome> biomes;

    const Biome *get_biome(f32 t, f32 hu, f32 h) {
        const auto dev = [](f32 v, const omega::math::Range<f32> &rng) -> f32 {
            f32 dist = rng.max - rng.min;
            dist /= 2.0f;
            return omega::math::abs(v - (rng.min + dist)) / dist;
        };
        f32 min_dist = 100.0f;
        const Biome *best_biome = nullptr;
        for (const auto &biome : biomes) {
            f32 dist = dev(t, biome.temperature) * 6.0f +
                       dev(hu, biome.humidity) * 4.0f +
                       dev(h, biome.height) * 7.0f;
            if (dist < min_dist) {
                min_dist = dist;
                best_biome = &biome;
            }
            // if (biome.is_biome(t, hu, h)) {
            // return &biome;
            // }
        }
        return best_biome;
    }
};

#endif // VOXEL_UTIL_BIOME_HPP
