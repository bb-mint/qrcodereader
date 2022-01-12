#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>

/* 英語キーボードのマップから
char *key_table_sample[] = {
  "Reserved", "Esc", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "Minus", "Equal", "Backspace",
              "Tab",    "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "Left Brace", "Right Brace", "Enter",
     "Left Control",     "A", "S", "D", "F", "G", "H", "J", "K", "L", "Semicolon", "Apostrophe", "Grave",
     "Left Shift",
          "Backslash",    "Z", "X", "C", "V", "B", "N", "M", "Comma", "Dot", "Slash", "Right Shift",        "Keypad Asterisk",
  "Left Alt", 
                                    "Space",
  "Caps Lock"
  // 0x00 to 0x3a (00 to 58)
};


char *key_table_shift_off[] = {
  "\0", "\0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "\0",
      "\0",    "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", "\n",
      "\0",     "a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "'", "`",
      "\0", "\\", "z", "x", "c", "v", "b", "n", "m", ",", ".", "/", "\0",        "*",
  "\0", 
                                    " ",
  "\0"
  // 0x00 to 0x3a (00 to 58)
};

char *key_table_shift_on[] = {
  "\0", "\0", "!", "@", "#", "$", "%", "^", "&", "*", "(", ")", "_", "+", "\0",
      "\0",    "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "{", "}", "\n",
      "\0",     "A", "S", "D", "F", "G", "H", "J", "K", "L", ":", "\"", "~",
      "\0", "|", "Z", "X", "C", "V", "B", "N", "M", "<", ">", "?", "\0",        "*",
  "\0", 
                                    " ",
  "\0"
};
*/

//安全性を考慮して使える文字列を絞る　("\0"は出力されない）
char *key_table_shift_off[] = {
  "\0", "\0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "\0",
      "\0",    "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", "\n",
      "\0",     "a", "s", "d", "f", "g", "h", "j", "k", "l", "\0", "\0", "\0",
      "\0", "\0", "z", "x", "c", "v", "b", "n", "m", "\0", ".", "/", "\0",        "*",
  "\0", 
                                    "\0",
  "\0"
};

char *key_table_shift_on[] = {
  "\0", "\0", "!", "@", "#", "$", "%", "^", "&", "*", "(", ")", "_", "+", "\0",
      "\0",    "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "{", "}", "\n",
      "\0",     "A", "S", "D", "F", "G", "H", "J", "K", "L", ":", "\0", "~",
      "\0", "\0", "Z", "X", "C", "V", "B", "N", "M", "<", ">", "?", "\0",        "*",
  "\0", 
                                    "\0",
  "\0"
};

static char **key_table[] = {
    key_table_shift_off,
    key_table_shift_on
};

int main(int argc, char *argv[])
{
  int fd;
  struct input_event events[64];

  int i;

  int shift_on = 0;
  char output_char;

  if (argc <= 1) {
    printf("Usage: command event_device_file");
    exit(EXIT_FAILURE);
  }
  fd = open(argv[1], O_RDONLY);
  if (fd < 0) {
    perror("open error");
    exit(EXIT_FAILURE);
  }

//デバイスの占有
  if(ioctl(fd, EVIOCGRAB, 1))
    printf("Couldn't grab %s. %s.\n", argv[1], strerror(errno));

//入力待ち
  for (;;) {
    size_t read_size;

    read_size = read(fd, events, sizeof(events));
    for (i = 0; i < (int)(read_size / sizeof(struct input_event)); i++){

      if (events[i].type == 0x1 && events[i].value == 0x1 && ( events[i].code == 0x2a || events[i].code == 0x36 )) {
          //Shift押下状態
          shift_on = 1;
      }else if(events[i].type == 0x1 && events[i].value == 0x0 && ( events[i].code == 0x2a || events[i].code == 0x36 )) {
          //Shift離した状態
          shift_on = 0;
      }else if(events[i].type == 0x1 && events[i].value == 0x1 && events[i].code <= 0x3a) {
         output_char = key_table[shift_on][events[i].code][0];
         //出力可否判定
         if (output_char != '\0'){
             printf("%c", output_char);
         }
         if (output_char == '\n'){
             //改行で終了とする
             close(fd);
             exit(0);
         }
      }


    }

  }

  close(fd);
}