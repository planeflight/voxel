#ifndef VOXEL_UTIL_WORLDGEN_HPP
#define VOXEL_UTIL_WORLDGEN_HPP

#include <vector>

#include "noise/perlin_noise.h"
#include "omega/util/util.hpp"
#include "voxel/entity/block.hpp"

class WorldGen {
  public:
    constexpr static f32 water_height = 30.0f;

    WorldGen(const WorldGen &) = delete;
    WorldGen(WorldGen &&) = delete;
    WorldGen operator=(const WorldGen &) = delete;
    WorldGen operator=(WorldGen &&) = delete;

    static WorldGen *instance() {
        static omega::util::uptr<WorldGen> i =
            omega::util::uptr<WorldGen>(new WorldGen());
        return i.get();
    }

    f32 get_height(f32 x, f32 y) {
        // continental
        f32 c = get_continental(x, y) * 0.5f + 0.5f;
        c = glm::clamp(c, 0.0f, 1.0f);
        f32 c_height = get_height_from_points(cont, c);
        // peaks and valleys
        f32 p = get_peaks_valleys(x, y) * 0.5f + 0.5f;
        p = glm::clamp(p, 0.0f, 1.0f);
        f32 p_height = get_height_from_points(pv, p);
        // erosion
        f32 e = get_erosion(x, y) * 0.5f + 0.5f;
        e = glm::clamp(e, 0.0f, 1.0f);
        f32 e_height = get_height_from_points(ero, e);

        return c_height * 0.33f + e_height * 0.41f + p_height * 0.25f;
    }

    f32 get_height_change(f32 x, f32 y) {
        static constexpr f32 factor = 1.0f / 4.0f;
        return height_change().noise2D(x * factor, y * factor);
    }

    BlockType get_biome(f32 height, f32 t, f32 h) {
        if (t > 0.8f && h > 0.8f) {
            return BlockType::GRASS;
        }
        // hot and dry
        if (t > 0.8f && h < 0.2f) {
            return BlockType::COAL;
        }
        // temperate
        return BlockType::SAND;
    }

  private:
    using Noise = siv::BasicPerlinNoise<f32>;

    static Noise get_noise_function() {
        siv::PerlinNoise::seed_type seed = omega::util::random<u32>(0, 1000000);
        return siv::BasicPerlinNoise<f32>{seed};
    }

    Noise &peaks_valleys() {
        static Noise noise = get_noise_function();
        return noise;
    }

    Noise &continentalness() {
        static Noise noise = get_noise_function();
        return noise;
    }

    Noise &erosion() {
        static Noise noise = get_noise_function();
        return noise;
    }

    Noise &height_change() {
        static Noise noise = get_noise_function();
        return noise;
    }

    f32 get_height_from_points(const std::vector<std::pair<f32, f32>> &values,
                               f32 noise_val) {
        size_t i = 0;
        for (; i < values.size() - 1; ++i) {
            if (values[i].first <= noise_val &&
                noise_val <= values[i + 1].first) {
                break;
            }
        }
        return glm::lerp(values[i].second,
                         values[i + 1].second,
                         (noise_val - values[i].first) /
                             (values[i + 1].first - values[i].first));
    }

    f32 get_continental(f32 x, f32 y) {
        static constexpr f32 factor = 1.0f / 256.0f;
        static constexpr u32 octaves = 4;
        static constexpr f32 amplitude = 0.5f;
        return continentalness().octave2D_11(
            x * factor, y * factor, octaves, amplitude);
    }

    f32 get_peaks_valleys(f32 x, f32 y) {
        static constexpr f32 factor = 1.0f / 64.0f;
        static constexpr u32 octaves = 4;
        static constexpr f32 amplitude = 0.4f;
        return peaks_valleys().octave2D_11(
            x * factor, y * factor, octaves, amplitude);
    }

    f32 get_erosion(f32 x, f32 y) {
        static constexpr f32 factor = 1.0f / 128.0f;
        static constexpr u32 octaves = 2;
        static constexpr f32 amplitude = 0.6f;
        return erosion().octave2D_11(
            x * factor, y * factor, octaves, amplitude);
    }

    WorldGen() {
        // generate continental terrain points
        cont.push_back({0.0f, 1.0f});
        cont.push_back({0.08f, 0.19f});
        cont.push_back({0.33f, 0.19f});
        cont.push_back({0.36f, 0.39f});
        cont.push_back({0.51f, 0.45f});
        cont.push_back({0.56f, 0.75f});
        cont.push_back({0.6f, 0.8f});
        cont.push_back({0.63f, 0.81f});
        cont.push_back({0.86f, 0.84f});
        cont.push_back({1.0f, 0.88f});

        // generate erosion points
        ero.push_back({0.0f, 0.97124f});
        ero.push_back({0.1387f, 0.66807f});
        ero.push_back({0.3246f, 0.45072f});
        ero.push_back({0.3613f, 0.22232f});
        ero.push_back({0.5052f, 0.11594f});
        ero.push_back({0.6937f, 0.19783f});
        ero.push_back({0.72f, 0.44f});
        ero.push_back({0.84f, 0.42f});
        ero.push_back({0.9058f, 0.14430f});
        ero.push_back({0.9215f, 0.09783f});
        ero.push_back({1.0f, 0.03986f});

        // generate peaks and valleys points
        pv.push_back({0.0f, 0.2f});
        pv.push_back({0.16, 0.26f});
        pv.push_back({0.37, 0.43f});
        pv.push_back({0.47, 0.7f});
        pv.push_back({0.56, 0.92f});
        pv.push_back({0.73, 0.84});
        pv.push_back({0.85, 0.7f});
        pv.push_back({0.95, 0.64f});
        pv.push_back({1.0f, 0.64f});
    }

    std::vector<std::pair<f32, f32>> cont; // continentalness (cliffs/plateaus)
    std::vector<std::pair<f32, f32>> ero;  // erosion (flatness)
    std::vector<std::pair<f32, f32>> pv;   // peaks and valleys
};

#endif // VOXEL_UTIL_WORLDGEN_HPP
