#include "DeviceInfoLedger.h"


DeviceInfoLedger *DeviceInfoLedger::_instance;

static Logger _deviceInfoLog("app.devinfo");

// [static]
DeviceInfoLedger &DeviceInfoLedger::instance() {
    if (!_instance) {
        _instance = new DeviceInfoLedger();
    }
    return *_instance;
}

DeviceInfoLedger::DeviceInfoLedger() {
}

DeviceInfoLedger::~DeviceInfoLedger() {
}

void DeviceInfoLedger::setup() {
#ifndef UNITTEST
    if (setupComplete) {
        // Cannot call setup more than once
        return;
    }

    resetReason = System.resetReason();
    if (resetReason ==  RESET_REASON_USER) {
        resetReasonData = System.resetReasonData();
    }

    // Set up ledgers
    infoLedger = Particle.ledger(infoLedgerName);

    if (configDefaultLedgerEnabled) {
        configDefaultLedger = Particle.ledger(configDefaultLedgerName);
        configDefaultLedger.onSync([this](Ledger ledger) {
            defaultConfig = ledger.get();
            updateConfig();
        });
        defaultConfig = configDefaultLedger.get();
    }


    if (configDeviceLedgerEnabled) {
        configDeviceLedger = Particle.ledger(configDeviceLedgerName);
        configDeviceLedger.onSync([this](Ledger ledger) {
            deviceConfig = ledger.get();
        });
        deviceConfig = configDeviceLedger.get();
    }
    updateConfig();

    // Check retained buffer
    if (retainedBuffer && retainedBufferSize && retainedBufferSize > sizeof(RetainedBufferHeader)) {
        retainedHdr = (RetainedBufferHeader *)retainedBuffer;
        retainedData = &retainedBuffer[sizeof(RetainedBufferHeader)];
        retainedDataSize = retainedBufferSize - sizeof(RetainedBufferHeader);

        if (retainedHdr->magic == retainedMagicBytes && retainedHdr->size == retainedBufferSize && retainedHdr->headerSize == sizeof(RetainedBufferHeader)) {
            _deviceInfoLog.info("retained buffer valid");

            if (retainedHdr->offset > 0) {
                size_t size = retainedHdr->offset;
                if (size > retainedDataSize) {
                    size = retainedDataSize;
                }
                int lastRunLogConfig = getConfigLastRunLog();
                if (lastRunLogConfig > 0) {
                    if (size > (size_t)lastRunLogConfig) {
                        size = (size_t)lastRunLogConfig;
                    }
                    lastRunLog = new char[size + 1];
                    if (lastRunLog) {
                        for(size_t ii = 0; ii < size; ii++) {
                            lastRunLog[ii] = retainedData[(retainedHdr->offset - size + ii) % retainedDataSize];
                        }
                        lastRunLog[size] = 0;
                    }

                }


                retainedHdr->offset = 0;
            }
        }
        else {
            // Reinitialize header
            _deviceInfoLog.info("retained buffer reinitialized");
            retainedHdr->magic = retainedMagicBytes;
            retainedHdr->size = retainedBufferSize;
            retainedHdr->headerSize = sizeof(RetainedBufferHeader);
            retainedHdr->reserved1 = 0;
            retainedHdr->offset = 0;
        }
    }
    else {
        // Invalid parameters
        retainedBuffer = nullptr;
        retainedBufferSize = 0;
    }

    setupComplete = true;
#endif // UNITTEST

}

void DeviceInfoLedger::loop() {
#ifndef UNITTEST

    if (Network.ready()) {
        if (!isNetworkConnected) {
            isNetworkConnected = true;
        }
    }
    else {
        if (isNetworkConnected) {
            isNetworkConnected = false;
        }

    }
#if Wiring_Cellular
    if (Cellular.ready()) {
        if (!isCellularConnected) {
            isCellularConnected = true;
        }
    }
    else {
        if (isCellularConnected) {
            isCellularConnected = false;
        }

    }
#endif
#if Wiring_WiFi
    if (WiFi.ready()) {
        if (!isWiFiConnected) {
            isWiFiConnected = true;
        }
    }
    else {
        if (isWiFiConnected) {
            isWiFiConnected = false;
        }
    }
#endif

    // TODO: Ethernet?

    if (Particle.connected()) {
        if (!isCloudConnected) {
            isCloudConnected = true;

            if (++connectionCount == 1) {
                // First connection after boot
                onFirstCloudConnection();
            }
            onCloudConnection();
        }
    }
    else {
        if (isCloudConnected) {
            isCloudConnected = false;
            writeToConnectionLog = true;
        }
    }
#endif // UNITTEST
}


Variant DeviceInfoLedger::getConfigVariant(const char *key, Variant defaultValue) const {
    Variant result = defaultValue;

    if (deviceConfig.has(key)) {
        result  = deviceConfig.get(key);
        // _deviceInfoLog.trace("device override key %s", key);
    }
    else
    if (defaultConfig.has(key)) {
        result  = defaultConfig.get(key);
        // _deviceInfoLog.trace("defaultConfig key %s", key);
    }
    else
    if (localConfig.has(key)) {
        result  = localConfig.get(key);
        // _deviceInfoLog.trace("localConfig key %s", key);
    }

    return result;
}


void DeviceInfoLedger::forEachConfigArray(const char *key, std::function<void(const Variant &el)> fn) const {

    Variant array = getConfigVariant(key);
    if (array.isArray()) {
        int size = array.size();
        if (size > 0) {
            for(int ii = 0; ii < size; ii++) {
                fn(array[ii]);
            }
        }
    }
}
bool DeviceInfoLedger::setLocalConfigLogLevel(LogLevel level, LogCategoryFilters filters) {
    bool result;
    
    result = setLocalConfigString("logLevel", logLevelToString(level));

    Variant array;
    for(LogCategoryFilter &filter: filters) {
        Variant entry;
        entry.set("category", filter.category());
        entry.set("level", logLevelToString(filter.level()));
        array.append(entry);
    }
    if (array.isArray()) {
        result = setLocalConfigVariant("logFilters", array);
    }

    return result;
}


void DeviceInfoLedger::updateConfig() {

    _deviceInfoLog.info("updateConfig lastRunLog=%d connectionLog=%d logLevel=%s", 
        DeviceInfoLedger::instance().getConfigLastRunLog(),
        DeviceInfoLedger::instance().getConfigConnectionLog(),
        DeviceInfoLedger::instance().getConfigString("logLevel").c_str());

    configureLogHandler();


    // Allocate log buffer
    connectionLogSize = DeviceInfoLedger::instance().getConfigConnectionLog();
    if (connectionLogBuffer) {
        delete[] connectionLogBuffer;
        connectionLogBuffer = nullptr;
    }
    if (connectionLogSize) {
        connectionLogBuffer = new uint8_t[connectionLogSize];
    }
    connectionLogOffset = 0;

}

const char *DeviceInfoLedger::logLevelToString(LogLevel level) const {
    const char *levelName = "NONE";
    switch(level) {
        // LOG_LEVEL_ALL has the same numeric value (1) as LOG_LEVEL_TRACE
        /*
        case LOG_LEVEL_ALL:
            levelName = "ALL";
            break;
        */

        case LOG_LEVEL_TRACE:
            levelName = "TRACE";
            break;

        case LOG_LEVEL_INFO:
            levelName = "INFO";
            break;

        case LOG_LEVEL_WARN:
            levelName = "WARN";
            break;

        case LOG_LEVEL_ERROR:
            levelName = "ERROR";
            break;

        case LOG_LEVEL_PANIC:
            levelName = "PANIC";
            break;

        default:
            break;
    }    
    return levelName;
}

LogLevel DeviceInfoLedger::stringToLogLevel(const char *levelStr) const {
    LogLevel level = LOG_LEVEL_NONE;

    if (strstr(levelStr, "ALL") != 0) {
        level = LOG_LEVEL_ALL;
    }
    else
    if (strstr(levelStr, "TRACE") != 0) {
        level = LOG_LEVEL_TRACE;
    }
    else
    if (strstr(levelStr, "INFO") != 0) {
        level = LOG_LEVEL_INFO;
    }
    else
    if (strstr(levelStr, "WARN") != 0) {
        level = LOG_LEVEL_WARN;
    }
    else
    if (strstr(levelStr, "ERROR") != 0) {
        level = LOG_LEVEL_ERROR;
    }
    else
    if (strstr(levelStr, "PANIC") != 0) {
        level = LOG_LEVEL_PANIC;
    }
    else {
        level = LOG_LEVEL_NONE;
    }
    

    return level;
}

void DeviceInfoLedger::getLogLevelFilters(LogLevel &level, LogCategoryFilters &filters) const {
    level = stringToLogLevel(getConfigString("logLevel").c_str());
    // _deviceInfoLog.info("level %d", level);

    forEachConfigArray("logFilters", [&filters,this](const Variant &el) {
        String category = el.get("category").toString();
        LogLevel level = stringToLogLevel(el.get("level").toString().c_str());
        // _deviceInfoLog.info("filter %d %s", level, category.c_str());       
        filters.append(LogCategoryFilter(category, level)); 
    });

}

void DeviceInfoLedger::configureLogHandler() {
#ifndef UNITTEST
    if (logHandler) {
        LogManager::instance()->removeHandler(logHandler);
        delete logHandler;
        logHandler = nullptr;
    }

    LogLevel level;
    LogCategoryFilters filters;
    getLogLevelFilters(level, filters);

    logHandler = new DeviceInfoLedgerLogHandler(level, filters);

	// Add this handler into the system log manager
	LogManager::instance()->addHandler(logHandler);
#endif

}



void DeviceInfoLedger::write(uint8_t c) {
    if (!setupComplete) {
        return;
    }

    if (c == '\r') {
        // Log lines are terminated by CRLF, but remove the CR here. Maybe make this configurable.
        return;
    }

    if (retainedBuffer) {
        retainedData[retainedHdr->offset++ % retainedDataSize] = c;
    }

    if (writeToConnectionLog && connectionLogSize && connectionLogBuffer) {
        connectionLogBuffer[connectionLogOffset++ % connectionLogSize] = c;
    }

}

void DeviceInfoLedger::onFirstCloudConnection() {
}


void DeviceInfoLedger::onCloudConnection() {
#ifndef UNITTEST
    Variant data;

    // Save connection log
    uint32_t offset = connectionLogOffset;
    if (offset) {
        size_t size = offset;
        if (size > connectionLogSize) {
            size = connectionLogSize;
        }
        // _deviceInfoLog.info("connectionLog offset=%lu size=%u", offset, size);

        char *buf = new char[size + 1];
        if (buf) {
            for(size_t ii = 0; ii < size; ii++) {
                buf[ii] = (char) connectionLogBuffer[(offset - size + ii) % connectionLogSize];
            }
            buf[size] = 0;
            
            data.set("connLog", buf);

            delete[] buf;
        }
        writeToConnectionLog = false;
        connectionLogOffset = 0;
    }

    // Save last run log
    if (lastRunLog) {
        data.set("lastRun", lastRunLog);
    }

    // Save reset reason. Value stored in member variable in setup().
    if (resetReason != RESET_REASON_NONE) {
        data.set("resetReason", resetReason);
        if (resetReason ==  RESET_REASON_USER) {
            data.set("resetReasonData", resetReasonData);
        }
    }

    // Save device diagnostics if enabled
    if (getConfigIncludeDiag()) {
    	String jsonStr;

	    jsonStr.reserve(256);

        struct {
            static bool appender(void* appender, const uint8_t* data, size_t size) {
                String *s = (String *)appender;
                return (bool) s->concat(String((const char *)data, size));
            }
        } Callback;

        system_format_diag_data(nullptr, 0, 0, Callback.appender, &jsonStr, nullptr);
        // _deviceInfoLog.info("diag %s ", jsonStr.c_str());

        data.set("diag", Variant::fromJSON(jsonStr));
    }

#if Wiring_Cellular
    // Save cell tower information if cellular, and enabled
    if (getConfigIncludeTower()) {
        CellularGlobalIdentity cgi = {0};
        cgi.size = sizeof(CellularGlobalIdentity);
        cgi.version = CGI_VERSION_LATEST;

        cellular_result_t res = cellular_global_identity(&cgi, NULL);
        if (res == SYSTEM_ERROR_NONE) {
            Variant tower;
            tower.set("mcc", cgi.mobile_country_code);
            tower.set("mnc", cgi.mobile_network_code);
            tower.set("cid", cgi.cell_id);
            tower.set("lac", cgi.location_area_code);

            data.set("tower", tower);
            _deviceInfoLog.trace("Cellular Info: cid=%lu lac=%u mcc=%u mnc=%u", cgi.cell_id, cgi.location_area_code, cgi.mobile_country_code, cgi.mobile_network_code);
        }
        else {
            _deviceInfoLog.info("cellular_global_identity failed %d", res);
        }
    }
    // Add modem information here?
    // Things like ICCID are stored in the cloud
#endif

    if (getConfigIncludeGeneral()) {
        data.set("sysver", System.version().c_str());        
    }

    infoLedger.set(data, Ledger::REPLACE); // Ledger::REPLACE Ledger::MERGE

    _deviceInfoLog.trace("infoLedger updated");
#endif // UNITTEST

}

#ifndef UNITTEST
DeviceInfoLedgerLogHandler::DeviceInfoLedgerLogHandler(LogLevel level, LogCategoryFilters filters) : StreamLogHandler(*this, level, filters) {

}

DeviceInfoLedgerLogHandler::~DeviceInfoLedgerLogHandler() {

}

size_t DeviceInfoLedgerLogHandler::write(uint8_t c) {
    DeviceInfoLedger::instance().write(c);
    return 1;
}
#endif // UNITTEST



