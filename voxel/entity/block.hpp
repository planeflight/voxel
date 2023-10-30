#ifndef VOXEL_ENTITY_BLOCK_HPP
#define VOXEL_ENTITY_BLOCK_HPP

#include "omega/util/types.hpp"
#include "omega/math/math.hpp"

enum class BlockType : i8 {
    NONE = 0,
    GRASS = 1,
};

struct Block {
    BlockType type = BlockType::NONE;
    bool is_active() {
        return type != BlockType::NONE;
    }
    u8 x = 0, y = 0, z = 0;
};

#endif // VOXEL_ENTITY_BLOCK_HPP
