import os
from pathlib import Path
import re
import shlex
import shutil
import subprocess


SCRIPT_PATH = Path(os.path.realpath(__file__)).parent


def findAnalyzeHeadless():
    ghidra_home = None
    if (SCRIPT_PATH / "prefix" / "ghidra").exists():
        ghidra_home = SCRIPT_PATH / "prefix" / "ghidra"

    # The standard way to locate ghidra is to look at the GHIDRA_HOME
    # environment variable, which points to the ghidra installation folder.
    elif os.getenv("GHIDRA_HOME") is not None:
        ghidra_home = Path(os.getenv("GHIDRA_HOME"))

    if ghidra_home is not None:
        if os.name == "nt":
            analyze_headless = ghidra_home / "support" / "analyzeHeadless.bat"
        else:
            analyze_headless = ghidra_home / "support" / "analyzeHeadless"
        if analyze_headless.exists():
            return str(analyze_headless)

    # ArchLinux and Nix add a ghidra-analyzeHeadless symlink that points to the
    # analyzeHeadless script of the ghidra installation.
    if shutil.which("ghidra-analyzeHeadless") is not None:
        return "ghidra-analyzeHeadless"

    # Some people just add the support folder to the PATH. Let's support that
    # too.
    if shutil.which("analyzeHeadless") is not None:
        return "analyzeHeadless"

    raise Exception(
        "Could not find Ghidra installation. Please install ghidra from https://ghidra-sre.org/ and set your GHIDRA_HOME environment variable to the installation directory"
    )


def runAnalyze(
    ghidra_repo_name,
    project_name,
    process=None,
    import_file=None,
    analysis=False,
    pre_scripts=None,
    post_scripts=None,
):
    pre_scripts = pre_scripts or []
    post_scripts = post_scripts or []
    if re.match("^ghidra://", ghidra_repo_name):
        raise ValueError(
            "remote Ghidra servers are not supported by this local exporter"
        )
    if project_name is None:
        raise ValueError("a local Ghidra project name is required")

    commonAnalyzeHeadlessArgs = [findAnalyzeHeadless(), ghidra_repo_name]
    commonAnalyzeHeadlessArgs += [project_name]

    if process and import_file:
        raise Exception("Cannot provide both import and process")
    elif process:
        commonAnalyzeHeadlessArgs += ["-process", process]
    elif import_file:
        commonAnalyzeHeadlessArgs += ["-import", import_file]

    commonAnalyzeHeadlessArgs += [
        "-readOnly",
        "-scriptPath",
        str(SCRIPT_PATH / "ghidra"),
    ]

    if not analysis:
        commonAnalyzeHeadlessArgs += ["-noanalysis"]

    for pre_script in pre_scripts:
        if isinstance(pre_script, list):
            commonAnalyzeHeadlessArgs += ["-prescript"] + pre_script
        elif isinstance(pre_script, str):
            commonAnalyzeHeadlessArgs += ["-prescript", pre_script]

    for post_script in post_scripts:
        if isinstance(post_script, list):
            commonAnalyzeHeadlessArgs += ["-postscript"] + post_script
        elif isinstance(post_script, str):
            commonAnalyzeHeadlessArgs += ["-postscript", post_script]

    print("Running " + " ".join(shlex.quote(x) for x in commonAnalyzeHeadlessArgs))
    return subprocess.run(commonAnalyzeHeadlessArgs, env=os.environ.copy(), check=True)
