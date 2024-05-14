#include <gtest/gtest.h>
#include "utils.hxx"
#include "canvas.hxx"

TEST(UtilsTest_clamp, BasicAssertions) {
	EXPECT_EQ(clamp(5, 2, 7), 5);
	EXPECT_EQ(clamp(5, 2, 4), 4);
	EXPECT_EQ(clamp(5, 7, 10), 7);
}

TEST(UtilsTest_coords, BasicAssertions) {
	EXPECT_EQ(coords<10>(0, 0), 0);
	EXPECT_EQ(coords<10>(9, 0), 9);
	EXPECT_EQ(coords<10>(0, 1), 10);
	EXPECT_EQ(coords<10>(1, 1), 11);
}

TEST(UtilsTest_clamped_coords, BasicAssertions) {
	auto cc1010 = clamped_coords<10, 10>;
	EXPECT_EQ(cc1010(0, 0), 0);
	EXPECT_EQ(cc1010(10, 0), 9);
	EXPECT_EQ(cc1010(0, 10), 90);
	EXPECT_EQ(cc1010(10, 10), 99);
	EXPECT_EQ(cc1010(-10, 10), 90);
	EXPECT_EQ(cc1010(-10, -10), 0);
}

TEST(CanvasTest_Cavas, BasicAssertions) {
	typedef Canvas<10, 10> C1010;
	EXPECT_NO_THROW(C1010 canvas{});
	C1010 canvas{};
	EXPECT_EQ(canvas.buffer, nullptr);
	EXPECT_EQ(canvas.cells, nullptr);
}

TEST(CanvasTest_init, BasicAssertions) {
	Canvas<10, 10> canvas{};
	EXPECT_NO_THROW(canvas.init());
	EXPECT_NE(canvas.buffer, nullptr);
	EXPECT_NE(canvas.cells, nullptr);
	EXPECT_NE(canvas.cells, canvas.buffer);
	for (int i=0; i<10*10; i++){
		EXPECT_EQ((*canvas.cells)[i], 0);
		EXPECT_EQ((*canvas.buffer)[i], 0);
	}
}

