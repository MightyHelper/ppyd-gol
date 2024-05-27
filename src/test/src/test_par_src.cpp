#include <gtest/gtest.h>
#include "../../main/include/canvas.h"
#include "../../main/include/vec2.h"

TEST(Vec2Test, BasicAssertions) {
    typedef Vec2<int> Vui;
    Vui size{3, 3};
    Vui dims{1, 1};
    Vui cart_coords{0, 0};
    EXPECT_EQ(Vui(0, 0).to_global(cart_coords, size, dims), Vui(2, 2));
    EXPECT_EQ(Vui(1, 1).to_global(cart_coords, size, dims), Vui(0, 0));
    EXPECT_EQ(Vui(4, 4).to_global(cart_coords, size, dims), Vui(0, 0));
    EXPECT_EQ(Vui(1, 0).to_global(cart_coords, size, dims), Vui(0, 2));
    EXPECT_EQ(Vui(4, 3).to_global(cart_coords, size, dims), Vui(0, 2));
    EXPECT_EQ(Vui(0, 1).to_global(cart_coords, size, dims), Vui(2, 0));

    EXPECT_EQ(Vui(0, 0).to_global_tag(cart_coords, size, dims), 8);
    EXPECT_EQ(Vui(1, 1).to_global_tag(cart_coords, size, dims), 0);
    EXPECT_EQ(Vui(4, 4).to_global_tag(cart_coords, size, dims), 0);
    EXPECT_EQ(Vui(1, 0).to_global_tag(cart_coords, size, dims), 6);
    EXPECT_EQ(Vui(4, 3).to_global_tag(cart_coords, size, dims), 6);
    EXPECT_EQ(Vui(0, 1).to_global_tag(cart_coords, size, dims), 2);
}
