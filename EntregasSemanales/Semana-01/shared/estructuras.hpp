#pragma once
#define LABEL_SIZE 64

struct msg {
   long mtype;
   int times;
   char mtext[LABEL_SIZE];
};
