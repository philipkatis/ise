#include "ise_base.h"
#include "acutest.h"

function void
Test(void)
{
    TEST_CHECK(true);
}

TEST_LIST = {
    { "Test", Test },
    { 0, 0 }
};
