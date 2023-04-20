
# TODOs

- change `platform` config (for emconfigtool) to current Vitis version (2022.2)
- use Xilinx XRT libraries for HMAC
- add tests to demonstrate validity of functions
- add `empty` kernel
- possibly kernel-deps as sub-repo


- intro background design eval analysis

- check how large paper should be 2 page report or larger...

intro:
- background hw accelerators
- improve performance
- on path of NW operation
- in past: trusted components
    - external service, process: large overhead

- also: we want to improve security properties of conv distr systems
    - e.g. raft... very popular

- smartNICs: expense but affordable
    - nice and extra capabilities

- contribution
    - design + eval of attestation kernel that lowers bound for BFT properties


Background:
- talk about distributed protocols
    - byzante, what transformation would be like
    - transformation paper ...
    - FPGA stuff, smartNICs, technical things
        - high level overview

Design:
- 2 figure (of Dimitra Paper - make some changes)
    - first figure of dimtira: the system overview (data centers / how it works
    - cluster , single node, what attestation kernel is doing
    - server code, attestation and verify functions, what they are doing
- design; no technical parts; conceptual story


implementation:
- sw versions used, platofrms, vitis libraries, how I did de/encryption
- no emphasis on this - lighter than a column

evaluation:
- one / two plots
- experimental setup, setup used, plots , discussion around plots


