#include "common/core/UUIDProvider.hpp"

s64 UUIDProvider::s_workerId = 1;
s64 UUIDProvider::s_datacenterId = 1;
s64 UUIDProvider::s_twepoch = 687888001020L;
snowflake<> UUIDProvider::s_snowFlake;