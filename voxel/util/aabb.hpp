#ifndef VOXEL_UTIL_AABB_HPP
#define VOXEL_UTIL_AABB_HPP

#include "omega/math/math.hpp"
#include "omega/util/types.hpp"

/**
 * AABB is defined by a position (a center)
 * and dimensions (length, width, height)
 * */
template <typename T>
struct AABB {
    using v = omega::math::vec<3, T>;
    v position, dimens;

    AABB(const v &pos) : position(pos), dimens(1.0) {}
    AABB(const v &pos, const v &dimens) : position(pos), dimens(dimens) {}

    static AABB<T> unit() {
        return AABB<T>(v(0.0), v(1.0));
    }

    static AABB<T> from_min(const v &min, const v &dimens) {
        return AABB<T>(min + dimens * (T)0.5, dimens);
    }

    void translate(const v &b) {
        position += b;
    }

    void scale(const v &sc) {
        dimens *= sc;
    }

    v center() const {
        return position;
    }

    v min() const {
        return position - dimens / (T)2.0;
    }

    v max() const {
        return position + dimens / (T)2.0;
    }

    bool intersect(const AABB<T> &b) {
        v min_a = min(), max_a = max(), min_b = b.min(), max_b = b.max();
        bool no_overlap = max_a.x < min_b.x || max_a.y < min_b.y ||
                          max_a.z < min_b.z || max_b.x < min_a.x ||
                          max_b.y < min_a.y || max_b.z < min_a.z;
        return !no_overlap;
    }

    static void resolve_collision(AABB<T> &moveable, const AABB<T> &collider) {
        auto overlap = moveable.overlap(collider);
    }
};

using AABBi = AABB<i32>;
using AABBf = AABB<f32>;

#endif // VOXEL_UTIL_AABB_HPP
