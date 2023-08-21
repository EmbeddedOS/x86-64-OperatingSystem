#include <stat.h>
#include <stdio.h>
#include <ctype.h>

static void GetRelativeFileName(stat* entry, char *buf)
{
    char *filename = entry->name;
    char *ext = entry->ext;
    int i = 0;

    while (*filename != '\0' && *filename != ' ') {
        *buf = *filename;
        filename++;
        buf += 1;

        if (i >= sizeof(entry->name) - 1) {
            break;
        }

        i++;
    }

    *buf = '.';
    buf += 1;
    i = 0;

    while (*ext != '\0' && *ext != ' ')
    {
        *buf = *ext;
        ext++;
        buf += 1;

        if (i >= sizeof(entry->ext) - 1) {
            break;
        }

        i++;
    }

    *buf = '\0';
}

static void toLowers(char *str) {
    for(int i = 0; str[i]; i++){
        str[i] = tolower(str[i]);
    }
}

int main(void) {
    stat buffer[100];
    int number_of_entries = lstat(".", buffer);
    printf("Total %d\n", number_of_entries);
    for (int i = 0; i < number_of_entries; i++) {

        int second = buffer[i].create_time & 0x001F;
        int minutes = (buffer[i].create_time & (0x003F << 5)) >> 5;
        int hours = ((buffer[i].create_time & (0x001F << 11)) >> 11 ) + 7;
        int day = buffer[i].create_date & 0x001F;
        int month = (buffer[i].create_date & (0x000F << 5)) >> 5;
        int year = ((buffer[i].create_date & (0x007F << 9)) >> 9) + 1980;

        char tmp[20] = {0};
        GetRelativeFileName(&buffer[i], tmp);
        toLowers(tmp);

        printf("%s %d %d:%d %d/%d/%d %s\n",
                buffer[i].attributes & FILE_ATTR_DIRECTORY ? "d":"f",
                buffer[i].file_size,
                hours,
                minutes,
                day,
                month,
                year,
                tmp);
    }
}