#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef void (*CmdFunc)(void);

static CmdFunc s_cmd_list[10];

uint64_t MemInfo(void);

static int ReadCmd(char *buffer);
static int ParseCmd(char *buffer, int length);
static void ExecuteCmd(int cmd);
static void TotalMemCmd(void);

int main(void) {
    char buffer[80] = {0};
    int buffer_size = 0;
    int cmd = 0;
    s_cmd_list[0] = TotalMemCmd;

    while(1) {
        printf("shell# ");
        buffer_size = ReadCmd(buffer);

        if (buffer_size == 0) {
            continue;
        }

        cmd = ParseCmd(buffer, buffer_size);
        if (cmd < 0) {
            buffer[buffer_size] = '\0';
            printf("Command '%s' not found.\n", buffer);
        } else {
            ExecuteCmd(cmd);
        }
    }

}

static int ReadCmd(char *buffer)
{
    char c[2] = {0};
    int buffer_size = 0;
    while(1) {
        read(0, c, 1);
        if (c[0] == '\n' || buffer_size >= 80) {
            /* End of line or line down. */
            printf("%s", c);
            break;
        } else if (c[0] == '\b') {
            /* Backspace. */
            if (buffer_size > 0) {
                buffer_size--;
                printf("%s", c);
            }
        } else {
            /* Normal characters. */
            buffer[buffer_size++] = c[0];
            printf("%s", c);
        }
    }

    return buffer_size;
}

static int ParseCmd(char *buffer, int length)
{
    int cmd = -1;
    if (length == 3 && (!memcmp("mem", buffer, 3))) {
        cmd = 0;
    }

    return cmd;
}

static void ExecuteCmd(int cmd)
{
    CmdFunc func = s_cmd_list[cmd];
    if (func != NULL) {
        func();
    }
}

static void TotalMemCmd(void)
{
    printf("Total memory is %dMB\n", MemInfo());
}
