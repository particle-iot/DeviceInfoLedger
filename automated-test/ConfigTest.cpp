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

const char jsonConfig1[] = 
"{"
    "\"lastRunLog\": 1024,"
    "\"connectionLog\": 2048,"
    "\"includeGeneral\": true,"
    "\"includeDiag\": false,"
    "\"includeTower\": false,"
    "\"logLevel\": \"LOG_LEVEL_INFO\","
    "\"logFilters\": []"
"}";

const char jsonConfig2[] = 
"{"
    "\"lastRunLog\": 4096,"
    "\"connectionLog\": 2048,"
    "\"logLevel\": \"LOG_LEVEL_TRACE\""
"}";


void runUnitTests() {
    // Local unit tests only used off-device 

    // Default values
    {
        // On-device, this constructor is protected and can't be called, but it's public in the unit tests
        DeviceInfoLedger t1;

        assertInt(0, t1.getConfigLastRunLog());
        assertInt(0, t1.getConfigConnectionLog());
        assertInt(false, t1.getConfigIncludeGeneral());
        assertInt(false, t1.getConfigIncludeDiag());

        LogLevel level;
        LogCategoryFilters filters;
        t1.getLogLevelFilters(level, filters);
        assertInt(LOG_LEVEL_NONE, level);
        assertInt(0, filters.size());
    }
    
    // Set from JSON
    {
        // On-device, this constructor is protected and can't be called, but it's public in the unit tests
        DeviceInfoLedger t1;

        t1.withLocalConfig(jsonConfig1);

        assertInt(1024, t1.getConfigLastRunLog());
        assertInt(2048, t1.getConfigConnectionLog());
        assertInt(true, t1.getConfigIncludeGeneral());
        assertInt(false, t1.getConfigIncludeDiag());

        LogLevel level;
        LogCategoryFilters filters;
        t1.getLogLevelFilters(level, filters);
        assertInt(LOG_LEVEL_INFO, level);
        assertInt(0, filters.size());

        // Test resetting value
        t1.setLocalConfigConnectionLog(4096);
        assertInt(4096, t1.getConfigConnectionLog());
    }

    // Set from code
    {
        // On-device, this constructor is protected and can't be called, but it's public in the unit tests
        DeviceInfoLedger t1;
        t1.setLocalConfigLastRunLog(1024);
        t1.setLocalConfigConnectionLog(2048);
        t1.setLocalConfigIncludeGeneral(true);
        t1.setLocalConfigLogLevel(LOG_LEVEL_INFO);


        assertInt(1024, t1.getConfigLastRunLog());
        assertInt(2048, t1.getConfigConnectionLog());
        assertInt(true, t1.getConfigIncludeGeneral());
        assertInt(false, t1.getConfigIncludeDiag());

        LogLevel level;
        LogCategoryFilters filters;
        t1.getLogLevelFilters(level, filters);
        assertInt(LOG_LEVEL_INFO, level);
        assertInt(0, filters.size());
    }

    // Cloud defaults, no local config
    {
        // On-device, this constructor is protected and can't be called, but it's public in the unit tests
        // Same for defaultConfig and deviceConfig
        DeviceInfoLedger t1;
        t1.defaultConfig = LedgerData::fromJSON(jsonConfig1);

        assertInt(1024, t1.getConfigLastRunLog());
        assertInt(2048, t1.getConfigConnectionLog());
        assertInt(true, t1.getConfigIncludeGeneral());
        assertInt(false, t1.getConfigIncludeDiag());

        LogLevel level;
        LogCategoryFilters filters;
        t1.getLogLevelFilters(level, filters);
        assertInt(LOG_LEVEL_INFO, level);
        assertInt(0, filters.size());
    }

    // Cloud default overrides local config
    {
        DeviceInfoLedger t1;

        t1.withLocalConfig(jsonConfig1);

        assertInt(1024, t1.getConfigLastRunLog());
        assertInt(2048, t1.getConfigConnectionLog());
        assertInt(true, t1.getConfigIncludeGeneral());
        assertInt(false, t1.getConfigIncludeDiag());

        LogLevel level;
        LogCategoryFilters filters;
        t1.getLogLevelFilters(level, filters);
        assertInt(LOG_LEVEL_INFO, level);
        assertInt(0, filters.size());

        t1.defaultConfig = LedgerData::fromJSON(jsonConfig2);

        assertInt(4096, t1.getConfigLastRunLog());
        assertInt(2048, t1.getConfigConnectionLog());
        assertInt(true, t1.getConfigIncludeGeneral());
        assertInt(false, t1.getConfigIncludeDiag());

        t1.getLogLevelFilters(level, filters);
        assertInt(LOG_LEVEL_TRACE, level);
        assertInt(0, filters.size());
    }

    // Device overrides local config
    {
        DeviceInfoLedger t1;

        t1.withLocalConfig(jsonConfig1);

        assertInt(1024, t1.getConfigLastRunLog());
        assertInt(2048, t1.getConfigConnectionLog());
        assertInt(true, t1.getConfigIncludeGeneral());
        assertInt(false, t1.getConfigIncludeDiag());

        LogLevel level;
        LogCategoryFilters filters;
        t1.getLogLevelFilters(level, filters);
        assertInt(LOG_LEVEL_INFO, level);
        assertInt(0, filters.size());

        t1.deviceConfig = LedgerData::fromJSON(jsonConfig2);

        assertInt(4096, t1.getConfigLastRunLog());
        assertInt(2048, t1.getConfigConnectionLog());
        assertInt(true, t1.getConfigIncludeGeneral());
        assertInt(false, t1.getConfigIncludeDiag());

        t1.getLogLevelFilters(level, filters);
        assertInt(LOG_LEVEL_TRACE, level);
        assertInt(0, filters.size());
    }    

    // Device config overrides cloud config
    {
        DeviceInfoLedger t1;

        t1.defaultConfig = LedgerData::fromJSON(jsonConfig1);

        assertInt(1024, t1.getConfigLastRunLog());
        assertInt(2048, t1.getConfigConnectionLog());
        assertInt(true, t1.getConfigIncludeGeneral());
        assertInt(false, t1.getConfigIncludeDiag());

        LogLevel level;
        LogCategoryFilters filters;
        t1.getLogLevelFilters(level, filters);
        assertInt(LOG_LEVEL_INFO, level);
        assertInt(0, filters.size());

        t1.deviceConfig = LedgerData::fromJSON(jsonConfig2);

        assertInt(4096, t1.getConfigLastRunLog());
        assertInt(2048, t1.getConfigConnectionLog());
        assertInt(true, t1.getConfigIncludeGeneral());
        assertInt(false, t1.getConfigIncludeDiag());

        t1.getLogLevelFilters(level, filters);
        assertInt(LOG_LEVEL_TRACE, level);
        assertInt(0, filters.size());
    }    

}




