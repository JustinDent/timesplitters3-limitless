# PC multiplayer bring-up

The current milestone is an **experimental native System Link transport**.
Two local game processes have discovered a match, joined a 2/16-player lobby,
and loaded into gameplay together. Separate computers, an entire match, and
internet play have not yet been validated. The Online menu is not implemented.

## Try System Link on a LAN

Build the same source and game content on both PCs, then run in PowerShell:

```powershell
.\tsfp.ps1 Build
.\tsfp.ps1 Run -SystemLink
```

Select **Xbox Live / System Link > System Link**. One player selects **Start New
Match**; the other selects that match, then Ready. The host starts the match.
The native transport uses UDP 1000 (gameplay), 3000 (session information), and
9999 (discovery). Allow the game through Windows Firewall on the private LAN.

If LAN broadcasts do not reach the other PC, explicitly name its LAN IPv4:

```powershell
.\tsfp.ps1 Run -SystemLink -Peers '192.168.1.25'
```

`-Peers` duplicates discovery broadcasts to the supplied addresses. It does not
solve NAT, translate addresses embedded in Xbox session data, or make internet
play ready. Normal launches leave the experimental transport disabled.
There is no Xbox console interoperability or Xbox Live emulation.

## Compatibility

Build generates `build/windows/generated/network-identity.json` and a compiled
SHA-256 identity from the protocol, native source, runtime source, lifted game
code, patches, and game data. Paths are relative and source line endings are
normalized. User settings, saves, diagnostic metadata, and external texture
packs are excluded. Other source changes are conservatively incompatible.

Every discovery and gameplay datagram carries the compiled identity. Traffic
with a different identity, an unknown header, or no header is discarded before
the game receives it. This covers connections that bypass a future browser.
The launcher also verifies the executable against a successful-build stamp,
hashes game content before a System Link launch, and refuses content changed
since the build; rebuild after editing game data. A failed build cannot authorize
an old executable against a new manifest. Launching the
EXE directly bypasses that launcher check.

The directory only returns public lobbies matching the requested identity.
Looking up an incompatible lobby by its private code returns HTTP 409. This is
compatibility enforcement for cooperating builds, not authentication, encryption,
or protection against a deliberately modified client.

## Player-hosted directory foundation

The selected target is a player's PC with port forwarding, with no Steam,
subscription, or paid hosting dependency. Friends will use that player's shared
directory address; there is no globally operated master server. The PC must be
reachable and running while its directory is in use.

`tools/lobby_server.py` currently provides the backend API, **not a connected
game browser**. It uses Python's standard library, keeps no database, bounds
requests/concurrent connections/lobby counts, and expires registrations after
90 seconds without an owner heartbeat. Private lobbies are omitted from listing
and can be looked up by code. Owner credentials are not returned in listings.

Local development:

```powershell
.\.venv\Scripts\python.exe tools/lobby_server.py
```

The default is HTTP on `127.0.0.1:30820`. Listening on a network interface requires
`--cert <certificate.pem> --key <private-key.pem>` for HTTPS, for example with
`--bind 0.0.0.0 --port 30820`. A future deployment would forward TCP 30820 to that
PC. The service derives host addresses from the TCP connection, so do not put it
behind a proxy that hides the caller address. TLS provisioning and certificate
trust/pinning in the game client still need a user-facing setup flow.

API routes:

| Method and path | Purpose |
| --- | --- |
| `GET /v1/lobbies?compatibility=<64 hex digits>` | Compatible public lobbies |
| `POST /v1/lobbies` | Register name, compatibility, port, public visibility |
| `GET /v1/lobbies/<code>?compatibility=<identity>` | Resolve a compatible, non-full lobby |
| `PUT /v1/lobbies/<code>` | Owner heartbeat with compatibility, players, capacity |
| `DELETE /v1/lobbies/<code>` | Close a lobby using the owner bearer token |

## Remaining stages

1. Validate native System Link on two physical PCs and complete a match.
2. Add internet endpoint mapping and a connection handshake. LAN addresses in
   the original session data cannot simply be sent across different networks.
3. Add private-code rendezvous, direct UDP connection attempts, and a relay
   fallback hosted on a player's PC. Port forwarding alone cannot reach a host
   behind carrier-grade NAT; a reachable player must provide the fallback.
4. Integrate Online beside Xbox Live and System Link, directory configuration,
   compatible-only listing and joining, hosting, heartbeat, and clear errors.

Directory discovery alone does not carry game traffic. These remaining stages
must be implemented and tested before advertising playable internet multiplayer.

## Validation and storage

Native tests cover UDP round trips, default-protocol sockets used by discovery,
XNADDR/key mapping, source ports, invalid guest pointers, and incompatible/raw
packet rejection. Python tests cover deterministic identities, changed game and
runtime data, directory filtering, private codes, owner authorization, full
lobbies, heartbeat compatibility, and expiry.

Manual two-process validation uses `private/network-validation/host` and `client`
with separate saves and loopback addresses 127.0.0.2 and 127.0.0.3. It reuses the
same game data and build. `TSFP_PC_INPUT_STATE` is an opt-in test input file with
`<virtual-key-number> <0-or-1>`, updated atomically; it permits background input
without changing desktop focus. Never point either test process at personal saves.

After both processes stop, run `tools/cleanup-network-tests.ps1`. It recycles only
the six known disposable `Partition*.img` files per test save directory, rejects
reparse points and out-of-root paths, and preserves test profiles and evidence.
