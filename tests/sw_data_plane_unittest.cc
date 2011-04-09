#include "gtest/gtest.h"

#include "sw_data_plane.h"


TEST(SWDataPlaneTest, Construct) {
  SWDataPlane::Ptr swdp = SWDataPlane::SWDataPlaneNew(NULL, NULL);

  ASSERT_TRUE(swdp.ptr());
}
