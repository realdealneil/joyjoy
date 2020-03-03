#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <libevdev/libevdev.h>
#include <glib.h>
#include <unistd.h>

// Most of this code came from this gist:
// https://gist.github.com/meghprkh/9cdce0cd4e0f41ce93413b250a207a55
// Which had code from SDL apparently...be aware...

void get_guid(struct libevdev *dev, guint16 * guid) {
    guid[0] = GINT16_TO_LE(libevdev_get_id_bustype(dev));
    guid[1] = 0;
    guid[2] = GINT16_TO_LE(libevdev_get_id_vendor(dev));
    guid[3] = 0;
    guid[4] = GINT16_TO_LE(libevdev_get_id_product(dev));
    guid[5] = 0;
    guid[6] = GINT16_TO_LE(libevdev_get_id_version(dev));
    guid[7] = 0;
}

void guid_to_string(guint16 * guid, char * guidstr) {
    static const char k_rgchHexToASCII[] = "0123456789abcdef";
    int i;
    for (i = 0; i < 8; i++) {
        unsigned char c = guid[i];

        *guidstr++ = k_rgchHexToASCII[c >> 4];
        *guidstr++ = k_rgchHexToASCII[c & 0x0F];

        c = guid[i] >> 8;
        *guidstr++ = k_rgchHexToASCII[c >> 4];
        *guidstr++ = k_rgchHexToASCII[c & 0x0F];
    }
    *guidstr = '\0';
}

class joyjoy 
{
private: 
    std::string dev_name_;
    int fd_;
    struct libevdev *dev_ = NULL;
    
    // buttons, hats, joys:
    int nbuttons_ = 0;
    int naxes_;
    int nhats_ = 0;;
    
public:
    joyjoy() {}
    
    guint8 key_map[KEY_MAX];
    guint8 abs_map[ABS_MAX];
    
    bool init() {
        int i;        
        int rc = -1;

        // Detect the first joystick event file
        GDir* dir = g_dir_open("/dev/input/by-path/", 0, NULL);
        const gchar * fname;
        while ((fname = g_dir_read_name (dir))) {
            if (g_str_has_suffix (fname, "event-joystick")) break;
        }
        dev_name_ = std::string(fname);
        printf("Opening event file /dev/input/by-path/%s\n", dev_name_.c_str());
        
        fd_ = open(g_strconcat("/dev/input/by-path/", fname, NULL), O_RDONLY|O_NONBLOCK);
        rc = libevdev_new_from_fd(fd_, &dev_);
        if (rc < 0) {
            fprintf(stderr, "Failed to init libevdev (%s)\n", strerror(-rc));
            return false;
        }
        printf("Input device name: \"%s\"\n", libevdev_get_name(dev_));
        printf("Input device ID: bus %#x vendor %#x product %#x\n",
                libevdev_get_id_bustype(dev_),
                libevdev_get_id_vendor(dev_),
                libevdev_get_id_product(dev_));
                
        // GUID stuff
        guint16 guid[8];
        get_guid(dev_, guid);

        char guidstr[33];
        guid_to_string(guid, guidstr);

        printf("Input device GUID %s\n", guidstr);
        
        printf("\n\n");

        // Get info about buttons
        for (i = BTN_JOYSTICK; i < KEY_MAX; ++i) {
            if (libevdev_has_event_code(dev_, EV_KEY, i)) {
                printf("%d - Joystick has button: 0x%x - %s\n", nbuttons_, i,
                        libevdev_event_code_get_name(EV_KEY, i));
                key_map[i - BTN_MISC] = nbuttons_;
                ++nbuttons_;
            }
        }
        for (i = BTN_MISC; i < BTN_JOYSTICK; ++i) {
            if (libevdev_has_event_code(dev_, EV_KEY, i)) {
                printf("%d - Joystick has button: 0x%x - %s\n", nbuttons_, i,
                        libevdev_event_code_get_name(EV_KEY, i));
                key_map[i - BTN_MISC] = nbuttons_;
                ++nbuttons_;
            }
        }

        printf("\n\n");

        // Get info about axes
        for (i = 0; i < ABS_MAX; ++i) {
            /* Skip hats */
            if (i == ABS_HAT0X) {
                i = ABS_HAT3Y;
                continue;
            }
            if (libevdev_has_event_code(dev_, EV_ABS, i)) {
                const struct input_absinfo *absinfo = libevdev_get_abs_info(dev_, i);
                printf("Joystick has absolute axis: 0x%.2x - %s\n", i, libevdev_event_code_get_name(EV_ABS, i));
                printf("Values = { %d, %d, %d, %d, %d }\n",
                       absinfo->value, absinfo->minimum, absinfo->maximum,
                       absinfo->fuzz, absinfo->flat);
                abs_map[i] = naxes_;
                ++naxes_;
            }
        }

        printf("\n\n");

        // Get info about hats
        for (i = ABS_HAT0X; i <= ABS_HAT3Y; i += 2) {
            if (libevdev_has_event_code(dev_, EV_ABS, i) || libevdev_has_event_code(dev_, EV_ABS, i+1)) {
                const struct input_absinfo *absinfo = libevdev_get_abs_info(dev_, i);
                if (absinfo == NULL) continue;

                printf("Joystick has hat %d\n", (i - ABS_HAT0X) / 2);
                printf("Values = { %d, %d, %d, %d, %d }\n",
                       absinfo->value, absinfo->minimum, absinfo->maximum,
                       absinfo->fuzz, absinfo->flat);
                ++nhats_;
            }
        }

        printf("\n\n");
                
        return true;
    }
    
    int poll(input_event& ev)
    {
        return libevdev_next_event(dev_, LIBEVDEV_READ_FLAG_NORMAL, &ev);      
    }
};
