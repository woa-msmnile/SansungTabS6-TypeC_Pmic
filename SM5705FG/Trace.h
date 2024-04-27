/*++

Module Name:

    Trace.h

Abstract:

    Header file for the debug tracing related function defintions and macros.

Environment:

    Kernel mode

--*/

#pragma once

//
// Define the tracing flags.
//
// Tracing GUID - 14561be7-750a-4bc3-a619-d116c2e3e4d0
//

#define WPP_CONTROL_GUIDS                                              \
    WPP_DEFINE_CONTROL_GUID(                                           \
        SM5705FGTraceGuid, (14561be7,750a,4bc3,a619,d116c2e3e4d0), \
                                                                            \
        WPP_DEFINE_BIT(SURFACE_BATTERY_ERROR)                              \
        WPP_DEFINE_BIT(SURFACE_BATTERY_WARN)                                   \
        WPP_DEFINE_BIT(SURFACE_BATTERY_TRACE)                                   \
        WPP_DEFINE_BIT(SURFACE_BATTERY_INFO)                                    \
        )                             

#define WPP_FLAG_LEVEL_LOGGER(flag, level)                                  \
    WPP_LEVEL_LOGGER(flag)

#define WPP_FLAG_LEVEL_ENABLED(flag, level)                                 \
    (WPP_LEVEL_ENABLED(flag) &&                                             \
     WPP_CONTROL(WPP_BIT_ ## flag).Level >= level)

#define WPP_LEVEL_FLAGS_LOGGER(lvl,flags) \
           WPP_LEVEL_LOGGER(flags)
               
#define WPP_LEVEL_FLAGS_ENABLED(lvl, flags) \
           (WPP_LEVEL_ENABLED(flags) && WPP_CONTROL(WPP_BIT_ ## flags).Level >= lvl)

//           
// WPP orders static parameters before dynamic parameters. To support the Trace function
// defined below which sets FLAGS=MYDRIVER_ALL_INFO, a custom macro must be defined to
// reorder the arguments to what the .tpl configuration file expects.
//
#define WPP_RECORDER_FLAGS_LEVEL_ARGS(flags, lvl) WPP_RECORDER_LEVEL_FLAGS_ARGS(lvl, flags)
#define WPP_RECORDER_FLAGS_LEVEL_FILTER(flags, lvl) WPP_RECORDER_LEVEL_FLAGS_FILTER(lvl, flags)

//
// This comment block is scanned by the trace preprocessor to define our
// Trace function.
//
// begin_wpp config
// FUNC Trace(LEVEL, FLAGS, MSG, ...);
// end_wpp
//