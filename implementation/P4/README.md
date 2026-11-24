# DualTrack P4 BMv2 Implementation

This directory contains a P4/v1model prototype of DualTrack. 

The supplied demo topology is:

```text
h1 --- s1 --- h2
       BMv2
```

`s1-commands.txt` forwards packets between switch ports 1 and 2. 

## Requirements

- Linux with Mininet.
- p4c with the BMv2 backend and v1model.
- BMv2, including `simple_switch`
  and `simple_switch_CLI`.
- Optional: `tcpreplay` for replaying a packet trace.

## Compilation and Execution

Run from this directory so that the source includes and configuration paths
resolve correctly.

1. Running the data plane codes of DualTrack

  ```sh
   sudo p4run
   ```

2. Start a new terminal and run the control plane codes to initialize DualTrack and its associated hash functions:
   
   ```sh
   ./start.sh
   ```
   
   P4Utils compiles the configured source, starts Mininet/BMv2, and loads
   `s1-commands.txt`. If using a separately started switch, load the entries
   with `simple_switch_CLI --thrift-port 9090 < s1-commands.txt`, replacing the
   port with the switch's actual Thrift port.

3. Start a new terminal and enter the host h1:
   
   ```sh
   mx h1
   ```

4. Replay packets from send.py:

   ```
   sudo python send.py
   ```
