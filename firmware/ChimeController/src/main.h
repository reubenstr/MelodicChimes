
const int stepperTimeoutDelayMs = 1000 * 60 * 5;

const char delimiter = ':';

enum class Commands
{
    RestringTighten,
    RestringLoosen,
    VolumePlus,
    VolumeMinus,
    SetTargetNote,  
    Pick
};
