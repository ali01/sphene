#include "gtest/gtest.h"

#include "sw_data_plane.h"


TEST(SWDataPlaneTest, Construct) {
  SWDataPlane::Ptr swdp = SWDataPlane::SWDataPlaneNew();

  ASSERT_TRUE(swdp.ptr());
}
