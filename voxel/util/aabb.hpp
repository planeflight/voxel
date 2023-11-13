#ifndef VOXEL_UTIL_AABB_HPP
#define VOXEL_UTIL_AABB_HPP

#include "omega/util/types.hpp"

/**
 * AABB is defined by a position (a corner)
 * and dimensions (length, width, height)
 * */
template <typename T>
struct AABB {
    T position, dimens;

    AABB(const T& pos) : position(pos), dimens(1.0) {}
    AABB(const T& pos, const T& dimens) : position(pos), dimens(dimens) {}

    static AABB<T> unit() {
        return AABB<T>(0.0);
    }

    void translate(const AABB<T> b) {
        position += b.position;
    }

    void scale(const T& sc) {
        dimens *= sc;
    }

    T center() const {
        return position + dimens / 2.0;
    }
};

using AABBf = AABB<f32>;
using AABBi = AABB<i32>;

#endif // VOXEL_UTIL_AABB_HPP
