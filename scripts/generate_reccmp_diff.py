import json
import subprocess
import tempfile


def main():
    with tempfile.TemporaryDirectory() as tempdir:
        report_filename = f"{tempdir}/reccmp.json"
        subprocess.run(
            ["reccmp-reccmp", "--target", "th08", "--json", report_filename],
            check=True,
            stdout=subprocess.DEVNULL,
        )
        with open(report_filename, "r") as f:
            reccmp_data = json.load(f)

    print("# Report")
    print()
    print("name | result")
    print("-----|-------")

    for function in reccmp_data["data"]:
        print(function["name"] + " | " + format(function["matching"], ".2%"))


if __name__ == "__main__":
    main()
