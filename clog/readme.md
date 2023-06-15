# CLog

## config

```ini
[clog]
msgsize=1024 # [message size]:number;default 1024
use_datetime=false # [show datetime]:true,false;default true
use_level=false # [show log level]:true,false;default true
use_name=false # [print log on stdout]:true,false;default false
use_function=false # [show function name]:true,false;default false
use_position=false # [show log position]:true,false;default false
use_stdout=true # [print log on stdout]:true,false;default false
level=DEBUG # [current log level]:CLOSE,ERROR,WARN,INFO,DEBUG;default WARN
dir=E:\Projects\clog # [log file directory]:directory string;default current dir or /tmp/
name=clogtest # [log file name]:name string;default clog
```

## demo

```c
#include <unistd.h>
#include "xlog.h"

int main(int argc, char* argv[])
{
    (void)argc;

    xlog_init("clogtest_name");

    xlogError("error message!");
    sleep(1);
    xlogError("%s", argv[0]);
    sleep(1);
    xlogWarn("warn message!");
    sleep(1);
    xlogWarn("%s", argv[0]);
    sleep(1);
    xlogInfo("info message!");
    sleep(1);
    xlogInfo("%s", argv[0]);
    sleep(1);
    xlogDebug("debug message!");
    sleep(1);
    xlogDebug("%s", argv[0]);

    xlog_deinit();
    return 0;
}
```
