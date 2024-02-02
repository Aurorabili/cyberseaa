#ifndef UUIDPROVIDER_HPP_
#define UUIDPROVIDER_HPP_

#include "common/utils/IntTypes.hpp"
#include "common/utils/SnowFlake.hpp"

class UUIDProvider {
public:
    static void init(s64 workerId, s64 datacenterId, s64 twepoch)
    {
        s_workerId = workerId;
        s_datacenterId = datacenterId;
        s_twepoch = twepoch;
        s_snowFlake.init(s_workerId, s_datacenterId, s_twepoch);
    }

    static s64 nextUUID()
    {
        return s_snowFlake.nextid();
    }

private:
    static s64 s_workerId;
    static s64 s_datacenterId;
    static s64 s_twepoch;
    static snowflake<> s_snowFlake;
};

#endif /* UUIDPROVIDER_HPP_ */