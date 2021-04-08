#include "gtest/gtest.h"
#include "learner.h"
#include <memory>

TEST(learner, loadKifu)
{
    Learner ln(std::make_shared<DeepNetwork>());
    ln.loadKifu(3);
}
