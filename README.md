# DeviceInfoLedger

This library perform two separate but related things using two separate classes:

- Store cloud-based configuration for devices (DeviceConfigLedger)
- Store Particle device log data and device information in Ledger (DeviceInfoLedger)

You can use the cloud-based configuration without using the DeviceInfoLedger, if desired. 
You can also use the DeviceInfoLedger with a local configuration, not using the remote configuration option.

## Cloud-based configuration

Using Ledger, this library supports a default configuration using a cloud-to-device ledger (scoped to a product, owner, or organization).

It also supports an optional per-device configuration using a cloud-to-device device ledger (scoped to a device). The per-device configuration overrides specific values in the product default, making it easy to just change one setting and leave the others as the default value.

Configuration support key (string) and value pairs, using types that can be represented in JSON and `Variant` including:

- boolean values (`bool` in C++)
- integer number values (`int`, `unsigned`, `int64_t`, `uint64_t`)
- floating point number values (`double`)
- string values (`String`)

Additionally, the top level can also include one-level deep of object and array types.

For a top-level object, keys are merged within the same key name. 

For a top-level array, 

Using cloud-based configuration is optional if you only want to use device log data and device information. These settings can be 
configured locally on device, if desire. However, you may want to use both so you can control the log settings remotely from
the cloud-side.


## Configuration

The following JSON structure is used for local configuration, default cloud configuration, and device-specific overrides, if those features are enabled.

```json
{
    "lastRunLog": 1024,
    "connectionLog": 2048,
    "includeGeneral": true,
    "includeDiag": true,
    "includeTower": true,
    "logLevel": "INFO",
    "logFilters": [],
}
```

All fields must be specified in local configuration and default cloud configuration. In particular, the default cloud configuration is not merged
with a local configuration; they are mutually exclusive. 

Cloud-based device overrides can specify only the fields that need to be changed from the default configuration.


```cpp
const char localConfig[] = 
"{"
    "\"lastRunLog\": 1024,"
    "\"connectionLog\": 2048,"
    "\"includeGeneral\": true,"
    "\"includeDiag\": false,"
    "\"includeTower\": false,"
    "\"logLevel\": \"LOG_LEVEL_INFO\","
    "\"logFilters\": {"
        "\"comm\": \"TRACE\""
    "}"
"}";
```

### Detailed description of fields

#### lastRunLog

The last run log is the latest log messages prior to the most recent reboot. This can be useful for troubleshooting device resets.

Value is the size of the text to include in the ledger. Must fit within the available space in the ledger, which is up to 16K but
can be lower on some platforms.

The last run log is stored in retained memory. This is specified in the application source because it's allocated by the compiler,
so the actual run log will be the lesser of lastRunLog and the value stored in the source.

On RTL872x devices (P2, Photon 2), the most recent log entries may not be available if a system panic has occurred.

#### connectionLog

The connection log is the most recent messages from boot until successfully cloud connected on the first connection after
reboot (default) or after every cloud disconnect (if logAllConnections is true). Note that logging every disconnection
may result in a large number of ledger synchronization is the device is frequently disconnecting from the cloud.


#### logLevel (string)

| Constant           | Configuration String |
| :----------------- | :------- |
| `LOG_LEVEL_ALL`    | `ALL`    |
| `LOG_LEVEL_TRACE`  | `TRACE`  | 
| `LOG_LEVEL_INFO`   | `INFO`   |
| `LOG_LEVEL_WARN`   | `WARN`   | 
| `LOG_LEVEL_ERROR`  | `ERROR`  |
| `LOG_LEVEL_PANIC`  | `PANIC`  |
| `LOG_LEVEL_NONE`   | `NONE`   |


#### logFilters (object)

This is a JSON representation of a logging configuration for `LogCategoryFilter`. It is an object whose
key is the category and value is the logging level.

For example, given this code:

```cpp
SerialLogHandler logHandler(LOG_LEVEL_WARN, { // Logging level for non-application messages
    { "app", LOG_LEVEL_INFO }, // Default logging level for all application messages
    { "app.network", LOG_LEVEL_TRACE } // Logging level for networking messages
});
```

An equivalent JSON configuration would be:

```json
{
    "logLevel": "LOG_LEVEL_WARN",
    "logFilters": {
        "app": "INFO",
        "app.network": "TRACE"
    }
}
```
