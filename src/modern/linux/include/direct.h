#pragma once
#include <sys/stat.h>
#include <unistd.h>
#define _mkdir(path) mkdir((path), 0777)
#define _chdir(path) chdir(path)
