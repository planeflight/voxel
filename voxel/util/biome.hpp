#ifndef VOXEL_UTIL_BIOME_HPP
#define VOXEL_UTIL_BIOME_HPP

#include <functional>
#include <numeric>

#include "omega/math/bezier.hpp"
#include "omega/util/log.hpp"
#include "omega/util/random.hpp"
#include "omega/util/types.hpp"
#include "voxel/entity/block.hpp"

enum class BiomeType : i8 {
    SNOWY_MOUNTAINS = 0,
    FOREST,
    PLAINS,
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

    static f32 param_weight(f32 v, const omega::math::Range<f32> &rng) {
        f32 dist = rng.max - rng.min;
        dist /= 2.0f;
        // return 1.0f - omega::math::abs(v - (rng.min + dist)) / dist;
        // max ensures that if we are out of the biome, then it can't be that
        // biome
        f32 s = 1.0f - (v - (rng.min + dist)) / dist;
        s = omega::math::max(0.0f, s);
        return s * s;
    }

    static f32 param_dist(f32 v, const omega::math::Range<f32> &rng) {
        f32 dist = rng.max - rng.min;
        dist /= 2.0f;
        return omega::math::abs(v - (rng.min + dist)) / dist;
    }

    f32 distance(f32 t, f32 hu, f32 h) const {
        // modify weights depending on preferences
        const f32 w1 = 6.0f, w2 = 4.0f, w3 = 0.0f;
        f32 s =
            param_weight(t, temperature) * w1 + param_weight(hu, humidity) * w2;
        return s / (w1 + w2 + w3); // normalize weight
    }

    BiomeType biome;

    omega::math::Range<f32> temperature;
    omega::math::Range<f32> humidity;
    omega::math::Range<f32> height;
};

struct BiomeManager {
    std::vector<Biome> biomes;
    std::vector<f32> weights;

    struct BiomeInfo {
        f32 height;
        const Biome *biome;
    };

    BiomeInfo compute_biome(f32 t, f32 hu, f32 h) {
        // create a weighted average, so each biome gives a weight
        // the higher the weight, the more the point is like this biome
        f32 min_dist = 100.0f;
        f32 sum = 0.0f;
        weights.clear();

        const Biome *best_biome = nullptr;

        for (const auto &biome : biomes) {
            f32 weight = biome.distance(t, hu, h);
            weights.push_back(weight);
            sum += weight;

            f32 dist = biome.param_dist(t, biome.temperature) * 6.0f +
                       biome.param_dist(hu, biome.humidity) * 4.0f +
                       biome.param_dist(h, biome.height) * 8.0f;
            if (dist < min_dist) {
                min_dist = dist;
                best_biome = &biome;
            }
        }
        // normalize the weights
        f32 blended_height = 0.0f;
        for (u32 i = 0; i < biomes.size(); ++i) {
            f32 &weight = weights[i];
            auto &height = biomes[i].height;
            weight /= sum;
            blended_height += weight * height.average();
        }

        BiomeInfo info;
        info.height = blended_height;
        info.biome = best_biome;
        return info;
    }
};

#endif // VOXEL_UTIL_BIOME_HPP
