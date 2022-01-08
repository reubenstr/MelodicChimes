const int stepperTimeoutDelayMs = 1000 * 60 * 1;

const char delimiter = ':';

enum class Commands
{
    Enable,
    Tighten,
    Loosen,
    VolumePlus,
    VolumeMinus,
    SetTargetNote,
    Pick,
    StatusEnabled,
    StatusDisabled,
    Error
};
