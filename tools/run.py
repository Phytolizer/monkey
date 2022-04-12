#!/usr/bin/env python3
import argparse
import os
import subprocess

scriptdir = os.path.dirname(os.path.realpath(__file__))
projdir = os.path.abspath(os.path.join(scriptdir, os.pardir))
os.chdir(projdir)

preset = "dev"

p = argparse.ArgumentParser()
p.add_argument("--preset")
p.add_argument("-r", "--run", action="store_true")
p.add_argument("-c", "--configure", action="store_true")
args = p.parse_args()

if args.preset is not None:
    preset = args.preset

cmake = os.getenv("CMAKE_EXECUTABLE")
if cmake is None:
    cmake = "cmake"

devenv = os.environ

if os.name == "nt":
    # Run vcvars64.bat to set up the environment
    vcvars_dir = os.getenv("VCVARS_DIR")
    assert vcvars_dir is not None
    output = subprocess.run(
        f"(\"{vcvars_dir}/vcvars64.bat\" >&2) && python -c \"import os; print(repr(os.environ).lstrip('environ'))\"", stdout=subprocess.PIPE, shell=True)
    devenv = eval(output.stdout.decode("utf-8"))

if args.configure:
    subprocess.run(["cmake", "--preset", preset], check=True, env=devenv)

# Build the project
subprocess.run(["cmake", "--build", "--preset", preset],
               check=True, env=devenv)
# Run the final executable
if args.run:
    subprocess.run("build/test/monkey_test" + (".exe" if os.name == "nt" else ""))
