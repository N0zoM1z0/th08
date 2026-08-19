import argparse
from pathlib import Path
import textwrap

from configure import BuildType, configure
from winhelpers import run_windows_program

SCRIPTS_DIR = Path(__file__).parent


def build(build_type, verbose=False, jobs=1, targets=None, fresh=False):
    configure(build_type)

    ninja_args = []
    if verbose:
        ninja_args += ["-v"]

    if jobs != 0:
        ninja_args += ["-j" + str(jobs)]

    if targets:
        ninja_args += targets
    elif build_type == BuildType.DLLBUILD:
        ninja_args += ["build/th08e.dll"]
    elif build_type == BuildType.OBJDIFFBUILD:
        ninja_args += ["objdiff"]
    else:
        ninja_args += ["build/th08.exe"]

    # Then, run the build. We use run_windows_program to automatically go through
    # wine if running on linux/macos. scripts/th08run.bat will setup PATH and other
    # environment variables for the MSVC toolchain to work before calling ninja.
    if fresh:
        # Ninja removes only outputs declared by the freshly generated graph.
        # Source, target inputs, toolchains, and private analysis are untouched.
        run_windows_program(
            [str(SCRIPTS_DIR / "th08run.bat"), "ninja", "-t", "clean"],
            cwd=str(SCRIPTS_DIR.parent),
        )
    run_windows_program(
        [str(SCRIPTS_DIR / "th08run.bat"), "ninja"] + ninja_args,
        cwd=str(SCRIPTS_DIR.parent),
    )


def main():
    parser = argparse.ArgumentParser(
        "th08-build", formatter_class=argparse.RawTextHelpFormatter
    )
    parser.add_argument(
        "--build-type",
        choices=["normal", "bugfix", "diffbuild", "dllbuild", "objdiffbuild"],
        default="normal",
        help=textwrap.dedent(
            """Note: the bugfix build contains bugfixes that would otherwise make the build non-functional."""
        ),
    )
    parser.add_argument(
        "-j",
        "--jobs",
        type=int,
        default=1,
        help=textwrap.dedent("""
            Number of jobs to run in parallel. Set to 0 to run one job per CPU core. Defaults to 1.
            Note that parallel builds may not work when running through wine.
            See https://github.com/happyhavoc/th06/issues/79 for more information."""),
    )
    parser.add_argument("--verbose", action="store_true")
    parser.add_argument("--object-name", required=False)
    parser.add_argument(
        "--fresh",
        action="store_true",
        help=(
            "clean all outputs declared by the generated Ninja graph before "
            "building; use for aggregate exact-state attestation"
        ),
    )
    parser.add_argument(
        "targets",
        nargs="*",
        help=textwrap.dedent("""
        Ninja targets to build. The default depends on the build type:
          - Normal, bugfix and diff builds will build th08.exe
          - dll builds will build th08e.dll
          - objdiff builds will build all the object files necessary for objdiff.
    """),
    )
    args = parser.parse_args()
    targets = []

    # First, create the build.ninja file that will be used to build.
    if args.build_type == "normal":
        build_type = BuildType.NORMAL
    elif args.build_type == "bugfix":
        build_type = BuildType.BUGFIX
    elif args.build_type == "diffbuild":
        build_type = BuildType.DIFFBUILD
    elif args.build_type == "dllbuild":
        build_type = BuildType.DLLBUILD
    elif args.build_type == "objdiffbuild":
        build_type = BuildType.OBJDIFFBUILD

    if args.object_name is not None:
        if args.targets:
            parser.error("--object-name cannot be combined with positional targets")
        object_name = Path(args.object_name).name
        targets = ["build/objdiff/reimpl/" + object_name]
    elif args.targets:
        targets = args.targets

    build(build_type, args.verbose, args.jobs, targets=targets, fresh=args.fresh)


if __name__ == "__main__":
    main()
