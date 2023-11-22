#ifndef VOXEL_ENTITY_BLOCK_HPP
#define VOXEL_ENTITY_BLOCK_HPP

#include "omega/util/types.hpp"
#include "omega/math/math.hpp"

enum class BlockType : i8 {
    NONE = -1,
    GRASS = 0,
    STONE = 1,
    BRICK = 2,
    COAL = 3,
    DIRT = 4,
    WATER = 5,
    SAND = 6,
    SNOW = 7,
    TREE_TRUNK = 8,
    LEAF = 9,
    GLASS = 10,
};

struct Block {
    BlockType type = BlockType::NONE;
    bool is_active() {
        return type != BlockType::NONE;
    }
    u8 x = 0, y = 0, z = 0;
};

#endif // VOXEL_ENTITY_BLOCK_HPP
