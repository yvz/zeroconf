#include <gmock/gmock.h>

#include "zeroconf-detail.hpp"

using testing::ElementsAre;
using testing::ElementsAreArray;

namespace
{
    const uint8_t RealPacket[] =
    {
        0x00, 0x00, 0x84, 0x00, 0x00, 0x01, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x05, 0x5F, 0x68, 0x74,
        0x74, 0x70, 0x04, 0x5F, 0x74, 0x63, 0x70, 0x05, 0x6C, 0x6F, 0x63, 0x61, 0x6C, 0x00, 0x00, 0x0C,
        0x00, 0x01, 0xC0, 0x0C, 0x00, 0x0C, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x10, 0x0D, 0x61,
        0x70, 0x70, 0x6C, 0x65, 0x20, 0x6D, 0x61, 0x63, 0x62, 0x6F, 0x6F, 0x6B, 0xC0, 0x0C, 0xC0, 0x2E,
        0x00, 0x10, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x46, 0x45, 0x4C, 0x6F, 0x72, 0x65, 0x6D,
        0x20, 0x69, 0x70, 0x73, 0x75, 0x6D, 0x20, 0x64, 0x6F, 0x6C, 0x6F, 0x72, 0x20, 0x73, 0x69, 0x74,
        0x20, 0x61, 0x6D, 0x65, 0x74, 0x20, 0x63, 0x6F, 0x6E, 0x73, 0x65, 0x63, 0x74, 0x65, 0x74, 0x75,
        0x72, 0x20, 0x61, 0x64, 0x69, 0x70, 0x69, 0x73, 0x63, 0x69, 0x6E, 0x67, 0x20, 0x65, 0x6C, 0x69,
        0x74, 0x20, 0x73, 0x65, 0x64, 0x20, 0x64, 0x6F, 0x20, 0x65, 0x69, 0x75, 0x73, 0x6D, 0x6F, 0x64,
        0xC0, 0x2E, 0x00, 0x21, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x00,
        0x22, 0xB3, 0x05, 0x61, 0x70, 0x70, 0x6C, 0x65, 0xC0, 0x17, 0xC0, 0xA2, 0x00, 0x1C, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x0A, 0x00, 0x10, 0xFD, 0xAD, 0xC9, 0xE2, 0x23, 0x28, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xC0, 0xA2, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0A,
        0x00, 0x04, 0xC0, 0xA8, 0x00, 0x01
    };

    //  0     1     2     3     4     5     6     7     8     9     10    11    12    13    14    15    16
    //  id          flags       qdcount     ancount     nscount     arcount     fqdn  qtype       qclass
    const uint8_t BlankPacket[] = 
    {
        0x00, 0x00, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    //  0     1     2     3     4     5     6     7     8     9     10    11
    //  tok   offs  type        class       ttl                     length
    const uint8_t BlankRecord[] = 
    {
        0xc0, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    const uint8_t Fqdn1[] = { 0x03, 'f', 'o', 'o', 0x00 };
    const uint8_t Fqdn2[] = { 0x03, 'f', 'o', 'o', 0x03, 'b', 'a', 'r', 0x00 };
}

TEST(Test_Parse, RealPacket)
{
    Zeroconf::Detail::raw_responce input;
    Zeroconf::Detail::mdns_responce output;
    input.data.assign(std::begin(RealPacket), std::end(RealPacket));

    ASSERT_TRUE(Zeroconf::Detail::Parse(input, &output));
    ASSERT_EQ(  5, output.records.size());

    EXPECT_EQ( 34, output.records[0].pos);
    EXPECT_EQ( 28, output.records[0].len);
    EXPECT_EQ( 62, output.records[1].pos);
    EXPECT_EQ( 82, output.records[1].len);
    EXPECT_EQ(144, output.records[2].pos);
    EXPECT_EQ( 26, output.records[2].len);
    EXPECT_EQ(170, output.records[3].pos);
    EXPECT_EQ( 28, output.records[3].len);
    EXPECT_EQ(198, output.records[4].pos);
    EXPECT_EQ( 16, output.records[4].len);

    EXPECT_STREQ("_http", output.records[0].name.c_str());
    EXPECT_STREQ("apple macbook", output.records[1].name.c_str());
    EXPECT_STREQ("apple macbook", output.records[2].name.c_str());
    EXPECT_STREQ("apple", output.records[3].name.c_str());
    EXPECT_STREQ("apple", output.records[4].name.c_str());

    EXPECT_STREQ("_http._tcp.local", output.qname.c_str());
}

TEST(Test_Parse, QueryTypeAndPeer)
{
    Zeroconf::Detail::raw_responce input;
    Zeroconf::Detail::mdns_responce output;

    input.data.assign(std::begin(BlankPacket), std::end(BlankPacket));
    input.data[13] = 0xab;
    input.data[14] = 0xcd;
    
    auto p = reinterpret_cast<uint8_t*>(&input.peer);
    for (size_t i = 0; i < sizeof(sockaddr_storage); i++)
        p[i] = i;
    
    ASSERT_TRUE(Zeroconf::Detail::Parse(input, &output));

    EXPECT_EQ(0, memcmp(&input.peer, &output.peer, sizeof(sockaddr_storage)));
    EXPECT_EQ(0xabcd, output.qtype);
    EXPECT_TRUE(output.qname.empty());
    EXPECT_TRUE(output.records.empty());
}

TEST(Test_Parse, QueryName)
{
    Zeroconf::Detail::raw_responce input;
    Zeroconf::Detail::mdns_responce output;

    input.data.assign(std::begin(BlankPacket), std::end(BlankPacket));
    input.data.erase(input.data.begin() + 12);
    input.data.insert(input.data.begin() + 12, std::begin(Fqdn1), std::end(Fqdn1));

    EXPECT_TRUE(Zeroconf::Detail::Parse(input, &output));
    EXPECT_STREQ("foo", output.qname.c_str());

    input.data.assign(std::begin(BlankPacket), std::end(BlankPacket));
    input.data.erase(input.data.begin() + 12);
    input.data.insert(input.data.begin() + 12, std::begin(Fqdn2), std::end(Fqdn2));

    EXPECT_TRUE(Zeroconf::Detail::Parse(input, &output));
    EXPECT_STREQ("foo.bar", output.qname.c_str());
}

TEST(Test_Parse, MultipleRecordsVariedSizeAndType)
{
    static const size_t SampleCount = 10;

    Zeroconf::Detail::raw_responce input;
    Zeroconf::Detail::mdns_responce output;

    input.data.assign(std::begin(BlankPacket), std::end(BlankPacket));
    
    for (size_t i = 0; i < SampleCount; i++)
    {
        auto it = input.data.insert(input.data.end(), std::begin(BlankRecord), std::end(BlankRecord));        

        // i + 256 -> type, size
        *(it + 2) = 1; 
        *(it + 3) = i;
        *(it + 10) = 1;
        *(it + 11) = i;

        // allocate payload
        input.data.resize(input.data.size() + i + 256);
    }

    ASSERT_TRUE(Zeroconf::Detail::Parse(input, &output));
    ASSERT_EQ(SampleCount, output.records.size());

    for (size_t i = 0; i < SampleCount; i++)
    {
        size_t pos = sizeof(BlankPacket);
        pos += i * (sizeof(BlankRecord) + 256);
        pos += i * (i - 1) / 2; // series

        size_t len = sizeof(BlankRecord) + i + 256;
    
        auto& rr = output.records[i];
        EXPECT_EQ(i + 256, rr.type);
        EXPECT_EQ(pos, rr.pos);
        EXPECT_EQ(len, rr.len);
    }
}

TEST(Test_Parse, RecordName)
{
    Zeroconf::Detail::raw_responce input;
    Zeroconf::Detail::mdns_responce output;

    input.data.assign(std::begin(BlankPacket), std::end(BlankPacket));
    
    input.data.erase(input.data.begin() + 12);
    input.data.insert(input.data.begin() + 12, std::begin(Fqdn2), std::end(Fqdn2));
    
    input.data.insert(input.data.end(), std::begin(BlankRecord), std::end(BlankRecord));

    ASSERT_TRUE(Zeroconf::Detail::Parse(input, &output));
    ASSERT_EQ(1, output.records.size());
    EXPECT_STREQ("foo", output.records[0].name.c_str());
}

TEST(Test_Parse, EmptyAndIncompletePacket)
{
    Zeroconf::Detail::raw_responce input;
    Zeroconf::Detail::mdns_responce output;

    for (auto i: BlankPacket)
    {
        EXPECT_FALSE(Zeroconf::Detail::Parse(input, &output));
        input.data.push_back(i);
    }
}

TEST(Test_Parse, WrongPacketFlags)
{
    static const size_t Offsets[] = { 2, 3 };

    for (auto i: Offsets)
    {
        Zeroconf::Detail::raw_responce input;
        Zeroconf::Detail::mdns_responce output;
        
        input.data.assign(std::begin(BlankPacket), std::end(BlankPacket));
        input.data[i] = ~input.data[i];

        EXPECT_FALSE(Zeroconf::Detail::Parse(input, &output));
    }
}

TEST(Test_Parse, WrongFqdn)
{
    static const uint8_t InvalidFqdnLengths[] = { 2, 3, 4, 5 };

    Zeroconf::Detail::raw_responce input;
    Zeroconf::Detail::mdns_responce output;
    
    input.data.assign(std::begin(BlankPacket), std::end(BlankPacket));

    for (auto i: InvalidFqdnLengths)
    {    
        input.data[12] = i;
        EXPECT_FALSE(Zeroconf::Detail::Parse(input, &output));
    }
}

TEST(Test_Parse, IncompleRecord)
{
    Zeroconf::Detail::raw_responce input;
    Zeroconf::Detail::mdns_responce output;

    input.data.assign(std::begin(BlankPacket), std::end(BlankPacket));

    for (size_t i = 0; i < sizeof(BlankRecord) - 1; i++)
    {
        input.data.push_back(BlankRecord[i]);
        EXPECT_FALSE(Zeroconf::Detail::Parse(input, &output));
    }

    input.data.push_back(1); // 1 -> length
    EXPECT_FALSE(Zeroconf::Detail::Parse(input, &output));
}

TEST(Test_Parse, WrongRecordToken)
{
    Zeroconf::Detail::raw_responce input;
    Zeroconf::Detail::mdns_responce output;
        
    input.data.assign(std::begin(BlankPacket), std::end(BlankPacket));
    auto it = input.data.insert(input.data.end(), std::begin(BlankRecord), std::end(BlankRecord));
    *it = 0;

    EXPECT_FALSE(Zeroconf::Detail::Parse(input, &output));
}
