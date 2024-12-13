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

#define assertString(exp, val) \
    if (strcmp(val, exp) != 0) { printf("exp=%s val=%s\n", exp, val); assert(false); }

const char jsonConfig1[] = 
"{"
    "\"lastRunLog\": 1024,"
    "\"connectionLog\": 2048,"
    "\"includeGeneral\": true,"
    "\"includeDiag\": false,"
    "\"includeTower\": false,"
    "\"logLevel\": \"INFO\","
    "\"logFilters\": {}"
"}";

const char jsonConfig2[] = 
"{"
    "\"lastRunLog\": 4096,"
    "\"connectionLog\": 2048,"
    "\"logLevel\": \"TRACE\""
"}";

const char jsonConfig3[] = 
"{"
    "\"lastRunLog\": 1024,"
    "\"connectionLog\": 1024,"
    "\"includeGeneral\": true,"
    "\"includeDiag\": false,"
    "\"includeTower\": false,"
    "\"logLevel\": \"ERROR\","
    "\"logFilters\": {}"
"}";

const char jsonConfig4[] = 
"{"
    "\"lastRunLog\": 1024,"
    "\"connectionLog\": 4096,"
    "\"includeTower\": true,"
    "\"logLevel\": \"INFO\","
"}";

/*
{
    "logLevel": "INFO",
    "logFilters": {
        "app.devinfo": "TRACE"
    }
}
*/
const char jsonConfig5[] = 
"{"
    "\"logLevel\": \"INFO\","
    "\"logFilters\": {"
        "\"app.devinfo\": \"TRACE\""
    "}"
"}";

const char jsonConfig6[] = 
"{"
    "\"logLevel\": \"INFO\","
    "\"logFilters\": {"
        "\"app.devinfo\": \"INFO\""
    "}"
"}";

const char jsonConfig7[] = 
"{"
    "\"logFilters\": {"
        "\"app.test\": \"TRACE\""
    "}"
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

    // Three levels of config
    {
        DeviceInfoLedger t1;
        LogLevel level;
        LogCategoryFilters filters;

        t1.withLocalConfig(jsonConfig3);

        assertInt(1024, t1.getConfigLastRunLog());
        assertInt(1024, t1.getConfigConnectionLog());
        assertInt(true, t1.getConfigIncludeGeneral());
        assertInt(false, t1.getConfigIncludeDiag());
        assertInt(false, t1.getConfigIncludeTower());

        t1.getLogLevelFilters(level, filters);
        assertInt(LOG_LEVEL_ERROR, level);
        assertInt(0, filters.size());


        t1.defaultConfig = LedgerData::fromJSON(jsonConfig1);

        assertInt(1024, t1.getConfigLastRunLog());
        assertInt(2048, t1.getConfigConnectionLog());
        assertInt(true, t1.getConfigIncludeGeneral());
        assertInt(false, t1.getConfigIncludeDiag());
        assertInt(false, t1.getConfigIncludeDiag());
        assertInt(false, t1.getConfigIncludeTower());

        t1.deviceConfig = LedgerData::fromJSON(jsonConfig4);

        assertInt(1024, t1.getConfigLastRunLog());
        assertInt(4096, t1.getConfigConnectionLog());
        assertInt(true, t1.getConfigIncludeGeneral());
        assertInt(false, t1.getConfigIncludeDiag());
        assertInt(false, t1.getConfigIncludeDiag());
        assertInt(true, t1.getConfigIncludeTower());

        t1.getLogLevelFilters(level, filters);
        assertInt(LOG_LEVEL_INFO, level);
        assertInt(0, filters.size());

    }

    // Filters from JSON
    {
        DeviceInfoLedger t1;
        LogLevel level;
        LogCategoryFilters filters;

        t1.withLocalConfig(jsonConfig5);

        t1.getLogLevelFilters(level, filters);
        assertInt(LOG_LEVEL_INFO, level);
        assertInt(1, filters.size());

        assertString("app.devinfo", filters.at(0).category());
        assertInt(LOG_LEVEL_TRACE, filters.at(0).level());
    }

    // Filters from code
    {
        DeviceInfoLedger t1;
        LogLevel level;
        LogCategoryFilters filters;

        t1.setLocalConfigLogLevel(LOG_LEVEL_INFO, {
            { "app.devinfo", LOG_LEVEL_TRACE }
        });

        t1.getLogLevelFilters(level, filters);
        assertInt(LOG_LEVEL_INFO, level);
        assertInt(1, filters.size());

        assertString("app.devinfo", filters.at(0).category());
        assertInt(LOG_LEVEL_TRACE, filters.at(0).level());
    }

    // Log filter merging - local config (JSON) + device config (no overlap)
    {
        DeviceInfoLedger t1;
        LogLevel level;
        LogCategoryFilters filters;

        t1.withLocalConfig(jsonConfig6);

        t1.getLogLevelFilters(level, filters);
        assertInt(LOG_LEVEL_INFO, level);
        assertInt(1, filters.size());

        assertString("app.devinfo", filters.at(0).category());
        assertInt(LOG_LEVEL_INFO, filters.at(0).level());

        t1.deviceConfig = LedgerData::fromJSON(jsonConfig7);

        t1.getLogLevelFilters(level, filters);
        assertInt(LOG_LEVEL_INFO, level);
        assertInt(2, filters.size());

        assertString("app.devinfo", filters.at(0).category());
        assertInt(LOG_LEVEL_INFO, filters.at(0).level());

        assertString("app.test", filters.at(1).category());
        assertInt(LOG_LEVEL_TRACE, filters.at(1).level());

    }

    // Log filter merging - local config (JSON) + device config override category
    {
        DeviceInfoLedger t1;
        LogLevel level;
        LogCategoryFilters filters;

        t1.withLocalConfig(jsonConfig6);

        t1.getLogLevelFilters(level, filters);
        assertInt(LOG_LEVEL_INFO, level);
        assertInt(1, filters.size());

        assertString("app.devinfo", filters.at(0).category());
        assertInt(LOG_LEVEL_INFO, filters.at(0).level());

        t1.deviceConfig = LedgerData::fromJSON(jsonConfig5);

        t1.getLogLevelFilters(level, filters);
        assertInt(LOG_LEVEL_INFO, level);
        assertInt(1, filters.size());

        assertString("app.devinfo", filters.at(0).category());
        assertInt(LOG_LEVEL_TRACE, filters.at(0).level());

    }

    // Filters array in device config overrides local    
    {
        DeviceInfoLedger t1;
        LogLevel level;
        LogCategoryFilters filters;

        t1.withLocalConfig(jsonConfig6);

        t1.getLogLevelFilters(level, filters);
        assertInt(LOG_LEVEL_INFO, level);
        assertInt(1, filters.size());

        assertInt(LOG_LEVEL_INFO, filters.at(0).level());
        assertString("app.devinfo", filters.at(0).category());


        t1.deviceConfig = LedgerData::fromJSON(jsonConfig5);

        t1.getLogLevelFilters(level, filters);
        assertInt(LOG_LEVEL_INFO, level);
        assertInt(1, filters.size());

        assertString("app.devinfo", filters.at(0).category());
        assertInt(LOG_LEVEL_TRACE, filters.at(0).level());
    }
    
    // Arbitrary configuration
    {
        DeviceInfoLedger t1;

    }

}




