#include <gmock/gmock.h>

#include "zeroconf-detail.hpp"

using testing::ElementsAre;

TEST(Test_WriteFqdn, Empty)
{
    static const std::string Samples[] = { "", ".", ".." };

    for (auto& s: Samples)
    {
        std::vector<uint8_t> result;
        Zeroconf::Detail::WriteFqdn(s, &result);
        EXPECT_THAT(result, ElementsAre(0));
    }
}

TEST(Test_WriteFqdn, OnePart)
{
    static const std::string Samples[] = { "foo", ".foo", "foo.", ".foo." };

    for (auto& s: Samples)
    {
        std::vector<uint8_t> result;
        Zeroconf::Detail::WriteFqdn(s, &result);
        EXPECT_THAT(result, ElementsAre(0x03, 'f', 'o', 'o', 0x00));
    }
}

TEST(Test_WriteFqdn, MultipleParts)
{
    static const std::string Samples[] = { "foo.bar", "foo..bar", ".foo.bar", "foo.bar.", ".foo.bar." };

    for (auto& s: Samples)
    {
        std::vector<uint8_t> result;
        Zeroconf::Detail::WriteFqdn(s, &result);
        EXPECT_THAT(result, ElementsAre(0x03, 'f', 'o', 'o', 0x03, 'b', 'a', 'r', 0x00));
    }
}

TEST(Test_WriteFqdn, MaxLength)
{
    std::vector<uint8_t> result;
    Zeroconf::Detail::WriteFqdn(std::string(255, 'a'), &result);
    EXPECT_EQ(257, result.size());
    
    result.clear();
    Zeroconf::Detail::WriteFqdn(std::string(256, 'a'), &result);
    EXPECT_EQ(0, result.size());
}
