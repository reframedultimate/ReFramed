#include "gmock/gmock.h"
#include "rfcommon/init.h"

int main(int argc, char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    rfcommon_init(".");
    int result = RUN_ALL_TESTS();
    rfcommon_deinit();
    return result;
}
