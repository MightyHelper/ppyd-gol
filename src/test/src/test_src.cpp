#include <gtest/gtest.h>
#include "../../main/include/utils.h"
#include "../../main/include/canvas.h"

TEST(UtilsTest_clamp, BasicAssertions) {
    EXPECT_EQ(Utils<int>::clamp(5, 2, 7), 5);
    EXPECT_EQ(Utils<int>::clamp(5, 2, 4), 4);
    EXPECT_EQ(Utils<int>::clamp(5, 7, 10), 7);
}

TEST(UtilsTest_coords, BasicAssertions) {
    EXPECT_EQ(Utils<int>::coords(10, 0, 0), 0);
    EXPECT_EQ(Utils<int>::coords(10, 9, 0), 9);
    EXPECT_EQ(Utils<int>::coords(10, 0, 1), 10);
    EXPECT_EQ(Utils<int>::coords(10, 1, 1), 11);
}

TEST(UtilsTest_clamped_coords, BasicAssertions) {
    EXPECT_EQ(Utils<int>::clamped_coords(10, 10, 0, 0), 0);
    EXPECT_EQ(Utils<int>::clamped_coords(10, 10, 10, 0), 9);
    EXPECT_EQ(Utils<int>::clamped_coords(10, 10, 0, 10), 90);
    EXPECT_EQ(Utils<int>::clamped_coords(10, 10, 10, 10), 99);
    EXPECT_EQ(Utils<int>::clamped_coords(10, 10, -10, 10), 90);
    EXPECT_EQ(Utils<int>::clamped_coords(10, 10, -10, -10), 0);
}

TEST(CanvasTest_Cavas, BasicAssertions) {
    EXPECT_NO_THROW(Canvas canvas(10, 10));
}

TEST(CanvasTest_init, BasicAssertions) {
    Canvas canvas(10, 10);
    EXPECT_NO_THROW(canvas.init());
    EXPECT_NE(canvas.buffer, nullptr);
    EXPECT_NE(canvas.cells, nullptr);
    EXPECT_NE(canvas.cells, canvas.buffer);
    for (int i = 0; i < 10 * 10; i++) {
        EXPECT_EQ(canvas.index(canvas.cells, i), 0);
        EXPECT_EQ(canvas.index(canvas.buffer, i), 0);
    }
}

TEST(CanvasTest_rle_decode_line, BasicAssertions) {
    Canvas canvas(10, 10);
    canvas.init();
    canvas.rle_decode_line("3o2b", 0);
#define check(x, v) EXPECT_EQ(canvas.at(x, 0), v)
    check(0, 1);
    check(1, 1);
    check(2, 1);
    check(3, 0);
    check(4, 0);
    check(5, 0);
    canvas.rle_decode_line("3b2o", 0);
    check(0, 0);
    check(1, 0);
    check(2, 0);
    check(3, 1);
    check(4, 1);
    check(5, 0);
    canvas.rle_decode_line("o3b2o", 0);
    check(0, 1);
    check(1, 0);
    check(2, 0);
    check(3, 0);
    check(4, 1);
    check(5, 1);
    check(6, 0);
#undef check
}

TEST(CanvasTest_load_file, BasicAssertions) {
    Canvas canvas(10, 10), canvas2(10, 10);
    canvas.load_file("../data/mini.rle");
    canvas2.init();
    std::string data = ".X...XXX.\n"
                       ".XXX..X..\n"
                       "..X......";
    canvas2.spawn(
            data,
            0, 0
    );
    for (int i = 0; i < 10 * 10; i++) {
        EXPECT_EQ(canvas.index(canvas.cells, i), canvas2.index(canvas.cells, i));
    }
}

TEST(CanvasTest_get_total, BasicAssertions) {
    Canvas canvas(10, 10);
    canvas.init();
    std::string data = ".X...XXX.\n"
                       ".XXX..X..\n"
                       "..X......";
    canvas.spawn(
            data,
            0, 0
    );
    canvas.print();
    EXPECT_EQ(canvas.get_total(0, 0), 2);
    EXPECT_EQ(canvas.get_total(1, 0), 2);
    EXPECT_EQ(canvas.get_total(2, 0), 4);
    EXPECT_EQ(canvas.get_total(3, 0), 2);
    EXPECT_EQ(canvas.get_total(0, 1), 2);
}

TEST(CanvasTest_iter, BasicAssertions) {
    Canvas canvas(10, 10), canvas2(10, 10);
    canvas.load_file("../data/mini.rle");
    canvas2.init();
    std::string data = ".X...XXX.\n"
                       ".XXX..X..\n"
                       "..X......";
    canvas2.spawn(
            data,
            0, 0
    );
    for (int i = 0; i < 10 * 10; i++) {
        EXPECT_EQ(canvas.index(canvas.cells, i), canvas2.index(canvas.cells, i));
    }
    canvas.iter();
    canvas2.init();
    std::string data2 = ".X...XXX.\n"
                        ".X.X.XXX.\n"
                        ".XXX.....";
    canvas2.spawn(
            data2,
            0, 0
    );
    canvas.print();
    canvas2.print();
    for (int i = 0; i < 10 * 10; i++) {
        EXPECT_EQ(canvas.index(canvas.cells, i), canvas2.index(canvas.cells, i));
    }
}
