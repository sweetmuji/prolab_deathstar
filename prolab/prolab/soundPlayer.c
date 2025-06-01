#define CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: sound_player.exe <soundfile.wav>\n");
        return 1;
    }

    PlaySound(argv[1], NULL, SND_ASYNC | SND_NODEFAULT);
    Sleep(1000);  // 잠시 대기, 바로 끊기지 않도록
    return 0;
}