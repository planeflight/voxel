#ifndef VOXEL_ENTITY_BLOCK_HPP
#define VOXEL_ENTITY_BLOCK_HPP

#include "omega/math/math.hpp"
#include "omega/util/types.hpp"

enum class BlockType : i8 {
    NONE = -1,
    GRASS = 0,
    STONE = 1,
    BRICK = 2,
    COAL = 3,
    DIRT = 4,
    ICE = 5,
    SAND = 6,
    SNOW = 7,
    TREE_TRUNK = 8,
    LEAF = 9,
    JUNGLE_GRASS = 10,
    RED_SAND = 11,
};

struct Block {
    BlockType type = BlockType::NONE;
    bool is_active() {
        return type != BlockType::NONE;
    }
    u8 shininess() const {
        if (type == BlockType::ICE) {
            return 32;
        }
        if (type == BlockType::SNOW) {
            return 4;
        }
        if (type == BlockType::STONE) {
            return 128;
        }
        return 0;
    }
    u8 x = 0, y = 0, z = 0;
};

#endif // VOXEL_ENTITY_BLOCK_HPP
