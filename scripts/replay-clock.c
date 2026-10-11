/* Optional host-side pacing for isolated Wine replay tests.
 * Only CLOCK_MONOTONIC_RAW is scaled. Game code, input records, other clocks,
 * and the Wine server are unchanged. Build as an ELF32 preload library.
 * No libc headers are needed to build with a host lacking multilib headers.
 */
typedef long long i64;
struct time64 { i64 seconds; i64 nanoseconds; };
extern void *dlsym(void *, const char *);
extern char *getenv(const char *);
extern long strtol(const char *, char **, int);
extern int access(const char *, int);

int __clock_gettime64(int clock_id, struct time64 *value)
{
    static int (*real_clock)(int, struct time64 *);
    static struct time64 origin;
    static long rate;
    static int initialized;
    int result;
    if (!real_clock)
        real_clock = dlsym((void *)-1, "__clock_gettime64");
    result = real_clock(clock_id, value);
    if (result || clock_id != 4)
        return result;
    if (!initialized) {
        const char *setting = getenv("TH08_REPLAY_CLOCK_RATE");
        const char *gate = getenv("TH08_REPLAY_CLOCK_GATE");
        if (gate && access(gate, 0))
            return result;
        rate = setting ? strtol(setting, (char **)0, 10) : 1;
        if (rate < 1 || rate > 1024)
            rate = 1;
        origin = *value;
        initialized = 1;
    }
    if (rate != 1) {
        double seconds = origin.seconds + origin.nanoseconds * 1e-9;
        seconds += ((value->seconds - origin.seconds)
                    + (value->nanoseconds - origin.nanoseconds) * 1e-9) * rate;
        value->seconds = (i64)seconds;
        value->nanoseconds = (i64)((seconds - value->seconds) * 1e9);
    }
    return result;
}
