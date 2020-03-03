#include "joyjoy.h"

int main()
{
    // The joyjoy class:
    joyjoy myjoyjoy;
    myjoyjoy.init();
    
    struct input_event ev;
    
    while (1)
    {
        int ret = myjoyjoy.poll(ev);
        if (ret == 1 || ret == 0)
        {
            switch (ev.type) {
            case EV_KEY:
                if (ev.code >= BTN_MISC) {
                    printf("Button %d\n", myjoyjoy.key_map[ev.code - BTN_MISC]);
                }
                break;
            case EV_ABS:
                switch (ev.code) {
                case ABS_HAT0X:
                case ABS_HAT0Y:
                case ABS_HAT1X:
                case ABS_HAT1Y:
                case ABS_HAT2X:
                case ABS_HAT2Y:
                case ABS_HAT3X:
                case ABS_HAT3Y:
                    ev.code -= ABS_HAT0X;
                    printf("Hat %d Axis %d Value %d\n", ev.code / 2, ev.code % 2, ev.value);
                    break;
                default:
                    printf("Axis %d Value %d\n", myjoyjoy.abs_map[ev.code], ev.value);
                    break;
                }
                break;
            case EV_REL:
                switch (ev.code) {
                case REL_X:
                case REL_Y:
                    ev.code -= REL_X;
                    printf("Ball %d Axis %d Value %d\n", ev.code / 2, ev.code % 2, ev.value);
                    break;
                default:
                    break;
                }
                break;
            }
          
        }
        else if (ret == -EAGAIN)
        {
            sleep(0.02);
        }
    }
}
