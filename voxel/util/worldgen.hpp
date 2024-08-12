#ifndef VOXEL_UTIL_WORLDGEN_HPP
#define VOXEL_UTIL_WORLDGEN_HPP

#include <vector>

#include "glm/common.hpp"
#include "glm/ext/vector_float3.hpp"
#include "noise/perlin_noise.h"
#include "omega/math/bezier.hpp"
#include "omega/math/glm.hpp"
#include "omega/util/random.hpp"
#include "omega/util/util.hpp"
#include "voxel/entity/block.hpp"
#include "voxel/entity/water.hpp"
#include "voxel/util/biome.hpp"

class WorldGen {
  public:
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

        // combining 3 layers of fbm with spline based
        // mountains/valleys/plateaus
        return (c + e + p + c_height + e_height + p_height) / 6.0f;
    }

    f32 get_height_change(f32 x, f32 y, f32 factor) {
        return height_change().noise2D(x * factor, y * factor);
    }

    void gen(Block *blocks, omega::math::vec3 pos, u32 w, u32 d, u32 h) {
        using omega::math::min, omega::math::map_range, omega::util::random;
        const auto idx = [&w, &d, &h](u32 x, u32 y, u32 z) {
            return (z * w * h) + (y * w) + x;
        };
        const auto add_leaf = [&](int x, int y, int z) {
            if (x < 0 || x > (int)w - 1) return;
            if (y < 0 || y > (int)h - 1) return;
            if (z < 0 || z > (int)d - 1) return;
            blocks[idx(x, y, z)].type = BlockType::LEAF;
        };

        const auto add_tree = [&](u32 x, u32 y, u32 z) {
            if (omega::util::random<i32>(0, 200) == 10 && y > Water::height) {
                u32 h = y;
                for (; y < h + 5; ++y) {
                    blocks[idx(x, y, z)].type = BlockType::TREE_TRUNK;
                }
                // add leaves
                // x axis
                add_leaf((int)x - 1, (int)h + 4, (int)z);
                add_leaf((int)x - 2, (int)h + 4, (int)z);
                add_leaf((int)x + 1, (int)h + 4, (int)z);
                add_leaf((int)x + 2, (int)h + 4, (int)z);

                add_leaf((int)x + 1, (int)h + 3, (int)z);
                add_leaf((int)x - 1, (int)h + 3, (int)z);
                // z axis
                add_leaf((int)x, (int)h + 4, (int)z - 1);
                add_leaf((int)x, (int)h + 4, (int)z - 2);
                add_leaf((int)x, (int)h + 4, (int)z + 1);
                add_leaf((int)x, (int)h + 4, (int)z + 2);

                add_leaf((int)x, (int)h + 3, (int)z + 1);
                add_leaf((int)x, (int)h + 3, (int)z - 1);
                // diagonal
                add_leaf((int)x + 1, (int)h + 4, (int)z + 1);
                add_leaf((int)x - 1, (int)h + 4, (int)z + 1);
                add_leaf((int)x + 1, (int)h + 4, (int)z - 1);
                add_leaf((int)x - 1, (int)h + 4, (int)z - 1);
                // y axis
                add_leaf((int)x, (int)h + 5, (int)z);
            }
        };
        for (u32 z = 0; z < d; ++z) {
            for (u32 x = 0; x < w; ++x) {
                f32 x_w = pos.x * (f32)w + x;
                f32 z_w = pos.z * (f32)d + z;
                // set base layer
                blocks[idx(x, 0, z)].type = BlockType::STONE;

                // sample height
                f32 f = 1.4f;
                f32 base_height = get_height(x_w * f, z_w * f);
                // generate biome type
                f = 0.005f; // temperature & humidity frequency modifier
                f32 temp =
                    temperature().octave2D(x_w * f, z_w * f, 4, 0.5f) * 0.5f +
                    0.5f;
                f32 humid =
                    humidity().octave2D(x_w * f, z_w * f, 4, 0.7f) * 0.5f +
                    0.5f;
                // modify temperature depending on height
                f32 sign = omega::math::sign(base_height - 0.5f);
                temp -=
                    0.6f * (base_height - 0.5f) * (base_height - 0.5f) * sign;
                temp = omega::math::clamp(temp, 0.0f, 1.0f);

                // get biome type and weighted height
                BiomeManager::BiomeInfo info =
                    bm.compute_biome(temp, humid, base_height);

                // calculate final height
                f32 height;
                height = omega::math::map_range(
                    0.0f,
                    1.0f,
                    0.0f,
                    190.0f,
                    base_height * 0.5f + info.height * 0.5f);
                height = omega::math::clamp(height, 0.0f, 255.0f);

                // place sand blocks
                f32 sand_height = Water::height + 4.0f +
                                  3.0f * get_height_change(x_w, z_w, 0.05f);
                u32 y = 1;
                for (; y < min(sand_height, height); ++y) {
                    blocks[idx(x, y, z)].type = BlockType::SAND;
                }
                switch (info.biome->biome) {
                    case BiomeType::SNOWY_MOUNTAINS: {
                        f32 stone_height =
                            height - 5.0f -
                            3.0f * get_height_change(x_w, z_w, 0.05f);
                        for (; y < stone_height; ++y) {
                            blocks[idx(x, y, z)].type = BlockType::STONE;
                        }
                        f32 snow_height =
                            stone_height + 2.0f +
                            4.0f * get_height_change(x_w, z_w, -5.0f);
                        for (; y < min(height, snow_height); ++y) {
                            blocks[idx(x, y, z)].type = BlockType::ICE;
                        }
                        for (; y < height; ++y) {
                            blocks[idx(x, y, z)].type = BlockType::SNOW;
                        }
                        break;
                    }
                    case BiomeType::STONY_MOUNTAINS: {
                        f32 stone_cutoff =
                            height - get_height_change(x_w, z_w, 0.2f);
                        f32 grass_cutoff =
                            70.0f + 5.0f * get_height_change(x_w, z_w, 0.1f);
                        if (height < grass_cutoff) {
                            for (; y < min(grass_cutoff, height); ++y) {
                                blocks[idx(x, y, z)].type = BlockType::GRASS;
                            }
                        } else {
                            for (; y < height; ++y) {
                                blocks[idx(x, y, z)].type = BlockType::STONE;
                            }
                        }
                        break;
                    }
                    case BiomeType::DESERT: {
                        for (; y < height; ++y) {
                            blocks[idx(x, y, z)].type = BlockType::SAND;
                        }
                        break;
                    }
                    case BiomeType::PLAINS: {
                        f32 dirt_height = height - 2.0f;
                        for (; y < dirt_height; ++y) {
                            blocks[idx(x, y, z)].type = BlockType::DIRT;
                        }
                        for (; y < height; ++y) {
                            blocks[idx(x, y, z)].type = BlockType::GRASS;
                        }
                        add_tree(x, y, z);
                        break;
                    }
                    case BiomeType::FOREST: {
                        f32 dirt_height =
                            height - 1.0f +
                            2.0f * get_height_change(x_w, z_w, 0.08f);
                        for (; y < min(dirt_height, height); ++y) {
                            blocks[idx(x, y, z)].type = BlockType::DIRT;
                        }
                        for (; y < height; ++y) {
                            blocks[idx(x, y, z)].type = BlockType::GRASS;
                        }
                        add_tree(x, y, z);
                        break;
                    }
                    case BiomeType::JUNGLE: {
                        f32 dirt_height =
                            height - 2.0f +
                            2.0f * get_height_change(x_w, z_w, 0.08f);
                        for (; y < min(dirt_height, height); ++y) {
                            blocks[idx(x, y, z)].type = BlockType::DIRT;
                        }
                        for (; y < height; ++y) {
                            blocks[idx(x, y, z)].type = BlockType::JUNGLE_GRASS;
                        }
                        break;
                    }
                    case BiomeType::TUNDRA: {
                        f32 stone_height =
                            height - 2.0f +
                            4.0f * get_height_change(
                                       x_w * -100.0f, z_w * -100.0f, 0.05f);
                        BlockType block =
                            get_height_change(x_w, z_w, 0.5f) > -0.2f
                                ? BlockType::STONE
                                : BlockType::DIRT;
                        for (; y < min(height, stone_height); ++y) {
                            blocks[idx(x, y, z)].type = block;
                        }
                        for (; y < height; ++y) {
                            blocks[idx(x, y, z)].type = BlockType::SNOW;
                        }
                        break;
                    }
                    case BiomeType::BADLANDS: {
                        f32 dirt_height =
                            y + 10.0f +
                            6.0f * get_height_change(x_w, z_w, 0.1f);
                        for (; y < min(height, dirt_height); ++y) {
                            blocks[idx(x, y, z)].type = BlockType::RED_SAND;
                        }
                        f32 sand_height = y + 2.0f;
                        for (; y < min(height, sand_height); ++y) {
                            blocks[idx(x, y, z)].type = BlockType::SAND;
                        }
                        dirt_height = y + 2.0f;
                        for (; y < min(height, sand_height); ++y) {
                            blocks[idx(x, y, z)].type = BlockType::DIRT;
                        }
                        for (; y < height; ++y) {
                            blocks[idx(x, y, z)].type = BlockType::RED_SAND;
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
        }
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

    Noise &temperature() {
        static Noise noise = get_noise_function();
        return noise;
    }

    Noise &humidity() {
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
        static constexpr f32 factor = 1.0f / 64.0f;
        static constexpr u32 octaves = 4;
        static constexpr f32 amplitude = 0.4f;
        return continentalness().octave2D_11(
            x * factor, y * factor, octaves, amplitude);
    }

    f32 get_peaks_valleys(f32 x, f32 y) {
        static constexpr f32 factor = 1.0f / 256.0f;
        static constexpr u32 octaves = 6;
        static constexpr f32 amplitude = 0.4f;
        return peaks_valleys().octave2D_11(
            x * factor, y * factor, octaves, amplitude);
    }

    f32 get_erosion(f32 x, f32 y) {
        static constexpr f32 factor = 1.0f / 512.0f;
        static constexpr u32 octaves = 4;
        f32 amplitude = 0.8f;
        // extra erosion layer
        if (erosion().noise2D(x * factor, y * factor) < 0.0f) {
            amplitude /= 1.5f;
        }
        return erosion().octave2D_11(
            x * factor, y * factor, octaves, amplitude);
    }

    WorldGen() {
        // generate continental terrain points
        cont.push_back({0.0f, 1.0f});
        cont.push_back({0.08f, 0.55f});
        cont.push_back({0.33f, 0.21f});
        cont.push_back({0.36f, 0.24f});
        cont.push_back({0.51f, 0.45f});
        cont.push_back({0.56f, 0.65f});
        cont.push_back({0.6f, 0.8f});
        cont.push_back({0.63f, 0.91f});
        cont.push_back({0.72f, 1.0f});
        cont.push_back({0.88f, 0.58f});
        cont.push_back({1.0f, 0.26f});

        // generate erosion points
        ero.push_back({0.0f, 0.567124f});
        ero.push_back({0.1387f, 0.56807f});
        ero.push_back({0.3246f, 0.45072f});
        ero.push_back({0.6937f, 0.48783f});
        ero.push_back({0.72f, 0.84f});
        ero.push_back({0.84f, 0.82f});
        ero.push_back({0.86f, 0.034f});
        ero.push_back({0.9215f, 0.033f});
        ero.push_back({1.0f, 0.019f});

        // generate peaks and valleys points
        pv.push_back({0.0f, 0.0f});
        pv.push_back({0.16, 0.16f});
        pv.push_back({0.37, 0.43f});
        pv.push_back({0.47, 0.7f});
        pv.push_back({0.56, 0.98f});
        pv.push_back({0.73, 0.88});
        pv.push_back({0.85, 0.7f});
        pv.push_back({0.95, 0.64f});
        pv.push_back({1.0f, 0.44f});

        // generate the biomes
        using rng = omega::math::Range<f32>;
        bm.biomes.push_back(Biome{rng{0.0f, 0.3f},
                                  rng{0.0f, 0.3f},
                                  rng{0.7f, 1.0f},
                                  BiomeType::SNOWY_MOUNTAINS});
        bm.biomes.push_back(Biome{rng{0.4f, 0.8f},
                                  rng{0.3f, 0.7f},
                                  rng{0.3f, 0.5f},
                                  BiomeType::FOREST});
        bm.biomes.push_back(Biome{rng{0.0f, 0.4f},
                                  rng{0.3f, 0.5f},
                                  rng{0.3f, 0.6f},
                                  BiomeType::PLAINS});
        bm.biomes.push_back(Biome{rng{0.5f, 0.7f},
                                  rng{0.0f, 0.3f},
                                  rng{0.6f, 1.0f},
                                  BiomeType::STONY_MOUNTAINS});
        bm.biomes.push_back(Biome{rng{0.7f, 1.0f},
                                  rng{0.0f, 0.3f},
                                  rng{0.0f, 0.4f},
                                  BiomeType::DESERT});
        bm.biomes.push_back(Biome{rng{0.7f, 1.0f},
                                  rng{0.7f, 1.0f},
                                  rng{0.1f, 0.4f},
                                  BiomeType::JUNGLE});
        bm.biomes.push_back(Biome{rng{0.3f, 0.5f},
                                  rng{0.0f, 0.3f},
                                  rng{0.5f, 0.8f},
                                  BiomeType::TUNDRA});
        bm.biomes.push_back(Biome{rng{0.8f, 1.0f},
                                  rng{0.3f, 0.7f},
                                  rng{0.2f, 0.5f},
                                  BiomeType::BADLANDS});
    }

    std::vector<std::pair<f32, f32>> cont; // continentalness (cliffs/plateaus)
    std::vector<std::pair<f32, f32>> ero;  // erosion (flatness)
    std::vector<std::pair<f32, f32>> pv;   // peaks and valleys
    BiomeManager bm;
};

#endif // VOXEL_UTIL_WORLDGEN_HPP
