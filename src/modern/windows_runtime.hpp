#pragma once

namespace th08
{
namespace modern
{

bool ConfigureDataDirectory();
bool ShouldForceWindowedMode();
void InstallCrashReporter();
void LogArchiveRequest(const char *path);

} // namespace modern
} // namespace th08
