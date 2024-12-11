#include <stdio.h>

#include "DeviceInfoLedger.h"

void runUnitTests();

int main(int argc, char *argv[]) {
    runUnitTests();
    return 0;
}

#define assertInt(exp, val) \
    if (val != exp) { printf("exp=%d val=%d\n", (int)exp, (int)val); assert(false); }

#define assertDouble(exp, val, tol) \
    if (val < (exp - tol) || val > (exp + tol)) { printf("exp=%lf val=%lf\n", (double)exp, (double)val); assert(false); }

const char localConfig1[] = 
"{"
    "\"lastRunLog\": 1024,"
    "\"connectionLog\": 2048,"
    "\"includeGeneral\": true,"
    "\"includeDiag\": false,"
    "\"includeTower\": false,"
    "\"logLevel\": \"LOG_LEVEL_INFO\","
    "\"logFilters\": []"
"}";


void runUnitTests() {
    // Local unit tests only used off-device 

    {
        // On-device, this constructor is protected and can't be called, but it's public in the unit tests
        DeviceInfoLedger t1;

        t1.withLocalConfig(localConfig1);

        assertInt(1024, t1.getConfigLastRunLog());
        assertInt(2048, t1.getConfigConnectionLog());
        assertInt(true, t1.getConfigIncludeGeneral());
        assertInt(false, t1.getConfigIncludeDiag());

        LogLevel level;
        LogCategoryFilters filters;
        t1.getLogLevelFilters(level, filters);
        assertInt(LOG_LEVEL_INFO, level);
        assertInt(0, filters.size());


        t1.setLocalConfigConnectionLog(4096);
        assertInt(4096, t1.getConfigConnectionLog());

    }

    
    
}




