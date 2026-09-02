# UDP Packet Statistics Collector

Software for collecting statistics on network traffic (UDP packets).

The software consists of two utilities:

* **util1** - reads data from a network interface using a raw socket and collects
  statistics on UDP packets (packet count and total payload length);
* **util2** - on start-up, receives the collected statistics from the first
  utility and prints them to the screen.

The utilities communicate through POSIX message queues (`/server`, `/client`).
`util1` sends an updated `"<packets> <bytes>"` message every time the counters
change; `util2` blocks on the queue and prints each update.

## Two versions of the first utility

| Binary | Storage of statistics |
| --- | --- |
| `util1_1` | Two plain counters (packet count and accumulated byte count), guarded by a mutex. |
| `util1_2` | A dynamic array (`cvector`, see `common/CVec.h`) holding the length of every captured packet; the total is summed on each report. |

Both versions share the same capture logic and the same command-line interface.
`util1_1` runs the packet-capture thread and the reporting thread in parallel and
guards the shared counters with a `pthread_mutex_t`.

## Requirements

* Linux (raw `AF_PACKET` sockets, `linux/if_packet.h`)
* GCC
* `librt` (POSIX message queues) and `pthread`
* `debhelper` (>= 9.0.0) and `dpkg-dev` — only for building the deb package
* Root privileges (`CAP_NET_RAW`) to run `util1_*`

## Build

All build files live in the `deb-package/` directory.

To build the utilities only:

```sh
make
```

To build the whole package:

```sh
dpkg-buildpackage -uc -us
```

or

```sh
make deb
```

To clean up:

```sh
make clean && make mrproper
```

> **Note:** the makefile is currently stored as `Makefile.txt`. Rename it to
> `Makefile` (or invoke it with `make -f Makefile.txt`) before running the
> commands above.

## Usage

Start the display utility first — it creates the `/server` queue that the first
utility writes to:

```sh
./util2
```

Then, in another terminal, start the capture utility (requires root):

```sh
sudo ./util1_1 <interface> <source IP> <destination IP> <source port> <destination port>
```

or

```sh
sudo ./util1_2 <interface> <source IP> <destination IP> <source port> <destination port>
```

All five arguments are required. Pass `0` for a field to disable filtering on it;
if all four filter fields are `0`, every UDP packet on the interface is counted.

### Example

Count all UDP traffic on `eth0`:

```sh
sudo ./util1_1 eth0 0 0 0 0
```

Count UDP packets whose destination port is 53:

```sh
sudo ./util1_1 eth0 0 0 0 53
```

Sample output from `util2`:

```
Total packets: 12	 Total bytes: 1536
Total packets: 15	 Total bytes: 1902
```

## Installation

After building the package, install it with:

```sh
sudo dpkg -i ../lb1_1.0-1_*.deb
```

The binaries are installed into `/usr/bin/`.

## Project layout

```
deb-package/
├── Makefile.txt          # build rules for the utilities and the deb package
├── common/CVec.h         # simple dynamic array (vector) implementation
├── debian/               # deb package metadata (control, rules, changelog, copyright)
└── src/
    ├── util1_1/util1_1.c # capture utility, counter-based
    ├── util1_2/util1_2.c # capture utility, vector-based
    └── util2/util2.c     # statistics display utility
```

## License

MIT
