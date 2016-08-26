#ifndef ZEROCONF_DETAIL_HPP
#define ZEROCONF_DETAIL_HPP

//////////////////////////////////////////////////////////////////////////
// zeroconf-detail.hpp

// (C) Copyright 2016 Yuri Yakovlev <yvzmail@gmail.com>
// Use, modification and distribution is subject to the GNU General Public License

#include <vector>
#include <memory>
#include <chrono>

#include "zeroconf-util.hpp"

namespace Zeroconf
{
    namespace Detail
    {
        const size_t MdnsMessageMaxLength = 512;
        const size_t MdnsRecordHeaderLength = 10;    
    
        const uint8_t MdnsOffsetToken = 0xC0;
        const uint16_t MdnsResponseFlag = 0x8400;

        const uint32_t SockTrue = 1;

        const uint8_t MdnsQueryHeader[] = 
        {
            0x00, 0x00, // ID
            0x00, 0x00, // Flags
            0x00, 0x01, // QDCOUNT
            0x00, 0x00, // ANCOUNT
            0x00, 0x00, // NSCOUNT
            0x00, 0x00  // ARCOUNT
        };

        const uint8_t MdnsQueryFooter[] = 
        {
            0x00, 0x0c, // QTYPE
            0x00, 0x01  // QCLASS
        };

        struct raw_responce
        {
            sockaddr_storage peer;
            std::vector<uint8_t> data;
        };

        struct mdns_record
        {
            uint16_t type;
            size_t pos;
            size_t len;
            std::string hostname;
        };

        struct mdns_responce
        {
            sockaddr_storage peer;
            uint16_t qtype;
            std::string qfqdn;
            std::vector<mdns_record> records;
        };

        inline int GetErrorCode()
        {
            return WSAGetLastError(); // todo: errorno
        }

        inline void WriteFqdn(const std::string& serviceName, std::vector<uint8_t>* result)
        {
            size_t len = 0;
            size_t pos = result->size();
            result->push_back(0);

            for (size_t i = 0; i < serviceName.size(); i++)
            {
                if (serviceName[i] == '.')
                {
                    result->at(pos) = len; // update component length
                    
                    len = 0;
                    pos = result->size();
                    result->push_back(0); // serves as placeholder or trailing zero
                }
                else
                {
                    result->push_back(serviceName[i]);
                    len++;
                }
            }
        }

        inline bool CreateSocket(int* result)
        {
            int fd = socket(AF_INET, SOCK_DGRAM, 0);
            if (fd < 0)
            {
                Log::Error("Failed to create socket with code " + std::to_string(GetErrorCode()));
                return false;
            }

            int st = setsockopt(fd, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<const char*>(&SockTrue), sizeof(SockTrue));
            if (st < 0)
            {
                closesocket(fd);
                Log::Error("Failed to set socket option SO_BROADCAST with code " + std::to_string(GetErrorCode()));
                return false;
            }

            *result = fd;
            return true;
        }

        inline bool Send(int fd, const std::vector<uint8_t>& data)
        {
            sockaddr_in broadcastAddr = {0};
            broadcastAddr.sin_family = AF_INET;
            broadcastAddr.sin_port = htons(5353);
            broadcastAddr.sin_addr.s_addr = INADDR_BROADCAST;

            int st = sendto(
                fd, 
                reinterpret_cast<const char*>(&data[0]), 
                data.size(), 
                0, 
                reinterpret_cast<const sockaddr*>(&broadcastAddr), 
                sizeof(broadcastAddr));

            // todo: st == data.size() ???
            if (st < 0)
            {
                Log::Error("Failed to send the query with code " + std::to_string(GetErrorCode()));
                return false; 
            }

            return true;
        }

        inline bool Receive(int fd, time_t scanTime, std::vector<raw_responce>* result)
        {
            auto start = std::chrono::system_clock::now();

            while (1)
            {
                auto now = std::chrono::system_clock::now();
                if (now - start > std::chrono::seconds(scanTime))
                    break;

                fd_set fds;
                FD_ZERO(&fds);
                FD_SET(fd, &fds);

                timeval tv = {0};
                tv.tv_sec = static_cast<long>(scanTime);

                int st = select(0, &fds, nullptr, nullptr, &tv);
                
                if (st < 0)
                {
                    Log::Error("Failed to wait on socket with code " + std::to_string(GetErrorCode()));
                    return false; 
                }

                if (st > 0)
                {
                    int salen = sizeof(sockaddr_storage);

                    raw_responce item;
                    item.data.resize(MdnsMessageMaxLength);                    

                    st = recvfrom(
                        fd, 
                        reinterpret_cast<char*>(&item.data[0]), 
                        item.data.size(), 
                        0, 
                        reinterpret_cast<sockaddr*>(&item.peer), 
                        &salen);

                    if (st < 0)
                    {
                        Log::Error("Failed to receive with code " + std::to_string(GetErrorCode()));
                        return false; 
                    }

                    item.data.resize(st);
                    result->push_back(item);
                }
            }

            return true;
        }

        inline bool Parse(const raw_responce& input, mdns_responce* result)
        {
            // Structure:
            //   header (12b) 
            //   hostname1 ... hostnameN 0x00
            //   QTYPE (2b)
            //   QCLASS (2b)
            //   0xC0 hostname-offset (1b)
            //   DNS RR

            memcpy(&result->peer, &input.peer, sizeof(sockaddr_storage));

            stdext::membuf buf(&input.data[0], input.data.size());
            std::istream is(&buf);
    
            is.exceptions(std::istream::failbit | std::istream::badbit);

            try
            {
                uint8_t u8;
                uint16_t u16;

                is.ignore(2); // ID 
    
                is.read(reinterpret_cast<char*>(&u16), 2); // Flags
                if (ntohs(u16) != MdnsResponseFlag)
                {
                    Log::Warning("Found unexpected Flags value while parsing responce");
                    return false;
                }

                is.ignore(8); // QDCOUNT, ANCOUNT, NSCOUNT, ARCOUNT
        
                while (1) // QFQDN
                {
                    is.read(reinterpret_cast<char*>(&u8), 1); // hostname length
                    if (u8 == 0)
                        break;

                    char str[256];
                    is.read(str, u8); // hostname data
        
                    if (!result->qfqdn.empty())
                        result->qfqdn.append(".");
        
                    result->qfqdn.append(str, u8);
                }

                is.read(reinterpret_cast<char*>(&u16), 2); // QTYPE
                result->qtype = ntohs(u16);

                is.ignore(2); // QCLASS

                while (1)
                {
                    is.peek();
                    if (is.eof())
                        break;

                    mdns_record rr = {0};

                    is.read(reinterpret_cast<char*>(&u8), 1); // offset token
                    if (u8 != MdnsOffsetToken)
                    {
                        Log::Warning("Found incorrect offset token while parsing responce");
                        return false;
                    }
        
                    is.read(reinterpret_cast<char*>(&u8), 1); // offset value
                    if (u8 >= (uint8_t)input.data.size() || u8 + input.data[u8] >= (uint8_t)input.data.size())
                    {
                        Log::Warning("Found inconsistent offset value while parsing responce");
                        return false;
                    }

                    rr.hostname = std::string(reinterpret_cast<const char*>(&input.data[u8 + 1]), input.data[u8]);
                    rr.pos = static_cast<size_t>(is.tellg());

                    is.read(reinterpret_cast<char*>(&u16), 2); // TYPE
                    rr.type = ntohs(u16);

                    is.ignore(6); // QCLASS, TTL

                    is.read(reinterpret_cast<char*>(&u16), 2); // length
                    is.ignore(ntohs(u16)); // data

                    rr.len = MdnsRecordHeaderLength + ntohs(u16);
                    result->records.push_back(rr);
                }
            }
            catch (const std::istream::failure& ex)
            {
                Log::Warning(std::string("Stream error while parsing responce: ") + ex.what());
                return false;
            }

            return true;
        }

        inline bool Resolve(const std::string& serviceName, time_t scanTime, std::vector<mdns_responce>* result)
        {
            result->clear();

            std::vector<uint8_t> query;
            query.insert(query.end(), std::begin(MdnsQueryHeader), std::end(MdnsQueryHeader));
            WriteFqdn(serviceName, &query);
            query.insert(query.end(), std::begin(MdnsQueryFooter), std::end(MdnsQueryFooter));

            int fd = 0;
            if (!CreateSocket(&fd))
                return false;

            std::shared_ptr<void> guard(0, [fd](void*) { closesocket(fd); });

            if (!Send(fd, query))
                return false;
            
            std::vector<raw_responce> responces;
            if (!Receive(fd, scanTime, &responces))
                return false;
            
            for (auto& raw: responces)
            {
                mdns_responce parsed = {0};
                if (Parse(raw, &parsed))
                    result->push_back(parsed);
            }

            return true;
        }
    }
}

#endif // ZEROCONF_DETAIL_HPP