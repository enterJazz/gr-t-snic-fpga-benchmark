# GR-t-snic-fpga-benchmark

## Important Notes
- asym is the only mode that is tested (sym is no longer relevant)
- before executing anything from the makefile, execute `source setup.sh`
- set the `COMPILATION_TARGET` to the target of your choice in the makefile (reccommended: `sw_emu`)
- execute `make test-asym-attest` or `make test-asym-verify` to run tests
for additional commands, see the makefile itself
